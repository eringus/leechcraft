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

#pragma once

#include <QObject>
#include <QImage>
#include <QPointer>
#include <QStringList>
#include <interfaces/azoth/iclentry.h>
#include "structures.h"

class QTimer;

namespace LeechCraft
{
namespace Azoth
{
namespace Murm
{
	class VkAccount;
	class VkMessage;
	class PhotoStorage;
	class VCardDialog;

	class VkEntry : public QObject
				  , public ICLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry)

		VkAccount * const Account_;
		UserInfo Info_;

		QList<VkMessage*> Messages_;

		QTimer *RemoteTypingTimer_;
		QTimer *LocalTypingTimer_;

		bool HasUnread_ = false;

		QImage Avatar_;

		QPointer<VCardDialog> VCardDialog_;

		QStringList Groups_;
	public:
		VkEntry (const UserInfo&, VkAccount*);

		void UpdateInfo (const UserInfo&);
		const UserInfo& GetInfo () const;

		void Send (VkMessage*);
		void Store (VkMessage*);

		VkMessage* FindMessage (qulonglong) const;
		void HandleMessage (const MessageInfo&);

		void HandleTypingNotification ();

		QObject* GetQObject ();
		QObject* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString& name);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList& groups);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType type, const QString& variant, const QString& body);
		QList<QObject*> GetAllMessages () const;
		void PurgeMessages (const QDateTime& before);
		void SetChatPartState (ChatPartState state, const QString& variant);
		EntryStatus GetStatus (const QString& variant = QString ()) const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();
		void ChatTabClosed ();
	private slots:
		void handleTypingTimeout ();
		void sendTyping ();

		void handleGotStorageImage (const QUrl&);
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void avatarChanged (const QImage&);
		void rawinfoChanged (const QString&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();
	};
}
}
}
