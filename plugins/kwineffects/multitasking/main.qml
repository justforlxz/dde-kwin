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

    DeepinDesktopThumbnail {
        width: 800
        height: 600
        model: multitaskingModel
    }
}

