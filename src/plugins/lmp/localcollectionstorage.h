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
#include <QHash>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "mediainfo.h"
#include "interfaces/lmp/collectiontypes.h"

namespace LeechCraft
{
namespace LMP
{
	class LocalCollectionStorage : public QObject
	{
		Q_OBJECT

		QHash<QString, int> PresentArtists_;
		QHash<QString, int> PresentAlbums_;

		QSqlDatabase DB_;

		QSqlQuery GetArtists_;
		QSqlQuery GetAlbums_;
		QSqlQuery GetAllTracks_;

		QSqlQuery AddArtist_;
		QSqlQuery AddAlbum_;
		QSqlQuery LinkArtistAlbum_;
		QSqlQuery AddTrack_;
		QSqlQuery AddGenre_;

		QSqlQuery RemoveTrack_;
		QSqlQuery RemoveAlbum_;
		QSqlQuery RemoveArtist_;

		QSqlQuery SetAlbumArt_;

		QSqlQuery GetTrackStats_;
		QSqlQuery SetTrackStats_;
		QSqlQuery UpdateTrackStats_;

		QSqlQuery GetFileMTime_;
		QSqlQuery SetFileMTime_;

		// 1 is loved, 2 is banned
		QSqlQuery GetLovedBanned_;
		QSqlQuery SetLovedBanned_;
		QSqlQuery RemoveLovedBanned_;
	public:
		struct LoadResult
		{
			Collection::Artists_t Artists_;

			QHash<QString, int> PresentArtists_;
			QHash<QString, int> PresentAlbums_;
		};

		LocalCollectionStorage (QObject* = 0);

		void Clear ();
		Collection::Artists_t AddToCollection (const QList<MediaInfo>&);
		LoadResult Load ();
		void Load (const LoadResult&);

		QStringList GetTracksPaths ();

		void RemoveTrack (int);
		void RemoveAlbum (int);
		void RemoveArtist (int);

		void SetAlbumArt (int, const QString&);

		Collection::TrackStats GetTrackStats (int);
		void SetTrackStats (const Collection::TrackStats&);
		void RecordTrackPlayed (int);

		QDateTime GetMTime (const QString&);
		void SetMTime (const QString&, const QDateTime&);

		void SetTrackLoved (int);
		void SetTrackBanned (int);
		void ClearTrackLovedBanned (int);
		QList<int> GetLovedTracks ();
		QList<int> GetBannedTracks ();
	private:
		void MarkLovedBanned (int, int);
		QList<int> GetLovedBanned (int);

		Collection::Artists_t GetAllArtists ();
		QHash<int, Collection::Album_ptr> GetAllAlbums ();

		void AddArtist (Collection::Artist&);
		void AddAlbum (const Collection::Artist&, Collection::Album&);
		void AddTrack (Collection::Track&, int, int);

		void AddToPresent (const Collection::Artist&);
		bool IsPresent (const Collection::Artist&, int&) const;
		void AddToPresent (const Collection::Artist&, const Collection::Album&);
		bool IsPresent (const Collection::Artist&, const Collection::Album&, int&) const;

		void PrepareQueries ();
		void CreateTables ();
	};
}
}
