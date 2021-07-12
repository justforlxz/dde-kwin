#include "backgroundimageprovider.h"
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

static XImage *pImage = nullptr;
BackgroundImageProvider::BackgroundImageProvider(QQmlImageProviderBase::ImageType type, QQmlImageProviderBase::Flags flags)
    : QQuickImageProvider(type, flags)
{
}

QImage BackgroundImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size);

    bool ok = false;
    qulonglong winId = id.toULongLong(&ok);
    if (!ok) {
        return QImage();
    }

    const auto &display = QX11Info::display();
    Window unusedWindow;
    int unusedInt;
    unsigned unusedUint, width, height;

    XGetGeometry(display, winId, &unusedWindow, &unusedInt, &unusedInt, &width, &height, &unusedUint, &unusedUint);
    if (pImage == nullptr)
    	pImage = XGetImage(display, winId, 0, 0, width, height, AllPlanes, ZPixmap);
    else
	pImage = XGetSubImage(display, winId, 0, 0, width, height, AllPlanes, ZPixmap, pImage, 0, 0);

    QImage image = QImage((const uchar *)(pImage->data), pImage->width, pImage->height, pImage->bytes_per_line, QImage::Format_RGB32);
    image = image.convertToFormat(QImage::Format_ARGB32);

    return image;
}
