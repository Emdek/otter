/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2010-2014 David Rosca <nowrep@gmail.com>
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

#include "ContentBlockingList.h"
#include "Console.h"
#include "ContentBlockingManager.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace Otter
{

ContentBlockingList::ContentBlockingList(QObject *parent) : QObject(parent),
	m_daysToExpire(4),
	m_isUpdated(false),
	m_isEnabled(false)
{
}

void ContentBlockingList::parseRules()
{
	QFile rulesFile(m_fullFilePath);
	bool isFileEmpty = true;

	if (!rulesFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		Console::addMessage(QCoreApplication::translate("main", "Failed to load adblock file: %0").arg(rulesFile.fileName()), Otter::OtherMessageCategory, ErrorMessageLevel);

		return;
	}

	QTextStream adFileStream(&rulesFile);
	const QString fileHeader = adFileStream.readLine().trimmed();

	while (!adFileStream.atEnd())
	{
		QString headerLine = adFileStream.readLine();

		if (!headerLine.startsWith(QLatin1Char('!')))
		{
			isFileEmpty = false;

			break;
		}

		if (headerLine.contains(QLatin1String("! Expires: ")))
		{
			m_daysToExpire = headerLine.remove(QLatin1String("! Expires: ")).split(QLatin1Char(' ')).at(0).toInt();
		}

		headerLine.remove(QLatin1Char(' '));

		if (headerLine.contains(QLatin1String("!URL:")))
		{
			m_updateUrl = headerLine.remove(QLatin1String("!URL:"));
		}
		else if (headerLine.contains(QLatin1String("!Lastmodified:")))
		{
			m_lastUpdate = QLocale(QLatin1String("UnitedStates")).toDateTime(headerLine.remove(QLatin1String("!Lastmodified:")).remove(QLatin1String("UTC")), QLatin1String("ddMMMyyyyhh:mm"));
			m_lastUpdate.setTimeSpec(Qt::UTC);
		}
	}

	if (!fileHeader.startsWith(QLatin1String("[Adblock Plus 2.")))
	{
		Console::addMessage(QCoreApplication::translate("main", "Loaded adblock file is not valid: %0").arg(rulesFile.fileName()), Otter::OtherMessageCategory, ErrorMessageLevel);

		return;
	}
	else if ((isFileEmpty && m_updateUrl.isValid()) || (m_lastUpdate.isValid() && m_lastUpdate.daysTo(QDateTime::currentDateTimeUtc()) > m_daysToExpire))
	{
		if (!m_isUpdated)
		{
			downloadUpdate();
		}
	}

	rulesFile.close();

	QtConcurrent::run(this, &ContentBlockingList::loadRuleFile);
}

void ContentBlockingList::loadRuleFile()
{
	m_root = new Node();
	m_domainExpression = QRegularExpression(QLatin1String("[:\?&/=]"));

	QFile rulesFile(m_fullFilePath);

	rulesFile.open(QIODevice::ReadOnly | QIODevice::Text);

	QTextStream adFileStream(&rulesFile);

	adFileStream.readLine(); // header

	while (!adFileStream.atEnd())
	{
		parseRuleLine(adFileStream.readLine());
	}

	if (m_cssHidingRules.length() > 0)
	{
		m_cssHidingRules = m_cssHidingRules.left(m_cssHidingRules.length() - 1);
		m_cssHidingRules += QLatin1String("{display:none;}");
	}

	m_isEnabled = true;

	emit updateCustomStyleSheets();

	rulesFile.close();
}

void ContentBlockingList::parseRuleLine(QString line)
{
	if (line.indexOf(QLatin1Char('!')) == 0 || line.isEmpty())
	{
		return;
	}

	if (line.startsWith(QLatin1String("##")))
	{
		m_cssHidingRules += line.mid(2) + QLatin1Char(',');

		return;
	}

	if (line.contains(QLatin1String("##")))
	{
		parseCssRule(line.split(QLatin1String("##")), m_cssSpecificDomainHidingRules);

		return;
	}

	if (line.contains(QLatin1String("#@#")))
	{
		parseCssRule(line.split(QLatin1String("#@#")), m_cssHidingRulesExceptions);

		return;
	}

	ContentBlockingRule rule;
	rule.isException = false;
	rule.needsDomainCheck = false;
	rule.ruleType = StandardFilterRule;
	rule.ruleOption = NoOption;
	rule.exceptionRuleOption = NoOption;

	int optionSeparator = line.indexOf(QLatin1Char('$'));

	if (optionSeparator >= 0)
	{
		QStringList options = line.mid(optionSeparator + 1).split(QLatin1Char(','), QString::SkipEmptyParts);

		for (int i = 0; i < options.count(); ++i)
		{
			const bool optionException = options.at(i).startsWith(QLatin1Char('~'));

			if (options.at(i).contains(QLatin1String("third-party")))
			{
				rule.ruleOption |= ThirdPartyOption;
				rule.exceptionRuleOption |= optionException ? ThirdPartyOption : NoOption;
			}
			else if (options.at(i).contains(QLatin1String("stylesheet")))
			{
				rule.ruleOption |= StyleSheetOption;
				rule.exceptionRuleOption |= optionException ? StyleSheetOption : NoOption;
			}
			else if (options.at(i).contains(QLatin1String("image")))
			{
				rule.ruleOption |= ImageOption;
				rule.exceptionRuleOption |= optionException ? ImageOption : NoOption;
			}
			else if (options.at(i).contains(QLatin1String("script")))
			{
				rule.ruleOption |= ScriptOption;
				rule.exceptionRuleOption |= optionException ? ScriptOption : NoOption;
			}
			else if (options.at(i).contains(QLatin1String("object")))
			{
				rule.ruleOption |= ObjectOption;
				rule.exceptionRuleOption |= optionException ? ObjectOption : NoOption;
			}
			else if (options.at(i).contains(QLatin1String("object-subrequest")) || options.at(i).contains(QLatin1String("object_subrequest")))
			{
				rule.ruleOption |= ObjectSubRequestOption;
				rule.exceptionRuleOption |= optionException ? ObjectSubRequestOption : NoOption;
				// TODO
				return;
			}
			else if (options.at(i).contains(QLatin1String("subdocument")))
			{
				rule.ruleOption |= SubDocumentOption;
				rule.exceptionRuleOption |= optionException ? SubDocumentOption : NoOption;
				// TODO
				return;
			}
			else if (options.at(i).contains(QLatin1String("xmlhttprequest")))
			{
				rule.ruleOption |= XmlHttpRequestOption;
				rule.exceptionRuleOption |= optionException ? XmlHttpRequestOption : NoOption;
			}
			else if (options.at(i).contains(QLatin1String("domain")))
			{
				const QStringList parsedDomains = options.at(i).mid(options.at(i).indexOf(QLatin1Char('=')) + 1).split(QLatin1Char('|'), QString::SkipEmptyParts);

				for (int j = 0; j < parsedDomains.count(); ++j)
				{
					if (parsedDomains.at(j).startsWith(QLatin1Char('~')))
					{
						rule.allowedDomains.append(parsedDomains.at(j).mid(1));

						continue;
					}

					rule.blockedDomains.append(parsedDomains.at(j));
				}
			}
			else
			{
				return;
			}
		}

		line = line.left(optionSeparator);
	}

	if (line.startsWith(QLatin1String("@@")))
	{
		line = line.mid(2);

		rule.isException = true;
	}

	if (line.startsWith(QLatin1String("||")))
	{
		line = line.mid(2);

		rule.needsDomainCheck = true;
	}

	if (line.endsWith(QLatin1Char('|')) || line.endsWith(QLatin1Char('*')) || line.endsWith(QLatin1Char('^')))
	{
		line = line.left(line.length() - 1);
	}

	if (line.startsWith(QLatin1Char('*')))
	{
		line = line.mid(1);
	}

	if (line.contains(QLatin1Char('*')) || line.contains(QLatin1Char('^')) || line.contains(QLatin1Char('|')))
	{
		// TODO

		return;
	}

	rule.rule = line;

	addRule(rule);

	return;
}

void ContentBlockingList::parseCssRule(const QStringList &line, QMultiHash<QString, QString> &list)
{
	const QStringList domains = line.at(0).split(QLatin1Char(','));

	for (int i = 0; i < domains.count(); ++i)
	{
		list.insert(domains.at(i), line.at(1));
	}
}

void ContentBlockingList::resolveRuleOptions(const ContentBlockingRule &rule, const QNetworkRequest &request, bool &isBlocked)
{
	const QString url = request.url().url();
	const QByteArray requestHeader = request.rawHeader(QByteArray("Accept"));
	const QString baseUrlHost = m_baseUrl.host();

	isBlocked = (rule.allowedDomains.count() > 0 ? (!resolveDomainExceptions(baseUrlHost, rule.allowedDomains)) : isBlocked);
	isBlocked = (rule.blockedDomains.count() > 0 ? (resolveDomainExceptions(baseUrlHost, rule.blockedDomains)) : isBlocked);

	if (rule.ruleOption & ThirdPartyOption)
	{
		if (baseUrlHost.isEmpty() || m_requestSubdomainList.contains(baseUrlHost))
		{
			isBlocked = (rule.exceptionRuleOption & ThirdPartyOption);
		}
		else
		{
			isBlocked = !(rule.exceptionRuleOption & ThirdPartyOption);
		}
	}

	if (rule.ruleOption & ImageOption)
	{
		if (requestHeader.contains(QByteArray("image/")) || url.endsWith(QLatin1String(".png")) || url.endsWith(QLatin1String(".jpg")) || url.endsWith(QLatin1String(".gif")))
		{
			isBlocked = (isBlocked ? !(rule.exceptionRuleOption & ImageOption) : isBlocked);
		}
		else
		{
			isBlocked = (isBlocked ? (rule.exceptionRuleOption & ImageOption) : isBlocked);
		}
	}

	if (rule.ruleOption & ScriptOption)
	{
		if (requestHeader.contains(QByteArray("script/")) || url.endsWith(QLatin1String(".js")))
		{
			isBlocked = (isBlocked ? !(rule.exceptionRuleOption & ScriptOption) : isBlocked);
		}
		else
		{
			isBlocked = (isBlocked ? (rule.exceptionRuleOption & ScriptOption) : isBlocked);
		}
	}

	if (rule.ruleOption & StyleSheetOption)
	{
		if (requestHeader.contains(QByteArray("text/css")) || url.endsWith(QLatin1String(".css")))
		{
			isBlocked = (isBlocked ? !(rule.exceptionRuleOption & StyleSheetOption) : isBlocked);
		}
		else
		{
			isBlocked = (isBlocked ? (rule.exceptionRuleOption & StyleSheetOption) : isBlocked);
		}
	}

	if (rule.ruleOption & ObjectOption)
	{
		if (requestHeader.contains(QByteArray("object")))
		{
			isBlocked = (isBlocked ? !(rule.exceptionRuleOption & ObjectOption) : isBlocked);
		}
		else
		{
			isBlocked = (isBlocked ? (rule.exceptionRuleOption & ObjectOption) : isBlocked);
		}
	}

	if (rule.ruleOption & SubDocumentOption)
	{
		// TODO
	}

	if (rule.ruleOption & ObjectSubRequestOption)
	{
		// TODO
	}

	if (rule.ruleOption & XmlHttpRequestOption)
	{
		if (request.rawHeader(QByteArray("X-Requested-With")) == QByteArray("XMLHttpRequest"))
		{
			isBlocked = (isBlocked ? !(rule.exceptionRuleOption & XmlHttpRequestOption) : isBlocked);
		}
		else
		{
			isBlocked = (isBlocked ? (rule.exceptionRuleOption & XmlHttpRequestOption) : isBlocked);
		}
	}
}

void ContentBlockingList::setFile(const QString &path, const QString &name)
{
	m_fileName = name;
	m_fullFilePath = path + name;
}

void ContentBlockingList::setEnabled(const bool enabled)
{
	if (!enabled && m_isEnabled)
	{
		m_isEnabled = enabled;

		clear();
	}
	else if (enabled && !m_isEnabled)
	{
		parseRules();
	}
}

void ContentBlockingList::addRule(const ContentBlockingRule &rule)
{
	Node *node = m_root;
	const QString ruleString = rule.rule;

	for (int i = 0; i < ruleString.length(); ++i)
	{
		const QChar value = ruleString.at(i);

		if (!node->children.contains(value))
		{
			Node *newNode = new Node;
			newNode->value = value;

			node->children[value] = newNode;
		}

		node = node->children[value];
	}

	node->rule = rule;
}

void ContentBlockingList::deleteNode(Node *node)
{
	QHashIterator<QChar, Node*> iterator(node->children);

	while (iterator.hasNext())
	{
		iterator.next();

		deleteNode(iterator.value());
	}

	delete node;
}

void ContentBlockingList::downloadUpdate()
{
	connect(&m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(updateDownloaded(QNetworkReply*)));

	QNetworkRequest request(m_updateUrl);

	m_networkManager.get(request);
}

void ContentBlockingList::updateDownloaded(QNetworkReply *reply)
{
	const QByteArray downloadedDataHeader = reply->readLine();
	const QByteArray downloadedDataChecksum = reply->readLine();
	const QByteArray downloadedData = reply->readAll();

	if (downloadedDataChecksum.contains(QByteArray("! Checksum: ")))
	{
		QByteArray checksum = downloadedDataChecksum;

		if (QCryptographicHash::hash(downloadedDataHeader + downloadedData, QCryptographicHash::Md5).toBase64().replace(QByteArray("="), QByteArray()) != checksum.replace(QByteArray("! Checksum: "), QByteArray()).replace(QByteArray("\n"), QByteArray()))
		{
			Console::addMessage(QCoreApplication::translate("main", "Content blocking file checksum mismatch: %0").arg(m_fullFilePath), Otter::OtherMessageCategory, ErrorMessageLevel);

			return;
		}
	}

	if (reply->error() != QNetworkReply::NoError || !downloadedDataHeader.trimmed().startsWith(QByteArray("[Adblock Plus 2.")))
	{
		Console::addMessage(QCoreApplication::translate("main", "Unable to download update for content blocking: %0.\nError: %1").arg(m_fullFilePath).arg(reply->errorString()), Otter::OtherMessageCategory, ErrorMessageLevel);

		return;
	}

	reply->deleteLater();

	QFile ruleFile(m_fullFilePath);

	if (!ruleFile.open(QFile::ReadWrite | QFile::Truncate))
	{
		Console::addMessage(QCoreApplication::translate("main", "Unable to write downloaded content blocking file: %0").arg(m_fullFilePath), Otter::OtherMessageCategory, ErrorMessageLevel);

		return;
	}

	ruleFile.write(downloadedDataHeader);
	ruleFile.write(QString("! URL: %0\n").arg(m_updateUrl.toString()).toUtf8());
	ruleFile.write(downloadedDataChecksum);

	if (!downloadedData.contains(QByteArray("! Last modified: ")))
	{
		ruleFile.write(QString("! Last modified: " + QLocale(QLatin1String("UnitedStates")).toString(QDateTime::currentDateTimeUtc(), QLatin1String("dd MMM yyyy hh:mm")) + " UTC\n").toUtf8());
	}

	ruleFile.write(downloadedData);
	ruleFile.close();

	if (ruleFile.error() != QFile::NoError)
	{
		// TODO
	}

	m_isUpdated = true;
	m_isEnabled = false;

	clear();
	parseRules();
}

void ContentBlockingList::clear()
{
	QtConcurrent::run(this, &ContentBlockingList::deleteNode, m_root);

	m_cssHidingRules.clear();
	m_cssHidingRulesExceptions.clear();
	m_cssSpecificDomainHidingRules.clear();
}

void ContentBlockingList::setListName(const QString &title)
{
	m_listName = title;
}

void ContentBlockingList::setConfigListName(const QString &name)
{
	m_configListName = name;
}

QString ContentBlockingList::getFileName() const
{
	return m_fileName;
}

QString ContentBlockingList::getListName() const
{
	return m_listName;
}

QString ContentBlockingList::getCssRules() const
{
	return m_cssHidingRules;
}

QString ContentBlockingList::getConfigListName() const
{
	return m_configListName;
}

QDateTime ContentBlockingList::getLastUpdate() const
{
	return m_lastUpdate.toLocalTime();
}

QMultiHash<QString, QString> ContentBlockingList::getSpecificDomainHidingRules() const
{
	return m_cssSpecificDomainHidingRules;
}

QMultiHash<QString, QString> ContentBlockingList::getHidingRulesExceptions() const
{
	return m_cssHidingRulesExceptions;
}

bool ContentBlockingList::resolveDomainExceptions(const QString &url, const QStringList &ruleList)
{
	for (int i = 0; i < ruleList.count(); ++i)
	{
		if (url.contains(ruleList.at(i)))
		{
			return true;
		}
	}

	return false;
}

bool ContentBlockingList::checkUrlSubstring(const QString &subString, const QNetworkRequest &request)
{
	Node *node = m_root;

	for (int i = 0; i < subString.length(); ++i)
	{
		const QChar treeChar = subString.at(i);

		if (!node->rule.rule.isEmpty() && checkRuleMatch(node->rule, request))
		{
			return true;
		}

		if (!node->children.contains(treeChar))
		{
			return false;
		}

		node = node->children[treeChar];
	}

	if (!node->rule.rule.isEmpty() && checkRuleMatch(node->rule, request))
	{
		return true;
	}

	return false;
}

bool ContentBlockingList::checkRuleMatch(const ContentBlockingRule &rule, const QNetworkRequest &request)
{
	bool isBlocked = false;

	if (rule.ruleType == StandardFilterRule)
	{
		if (request.url().url().contains(rule.rule))
		{
			m_requestSubdomainList = ContentBlockingManager::createSubdomainList(request.url().host());

			if (rule.needsDomainCheck)
			{
				if (!m_requestSubdomainList.contains(rule.rule.left(rule.rule.indexOf(m_domainExpression))))
				{
					return false;
				}
				else
				{
					isBlocked = true;
				}
			}

			if (isBlocked)
			{
				isBlocked = !rule.isException;
			}

			resolveRuleOptions(rule, request, isBlocked);
		}
	}

	return isBlocked;
}

bool ContentBlockingList::isEnabled() const
{
	return m_isEnabled;
}

bool ContentBlockingList::isUrlBlocked(const QNetworkRequest &request, const QUrl &baseUrl)
{
	const QString url = request.url().url();
	const int urlLenght = url.length();

	m_baseUrl = baseUrl;

	for (int i = 0; i < urlLenght; ++i)
	{
		const QString testString = url.right(urlLenght - i);

		if (checkUrlSubstring(testString, request))
		{
			return true;
		}
	}

	return false;
}

}
