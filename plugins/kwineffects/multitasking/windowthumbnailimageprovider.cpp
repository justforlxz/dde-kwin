/*
 * Copyright (C) 2021 Uniontech Technology Co., Ltd.
 *
 * Author:     wangxinbo <wangxinbo@uniontech.com>
 *
 * Maintainer: wangxinbo <wangxinbo@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "windowthumbnailimageprovider.h"
#include "multitasking_model.h"
#include "kwinutils.h"

#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

WindowThumbnailImageProvider::WindowThumbnailImageProvider(QQmlImageProviderBase::ImageType type, QQmlImageProviderBase::Flags flags)
    : QQuickImageProvider(type, flags)
{

}

QImage WindowThumbnailImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size);
    Q_UNUSED(requestedSize);
    return KWinUtils::instance()->GetWindowThumbnailImage(id);
}
