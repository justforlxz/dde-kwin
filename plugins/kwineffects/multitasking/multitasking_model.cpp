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
    return m_windows.count();
}

QVariant MultitaskingModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= m_windows.count()) {
        return QVariant();
    }

    switch (role) {
    case ThumbnailRole:
        return m_windows[row];
    default:
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> MultitaskingModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ThumbnailRole] = "ThumbnailRole";

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


void MultitaskingModel::setWindows(const QVariantList &windows)
{
    int index = rowCount();
    emit beginInsertRows(QModelIndex(), index, index);
    for (const QVariant &win : windows) {
        m_windows.append(win);
    }
    emit endInsertRows();
}


void MultitaskingModel::remove(int index)
{
    if (index < 0 || index >= rowCount()) {
        return;
    }

    if (rowCount() == 1)
    {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_windows.removeAt(index);
    endRemoveRows();
}

QPixmap MultitaskingModel::getWindowIcon( QVariant winId )
{
    EffectWindow *ew = effects->findWindow(winId.toULongLong());
    if (ew) {
        return ew->icon().pixmap(QSize(Constants::ICON_SIZE, Constants::ICON_SIZE));
    }

    return QPixmap();
}
