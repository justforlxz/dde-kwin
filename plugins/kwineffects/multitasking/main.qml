import QtQuick 2.0
import org.kde.plasma 2.0 as PlasmaCore
import QtQuick.Window 2.11

Rectangle {
    id: root
    x: 0
    y: 0
    width: Screen.desktopAvailableWidth
    height: Screen.desktopAvailableHeight
    color: "transparent"

    function log(msg) {
        manager.debugLog(msg)
    }

    //sent signal to c++
    signal qmlRequestCloseWindow(var window)

    DeepinDesktopThumbnail {
        anchors.fill: root
        model: multitaskingModel
        color: "transparent"
    }
}

