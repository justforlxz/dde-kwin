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


#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <KF5/KWindowSystem/KWindowSystem>
#include <QAbstractListModel>
#include "multitasking.h"
#include "windowinfo.h"

class MultitaskingModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum DataRoles{
        WindowThumbnailRole = Qt::UserRole + 1,
        WindowTitleRole     = Qt::UserRole + 2,
        WindowIconRole      = Qt::UserRole + 3
    };

    explicit MultitaskingModel(QObject *parent = nullptr);
    ~MultitaskingModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;


    void setWindowInfoList(const QList<WindowInfo> &windowInfoList);
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE int numScreens() const;
    Q_INVOKABLE QRect screenGeometry(int screen) const;

    Q_INVOKABLE QPixmap getWindowIcon( QVariant winId );
    bool isCurrentScreensEmpty();

protected:
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<WindowInfo> m_windowInfoList;
};

#endif // DATAMODEL_H
