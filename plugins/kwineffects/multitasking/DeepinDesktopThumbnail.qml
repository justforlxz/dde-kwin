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
    property int pressX : 0
    property int pressY : 0
    property int screenWidth : Screen.desktopAvailableWidth
    property int screenHeight : Screen.desktopAvailableHeight

    GridView {
        id: view
	anchors.centerIn: parent
	width: screenWidth * 0.92
	height: screenHeight
        layoutDirection : Qt.RightToLeft
        flow: GridView.FlowTopToBottom

        cellWidth: {
            if (count === 2)
                return screenWidth * 0.47
            else if (count === 1)
                return screenWidth * 0.8
            else
                return screenWidth * 0.4
        }
        cellHeight: {
            if (count === 2)
                return screenHeight * 0.47 + 70
            else if (count === 1)
                return screenHeight * 0.8 + 70
            else
                return screenHeight * 0.4 + 70
        }

        removeDisplaced: Transition {
            NumberAnimation { properties: "x, y"; duration: 600; easing.type: Easing.Linear }
        }

        MouseArea {
            anchors.fill: parent
            z: -1
            propagateComposedEvents: true

            onPressed: {
                pressX = mouseX
                pressY = mouseY
            }

            onReleased: {
                if (Math.abs(pressX - mouseX) <= 10 && Math.abs(pressY - mouseY) <= 10) {
                    if (view.count > 0)
                        closeMultiTask()
                    else
                        qmlCloseMultitask()
                }
            }
        }

        onCountChanged: {
            if (count === 0)
                qmlCloseMultitask()
        }

        delegate: Item {
            id: item
            width: view.cellWidth
            height: view.cellHeight
            anchors.margins: 10
            property bool taskEnter: view.model.isEnterEffect()
            property bool pressAndHold: false
            property int column: view.model.columnAt(index)
            property int offScreenX: -(root.width + column * view.cellWidth)
	    property int inScreenX: {
		if (view.count === 1)
			return 10
		else
			return view.cellWidth * 0.1
	    }



            DeepinWindowThumbnail {
                id: windowThumbnail
                winId: WindowThumbnailRole
                winTitle: WindowTitleRole
                winIcon: WindowIconRole
                winImage: WindowThumbnailImageRole

                Component.onCompleted: {
                    if (taskEnter)
                        windowThumbnail.state = "taskEnter"
                    else
                        windowThumbnail.state = ""
                }

                x: inScreenX
                y: {
                    if (view.count === 2)
                        return view.cellHeight * 0.5
                    else if (view.count === 1)
                        return view.cellHeight * 0.15
                    else
                        return view.cellHeight * 0.2
                }

                width: view.cellWidth * 0.8
                height: view.cellHeight * 0.8

                ParallelAnimation {
                    id: goBack
                    YAnimator {
                        target: windowThumbnail
                        from: windowThumbnail.y
                        to: {
                            if (view.count === 2)
                                return view.cellHeight * 0.5
                            else if (view.count === 1)
                                return view.cellHeight * 0.15
                            else
                                return view.cellHeight * 0.2
                        }
                        duration: 400
                        easing.type: Easing.OutCubic
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
                        qmlRequestCloseWindow(windowThumbnail.winId, index)
                    }
                }

                state: "taskLeave"

                states: [
                    State {
                        name: "taskEnter"
                        PropertyChanges { target: windowThumbnail; x: inScreenX }
                    },
                    State {
                        name: "taskLeave"
                        PropertyChanges { target: windowThumbnail; x: offScreenX }
                    },
                    State {
                        when: item.pressAndHold
                        name: "enlargeSize"
                        PropertyChanges { target: windowThumbnail; scale: 1.05 }
                    }
                ]

                transitions: [
                    Transition {
                        from: "taskLeave"; to: "taskEnter"
                        XAnimator { target: windowThumbnail; duration: (800 + column * 100); easing.type: Easing.OutCubic }
                    },
                    Transition {
                        to: "taskLeave"
                        XAnimator { target: windowThumbnail; duration: 500; easing.type: Easing.Linear }

                        onRunningChanged: {
                            if(!running && index === 0 )
                                qmlCloseMultitask()
                        }
                    },
                    Transition {
                        to: "enlargeSize"
                        reversible: true
                        ScaleAnimator { target: windowThumbnail; duration: 400; easing.type: Easing.Linear }
                    }
                ]

                Connections {
                    target: root

                    onCloseMultiTask: {
                        windowThumbnail.state = "taskLeave"
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
                    property var dateT
                    drag.target: windowThumbnail
                    drag.axis: Drag.YAxis
                    drag.maximumY: {
                        if (view.count === 2)
                            return view.cellHeight * 2 / 3
                        else
                            return view.cellHeight / 3
                    }

                    onPressed: {
                        clickPos = Qt.point(mouse.x, mouse.y)
                        var currentDate = new Date()
                        dateT = currentDate.getTime()
                    }

                    onReleased: {
                        item.pressAndHold = false
                        var delta = windowThumbnail.y + (mouse.y - clickPos.y)
                        var currentDateE = new Date()
                        var t = currentDateE.getTime()
                        if ((t - dateT) < 200) {
                            if ((view.count >= 3) && delta < (view.cellHeight * 0.08))
                                windowThumbnail.goCloseAnimation()
                            else if (view.count === 2 && delta < (view.cellHeight * 0.42))
                                windowThumbnail.goCloseAnimation()
                            else if (view.count === 1 && delta < (view.cellHeight * 0.11))
                                windowThumbnail.goCloseAnimation()
                            else
                                windowThumbnail.goBackAnimation()

                        }
                        else {
                            if (view.count >= 3 || view.count === 1) {
                                if (delta < -(view.cellHeight * 0.25))
                                    windowThumbnail.goCloseAnimation()
                                else
                                    windowThumbnail.goBackAnimation()
                            }
                            else {
                                if (delta < -(view.cellHeight * 0.07))
                                    windowThumbnail.goCloseAnimation()
                                else
                                    windowThumbnail.goBackAnimation()
                            }
                        }
                    }

                    onPressAndHold: {
                        item.pressAndHold = true
                    }

                    onClicked: {
                        var point = windowThumbnail.mapToItem(root, 0, 0)
                        qmlRequestSwitchWindow(windowThumbnail.winId, point.x, point.y,
                                               windowThumbnail.width, windowThumbnail.height)
                    }
                }
            }
        }
    }
}


