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

#ifndef OTTER_MAINWINDOW_H
#define OTTER_MAINWINDOW_H

#include "../core/SessionsManager.h"
#include "../core/WindowsManager.h"

#include <QtWidgets/QActionGroup>
#include <QtWidgets/QMainWindow>

namespace Otter
{

namespace Ui
{
	class MainWindow;
}

class WindowsManager;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(bool privateSession = false, const SessionEntry &windows = SessionEntry(), QWidget *parent = NULL);
	~MainWindow();

	WindowsManager* getWindowsManager();
	bool eventFilter(QObject *object, QEvent *event);

public slots:
	void openUrl(const QUrl &url = QUrl());

protected:
	void changeEvent(QEvent *event);
	void closeEvent(QCloseEvent *event);
	void gatherBookmarks(int folder);
	void updateAction(QAction *source, QAction *target);
	bool event(QEvent *event);

protected slots:
	void actionNewTabPrivate();
	void actionNewWindowPrivate();
	void actionOpen();
	void actionSaveSession();
	void actionManageSessions();
	void actionSession(QAction *action);
	void actionImportBookmarks();
	void actionWorkOffline(bool enabled);
	void actionTextEncoding(QAction *action);
	void actionClearClosedWindows();
	void actionRestoreClosedWindow();
	void actionViewHistory();
	void actionClearHistory();
	void actionAddBookmark(const QUrl &url = QUrl(), const QString &title = QString());
	void actionManageBookmarks();
	void actionOpenBookmark();
	void actionOpenBookmarkFolder();
	void actionCookies();
	void actionTransfers();
	void actionPreferences();
	void actionAboutApplication();
	void menuFileAboutToShow();
	void menuSessionsAboutToShow();
	void menuTextEncodingAboutToShow();
	void menuClosedWindowsAboutToShow();
	void menuBookmarksAboutToShow();
	void openBookmark();
	void triggerWindowAction();
	void updateClosedWindows();
	void updateBookmarks(int folder);
	void updateActions();

private:
	WindowsManager *m_windowsManager;
	QActionGroup *m_sessionsGroup;
	QActionGroup *m_textEncodingGroup;
	QMenu *m_closedWindowsMenu;
	QString m_currentBookmark;
	QList<QString> m_bookmarksToOpen;
	Ui::MainWindow *m_ui;

signals:
	void requestedNewWindow(bool privateSession = false, bool background = false, QUrl url = QUrl());
};

}

#endif
