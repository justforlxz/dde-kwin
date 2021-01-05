/*
 * Copyright (C) 2021 UnionTech Software Technology Co., Ltd.
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


import QtQuick 2.11
import QtQuick.Window 2.11
import org.kde.plasma 2.0 as PlasmaCore

Rectangle {
    id: root
    property alias winTitle: windowTitle.text
    property alias winIcon: windowIcon.source
    property alias winId: windowThumbnail.winId
    color: "transparent"

    Column{
        Row {
            Image {
                id: windowIcon
                width: 50
                height: 50
            }

            Text {
                id: windowTitle
                x: windowIcon.width
                width: root.width - windowIcon.width
                elide: Text.ElideRight
            }
        }

        Row {
            PlasmaCore.WindowThumbnail {
                id: windowThumbnail
                width: root.width
                height: root.height - windowIcon.height
            }
        }
    }
}
