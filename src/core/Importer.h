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

#ifndef OTTER_IMPORTER_H
#define OTTER_IMPORTER_H

#include <QtCore/QObject>

namespace Otter
{

enum ImportType
{
	OtherImport = 0,
	FullImport = 1,
	BookmarksImport = 2,
	CookiesImport = 3,
	FeedsImport = 4,
	HistoryImport = 5,
	MailImport = 6,
	PasswordsImport = 7,
	SettingsImport = 8
};

class Importer : public QObject
{
	Q_OBJECT

public:
	explicit Importer(QObject *parent = NULL);

	virtual QWidget* getOptionsWidget() = 0;
	virtual QString getTitle() const = 0;
	virtual QString getDescription() const = 0;
	virtual QString getVersion() const = 0;
	virtual QString getSuggestedPath() const = 0;
	virtual QString getBrowser() const = 0;
	virtual ImportType getType() const = 0;
	virtual bool import() = 0;
	virtual void setPath(const QString &path) = 0;

signals:
	void importProgress(int amount, int total, ImportType type);
};

}

#endif
