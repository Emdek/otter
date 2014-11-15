/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2014 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#include "Window.h"
#include "AddressWidget.h"
#include "GoBackActionWidget.h"
#include "GoForwardActionWidget.h"
#include "SearchWidget.h"
#include "../core/NetworkManagerFactory.h"
#include "../core/SettingsManager.h"
#include "../core/WebBackend.h"
#include "../core/WebBackendsManager.h"
#include "../modules/windows/bookmarks/BookmarksContentsWidget.h"
#include "../modules/windows/cache/CacheContentsWidget.h"
#include "../modules/windows/cookies/CookiesContentsWidget.h"
#include "../modules/windows/configuration/ConfigurationContentsWidget.h"
#include "../modules/windows/history/HistoryContentsWidget.h"
#include "../modules/windows/transfers/TransfersContentsWidget.h"
#include "../modules/windows/web/WebContentsWidget.h"

#include <QtCore/QTimer>
#include <QtGui/QClipboard>
#include <QtWidgets/QApplication>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMenu>
#include <QtWidgets/QToolButton>

namespace Otter
{

qint64 Window::m_identifierCounter = 0;

Window::Window(bool isPrivate, ContentsWidget *widget, QWidget *parent) : QWidget(parent),
	m_navigationBar(NULL),
	m_addressWidget(NULL),
	m_searchWidget(NULL),
	m_contentsWidget(NULL),
	m_identifier(++m_identifierCounter),
	m_isPinned(false),
	m_isPrivate(isPrivate)
{
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);

	if (widget)
	{
		widget->setParent(this);

		setContentsWidget(widget);
	}
}

void Window::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);

	if (!m_contentsWidget)
	{
		setUrl(m_session.getUrl(), false);
	}
}

void Window::focusInEvent(QFocusEvent *event)
{
	QWidget::focusInEvent(event);

	if (isUrlEmpty() && !m_contentsWidget->isLoading() && m_addressWidget)
	{
		m_addressWidget->setFocus();
	}
	else if (m_contentsWidget)
	{
		m_contentsWidget->setFocus();
	}
}

void Window::clear()
{
	if (m_addressWidget)
	{
		m_addressWidget->clear();
	}

	setContentsWidget(new WebContentsWidget(m_isPrivate, NULL, this));
}

void Window::close()
{
	emit aboutToClose();

	QTimer::singleShot(50, this, SLOT(notifyRequestedCloseWindow()));
}

void Window::search(const QString &query, const QString &engine)
{
	WebContentsWidget *widget = qobject_cast<WebContentsWidget*>(m_contentsWidget);

	if (!widget)
	{
		widget = new WebContentsWidget(isPrivate(), NULL, this);

		setContentsWidget(widget);
	}

	if (m_addressWidget)
	{
		m_addressWidget->clearFocus();
	}

	widget->search(query, engine);

	if (m_addressWidget)
	{
		m_addressWidget->setUrl(getUrl());
	}
}

void Window::triggerAction(ActionIdentifier action, bool checked)
{
	if (m_addressWidget)
	{
		if (action == ActivateAddressFieldAction)
		{
			m_addressWidget->setFocus();
		}
		else if (action == PasteAndGoAction)
		{
			if (!QApplication::clipboard()->text().isEmpty())
			{
				m_addressWidget->handleUserInput(QApplication::clipboard()->text().trimmed());
			}

			return;
		}
	}

	getContentsWidget()->triggerAction(action, checked);
}

void Window::handleSearchRequest(const QString &query, const QString &engine)
{
	if (getType() == QLatin1String("web") && getUrl().scheme() == QLatin1String("about") && isUrlEmpty())
	{
		search(query, engine);
	}
	else
	{
		emit requestedSearch(query, engine);
	}
}

void Window::notifyLoadingStateChanged(bool loading)
{
	emit loadingStateChanged(loading ? LoadingState : LoadedState);
}

void Window::notifyRequestedCloseWindow()
{
	emit requestedCloseWindow(this);
}

void Window::notifyRequestedOpenUrl(const QUrl &url, OpenHints hints)
{
	if (isPrivate())
	{
		hints |= PrivateOpen;
	}

	emit requestedOpenUrl(url, hints);
}

void Window::setSession(const SessionWindow &session)
{
	m_session = session;

	setSearchEngine(session.searchEngine);
	setPinned(session.pinned);

	if (!SettingsManager::getValue(QLatin1String("Browser/DelayRestoringOfBackgroundTabs")).toBool())
	{
		setUrl(session.getUrl(), false);
	}
}

