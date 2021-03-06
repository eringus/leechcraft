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
#include <interfaces/media/iartistbiofetcher.h>

class QStandardItemModel;

namespace LeechCraft
{
namespace LMP
{
	class BioPropProxy : public QObject
	{
		Q_OBJECT

		Q_PROPERTY (QString artistName READ GetArtistName NOTIFY artistNameChanged)
		Q_PROPERTY (QUrl artistImageURL READ GetArtistImageURL NOTIFY artistImageURLChanged)
		Q_PROPERTY (QUrl artistBigImageURL READ GetArtistBigImageURL NOTIFY artistBigImageURLChanged)
		Q_PROPERTY (QString artistTags READ GetArtistTags NOTIFY artistTagsChanged)
		Q_PROPERTY (QString artistInfo READ GetArtistInfo NOTIFY artistInfoChanged)
		Q_PROPERTY (QUrl artistPageURL READ GetArtistPageURL NOTIFY artistPageURLChanged)
		Q_PROPERTY (QObject* artistImagesModel READ GetArtistImagesModel NOTIFY artistImagesModelChanged)

		Media::ArtistBio Bio_;

		QString CachedTags_;
		QString CachedInfo_;

		QStandardItemModel *ArtistImages_;
	public:
		BioPropProxy (QObject* = 0);

		void SetBio (const Media::ArtistBio&);

		QString GetArtistName () const;
		QUrl GetArtistImageURL () const;
		QUrl GetArtistBigImageURL () const;
		QString GetArtistTags () const;
		QString GetArtistInfo () const;
		QUrl GetArtistPageURL () const;
		QObject* GetArtistImagesModel () const;
	signals:
		void artistNameChanged (const QString&);
		void artistImageURLChanged (const QUrl&);
		void artistBigImageURLChanged (const QUrl&);
		void artistTagsChanged (const QString&);
		void artistInfoChanged (const QString&);
		void artistPageURLChanged (const QUrl&);
		void artistImagesModelChanged (QObject*);
	};
}
}
