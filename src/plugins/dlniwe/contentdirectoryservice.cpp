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

#include "contentdirectoryservice.h"
#include <QFile>
#include <QtDebug>
#include <HUpnpCore/HActionArguments>

namespace LeechCraft
{
namespace DLNiwe
{
	namespace HU = Herqq::Upnp;

	ContentDirectoryService::ContentDirectoryService ()
	{
	}

	qint32 ContentDirectoryService::GetSystemUpdateID (const HU::HActionArguments& inArgs, HU::HActionArguments *outArgs)
	{
		qDebug () << Q_FUNC_INFO;
		outArgs->setValue ("Id", QString::number (SystemUpdateID_));
		return HU::UpnpSuccess;
	}

	qint32 ContentDirectoryService::GetSearchCapabilities (const HU::HActionArguments& inArgs, HU::HActionArguments *outArgs)
	{
		qDebug () << Q_FUNC_INFO;
		outArgs->setValue ("SearchCaps", "dc:title,dc:description,av:keyword,upnp:genre,upnp:album");
		return HU::UpnpSuccess;
	}

	qint32 ContentDirectoryService::GetSortCapabilities (const HU::HActionArguments& inArgs, HU::HActionArguments *outArgs)
	{
		qDebug () << Q_FUNC_INFO;
		outArgs->setValue ("SortCaps", "dc:title,dc:description,av:keyword,upnp:genre,upnp:album");
		return HU::UpnpSuccess;
	}

	qint32 ContentDirectoryService::Browse (const HU::HActionArguments& inArgs, HU::HActionArguments *outArgs)
	{
		qDebug () << Q_FUNC_INFO;
		return HU::UpnpSuccess;
	}

	qint32 ContentDirectoryService::X_GetFeatureList (const HU::HActionArguments& inArgs, HU::HActionArguments *outArgs)
	{
		qDebug () << Q_FUNC_INFO;
		QFile file (":/dlniwe/resources/desc/featurelist.xml");
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open featureslist file"
					<< file.errorString ();

			return HU::UpnpUndefinedFailure;
		}

		outArgs->setValue ("FeatureList", file.readAll ());
		return HU::UpnpSuccess;
	}

	qint32 ContentDirectoryService::X_SetBookmark (const HU::HActionArguments& inArgs, HU::HActionArguments *outArgs)
	{
		qDebug () << Q_FUNC_INFO;
		return HU::UpnpSuccess;
	}
}
}