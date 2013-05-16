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

#include "serverobject.h"
#include <util/xpc/util.h>
#include <util/util.h>
#include <util/notificationactionhandler.h>
#include <interfaces/an/constants.h>
#include <interfaces/core/ientitymanager.h>
#include <QUrl>

namespace LeechCraft
{
namespace Laughty
{
	namespace
	{
		const QString LaughtyID = "org.LeechCraft.Laughty";
	}

	ServerObject::ServerObject (ICoreProxy_ptr proxy)
	: Proxy_ (proxy)
	, LastID_ (0)
	{
	}

	QStringList ServerObject::GetCapabilities () const
	{
		return
		{
			"actions",
			"body",
			"body-hyperlinks",
			"body-images",
			"body-markup",
			"persistence"
		};
	}

	namespace
	{
		Priority GetPriority (const QVariantMap& hints)
		{
			switch (hints.value ("urgency", 1).toInt ())
			{
			case 0:
				return PInfo_;
			case 1:
				return PInfo_;
			default:
				return PWarning_;
			}
		}

		QPair<QString, QString> GetCatTypePair (const QVariantMap& hints)
		{
			return { AN::CatGeneric, AN::TypeGeneric };
		}
	}

	uint ServerObject::Notify (const QString& app_name, uint replaces_id,
			const QString& app_icon, const QString& summary, const QString& body,
			const QStringList& actions, const QVariantMap& hints, uint expire_timeout)
	{
		const auto replaces = hints.value ("replaces_id", 0).toInt ();
		const auto id = replaces > 0 ? replaces : ++LastID_;

		const auto prio = GetPriority (hints);

		Entity e;
		if (hints.value ("transient", false).toBool () == true)
			e = Util::MakeNotification (app_name, summary, prio);
		else
		{
			const auto& catTypePair = GetCatTypePair (hints);
			e = Util::MakeAN (app_name,
					summary,
					prio,
					LaughtyID,
					catTypePair.first, catTypePair.second,
					LaughtyID + '/' + QString::number (id),
					QStringList (),
					0,
					1,
					summary,
					body);
		}

		HandleActions (e, id, actions, hints);
		HandleSounds (hints);

		Proxy_->GetEntityManager ()->HandleEntity (e);

		return id;
	}

	void ServerObject::CloseNotification (uint id)
	{
		const auto& e = Util::MakeANCancel (LaughtyID, LaughtyID + '/' + id);
		Proxy_->GetEntityManager ()->HandleEntity (e);

		emit NotificationClosed (id, 3);
	}

	void ServerObject::HandleActions (Entity& e, int id, const QStringList& actions, const QVariantMap& hints)
	{
		if (actions.isEmpty () || actions.size () % 2)
			return;

		const auto resident = hints.value ("resident", false).toBool ();

		auto nah = new Util::NotificationActionHandler (e);

		for (int i = 0; i < actions.size (); i += 2)
		{
			auto key = actions.at (i);
			nah->AddFunction (actions.at (i + 1),
					[this, key, id, resident] () -> void
					{
						emit ActionInvoked (id, key);
						if (!resident)
							emit NotificationClosed (id, 2);
					});
		}

		if (resident)
			nah->AddFunction (tr ("Dismiss"),
					[this, id] { emit NotificationClosed (id, 2); });
	}

	void ServerObject::HandleSounds (const QVariantMap& hints)
	{
		if (hints.contains ("sound-name"))
			qWarning () << Q_FUNC_INFO
					<< "sounds aren't supported yet :(";

		if (!hints.contains ("sound-file"))
			return;

		const auto& filename = hints.value ("sound-file").toString ();
		const auto& e = Util::MakeEntity (QUrl::fromLocalFile (filename),
				QString (),
				TaskParameter::Internal);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}