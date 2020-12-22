import QtQuick 2.0
import org.kde.plasma 2.0 as PlasmaCore

Rectangle {
    id: root
    x: 0
    y: 0
    width: 1920
    height: 1080
    color: "gray"
    objectName: "root"

    function log(msg) {
        manager.debugLog(msg)
    }

    //sent signal to c++
    signal qmlRequestCloseWindow(var window)

    Component {
        id: desktopThumbnailView;
        GridView  {
            id: windowThumbnail
            layoutDirection : Qt.RightToLeft
            flow: GridView.FlowTopToBottom
            model: multitaskingModel
            cellHeight: 1920 / 3
            cellWidth: 1080 / 2

            delegate: Item {
                id: item
                width: 200 //parent.width/3;
                height: 200 //parent.height/2

                DeepinWindowThumbnail {
                    winId: ThumbnailRole
                    winTitle: "hello"
                    //winIcon:
                    anchors.fill: item
                }
            }
        }
    }

    Loader {
        id: view
        sourceComponent: desktopThumbnailView
        x: root.x
        y: root.y
        width: root.width
        height: root.height
    }
}

