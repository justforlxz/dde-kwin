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
        cellWidth: view.width * 0.4
        cellHeight: view.height * 0.5

        delegate: Item {
            id: item
            anchors.margins: 10

            DeepinWindowThumbnail {
                id: windowThumbnail
                winId: WindowThumbnailRole
                winTitle: WindowTitleRole
                winIcon: WindowIconRole
                x: 10
                y: view.cellHeight * 0.1
                width: view.cellWidth * 0.8
                height: view.cellHeight * 0.8

                ParallelAnimation {
                    id: goBack
                    YAnimator {
                        target: windowThumbnail
                        to: view.cellHeight * 0.1
                        duration: 500
                        easing.type: Easing.OutBounce
                    }
                }

                ParallelAnimation {
                    id: goClose
                    YAnimator {
                        target: windowThumbnail
                        from: windowThumbnail.y
                        to: -view.height
                        duration: 500
                        easing.type: Easing.Linear
                    }
                    onStopped: {
                        qmlRequestCloseWindow(windowThumbnail.winId)
                    }
                }

                function goBackAnimation() {
                    goBack.start()
                }

                function goCloseAnimation() {
                    goClose.start()
                }

                MouseArea {
                    anchors.fill: windowThumbnail
                    property point clickPos
                    drag.target: windowThumbnail
                    drag.axis: Drag.YAxis

                    onPressed: {
                        clickPos = Qt.point(mouse.x, mouse.y)
                    }
                    onPositionChanged: {
                        var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
                        var y = windowThumbnail.y
                        windowThumbnail.y = y + delta.y
                    }
                    onReleased: {
                        if (windowThumbnail.y < -(view.cellHeight * 0.3))
                            windowThumbnail.goCloseAnimation()
                        else
                            windowThumbnail.goBackAnimation()
                    }

                    onClicked: {
                        qmlRequestSwitchWindow(windowThumbnail.winId)
                    }
                }

            }

        }
    }
}


