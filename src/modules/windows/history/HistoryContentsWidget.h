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

#ifndef OTTER_HISTORYCONTENTSWIDGET_H
#define OTTER_HISTORYCONTENTSWIDGET_H

#include "../../../ui/ContentsWidget.h"

#include <QtGui/QStandardItemModel>

namespace Otter
{

namespace Ui
{
	class HistoryContentsWidget;
}

struct HistoryEntry;

class Window;

class HistoryContentsWidget : public ContentsWidget
{
	Q_OBJECT

public:
	explicit HistoryContentsWidget(Window *window);
	~HistoryContentsWidget();

	void print(QPrinter *printer);
	QString getTitle() const;
	QLatin1String getType() const;
	QUrl getUrl() const;
	QIcon getIcon() const;
	bool isLoading() const;

protected:
	void changeEvent(QEvent *event);
	QStandardItem* findEntry(qint64 entry);
	qint64 getEntry(const QModelIndex &index) const;

protected slots:
	void filterHistory(const QString &filter);
	void populateEntries();
	void clearEntries();
	void addEntry(qint64 entry);
	void addEntry(const HistoryEntry &entry);
	void updateEntry(qint64 entry);
	void removeEntry(qint64 entry);
	void removeEntry();
	void removeDomainEntries();
	void openEntry(const QModelIndex &index = QModelIndex());
	void bookmarkEntry();
	void copyEntryLink();
	void showContextMenu(const QPoint &point);

private:
	QStandardItemModel *m_model;
	QHash<WindowAction, QAction*> m_actions;
	bool m_isLoading;
	Ui::HistoryContentsWidget *m_ui;
};

}

#endif
