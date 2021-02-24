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

#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/composite.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <QX11Info>

WindowThumbnailImageProvider::WindowThumbnailImageProvider(QQmlImageProviderBase::ImageType type, QQmlImageProviderBase::Flags flags)
    : QQuickImageProvider(type, flags)
{

}

QImage WindowThumbnailImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size);

    bool ok = false;
    qulonglong winId = id.toULongLong(&ok);
    if (!ok) {
        return QImage();
    }

    EffectWindow *window = effects->findWindow(winId);
    if (!window) {
        return QImage();
    }

    const auto &display = QX11Info::display();
    Window unusedWindow;
    int unusedInt;
    unsigned unusedUint, width, height;

    XGetGeometry(display, winId, &unusedWindow, &unusedInt, &unusedInt, &width, &height, &unusedUint, &unusedUint);
    XImage *pImage = XGetImage(display, winId, 0, 0, width, height, AllPlanes, ZPixmap);

    QImage image = QImage((const uchar *)(pImage->data), pImage->width, pImage->height, pImage->bytes_per_line, QImage::Format_RGB32);
    image = image.convertToFormat(QImage::Format_ARGB32);

    return image;
}
