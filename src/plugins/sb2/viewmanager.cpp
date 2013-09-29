/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "viewmanager.h"
#include <QStandardItemModel>

#if USE_QT5
#include <QQmlEngine>
#include <QQmlContext>
#else
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#endif

#include <QtDebug>
#include <QDir>
#include <QSettings>
#include <QCoreApplication>
#include <QToolBar>
#include <QMainWindow>
#include <QAction>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/shortcuts/shortcutmanager.h>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "sbview.h"
#include "quarkproxy.h"
#include "quarkmanager.h"
#include "viewgeometrymanager.h"
#include "viewsettingsmanager.h"

namespace LeechCraft
{
namespace SB2
{
	namespace
	{
		const QString ImageProviderID = "ThemeIcons";

		class ViewItemsModel : public QStandardItemModel
		{
		public:
			enum Role
			{
				SourceURL= Qt::UserRole + 1,
				QuarkHasSettings,
				QuarkClass
			};

			ViewItemsModel (QObject *parent)
			: QStandardItemModel (parent)
			{
#ifndef USE_QT5
				setRoleNames (roleNames ());
#endif
			}

			QHash<int, QByteArray> roleNames () const
			{
				QHash<int, QByteArray> names;
				names [Role::SourceURL] = "sourceURL";
				names [Role::QuarkHasSettings] = "quarkHasSettings";
				names [Role::QuarkClass] = "quarkClass";
				return names;
			}
		};
	}

