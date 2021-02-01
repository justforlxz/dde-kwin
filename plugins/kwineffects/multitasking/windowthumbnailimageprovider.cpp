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

WindowThumbnailImageProvider::WindowThumbnailImageProvider(QQmlImageProviderBase::ImageType type, QQmlImageProviderBase::Flags flags)
    : QQuickImageProvider(type, flags)
{

}

QPixmap WindowThumbnailImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size);

    bool ok = false;
    qulonglong winId = id.toULongLong(&ok);
    if (!ok) {
        return QPixmap();
    }

    EffectWindow *window = effects->findWindow(winId);
    if (window) {
        QPixmap pixmap;
        xcb_connection_t *pConnection = xcb_connect(NULL, NULL);
        xcb_generic_error_t *pErr = NULL;
        const xcb_setup_t *pSetup = xcb_get_setup(pConnection);
        xcb_screen_iterator_t screenIter = xcb_setup_roots_iterator(pSetup);
        xcb_screen_t *pScreen = screenIter.data;

        xcb_composite_redirect_window(pConnection, winId, XCB_COMPOSITE_REDIRECT_AUTOMATIC);
        int nHeight, nWidth, nDepth;

        xcb_get_geometry_cookie_t getGeomeryCookie = xcb_get_geometry(pConnection, winId);
        xcb_get_geometry_reply_t *getGeomeryReply = xcb_get_geometry_reply(pConnection, getGeomeryCookie, &pErr);
        if (getGeomeryReply) {
            nWidth = getGeomeryReply->width;
            nHeight = getGeomeryReply->height;
            nDepth = getGeomeryReply->depth;
            free(getGeomeryReply);
        }

        xcb_pixmap_t windowPixmap = xcb_generate_id(pConnection);
        xcb_composite_name_window_pixmap(pConnection, winId, windowPixmap);

        xcb_get_image_cookie_t getImageCookie = xcb_get_image(pConnection, XCB_IMAGE_FORMAT_Z_PIXMAP, windowPixmap, 0, 0, nWidth, nHeight, (uint32_t)(~0UL));
        xcb_get_image_reply_t *pGetImageReply = xcb_get_image_reply(pConnection, getImageCookie, &pErr);

        QImage image(xcb_get_image_data(pGetImageReply), nWidth, nHeight, QImage::Format_ARGB32);
        pixmap = QPixmap::fromImage(image.scaled(QSize(nWidth, nHeight), Qt::KeepAspectRatio));
        free(pGetImageReply);

        return pixmap;
    }

    return QPixmap();
}