void Window::setDefaultCharacterEncoding(const QString &encoding)
{
	if (m_contentsWidget->getType() == QLatin1String("web"))
	{
		WebContentsWidget *webWidget = qobject_cast<WebContentsWidget*>(m_contentsWidget);

		if (webWidget)
		{
			return webWidget->setDefaultCharacterEncoding(encoding);
		}
	}
}

void Window::setUserAgent(const QString &identifier)
{
	WebContentsWidget *webWidget = NULL;

	if (m_contentsWidget && m_contentsWidget->getType() == QLatin1String("web"))
	{
		webWidget = qobject_cast<WebContentsWidget*>(m_contentsWidget);

		if (!webWidget)
		{
			return;
		}
	}

	QString value;

	if (identifier == QLatin1String("custom"))
	{
		value = QInputDialog::getText(this, tr("Select User Agent"), tr("Enter User Agent:"), QLineEdit::Normal, webWidget->getUserAgent().second);
	}
	else
	{
		value = NetworkManagerFactory::getUserAgent(identifier).value;
	}

	webWidget->setUserAgent(identifier, value);
}

void Window::setSearchEngine(const QString &engine)
{
	if (m_searchWidget)
	{
		m_searchWidget->setCurrentSearchEngine(engine);
	}
}

void Window::setUrl(const QUrl &url, bool typed)
{
	ContentsWidget *newWidget = NULL;

	if (url.scheme() == QLatin1String("about"))
	{
		if (m_addressWidget && m_session.index < 0 && !isUrlEmpty() && SessionsManager::hasUrl(url, true))
		{
			m_addressWidget->setUrl(m_contentsWidget ? m_contentsWidget->getUrl() : m_session.getUrl());

			return;
		}

		if (url.path() == QLatin1String("bookmarks"))
		{
			newWidget = new BookmarksContentsWidget(this);
		}
		else if (url.path() == QLatin1String("cache"))
		{
			newWidget = new CacheContentsWidget(this);
		}
		else if (url.path() == QLatin1String("config"))
		{
			newWidget = new ConfigurationContentsWidget(this);
		}
		else if (url.path() == QLatin1String("cookies"))
		{
			newWidget = new CookiesContentsWidget(this);
		}
		else if (url.path() == QLatin1String("history"))
		{
			newWidget = new HistoryContentsWidget(this);
		}
		else if (url.path() == QLatin1String("transfers"))
		{
			newWidget = new TransfersContentsWidget(this);
		}

		if (newWidget && !newWidget->canClone())
		{
			SessionsManager::removeStoredUrl(newWidget->getUrl().toString());
		}
	}

	const bool isRestoring = (!m_contentsWidget && m_session.index >= 0);

	if (!newWidget && (!m_contentsWidget || m_contentsWidget->getType() != QLatin1String("web")))
	{
		newWidget = new WebContentsWidget(m_isPrivate, NULL, this);
	}

	if (newWidget)
	{
		setContentsWidget(newWidget);
	}

	if (m_contentsWidget && url.isValid())
	{
		if (!isRestoring)
		{
			m_contentsWidget->setUrl(url, typed);
		}

		if (m_addressWidget)
		{
			if (!isUrlEmpty() || m_contentsWidget->isLoading())
			{
				m_addressWidget->clearFocus();
			}

			m_addressWidget->setUrl(url);
		}
	}
}

void Window::setPinned(bool pinned)
{
	if (pinned != m_isPinned)
	{
		m_isPinned = pinned;

		emit isPinnedChanged(pinned);
	}
}

