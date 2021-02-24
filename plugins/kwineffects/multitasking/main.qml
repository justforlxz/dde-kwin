import QtQuick 2.0
import QtQuick.Window 2.11
import QtGraphicalEffects 1.0

Rectangle {
    id: root
    x: 0
    y: 0
    width: Screen.desktopAvailableWidth
    height: Screen.desktopAvailableHeight
    color: "transparent"
    property var desktopId: multitaskingModel.desktopId();

    Image {
        id: blur
        source: "image://BackgroundImageProvider/" + desktopId;
        fillMode:Image.PreserveAspectFit
    }

    FastBlur {
        anchors.fill: root
        source: blur
        radius: 70
    }

    //sent signal to c++
    signal qmlRequestCloseWindow(var window, int index)
    signal qmlRequestSwitchWindow(int winid, int ox, int oy, int width, int height)
    signal qmlCloseMultitask()
    signal closeMultiTask()

    DeepinDesktopThumbnail {
        anchors.fill: root
        model: multitaskingModel
        color: "transparent"
    }


}


