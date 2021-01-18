/*
 * Copyright (C) 2020 Uniontech Technology Co., Ltd.
 *
 * Author:     Lei Su <axylp@qq.com>
 *
 * Maintainer: Lei Su <axylp@qq.com>
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

#include <QDBusInterface>
#include <QDBusReply>
#include <QScreen>
#include <QtMath>

#include "multitasking_model.h"

MultitaskingModel::MultitaskingModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

MultitaskingModel::~MultitaskingModel()
{
}

int MultitaskingModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_windowInfoList.count();
}

QVariant MultitaskingModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= m_windowInfoList.count()) {
        return QVariant();
    }

    switch (role) {
    case WindowThumbnailRole:
        return m_windowInfoList.at(row).windowId();
    case WindowTitleRole:
        return m_windowInfoList.at(row).windowTitle();
    case WindowIconRole:
        return m_windowInfoList.at(row).windowIcon();
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> MultitaskingModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[WindowThumbnailRole] = "WindowThumbnailRole";
    roles[WindowTitleRole]     = "WindowTitleRole";
    roles[WindowIconRole]      = "WindowIconRole";
    return roles;
}

int MultitaskingModel::numScreens() const
{
    return effects->numScreens();
}

QRect MultitaskingModel::screenGeometry(int screen) const
{
    return effects->clientArea(FullScreenArea,screen,effects->currentDesktop());
}

int MultitaskingModel::columnAt(int index) const
{
    if ( index < 0)
        return 0;

    int count = m_windowInfoList.length();
    if (count > 2) {
       int mid = count - qFloor(count / 2.0);
       if (index > mid)
           index = index - mid;
    }

    return index;
}

void MultitaskingModel::setWindowInfoList(const QList<WindowInfo> &windowInfoList)
{
    int index = rowCount();
    emit beginInsertRows(QModelIndex(), index, index);
    m_windowInfoList = windowInfoList;
    emit endInsertRows();
}

void MultitaskingModel::closeWindow(QVariant windowId)
{
    for (int index = 0; index < m_windowInfoList.length(); ++index) {
        const WindowInfo &winInfo = m_windowInfoList.at(index);
        if (winInfo.windowId() == windowId) {

            beginRemoveRows(QModelIndex(), index, index);
            m_windowInfoList.removeAt(index);
            endRemoveRows();

            return;
        }
    }
}

QPixmap MultitaskingModel::getWindowIcon( QVariant winId )
{
    EffectWindow *ew = effects->findWindow(winId.toULongLong());
    if (ew) {
        return ew->icon().pixmap(QSize(Constants::ICON_SIZE, Constants::ICON_SIZE));
    }

    return QPixmap();
}