void Window::setContentsWidget(ContentsWidget *widget)
{
	if (m_contentsWidget)
	{
		layout()->removeWidget(m_contentsWidget);

		m_contentsWidget->deleteLater();
	}

	m_contentsWidget = widget;

	if (!m_contentsWidget)
	{
		return;
	}

	if (m_contentsWidget->getType() == QLatin1String("web") && !m_navigationBar)
	{
		m_navigationBar = new QWidget(this);

		QToolButton *reloadOrStopButton = new QToolButton(this);
		reloadOrStopButton->setDefaultAction(m_contentsWidget->getAction(ReloadOrStopAction));
		reloadOrStopButton->setAutoRaise(true);

		m_addressWidget = new AddressWidget(this, this);
		m_addressWidget->setUrl(m_contentsWidget->getUrl());

		m_searchWidget = new SearchWidget(this);

		QBoxLayout *navigationLayout = new QBoxLayout(QBoxLayout::LeftToRight, m_navigationBar);
		navigationLayout->setContentsMargins(0, 0, 0, 0);
		navigationLayout->addWidget(new GoBackActionWidget(this, m_navigationBar));
		navigationLayout->addWidget(new GoForwardActionWidget(this, m_navigationBar));
		navigationLayout->addWidget(reloadOrStopButton);
		navigationLayout->addWidget(m_addressWidget, 3);
		navigationLayout->addWidget(m_searchWidget);

		m_navigationBar->setLayout(navigationLayout);

		layout()->addWidget(m_navigationBar);

		connect(m_contentsWidget, SIGNAL(urlChanged(QUrl)), m_addressWidget, SLOT(setUrl(QUrl)));
		connect(m_addressWidget, SIGNAL(requestedLoadUrl(QUrl)), this, SLOT(setUrl(QUrl)));
		connect(m_addressWidget, SIGNAL(requestedSearch(QString,QString)), this, SLOT(search(QString,QString)));
		connect(m_searchWidget, SIGNAL(requestedSearch(QString,QString)), this, SLOT(handleSearchRequest(QString,QString)));
	}
	else if (m_contentsWidget->getType() != QLatin1String("web") && m_navigationBar)
	{
		m_navigationBar->deleteLater();

		layout()->removeWidget(m_navigationBar);

		m_navigationBar = NULL;
		m_addressWidget = NULL;
		m_searchWidget = NULL;
	}

	layout()->addWidget(m_contentsWidget);

	if (m_session.index >= 0)
	{
		if (!m_session.userAgent.isEmpty() && m_contentsWidget->getType() == QLatin1String("web"))
		{
			WebContentsWidget *webWidget = qobject_cast<WebContentsWidget*>(m_contentsWidget);

			if (webWidget)
			{
				if (m_session.userAgent.contains(QLatin1Char(';')))
				{
					webWidget->setUserAgent(m_session.userAgent.section(QLatin1Char(';'), 0, 0), m_session.userAgent.section(QLatin1Char(';'), 1));
				}
				else
				{
					webWidget->setUserAgent(m_session.userAgent, NetworkManagerFactory::getUserAgent(m_session.userAgent).value);
				}
			}
		}

		if (m_session.reloadTime != -1 && m_contentsWidget->getType() == QLatin1String("web"))
		{
			WebContentsWidget *webWidget = qobject_cast<WebContentsWidget*>(m_contentsWidget);

			if (webWidget)
			{
				webWidget->setReloadTime(m_session.reloadTime);
			}
		}

		WindowHistoryInformation history;
		history.index = m_session.index;
		history.entries = m_session.history;

		m_contentsWidget->setHistory(history);
		m_contentsWidget->setZoom(m_session.getZoom());
		m_contentsWidget->setFocus();

		m_session = SessionWindow();
	}
	else
	{
		if (isUrlEmpty() && m_addressWidget)
		{
			m_addressWidget->setFocus();
		}

		if (m_contentsWidget->getUrl().scheme() == QLatin1String("about") || isUrlEmpty())
		{
			emit titleChanged(m_contentsWidget->getTitle());
		}
	}

	emit actionsChanged();
	emit canZoomChanged(m_contentsWidget->canZoom());
	emit iconChanged(m_contentsWidget->getIcon());

	connect(this, SIGNAL(aboutToClose()), m_contentsWidget, SLOT(close()));
	connect(m_contentsWidget, SIGNAL(requestedAddBookmark(QUrl,QString)), this, SIGNAL(requestedAddBookmark(QUrl,QString)));
	connect(m_contentsWidget, SIGNAL(requestedOpenUrl(QUrl,OpenHints)), this, SIGNAL(requestedOpenUrl(QUrl,OpenHints)));
	connect(m_contentsWidget, SIGNAL(requestedNewWindow(ContentsWidget*,OpenHints)), this, SIGNAL(requestedNewWindow(ContentsWidget*,OpenHints)));
	connect(m_contentsWidget, SIGNAL(requestedSearch(QString,QString)), this, SIGNAL(requestedSearch(QString,QString)));
	connect(m_contentsWidget, SIGNAL(actionsChanged()), this, SIGNAL(actionsChanged()));
	connect(m_contentsWidget, SIGNAL(canZoomChanged(bool)), this, SIGNAL(canZoomChanged(bool)));
	connect(m_contentsWidget, SIGNAL(statusMessageChanged(QString)), this, SIGNAL(statusMessageChanged(QString)));
	connect(m_contentsWidget, SIGNAL(titleChanged(QString)), this, SIGNAL(titleChanged(QString)));
	connect(m_contentsWidget, SIGNAL(urlChanged(QUrl)), this, SIGNAL(urlChanged(QUrl)));
	connect(m_contentsWidget, SIGNAL(iconChanged(QIcon)), this, SIGNAL(iconChanged(QIcon)));
	connect(m_contentsWidget, SIGNAL(loadingChanged(bool)), this, SLOT(notifyLoadingStateChanged(bool)));
	connect(m_contentsWidget, SIGNAL(zoomChanged(int)), this, SIGNAL(zoomChanged(int)));
}

