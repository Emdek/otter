/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2015 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
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

#include "ToolBarWidget.h"
#include "AddressWidget.h"
#include "ContentsWidget.h"
#include "SearchWidget.h"
#include "Window.h"
#include "toolbars/GoBackActionWidget.h"
#include "toolbars/GoForwardActionWidget.h"
#include "toolbars/PanelChooserWidget.h"

namespace Otter
{

ToolBarWidget::ToolBarWidget(const ToolBarDefinition &definition, Window *window, QWidget *parent) : QToolBar(parent),
	m_window(window)
{
	setObjectName(definition.name);
	setStyleSheet(QLatin1String("QToolBar {padding:1px 3px;spacing:3px;}"));
	setAllowedAreas(Qt::AllToolBarAreas);
	setFloatable(false);

	for (int i = 0; i < definition.actions.count(); ++i)
	{
		if (definition.actions.at(i).action == QLatin1String("separator"))
		{
			addSeparator();
		}
		else if (definition.actions.at(i).action == QLatin1String("spacer"))
		{
			QWidget *spacer = new QWidget(this);
			spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

			addWidget(spacer);
		}
		else if (definition.actions.at(i).action == QLatin1String("AddressWidget"))
		{
			AddressWidget *addressWidget = new AddressWidget(m_window, false, this);

			if (m_window)
			{
				m_window->attachAddressWidget(addressWidget);
			}

			addWidget(addressWidget);
		}
		else if (definition.actions.at(i).action == QLatin1String("SearchWidget"))
		{
			SearchWidget *searchWidget = new SearchWidget(this);

			if (m_window)
			{
				m_window->attachSearchWidget(searchWidget);
			}

			addWidget(searchWidget);
		}
		else if (definition.actions.at(i).action == QLatin1String("PanelChooserWidget"))
		{
			addWidget(new PanelChooserWidget(this));
		}
		else
		{
			const int identifier = ActionsManager::getActionIdentifier(definition.actions.at(i).action.left(definition.actions.at(i).action.length() - 6));

			if (identifier >= 0)
			{
				Action *action = NULL;

				if (m_window && Action::isLocal(identifier))
				{
					action = m_window->getContentsWidget()->getAction(identifier);
				}
				else
				{
					action = ActionsManager::getAction(identifier, this);
				}

				if (action)
				{
					if (m_window)
					{
						action->setWindow(m_window);
					}

					addAction(action);
				}
			}
		}
	}
}

}
