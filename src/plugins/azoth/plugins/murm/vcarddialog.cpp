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

#include "vcarddialog.h"
#include <util/util.h>
#include <interfaces/core/ientitymanager.h>
#include "structures.h"
#include "photostorage.h"
#include "georesolver.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Murm
{
	VCardDialog::VCardDialog (const UserInfo& info, PhotoStorage *storage,
			GeoResolver *geo, ICoreProxy_ptr proxy, QWidget *parent)
	: QDialog (parent)
	, Proxy_ (proxy)
	, Info_ (info)
	, Storage_ (storage)
	{
		Ui_.setupUi (this);
		setAttribute (Qt::WA_DeleteOnClose);

		Ui_.FirstName_->setText (info.FirstName_);
		Ui_.LastName_->setText (info.LastName_);
		Ui_.Nickname_->setText (info.Nick_);

		Ui_.Birthday_->setDate (info.Birthday_);
		Ui_.Birthday_->setDisplayFormat (info.Birthday_.year () != 1800 ? "dd MMMM yyyy" : "dd MMMM");

		if (info.Gender_)
			Ui_.Gender_->setText (info.Gender_ == 1 ? tr ("female") : tr ("male"));

		Ui_.HomePhone_->setText (info.HomePhone_);
		Ui_.MobilePhone_->setText (info.MobilePhone_);

		auto timezoneText = QString::number (info.Timezone_) + " GMT";
		if (info.Timezone_ > 0)
			timezoneText.prepend ('+');
		Ui_.Timezone_->setText (timezoneText);

		if (info.Country_ > 0)
			geo->GetCountry (info.Country_,
					[this] (const QString& country) { Ui_.Country_->setText (country); });
		if (info.City_ > 0)
			geo->GetCity (info.City_,
					[this] (const QString& country) { Ui_.City_->setText (country); });

		if (!info.BigPhoto_.isValid ())
			return;
		const auto& image = storage->GetImage (info.BigPhoto_);
		if (image.isNull ())
			connect (storage,
					SIGNAL (gotImage (QUrl)),
					this,
					SLOT (handleImage (QUrl)));
		else
			Ui_.PhotoLabel_->setPixmap (QPixmap::fromImage (image)
					.scaled (Ui_.PhotoLabel_->size (),
							Qt::KeepAspectRatio,
							Qt::SmoothTransformation));
	}

	void VCardDialog::handleImage (const QUrl& url)
	{
		if (url != Info_.BigPhoto_)
			return;

		const auto& image = Storage_->GetImage (url);
		Ui_.PhotoLabel_->setPixmap (QPixmap::fromImage (image)
				.scaled (Ui_.PhotoLabel_->size (),
						Qt::KeepAspectRatio,
						Qt::SmoothTransformation));
	}

	void VCardDialog::on_OpenVKPage__released ()
	{
		const auto& pageUrlStr = "http://vk.com/id" + QString::number (Info_.ID_);

		const auto& e = Util::MakeEntity (QUrl (pageUrlStr),
				QString (), FromUserInitiated | OnlyHandle);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}
}
