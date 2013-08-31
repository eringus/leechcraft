/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Vladislav Tyulbashev
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "vlcscrollbar.h"
#include "soundwidget.h"
#include "signalledwidget.h"

class QToolBar;
class QMenu;
class QLabel;
class QTimer;
class QToolButton;
class QResizeEvent;

namespace LeechCraft
{
namespace vlc
{
	class VlcPlayer;
	class VlcWidget : public QWidget
					, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)
		
		QObject *Parent_;
		VlcPlayer *VlcPlayer_;
		QToolBar *Bar_;
		QAction *Open_;
		QToolButton *OpenButton_;
		QAction *TogglePlay_;
		QAction *Stop_;
		QAction *FullScreenAction_;
		
		VlcScrollBar *ScrollBar_;
		QLabel *TimeLeft_;
		QLabel *TimeAll_;
		
		QLabel *FullScreenTimeLeft_;
		QLabel *FullScreenTimeAll_;
		VlcScrollBar *FullScreenVlcScrollBar_;
		SignalledWidget *FullScreenPanel_;
		QTimer *TerminatePanel_;
		
		QToolButton *TogglePlayButton_;
		QToolButton *StopButton_;
		QToolButton *FullScreenButton_;
		
		bool ForbidFullScreen_;
		bool FullScreen_;
		bool AllowFullScreenPanel_;
		SignalledWidget *FullScreenWidget_;
		QTimer *InterfaceUpdater_;
		SignalledWidget *VlcMainWidget_;
		SoundWidget *SoundWidget_;
		SoundWidget *FullScreenSoundWidget_;
		QMenu *ContextMenu_;
		
		QString GetNewSubtitles ();
		void GenerateToolBar ();
		QMenu* GenerateMenuForOpenAction ();
		void PrepareFullScreen ();
		void ForbidFullScreen ();
		void ConnectWidgetToMe (SignalledWidget*);
		
	public:
		explicit VlcWidget (QWidget *parent = 0);
		~VlcWidget();
		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
		static TabClassInfo GetTabInfo ();
		void TabMadeCurrent ();
		void TabLostCurrent ();
		
	private slots:
		void addFile ();
		void addFolder ();
		void addUrl ();
		void addDVD ();
		
		void updateInterface ();
		void toggleFullScreen ();
		void allowFullScreen ();
		
		void generateContextMenu (QPoint);
		void setAudioTrack (QAction*);
		void setSubtitles (QAction*);
		
		void keyPressEvent (QKeyEvent*);
		void mousePressEvent (QMouseEvent*);
		void mouseDoubleClickEvent (QMouseEvent*);
		void mouseMoveEvent (QMouseEvent*);
		void wheelEvent (QWheelEvent*);
		
		void fullScreenPanelRequested ();
		void hideFullScreenPanel ();
		
		void AllowPanel ();
		void ForbidPanel ();
		
	signals:
		void deleteMe (QWidget*);
	};
}
}
