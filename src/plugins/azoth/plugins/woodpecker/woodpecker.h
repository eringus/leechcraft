/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Slava Barinov <rayslava@gmail.com>
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

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ientityhandler.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/structures.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "xmlsettingsmanager.h"
#include "twitterinterface.h"

class QTranslator;

namespace LeechCraft
{
namespace Azoth
{
namespace Woodpecker
{
	class Plugin	: public QObject
					, public IInfo
					, public IHaveTabs
					, public IHaveSettings
					, public IHaveRecoverableTabs
					, public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IHaveTabs IHaveSettings IHaveRecoverableTabs IPlugin2)

		QList<QPair<TabClassInfo, std::function<void (TabClassInfo)>>> TabClasses_;
		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

		void MakeTab (QWidget*, const TabClassInfo&);

	public:
		TabClassInfo HomeTC_;
		TabClassInfo UserTC_;
		TabClassInfo SearchTC_;
		TabClassInfo FavoriteTC_;

		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
		void RecoverTabs (const QList<TabRecoverInfo>& infos);

		/** @brief Create new tab with certain parameters
		 *
		 * @param[in] tc New tab class
		 * @param[in] name Tab creation menu caption
		 * @param[in] mode Twitter API connection mode
		 * @param[in] params Twitter API parameters which should be added to every request
		 */
		void AddTab (const TabClassInfo& tc, const QString& name = QString (),
					 const FeedMode mode = FeedMode::HomeTimeline,
					 const KQOAuthParameters& params = KQOAuthParameters ());

	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void changeTooltip (QWidget*, QWidget*);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);
	};
};
};
};
