/*
 * Copyright (C) 2020 Uniontech Technology Co., Ltd.
 *
 * Author:     Lei Su <sulei@uniontech.com>
 *
 * Maintainer: Lei Su <sulei@uniontech.com>
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

#include "windowinfo.h"

WindowInfo::WindowInfo(QObject *parent)
    : QObject(parent)
{

}

WindowInfo::WindowInfo(const WindowInfo &other)
{
    operator=(other);
}

WindowInfo &WindowInfo::operator=(const WindowInfo &other)
{
    if (this != &other) {
        m_windowId    = other.m_windowId;
        m_windowTitle = other.m_windowTitle;
        m_windowIcon  = other.m_windowIcon;
    }

    return (*this);
}

void WindowInfo::setWindowId(const QVariant &winId)
{
    if (m_windowId != winId) {
        m_windowId = winId;

        emit windowIdChanged(m_windowId);
    }
}

QVariant WindowInfo::windowId() const
{
    return m_windowId;
}

void WindowInfo::setWindowTitle(const QString &windowTitle)
{
    if (m_windowTitle != windowTitle) {
        m_windowTitle = windowTitle;

        emit windowTitleChanged(m_windowTitle);
    }
}

QString WindowInfo::windowTitle() const
{
    return m_windowTitle;
}

QString WindowInfo::windowIcon() const
{
    return QString("image://imageProvider/%1").arg(m_windowId.toULongLong());
}

QString WindowInfo::windowThumbnailImage() const
{
    return QString("image://windowthumbnailimageprovider/%1").arg(m_windowId.toULongLong());
}
