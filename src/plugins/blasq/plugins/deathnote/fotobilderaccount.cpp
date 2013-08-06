/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
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

#include "fotobilderaccount.h"
#include <QDomDocument>
#include <QCryptographicHash>
#include <QInputDialog>
#include <QMainWindow>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardItemModel>
#include <QtDebug>
#include <QUuid>
#include <QXmlQuery>
#include <boost/concept_check.hpp>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <util/passutils.h>
#include <util/util.h>
#include "fotobilderservice.h"

namespace LeechCraft
{
namespace Blasq
{
namespace DeathNote
{
	namespace
	{
		const QString Url ("http://pics.livejournal.com/interface/simple");
		//Available sizes: 100, 300, 320, 600, 640, 900, 1000

		const QString SmallSize ("320");
		const QString MediumSize ("640");
	}

	FotoBilderAccount::FotoBilderAccount (const QString& name, FotoBilderService *service,
			ICoreProxy_ptr proxy, const QString& login, const QByteArray& id)
	: QObject (service)
	, Name_ (name)
	, Service_ (service)
	, Proxy_ (proxy)
	, ID_ (id.isEmpty () ? QUuid::createUuid ().toByteArray () : id)
	, Login_ (login)
	, FirstRequest_ (true)
	, CollectionsModel_ (new NamedModel<QStandardItemModel> (this))
	, AllPhotosItem_ (0)
	{
		CollectionsModel_->setHorizontalHeaderLabels ({ tr ("Name") });
	}

	ICoreProxy_ptr FotoBilderAccount::GetProxy () const
	{
		return Proxy_;
	}

	QByteArray FotoBilderAccount::Serialize () const
	{
		QByteArray result;
		{
			QDataStream out (&result, QIODevice::WriteOnly);
			out << static_cast<quint8> (1)
					<< Name_
					<< ID_
					<< Login_;
		}
		return result;
	}

	FotoBilderAccount* FotoBilderAccount::Deserialize (const QByteArray& ba,
			FotoBilderService *service, ICoreProxy_ptr proxy)
	{
		QDataStream in (ba);

		quint8 version = 0;
		in >> version;
		if (version < 1 || version > 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return nullptr;
		}

		QString name;
		QString login;
		QByteArray id;
		in >> name
				>> id
				>> login;

		return new FotoBilderAccount (name, service, proxy, login, id);
	}

	QObject* FotoBilderAccount::GetQObject ()
	{
		return this;
	}

	IService* FotoBilderAccount::GetService () const
	{
		return Service_;
	}

	QString FotoBilderAccount::GetName () const
	{
		return Name_;
	}

	QByteArray FotoBilderAccount::GetID () const
	{
		return ID_;
	}

	QAbstractItemModel* FotoBilderAccount::GetCollectionsModel () const
	{
		return CollectionsModel_;
	}

	namespace
	{
		QNetworkRequest CreateRequest (const QMap<QByteArray, QByteArray>& fields)
		{
			QNetworkRequest request (Url);
			for (const auto& field : fields.keys ())
				request.setRawHeader (field, fields [field]);

			return request;
		}

		QByteArray CreateDomDocumentFromReply (QNetworkReply *reply, QDomDocument &document)
		{
			if (!reply)
				return QByteArray ();

			const auto& content = reply->readAll ();
			reply->deleteLater ();
			QString errorMsg;
			int errorLine = -1, errorColumn = -1;
			if (!document.setContent (content, &errorMsg, &errorLine, &errorColumn))
			{
				qWarning () << Q_FUNC_INFO
						<< errorMsg
						<< "in line:"
						<< errorLine
						<< "column:"
						<< errorColumn;
				return QByteArray ();
			}

			return content;
		}

		QByteArray GetHashedChallenge (const QString& password, const QString& challenge)
		{
			const QByteArray passwordHash = QCryptographicHash::hash (password.toUtf8 (),
					QCryptographicHash::Md5).toHex ();
			return QCryptographicHash::hash ((challenge + passwordHash).toUtf8 (),
					QCryptographicHash::Md5).toHex ();
		}
	}

	bool FotoBilderAccount::FotoBilderErrorExists (const QByteArray& content)
	{
		QXmlQuery query;
		query.setFocus (content);

		QString code;
		query.setQuery ("/FBResponse/Error/@code/data(.)");
		if (!query.evaluateTo (&code))
			return false;
		code = code.simplified ();

		QString string;
		query.setQuery ("/FBResponse/Error/text()");
		if (!query.evaluateTo (&string))
			return false;
		string = string.simplified ();

		if (code.isEmpty () || string.isEmpty ())
			return false;

		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Blasq DeathNote",
				tr ("Error code:%1 (%2)")
						.arg (code)
						.arg (string),
				Priority::PWarning_));

		return true;
	}

	void FotoBilderAccount::Login ()
	{
		CallsQueue_ << [this] (const QString& challenge) { LoginRequest (challenge); };
		GetChallenge ();
	}