Window* Window::clone(bool cloneHistory, QWidget *parent)
{
	if (!m_contentsWidget || !canClone())
	{
		return NULL;
	}

	Window *window = new Window(false, m_contentsWidget->clone(cloneHistory), parent);
	window->setSearchEngine(getSearchEngine());
	window->setPinned(isPinned());

	return window;
}

ContentsWidget* Window::getContentsWidget()
{
	if (!m_contentsWidget)
	{
		setUrl(m_session.getUrl(), false);
	}

	return m_contentsWidget;
}

QString Window::getDefaultCharacterEncoding() const
{
	if (m_contentsWidget && m_contentsWidget->getType() == QLatin1String("web"))
	{
		WebContentsWidget *webWidget = qobject_cast<WebContentsWidget*>(m_contentsWidget);

		if (webWidget)
		{
			return webWidget->getDefaultCharacterEncoding();
		}
	}

	return QString();
}

QString Window::getSearchEngine() const
{
	return (m_searchWidget ? m_searchWidget->getCurrentSearchEngine() : QString());
}

QString Window::getTitle() const
{
	return (m_contentsWidget ? m_contentsWidget->getTitle() : m_session.getTitle());
}

QLatin1String Window::getType() const
{
	return (m_contentsWidget ? m_contentsWidget->getType() : QLatin1String("unknown"));
}

QUrl Window::getUrl() const
{
	return (m_contentsWidget ? m_contentsWidget->getUrl() : m_session.getUrl());
}

QIcon Window::getIcon() const
{
	return (m_contentsWidget ? m_contentsWidget->getIcon() : WebBackendsManager::getBackend()->getIconForUrl(m_session.getUrl()));
}

QPixmap Window::getThumbnail() const
{
	return (m_contentsWidget ? m_contentsWidget->getThumbnail() : QPixmap());
}

SessionWindow Window::getSession() const
{
	if (!m_contentsWidget)
	{
		return m_session;
	}

	const WindowHistoryInformation history = m_contentsWidget->getHistory();
	const QPair<QString, QString> userAgent = getUserAgent();
	SessionWindow session;
	session.searchEngine = getSearchEngine();
	session.userAgent = userAgent.first + ((userAgent.first == QLatin1String("custom")) ? QLatin1Char(';') + userAgent.second : QString());
	session.history = history.entries;
	session.group = 0;
	session.index = history.index;
	session.pinned = isPinned();

	if (m_contentsWidget->getType() == QLatin1String("web"))
	{
		WebContentsWidget *webWidget = qobject_cast<WebContentsWidget*>(m_contentsWidget);

		if (webWidget && webWidget->getReloadTime() != 1)
		{
			session.reloadTime = webWidget->getReloadTime();
		}
	}

	return session;
}

QPair<QString, QString> Window::getUserAgent() const
{
	if (m_contentsWidget && m_contentsWidget->getType() == QLatin1String("web"))
	{
		WebContentsWidget *webWidget = qobject_cast<WebContentsWidget*>(m_contentsWidget);

		if (webWidget)
		{
			return webWidget->getUserAgent();
		}
	}

	return qMakePair(QString(), QString());
}

WindowLoadingState Window::getLoadingState() const
{
	return (m_contentsWidget ? (m_contentsWidget->isLoading() ? LoadingState : LoadedState) : DelayedState);
}

qint64 Window::getIdentifier() const
{
	return m_identifier;
}

bool Window::canClone() const
{
	return (m_contentsWidget ? m_contentsWidget->canClone() : false);
}

bool Window::isPinned() const
{
	return m_isPinned;
}

bool Window::isPrivate() const
{
	return (m_contentsWidget ? m_contentsWidget->isPrivate() : m_isPrivate);
}

bool Window::isUrlEmpty() const
{
	const QString url = getUrl().path();

	if (url == QLatin1String("blank") || url == QLatin1String("start") || url.isEmpty())
	{
		return true;
	}

	return false;
}

}
