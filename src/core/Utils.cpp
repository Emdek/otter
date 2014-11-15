/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2014 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
* Copyright (C) 2014 Jan Bajer aka bajasoft <jbajer@gmail.com>
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

#include "Utils.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QTime>
#include <QtCore/QtMath>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

namespace Otter
{

namespace Utils
{

QString elideText(const QString &text, QWidget *widget, int width)
{
	if (width < 0)
	{
		width = (QApplication::desktop()->screenGeometry().width() / 4);
	}

	return (widget ? widget->fontMetrics() : QApplication::fontMetrics()).elidedText(text, (QGuiApplication::isLeftToRight() ? Qt::ElideRight : Qt::ElideLeft), qMax(100, width));
}

QString formatConfigurationEntry(const QLatin1String &key, const QString &value, bool quote)
{
	QString escapedValue(value);
	escapedValue.replace(QLatin1Char('\n'), QLatin1String("\\n"));

	if (quote)
	{
		return QStringLiteral("%1=\"%2\"\n").arg(key).arg(escapedValue.replace(QLatin1Char('\"'), QLatin1String("\\\"")));
	}

	return QStringLiteral("%1=%2\n").arg(key).arg(escapedValue);
}

QString formatTime(int value)
{
	QTime time(0, 0);
	time = time.addSecs(value);

	if (value > 3600)
	{
		QString string = time.toString(QLatin1String("hh:mm:ss"));

		if (value > 86400)
		{
			string = QCoreApplication::translate("utils", "%n days %1", "", (qFloor((qreal) value / 86400))).arg(string);
		}

		return string;
	}

	return time.toString(QLatin1String("mm:ss"));
}

QString formatDateTime(const QDateTime &dateTime, const QString &format)
{
	return (format.isEmpty() ? QLocale().toString(dateTime, QLocale::ShortFormat) : QLocale().toString(dateTime, format));
}

QString formatUnit(qint64 value, bool isSpeed, int precision)
{
	if (value < 0)
	{
		return QString('?');
	}

	if (value > 1024)
	{
		if (value > 1048576)
		{
			if (value > 1073741824)
			{
				return QCoreApplication::translate("utils", (isSpeed ? "%1 GB/s" : "%1 GB")).arg((value / 1073741824.0), 0, 'f', precision);
			}

			return QCoreApplication::translate("utils", (isSpeed ? "%1 MB/s" : "%1 MB")).arg((value / 1048576.0), 0, 'f', precision);
		}

		return QCoreApplication::translate("utils", (isSpeed ? "%1 KB/s" : "%1 KB")).arg((value / 1024.0), 0, 'f', precision);
	}

	return QCoreApplication::translate("utils", (isSpeed ? "%1 B/s" : "%1 B")).arg(value);
}

QIcon getIcon(const QLatin1String &name, bool fromTheme)
{
	const QIcon icon(QStringLiteral(":/icons/%1.png").arg(name));

	return (fromTheme ? QIcon::fromTheme(name, icon) : icon);
}

}

}
