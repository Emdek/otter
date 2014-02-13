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

#ifndef OTTER_TRANSFERSCONTENTSWIDGET_H
#define OTTER_TRANSFERSCONTENTSWIDGET_H

#include "../../../core/TransfersManager.h"
#include "../../../ui/ContentsWidget.h"

#include <QtGui/QStandardItemModel>

namespace Otter
{

namespace Ui
{
	class TransfersContentsWidget;
}

class Window;

class TransfersContentsWidget : public ContentsWidget
{
	Q_OBJECT

public:
	explicit TransfersContentsWidget(Window *window);
	~TransfersContentsWidget();

	void print(QPrinter *printer);
	QAction* getAction(WindowAction action);
	QString getTitle() const;
	QLatin1String getType() const;
	QUrl getUrl() const;
	QIcon getIcon() const;
	bool isLoading() const;
	bool eventFilter(QObject *object, QEvent *event);

public slots:
	void triggerAction(WindowAction action, bool checked = false);

protected:
	void changeEvent(QEvent *event);
	TransferInformation* getTransfer(const QModelIndex &index);
	int findTransfer(TransferInformation *transfer) const;

protected slots:
	void triggerAction();
	void addTransfer(TransferInformation *transfer);
	void removeTransfer(TransferInformation *transfer);
	void removeTransfer();
	void updateTransfer(TransferInformation *transfer);
	void openTransfer(const QModelIndex &index = QModelIndex());
	void openTransferFolder(const QModelIndex &index = QModelIndex());
	void copyTransferInformation();
	void stopResumeTransfer();
	void redownloadTransfer();
	void startQuickTransfer();
	void clearFinishedTransfers();
	void showContextMenu(const QPoint &point);
	void updateActions();

private:
	QStandardItemModel *m_model;
	QHash<WindowAction, QAction*> m_actions;
	QHash<TransferInformation*, QQueue<qint64> > m_speeds;
	bool m_isLoading;
	Ui::TransfersContentsWidget *m_ui;
};

}

#endif