	ViewManager::ViewManager (ICoreProxy_ptr proxy, Util::ShortcutManager *shortcutMgr, QMainWindow *window, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, ViewItemsModel_ (new ViewItemsModel (this))
	, View_ (new SBView)
	, Toolbar_ (new QToolBar (tr ("SB2 panel")))
	, Window_ (window)
	, IsDesktopMode_ (qApp->arguments ().contains ("--desktop"))
	, OnloadWindowIndex_ (GetWindowIndex ())
	, SettingsManager_ (new ViewSettingsManager (this))
	, GeomManager_ (new ViewGeometryManager (this))
	{
		const auto& file = Util::GetSysPath (Util::SysPath::QML, "sb2", "SideView.qml");
		if (file.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "file not found";
			return;
		}

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			View_->engine ()->addImportPath (cand);

		View_->rootContext ()->setContextProperty ("itemsModel", ViewItemsModel_);
		View_->rootContext ()->setContextProperty ("quarkProxy", new QuarkProxy (this, proxy, this));
		View_->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		View_->rootContext ()->setContextProperty ("SB2_settingsModeTooltip", tr ("Settings mode"));
		View_->rootContext ()->setContextProperty ("SB2_quarkOrderTooltip", tr ("Quarks order"));
		View_->rootContext ()->setContextProperty ("SB2_addQuarkTooltip", tr ("Add quark"));
		View_->rootContext ()->setContextProperty ("SB2_showPanelSettingsTooltip", tr ("Show panel settings"));
		View_->rootContext ()->setContextProperty ("quarkContext", "panel_" + QString::number (GetWindowIndex ()));
		View_->engine ()->addImageProvider (ImageProviderID, new Util::ThemeImageProvider (proxy));

#ifdef USE_QT5
		auto container = QWidget::createWindowContainer (View_);
		container->setFocusPolicy (Qt::TabFocus);
		View_->SetParent (container);
		Toolbar_->addWidget (container);
#else
		View_->SetParent (Toolbar_);
		Toolbar_->addWidget (View_);
		View_->setVisible (true);
#endif

		GeomManager_->Manage ();

		View_->setSource (QUrl::fromLocalFile (file));

		LoadRemovedList ();
		LoadQuarkOrder ();

		auto toggleAct = Toolbar_->toggleViewAction ();
		toggleAct->setProperty ("ActionIcon", "layer-visible-on");
		toggleAct->setShortcut (QString ("Ctrl+J,S"));
		shortcutMgr->RegisterAction ("TogglePanel", toggleAct, true);

		window->addAction (toggleAct);
	}

	SBView* ViewManager::GetView () const
	{
		return View_;
	}

	QToolBar* ViewManager::GetToolbar () const
	{
		return Toolbar_;
	}

	QMainWindow* ViewManager::GetManagedWindow () const
	{
		return Window_;
	}

	QRect ViewManager::GetFreeCoords () const
	{
		QRect result = Window_->rect ();
		result.moveTopLeft (Window_->mapToGlobal ({ 0, 0 }));

		if (!IsDesktopMode_)
			switch (Window_->toolBarArea (Toolbar_))
			{
			case Qt::LeftToolBarArea:
				result.setLeft (result.left () + Toolbar_->width ());
				break;
			case Qt::RightToolBarArea:
				result.setRight (result.right () - Toolbar_->width ());
				break;
			case Qt::TopToolBarArea:
				result.setTop (result.top () + Toolbar_->height ());
				break;
			case Qt::BottomToolBarArea:
				result.setBottom (result.bottom () - Toolbar_->height ());
				break;
			default:
				break;
			}

		return result;
	}

	ViewSettingsManager* ViewManager::GetViewSettingsManager () const
	{
		return SettingsManager_;
	}

	bool ViewManager::IsDesktopMode () const
	{
		return IsDesktopMode_;
	}

	void ViewManager::SecondInit ()
	{
		for (const auto& component : FindAllQuarks ())
			AddComponent (component);
	}

	void ViewManager::RegisterInternalComponent (QuarkComponent_ptr c)
	{
		InternalComponents_ << c;
	}

	void ViewManager::ShowSettings (const QUrl& url)
	{
		auto manager = Quark2Manager_ [url];
		manager->ShowSettings ();
	}

	void ViewManager::RemoveQuark (const QUrl& url)
	{
		for (int i = 0; i < ViewItemsModel_->rowCount (); ++i)
		{
			auto item = ViewItemsModel_->item (i);
			if (item->data (ViewItemsModel::Role::SourceURL) != url)
				continue;

			ViewItemsModel_->removeRow (i);
		}

		auto mgr = Quark2Manager_.take (url);
		AddToRemoved (mgr->GetID ());
	}

	void ViewManager::RemoveQuark (const QString& id)
	{
		QUrl url;
		for (int i = 0; i < ViewItemsModel_->rowCount (); ++i)
		{
			auto item = ViewItemsModel_->item (i);
			if (item->data (ViewItemsModel::Role::QuarkClass) != id)
				continue;

			url = item->data (ViewItemsModel::Role::SourceURL).toUrl ();
			ViewItemsModel_->removeRow (i);
		}

		if (!url.isValid ())
			return;

		auto mgr = Quark2Manager_.take (url);
		AddToRemoved (mgr->GetID ());
	}

	void ViewManager::UnhideQuark (QuarkComponent_ptr component, QuarkManager_ptr manager)
	{
		if (!manager)
			return;

		RemoveFromRemoved (manager->GetID ());

		AddComponent (component, manager);
	}

	void ViewManager::MoveQuark (int from, int to)
	{
		if (from < to)
			--to;
		ViewItemsModel_->insertRow (to, ViewItemsModel_->takeRow (from));

		SaveQuarkOrder ();
	}

	void ViewManager::MovePanel (Qt::ToolBarArea area)
	{
		GeomManager_->SetPosition (area);
	}

	QuarkComponents_t ViewManager::FindAllQuarks () const
	{
		auto result = InternalComponents_;

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, "quarks"))
			result += ScanRootDir (QDir (cand));

		QDir local = QDir::home ();
		if (local.cd (".leechcraft") &&
			local.cd ("data") &&
			local.cd ("quarks"))
			result += ScanRootDir (local);

		auto pm = Proxy_->GetPluginsManager ();
		for (auto prov : pm->GetAllCastableTo<IQuarkComponentProvider*> ())
			result += prov->GetComponents ();

		return result;
	}

	QList<QUrl> ViewManager::GetAddedQuarks () const
	{
		QList<QUrl> result;

		for (int i = 0, rc = ViewItemsModel_->rowCount (); i < rc; ++i)
		{
			const auto item = ViewItemsModel_->item (i);
			result << item->data (ViewItemsModel::Role::SourceURL).toUrl ();
		}

		return result;
	}

	QuarkManager_ptr ViewManager::GetAddedQuarkManager (const QUrl& url) const
	{
		return Quark2Manager_ [url];
	}

