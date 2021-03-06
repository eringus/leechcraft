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

#include <QStandardItemModel>
#include <QVector>
#include <QFontMetrics>

struct libvlc_media_list_t;
struct libvlc_media_t;
struct libvlc_instance_t;

class QMimeData;

namespace LeechCraft
{
namespace vlc
{
	class PlaylistWidget;
	
	class PlaylistModel : public QStandardItemModel
	{
		Q_OBJECT
		
		PlaylistWidget *Parent_;
		libvlc_media_list_t *Playlist_;
		QVector<QStandardItem*> Items_;
		libvlc_instance_t *Instance_;
	
	public:
		explicit PlaylistModel (PlaylistWidget *parent, libvlc_media_list_t *playlist, libvlc_instance_t *instance);
		~PlaylistModel ();
		
		bool dropMimeData (const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
		QStringList mimeTypes () const;
		QMimeData* mimeData (const QModelIndexList&) const;
		Qt::DropActions supportedDropActions () const;
		
		void AddUrl (const QUrl&);
		libvlc_media_t* Take (const QUrl&);
		
		int Width_;
		QFontMetrics FontMetrics_;
		
		
	private:
		QString ShrinkText (const QString&, const QString&);
		
	public slots:
		void updateTable ();
	};
}
}
