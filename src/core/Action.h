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

#ifndef OTTER_ACTION_H
#define OTTER_ACTION_H

#include <QtWidgets/QAction>

namespace Otter
{

enum ActionIdentifier
{
	UnknownAction = 0,
	ActivateAddressFieldAction,
	ActivateTabOnLeftAction,
	ActivateTabOnRightAction,
	BookmarkAction,
	BookmarkLinkAction,
	ClearAllAction,
	ContentBlockingAction,
	CopyAction,
	CopyAddressAction,
	CopyAsPlainTextAction,
	CopyFrameLinkToClipboardAction,
	CopyImageToClipboardAction,
	CopyImageUrlToClipboardAction,
	CopyLinkToClipboardAction,
	CopyMediaUrlToClipboardAction,
	CreateSearchAction,
	CutAction,
	DeleteAction,
	FastForwardAction,
	FindAction,
	FindNextAction,
	FindPreviousAction,
	GoBackAction,
	GoForwardAction,
	ImagePropertiesAction,
	InspectElementAction,
	InspectPageAction,
	OpenFrameInNewTabAction,
	OpenFrameInNewTabBackgroundAction,
	OpenFrameInThisTabAction,
	OpenImageInNewTabAction,
	OpenLinkAction,
	OpenLinkInNewTabAction,
	OpenLinkInNewTabBackgroundAction,
	OpenLinkInNewWindowAction,
	OpenLinkInNewWindowBackgroundAction,
	OpenLinkInThisTabAction,
	OpenSelectionAsLinkAction,
	PasteAction,
	PasteAndGoAction,
	PrintAction,
	QuickFindAction,
	QuickPreferencesAction,
	RedoAction,
	ReloadAction,
	ReloadAndBypassCacheAction,
	ReloadFrameAction,
	ReloadImageAction,
	ReloadOrStopAction,
	ReloadTimeAction,
	RewindAction,
	SaveImageToDiskAction,
	SaveLinkToDiskAction,
	SaveLinkToDownloadsAction,
	SaveMediaToDiskAction,
	ScrollPageDownAction,
	ScrollPageLeftAction,
	ScrollPageRightAction,
	ScrollPageUpAction,
	ScrollToEndAction,
	ScrollToStartAction,
	SearchAction,
	SearchMenuAction,
	SelectAllAction,
	SpellCheckAction,
	StopAction,
	StopScheduledPageRefreshAction,
	ToggleMediaControlsAction,
	ToggleMediaLoopAction,
	ToggleMediaMuteAction,
	ToggleMediaPlayPauseAction,
	UndoAction,
	ValidateAction,
	ViewSourceAction,
	ViewSourceFrameAction,
	WebsitePreferencesAction,
	ZoomInAction,
	ZoomOriginalAction,
	ZoomOutAction
};

enum ActionScope
{
	MainWindowScope = 0,
	WindowScope = 1
};

class Action : public QAction
{
	Q_OBJECT

public:
	explicit Action(const QIcon &icon, const QString &text, QObject *parent = NULL);

	void setName(const QString &name);
	void setIdentifier(ActionIdentifier identifier);
	void setScope(ActionScope scope);
	QString getName() const;
	ActionIdentifier getIdentifier() const;
	ActionScope getScope() const;
	bool event(QEvent *event);

private:
	ActionIdentifier m_identifier;
	ActionScope m_scope;
};

}

#endif