	std::shared_ptr<QSettings> ViewManager::GetSettings () const
	{
		const auto subSet = GetWindowIndex () || IsDesktopMode_;

		const auto& org = QCoreApplication::organizationName ();
		const auto& app = QCoreApplication::applicationName () + "_SB2";
		std::shared_ptr<QSettings> result (new QSettings (org, app),
				[subSet] (QSettings *settings) -> void
				{
					if (subSet)
						settings->endGroup ();
					delete settings;
				});

		if (subSet)
			result->beginGroup (QString ("%1_%2")
					.arg (OnloadWindowIndex_)
					.arg (IsDesktopMode_));

		return result;
	}

	void ViewManager::AddComponent (QuarkComponent_ptr comp)
	{
		QuarkManager_ptr mgr;
		try
		{
			mgr.reset (new QuarkManager (comp, this, Proxy_));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< e.what ();
			return;
		}

		AddComponent (comp, mgr);
	}

	void ViewManager::AddComponent (QuarkComponent_ptr comp, QuarkManager_ptr mgr)
	{
		if (!mgr->IsValidArea ())
			return;

		if (RemovedIDs_.contains (mgr->GetID ()))
			return;

		Quark2Manager_ [comp->Url_] = mgr;

		auto item = new QStandardItem;
		item->setData (comp->Url_, ViewItemsModel::Role::SourceURL);
		item->setData (mgr->HasSettings (), ViewItemsModel::Role::QuarkHasSettings);
		item->setData (mgr->GetID (), ViewItemsModel::Role::QuarkClass);

		const int pos = PreviousQuarkOrder_.indexOf (mgr->GetID ());
		if (pos == -1 || pos == PreviousQuarkOrder_.size () - 1)
			ViewItemsModel_->appendRow (item);
		else
		{
			bool added = false;
			for (int i = pos + 1; i < PreviousQuarkOrder_.size (); ++i)
			{
				const auto& thatId = PreviousQuarkOrder_.at (i);
				for (int j = 0; j < ViewItemsModel_->rowCount (); ++j)
				{
					if (ViewItemsModel_->item (j)->data (ViewItemsModel::Role::QuarkClass) != thatId)
						continue;

					ViewItemsModel_->insertRow (j, item);
					added = true;
					break;
				}

				if (added)
					break;
			}
			if (!added)
				ViewItemsModel_->appendRow (item);
		}
	}

	QuarkComponents_t ViewManager::ScanRootDir (const QDir& dir) const
	{
		QuarkComponents_t result;
		for (const auto& entry : dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable))
		{
			QDir quarkDir (dir);
			quarkDir.cd (entry);
			if (!quarkDir.exists (entry + ".qml"))
				continue;

			QuarkComponent_ptr c (new QuarkComponent);
			c->Url_ = QUrl::fromLocalFile (quarkDir.absoluteFilePath (entry + ".qml"));
			result << c;
		}
		return result;
	}

	void ViewManager::AddToRemoved (const QString& id)
	{
		RemovedIDs_ << id;
		SaveRemovedList ();
	}

	void ViewManager::RemoveFromRemoved (const QString& id)
	{
		RemovedIDs_.remove (id);
		SaveRemovedList ();
	}

	void ViewManager::SaveRemovedList () const
	{
		auto settings = GetSettings ();
		settings->beginGroup ("RemovedList");
		settings->setValue ("IDs", QStringList (RemovedIDs_.toList ()));
		settings->endGroup ();
	}

	void ViewManager::LoadRemovedList ()
	{
		auto settings = GetSettings ();
		settings->beginGroup ("RemovedList");
		RemovedIDs_ = QSet<QString>::fromList (settings->value ("IDs").toStringList ());
		settings->endGroup ();
	}

	void ViewManager::SaveQuarkOrder ()
	{
		PreviousQuarkOrder_.clear ();
		for (int i = 0; i < ViewItemsModel_->rowCount (); ++i)
		{
			auto item = ViewItemsModel_->item (i);
			PreviousQuarkOrder_ << item->data (ViewItemsModel::Role::QuarkClass).toString ();
		}

		auto settings = GetSettings ();
		settings->beginGroup ("QuarkOrder");
		settings->setValue ("IDs", PreviousQuarkOrder_);
		settings->endGroup ();
	}

	void ViewManager::LoadQuarkOrder ()
	{
		auto settings = GetSettings ();
		settings->beginGroup ("QuarkOrder");
		PreviousQuarkOrder_ = settings->value ("IDs").toStringList ();
		settings->endGroup ();
	}

	int ViewManager::GetWindowIndex () const
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();
		return rootWM->GetWindowIndex (Window_);
	}
}
}
