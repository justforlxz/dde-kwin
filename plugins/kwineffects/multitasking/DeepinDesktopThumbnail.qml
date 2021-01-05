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

Rectangle {
    property alias model: view.model

    GridView {
        id: view
        anchors.fill: parent
        layoutDirection : Qt.RightToLeft
        flow: GridView.FlowTopToBottom
        cellWidth: 100
        cellHeight: 100

        delegate: Item {
            id: item
            width: 100
            height: 100
            DeepinWindowThumbnail {
                winId: ThumbnailRole
                winTitle: "hello"
                //winIcon:
                anchors.fill: item
            }
        }

        Connections {
            target: view;

            onCountChanged: {
                //在model长度发生变化时调整布局(调整每一个窗口的大小)
            }
        }
    }
}


