/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "util.h"
#include <QSize>
#include <QApplication>
#include <QDesktopWidget>
#include <QtDebug>

namespace LeechCraft
{
namespace Util
{
	QPoint FitRectScreen (QPoint pos, const QSize& size, FitFlags flags, const QPoint& shiftAdd)
	{
		const QRect& geometry = QApplication::desktop ()->screenGeometry (pos);

		int xDiff = std::max (0, pos.x () + size.width () - (geometry.width () + geometry.x ()));
		int yDiff = std::max (0, pos.y () + size.height () - (geometry.height () + geometry.y ()));

		if (flags & FitFlag::NoOverlap)
		{
			auto overlapFixer = [] (int& diff, int dim)
			{
				if (diff > 0)
					diff = dim > diff ? dim : diff;
			};

			const QRect newRect (pos - QPoint (xDiff, yDiff), size);
			if (newRect.contains (pos))
			{
				overlapFixer (xDiff, size.width ());
				overlapFixer (yDiff, size.height ());
			}
		}

		if (xDiff > 0)
			pos.rx () -= xDiff + shiftAdd.x ();
		if (yDiff > 0)
			pos.ry () -= yDiff + shiftAdd.y ();

		return pos;
	}
}
}