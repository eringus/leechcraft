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

#ifndef PLUGINS_AZOTH_PLUGINS_ZHEET_MSNMESSAGE_H
#define PLUGINS_AZOTH_PLUGINS_ZHEET_MSNMESSAGE_H
#include <QObject>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/iadvancedmessage.h>

namespace MSN
{
	class Message;
};

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
	class MSNBuddyEntry;

	class MSNMessage : public QObject
					 , public IMessage
					 , public IAdvancedMessage
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMessage LeechCraft::Azoth::IAdvancedMessage);

		MSNBuddyEntry *Entry_;

		Direction Dir_;
		MessageType MT_;
		MessageSubType MST_;
		QString Body_;
		QDateTime DateTime_;

		bool IsDelivered_;

		int MsgID_;
	public:
		MSNMessage (Direction, MessageType, MSNBuddyEntry*);
		MSNMessage (MSN::Message*, MSNBuddyEntry*);

		int GetID () const;
		void SetID (int);
		void SetDelivered ();

		QObject* GetQObject ();
		void Send ();
		void Store ();
		Direction GetDirection () const;
		MessageType GetMessageType () const;
		MessageSubType GetMessageSubType () const;
		QObject* OtherPart () const;
		QString GetOtherVariant () const;
		QString GetBody () const;
		void SetBody (const QString& body);
		QDateTime GetDateTime () const;
		void SetDateTime (const QDateTime& timestamp);

		bool IsDelivered () const;
	signals:
		void messageDelivered ();
	};
}
}
}

#endif
