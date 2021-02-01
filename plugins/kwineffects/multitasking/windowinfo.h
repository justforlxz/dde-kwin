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


#ifndef _WINDOW_INFO_H_
#define _WINDOW_INFO_H_

#include <QIcon>
#include <QObject>
#include <QVariant>

class WindowInfo : public QObject
{
    Q_OBJECT
public:
    explicit WindowInfo(QObject *prarent = nullptr);
    WindowInfo(const WindowInfo &other);
    WindowInfo &operator= (const WindowInfo &other);

    void setWindowId(const QVariant &winId);
    QVariant windowId() const;

    void setWindowTitle(const QString &windowTitle);
    QString windowTitle() const;

    void setWindowIcon(const QString &windowIcon);
    QString windowIcon() const;

    QString windowThumbnailImage() const;

signals:
    void windowIdChanged(QVariant winId);
    void windowTitleChanged(QString windowTitle);
    void windowIconChanged(QIcon windowIcon);

private:
    QVariant m_windowId;
    QString  m_windowTitle;
    QString  m_windowIcon;
};

Q_DECLARE_METATYPE(WindowInfo)

#endif //_WINDOW_INFO_H_