	void FotoBilderAccount::RequestGalleries ()
	{
		CallsQueue_ << [this] (const QString& challenge) { GetGalsRequest (challenge); };
		GetChallenge ();
	}

	void FotoBilderAccount::RequestPictures ()
	{
		CallsQueue_ << [this] (const QString& challenge) { GetPicsRequest (challenge); };
		GetChallenge ();
	}

	void FotoBilderAccount::UpdateCollections ()
	{
		if (FirstRequest_)
		{
			Login ();
			FirstRequest_ = false;
		}

		RequestGalleries ();
		RequestPictures ();
	}

	QString FotoBilderAccount::GetPassword () const
	{
		QString key ("org.LeechCraft.Blasq.PassForAccount/" + GetID ());
		return Util::GetPassword (key, tr ("Enter password"), Service_);
	}

	void FotoBilderAccount::GetChallenge ()
	{
		auto reply = Proxy_->GetNetworkAccessManager ()->
				get (CreateRequest (Util::MakeMap<QByteArray, QByteArray> ({
						{ "X-FB-User", Login_.toUtf8 () },
						{ "X-FB-Mode", "GetChallenge" } })));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGetChallengeRequestFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void FotoBilderAccount::LoginRequest (const QString& challenge)
	{
		auto reply = Proxy_->GetNetworkAccessManager ()->
				get (CreateRequest (Util::MakeMap<QByteArray, QByteArray> ({
						{ "X-FB-User", Login_.toUtf8 () },
						{ "X-FB-Mode", "Login" },
						{ "X-FB-Auth", ("crp:" + challenge + ":" +
								GetHashedChallenge (GetPassword (), challenge))
									.toUtf8 () },
						{ "X-FB-Login.ClientVersion",
								"LeechCraft Blasq/" + Proxy_->GetVersion ()
										.toUtf8 () } })));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleLoginRequestFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void FotoBilderAccount::GetGalsRequest (const QString& challenge)
	{
		auto reply = Proxy_->GetNetworkAccessManager ()->
				get (CreateRequest (Util::MakeMap<QByteArray, QByteArray> ({
					{ "X-FB-User", Login_.toUtf8 () },
					{ "X-FB-Mode", "GetGals" },
					{ "X-FB-Auth", ("crp:" + challenge + ":" +
							GetHashedChallenge (GetPassword (), challenge))
								.toUtf8 () } })));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotAlbums ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void FotoBilderAccount::GetPicsRequest (const QString& challenge)
	{
		auto reply = Proxy_->GetNetworkAccessManager ()->
				get (CreateRequest (Util::MakeMap<QByteArray, QByteArray> ({
						{ "X-FB-User", Login_.toUtf8 () },
						{ "X-FB-Mode", "GetPics" },
						{ "X-FB-Auth", ("crp:" + challenge + ":" +
								GetHashedChallenge (GetPassword (), challenge))
									.toUtf8 () } })));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotPhotos ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void FotoBilderAccount::handleGetChallengeRequestFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		QXmlQuery query;
		query.setFocus (content);

		QString challenge;
		query.setQuery ("/FBResponse/GetChallengeResponse/Challenge/text()");
		if (!query.evaluateTo (&challenge))
			return;

		if (!CallsQueue_.isEmpty ())
			CallsQueue_.dequeue () (challenge.trimmed ());
	}

	namespace
	{
		Quota ParseLoginResponse (const QDomDocument& document)
		{
			Quota quota;

			const auto& list = document.elementsByTagName ("Quota");
			if (!list.isEmpty ())
			{
				const auto& fieldsList = list.at (0).childNodes ();
				for (int i = 0, size = fieldsList.size (); i < size; ++i)
				{
					const auto& fieldElem = fieldsList.at (i).toElement ();
					if (fieldElem.tagName () == "Total")
						quota.Total_ = fieldElem.text ().toULongLong ();
					else if (fieldElem.tagName () == "Used")
						quota.Used_ = fieldElem.text ().toULongLong ();
					else if (fieldElem.tagName () == "Remaining")
						quota.Remaining_ = fieldElem.text ().toULongLong ();
				}
			}

			return quota;
		}

		QList<Album> ParseGetGalsRequest (const QDomDocument& document)
		{
			QList<Album> albums;
			const auto& list = document.elementsByTagName ("Gal");
			for (int i = 0, size = list.size (); i < size; ++i)
			{
				Album album;
				const auto& fieldsList = list.at (0).childNodes ();
				for (int j = 0, sz = fieldsList.size (); j < sz; ++j)
				{
					const auto& fieldElem = fieldsList.at (j).toElement ();
					if (fieldElem.tagName () == "Name")
						album.Title_ = fieldElem.text ();
					else if (fieldElem.tagName () == "Date")
						album.CreationDate_ = QDateTime::fromString (fieldElem.text (),
								"yyyy-dd-mm hh:MM:ss");
					else if (fieldElem.tagName () == "URL")
						album.Url_ = QUrl (fieldElem.text ());
				}
				albums << album;
			}

			return albums;
		}

		QList<Photo> ParseGetPicsRequest (const QDomDocument& document)
		{
			QList<Photo> photos;
			const auto& list = document.elementsByTagName ("Pic");
			for (int i = 0, size = list.size (); i < size; ++i)
			{
				Photo photo;
				const auto& picNode = list.at (i);
				photo.ID_ = picNode.toElement ().attribute ("id").toUtf8 ();
				const auto& fieldsList = picNode.childNodes ();
				for (int j = 0, sz = fieldsList.size (); j < sz; ++j)
				{
					const auto& fieldElem = fieldsList.at (j).toElement ();
					if (fieldElem.tagName () == "Bytes")
						photo.Size_ = fieldElem.text ().toULongLong ();
					else if (fieldElem.tagName () == "Format")
						photo.Format_ = fieldElem.text ();
					else if (fieldElem.tagName () == "Height")
						photo.Height_ = fieldElem.text ().toInt ();
					else if (fieldElem.tagName () == "MD5")
						photo.MD5_ = fieldElem.text ().toUtf8 ();
					else if (fieldElem.tagName () == "Meta")
					{
						if (fieldElem.attribute ("name") == "title")
							photo.Title_ = fieldElem.text ();
						else if (fieldElem.attribute ("name") == "description")
							photo.Description_ = fieldElem.text ();
						else if (fieldElem.attribute ("name") == "filename")
							photo.OriginalFileName_ = fieldElem.text ();
					}
					else if (fieldElem.tagName () == "URL")
						photo.Url_ = QUrl (fieldElem.text ());
					else if (fieldElem.tagName () == "Width")
						photo.Width_ = fieldElem.text ().toInt ();
				}
				Thumbnail small;
				small.Url_ = photo.Url_.toString ().replace ("original", SmallSize);
				small.Height_ = SmallSize.toInt ();
				small.Width_ = SmallSize.toInt ();
				Thumbnail medium;
				medium.Url_ = photo.Url_.toString ().replace ("original", MediumSize);
				medium.Height_ = MediumSize.toInt ();
				medium.Width_ = MediumSize.toInt ();
				photo.Thumbnails_ << small << medium;
				photos << photo;
			}

			return photos;
		}
	}

	void FotoBilderAccount::handleLoginRequestFinished ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (FotoBilderErrorExists (content))
			return;

		Quota_ = ParseLoginResponse (document);
	}

	void FotoBilderAccount::handleNetworkError (QNetworkReply::NetworkError err)
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();
		qWarning () << Q_FUNC_INFO
				<< err
				<< reply->errorString ();
		emit networkError (err, reply->errorString ());
	}

	void FotoBilderAccount::handleGotAlbums ()
	{
		QDomDocument document;
		QByteArray content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (FotoBilderErrorExists (content))
			return;

		if (auto rc = CollectionsModel_->rowCount ())
			CollectionsModel_->removeRows (0, rc);
		CollectionsModel_->setHorizontalHeaderLabels ({ tr ("Name") });

		AllPhotosItem_ = new QStandardItem (tr ("All photos"));
		AllPhotosItem_->setData (ItemType::AllPhotos, CollectionRole::Type);
		AllPhotosItem_->setEditable (false);
		CollectionsModel_->appendRow (AllPhotosItem_);

		const auto& albums = ParseGetGalsRequest (document);
		for (const auto& album : albums)
		{
			auto item = new QStandardItem (album.Title_);
			item->setData (ItemType::Collection, CollectionRole::Type);
			item->setEditable (false);
			CollectionsModel_->appendRow (item);
		}
	}

	void FotoBilderAccount::handleGotPhotos ()
	{
		QDomDocument document;
		auto content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (FotoBilderErrorExists (content))
			return;

		for (const auto& photo : ParseGetPicsRequest (document))
		{
			auto mkItem = [&photo] () -> QStandardItem*
			{
				const auto& name = photo.Title_.isEmpty () ?
					photo.OriginalFileName_ :
					photo.Title_;
				auto item = new QStandardItem (name);
				item->setEditable (false);
				item->setData (ItemType::Image, CollectionRole::Type);
				item->setData (photo.ID_, CollectionRole::ID);
				item->setData (name , CollectionRole::Name);

				item->setData (photo.Url_, CollectionRole::Original);
				item->setData (QSize (photo.Width_, photo.Height_),
						CollectionRole::OriginalSize);
				if (!photo.Thumbnails_.isEmpty ())
				{
					auto first = photo.Thumbnails_.first ();
					auto last = photo.Thumbnails_.last ();
					item->setData (first.Url_, CollectionRole::SmallThumb);
					item->setData (QSize (first.Width_, first.Height_), CollectionRole::SmallThumbSize);
					item->setData (last.Url_, CollectionRole::MediumThumb);
					item->setData (QSize (last.Width_, last.Height_), CollectionRole::MediumThumb);
				}
				return item;
			};

			AllPhotosItem_->appendRow (mkItem ());
		}
		emit doneUpdating ();
	}
}
}
}
