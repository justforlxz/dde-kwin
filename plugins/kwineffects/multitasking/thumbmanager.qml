import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.2
import QtQuick.Window 2.0
import com.deepin.kwin 1.0
import QtGraphicalEffects 1.0
import org.kde.plasma 2.0 as PlasmaCore
import org.kde.kwin 2.0 as KWin

Rectangle {
    id: root
    x: 0
    y: 0
    width: manager.containerSize.width;
    height: manager.containerSize.height;
    color: "transparent"
    objectName: "root"

    function log(msg) {
        manager.debugLog(msg)
    }

    signal qmlRequestMove2Desktop(int screen, int desktop, var winId);
    signal resetModel();
    signal qmlCloseMultitask();
    signal qmlRemoveWindowThumbnail(int screen, int desktop, var winId);
    signal qmlForceResetDesktopModel();
    signal qmlUpdateDesktopThumBackground();
    signal qmlUpdateBackground();
    signal qmlRequestGetBackground(int desktop, int monitor,int width,int height);

    Component {
        id: desktopThumbnailView;
        Rectangle {
            Rectangle {
                id: mostBigbackgroundRect
                width: screenWidth;
                height: screenHeight;
                Image {
                    id: backgroundImage;
                    source: "image://BackgroundImageProvider/" + (currentIndex+1 + "/" + screenname);
                    cache : false
                }
                FastBlur {
                    anchors.fill: backgroundImage
                    source: backgroundImage
                    radius: 70
                }
                Connections {
                    target: root
                    onQmlUpdateBackground: {
                        backgroundImage.source = "";
                        backgroundImage.source = "image://BackgroundImageProvider/" + (multitaskingModel.currentIndex()+1 + "/" + screenname);
                    }
                }
            }

            property int desktopThumbnailItemWidth: screenWidth/7;
            property int desktopThumbnailItemHeight: screenHeight/6;
            id:wholeDesktopThumbnailView
            width: screenWidth;
            height: parent.height;
            color: "transparent"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    qmlCloseMultitask();
                }
                Accessible.role: Accessible.Button
                Accessible.name: "Ma_background_1" 
                Accessible.description: "background of whole desktopThumbnail area"
                Accessible.onPressAction: pressed()
            }

            //window thumbnail

            function setGridviewData() {
               if (multitaskingModel.getDesktopClientCount(currentScreen,multitaskingModel.currentIndex()+1) !== 0) {

                    grid.rows = multitaskingModel.getCalculateRowCount(currentScreen,multitaskingModel.currentIndex()+1)
                    grid.columns = multitaskingModel.getCalculateColumnsCount(currentScreen,multitaskingModel.currentIndex()+1);
                    windowThumbnail.model = multitaskingModel.windows(currentScreen, multitaskingModel.currentIndex()+1);

                    for (var i=0; i < windowThumbnail.count; i++) {

                        var scale = multitaskingModel.getWindowHeight(windowThumbnail.itemAt(i).winId)/multitaskingModel.getWindowWidth(windowThumbnail.itemAt(i).winId);
                        var calculationwidth= (screenWidth*5/7/grid.columns)*4/5;
                        var calculationheight = calculationwidth * scale;
                        var narrow = 0.8;
                        while (calculationheight > grid.height/grid.rows) {
                           calculationwidth = calculationwidth * narrow;
                           calculationheight = calculationheight * narrow;
                        }

                        windowThumbnail.itemAt(i).Layout.preferredWidth = calculationwidth;
                        windowThumbnail.itemAt(i).Layout.preferredHeight = calculationheight;
                    }
                } else {
                    grid.rows = multitaskingModel.getCalculateRowCount(currentScreen,multitaskingModel.currentIndex()+1)
                    grid.columns = multitaskingModel.getCalculateColumnsCount(currentScreen,multitaskingModel.currentIndex()+1);
                    windowThumbnail.model = multitaskingModel.windows(currentScreen, multitaskingModel.currentIndex()+1);
                }
                grid.update();
            }
            Rectangle{
                id: bigWindowThrumbContainer
                x: 0
                y: 0;//view.y + view.height;
                width: screenWidth  //  other area except grid  can receove
                height: screenHeight - 35;//screenHeight - view.height - 35;
                color:"transparent"

                property int curdesktop:1

                //zhd add for receive window thrumbnail
                DropArea { 
                    id: workspaceThumbnailDropArea
                    anchors.fill: parent
                    keys: ['DragwindowThumbnailitemdata','DraggingWindowAvatar']

                    onDropped: {
                        //console.log("bigWindowThrumbContainer droped");

                        var from = drop.source.desktop

                        if (from !== bigWindowThrumbContainer.curdesktop && bigWindowThrumbContainer.curdesktop!=null && drop.keys[0] === "DraggingWindowAvatar"){

                            console.log("DraggingWindow on big view  :Dropsource:" +drag.source.draggingdata +"  desktop index:" +  bigWindowThrumbContainer.curdesktop+ "  current screen: "+ currentScreen);
                            qmlRequestMove2Desktop(currentScreen,bigWindowThrumbContainer.curdesktop,drag.source.draggingdata);
                            setGridviewData();
                        }
                        if (drop.keys[0] === "DragwindowThumbnailitemdata") {
                            //console.log(currentScreen+"---------"+multitaskingModel.currentIndex()+"---------"+drag.source.draggingdata);
                            if (!multitaskingModel.isCurrentScreenWindows(currentScreen,multitaskingModel.currentIndex()+1, drag.source.draggingdata)) {
                                multitaskingModel.moveToScreen(currentScreen,multitaskingModel.currentIndex()+1, drag.source.draggingdata);
                                qmlRequestMove2Desktop(currentScreen,multitaskingModel.currentIndex()+1,drag.source.draggingdata);
                                setGridviewData();
                            }
                        }
                    }
                    onEntered: {
                        drag.accepted=true;
                        //console.log("bigWindowThrumbContainer enter");
                    }
                }
                //zhd add end

                MouseArea {
                    anchors.fill: parent

                    Accessible.role: Accessible.Button
                    Accessible.name: "Ma_background_2"
                    Accessible.description: "background of windows thumbnail area"
                    Accessible.onPressAction: pressed()

                    onClicked: {
                       qmlCloseMultitask();
                      // console.log("click to close multimask ")
                    }
                }
                GridLayout {
                    id:grid
                    width: screenWidth*5/7;
                    height: screenHeight - 35;//screenHeight - view.height-35;
                    anchors.centerIn: parent;
                    columns : multitaskingModel.getCalculateColumnsCount(currentScreen,multitaskingModel.currentIndex()+1);
                    Repeater {
                        id: windowThumbnail;
                        //model: multitaskingModel.windows(currentScreen)

                        Rectangle {
                            id:windowThumbnailitem
                            color: "transparent"
                            property bool isHightlighted: winId == multitaskingModel.currentWindowThumbnail;
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth:-1;
                            Layout.preferredHeight:-1;
                            property var winId: plasmaCoreWindowThumbnail.winId;
                            property var draggingdata: winId
                            property bool  dropreceived:false

                            Drag.keys:["DragwindowThumbnailitemdata", "PlusButton"];
                            Drag.active: false// windowThumbnailitemMousearea.drag.active
                            Drag.hotSpot {
                                x:0
                                y:0
                            }

                            PlasmaCore.WindowThumbnail {
                                id : plasmaCoreWindowThumbnail
                                winId: modelData
                                width: parent.width * 0.85
                                height: parent.height * 0.85
                                anchors.centerIn: parent

                                Rectangle {
                                    id:backgroundrect;
                                    width: parent.width;
                                    height: parent.height;
                                    border.color: "lightgray";
                                    border.width: 0;
                                    color: "transparent";
                                }
                            }

//                            Rectangle {
//                                id:backgroundrect;
//                                width: parent.width;
//                                height: parent.height;
//                                border.color: "lightgray";
//                                border.width: 0;
//                                color: "transparent";
//                            }
                          
                            MouseArea {
                                id:windowThumbnailitemMousearea
                                anchors.fill: parent //windowThumbnailitem
                                acceptedButtons: Qt.LeftButton| Qt.RightButton;
                                hoverEnabled: true;
                                property var pressedTime;

                                Accessible.role: Accessible.Button
                                Accessible.name: "Ma_winThumb_"+(multitaskingModel.currentIndex()+1) + "_" + currentScreen + "_" + winId
                                Accessible.description: "windowThumbnail_desktop_screen_winId"
                                Accessible.onPressAction: pressed()

                                property int originWidth
                                property int originHeight
                                property int originX
                                property int originY
                                property int originMouseX
                                property int originMouseY

                                property bool enableCalcThumbnailGeometry: false

                               // property var lastPos:0;


                                //drag.target:windowThumbnailitem
                                drag.smoothed :false
                                drag.threshold:0
                                preventStealing:true


                                onEntered: {
                                    if (windowThumbnailitem.Drag.active && pressed) {
                                        return;
                                    }
                                    multitaskingModel.setCurrentSelectIndex(modelData);
                                    closeClientBtn.visible = true;
                                    stickedBtn.visible = true;
                                    if (multitaskingModel.getWindowKeepAbove(modelData)) {
                                        stickedBtnIcon.source = "qrc:///icons/data/sticked_normal.svg"
                                    } else {
                                        stickedBtnIcon.source = "qrc:///icons/data/unsticked_normal.svg"
                                    }
                                }
                              
                                //  excute on released
                                // onClicked: {
                                //     multitaskingModel.setCurrentSelectIndex(modelData);
                                //     multitaskingModel.windowSelected( modelData );

                                // }
                                onExited: {
                                     closeClientBtn.visible = false;
                                     stickedBtn.visible = false;
                                }
                                Timer {
                                    id: pressedTimer
                                    interval: 200; running: false; repeat: false
                                    onTriggered: {
                                        windowThumbnailitemMousearea.enableCalcThumbnailGeometry = true
                                    }
                                }
                           
                                onPressed: {
                                     originWidth = windowThumbnailitem.width
                                     originHeight = windowThumbnailitem.height
                                     originX = windowThumbnailitem.x
                                     originY = windowThumbnailitem.y
                                     enableCalcThumbnailGeometry = false
                                     originMouseX = mouse.x
                                     originMouseY = mouse.y

                                     if (!pressedTimer.running) {
                                         pressedTimer.start()
                                     }

                                     closeClientBtn.visible = false;
                                     stickedBtn.visible = false;
                                }
                                function calcDragingThumbnailGeometry() {

                                    if (!enableCalcThumbnailGeometry) {
                                        return ;
                                    }

                                    var dragingImgWidth
                                    var dragingImgHeight
                                    var imgheight = multitaskingModel.getWindowHeight(windowThumbnailitem.winId);
                                    var imgwidth = multitaskingModel.getWindowWidth(windowThumbnailitem.winId);
                                    var scale = 1;

                                    enableCalcThumbnailGeometry = false

                                    if (imgwidth > 0 && imgheight > 0) {
                                        scale = imgwidth/imgheight;
                                    } else {
                                        scale = 0.75
                                    }
                                    if (scale>1) {
                                        dragingImgWidth = 120
                                        dragingImgHeight = dragingImgWidth /scale;
                                    } else {
                                        dragingImgHeight = 120
                                        dragingImgWidth = dragingImgHeight * scale;
                                    }
                                    //只缩小，不设置位置，因为大小变了。这个ｍｏｕｓｅ　已经不是正确的位置了。只能下次再取（就是到　ｐｏｓｉｔｉｏｎ　中取得）
                                    windowThumbnailitem.width = dragingImgWidth
                                    windowThumbnailitem.height = dragingImgHeight

                                    windowThumbnailitem.Drag.active = true
                                }

                                //onpress 后，会马上执行　onMouseXChanged，然后再执行　onPositionChanged
                                //此时要先用pressed 做条件，不能使用windowThumbnailitem.Drag.active
                                onMouseXChanged: {
                                    calcDragingThumbnailGeometry()

                                    if (pressed && !pressedTimer.running) { 
                                        //绝对坐标方法
                                        var mousePosInRoot = mapToItem(root, mouse.x, mouse.y) 
                                        windowThumbnailitem.x = (mousePosInRoot.x - windowThumbnailitem.width / 2);
                                    }
                                        
                                }
                                onMouseYChanged: {
                                    calcDragingThumbnailGeometry()

                                    if (pressed && !pressedTimer.running) {
                                        var mousePosInRoot = mapToItem(root, mouse.x, mouse.y) 
                                        windowThumbnailitem.y = (mousePosInRoot.y - windowThumbnailitem.height / 2);
                                    }
                                }
                                onReleased: {
                                    if (pressedTimer.running) {
                                        pressedTimer.stop()
                                        multitaskingModel.setCurrentSelectIndex(modelData);
                                        if (Math.abs(originMouseX-mouse.x)<20 && Math.abs(originMouseY-mouse.y)<20) {
                                            multitaskingModel.windowSelected(modelData);
                                        }
                                    } else {
                                        if (!windowThumbnailitem.Drag.active || enableCalcThumbnailGeometry) {
                                            ////恢复现场
                                            windowThumbnailitem.width = originWidth
                                            windowThumbnailitem.height = originHeight
                                            windowThumbnailitem.x = originX
                                            windowThumbnailitem.y = originY
                                            closeClientBtn.visible = true;
                                            stickedBtn.visible = true;
                                            enableCalcThumbnailGeometry = false
                                        }
                                    }
                                    windowThumbnailitem.Drag.drop()
                                    windowThumbnailitem.Drag.active = false
                                }
                             
                                states: [
                                    State {
                                    when: windowThumbnailitem.Drag.active;
                                    ParentChange {
                                        target: windowThumbnailitem;
                                        parent: root;
                                    }
                                    PropertyChanges {
                                        target: windowThumbnailitem;
                                        z: 100;
                                    }
                                    // PropertyChanges{
                                    //     target:windowThumbnailitemMousearea
                                    //     width:120
                                    //     height:80
                                    // }
                                    // AnchorChanges{
                                    //     target: windowThumbnailitem;

                                    //     Layout.fillWidth: false
                                    //     Layout.fillHeight:false
                                    // }
                                   
                                }]
                            }
                            Rectangle {
                                id: closeClientBtn;
                                visible:false;
                                x: parent.width - (parent.width - parent.width*0.85)/2 - closeClientBtnIcon.width/2
                                y: (parent.height - parent.height*0.85)/2 - closeClientBtnIcon.height/2
//                                anchors.right: parent.right;
                                width: closeClientBtnIcon.width;
                                height: closeClientBtnIcon.height;
                                color: "transparent";
                                Image {
                                    id: closeClientBtnIcon;
                                    source: "qrc:///icons/data/close_normal.svg"
                                }
                                MouseArea {
                                    anchors.fill: closeClientBtn;
                                    Accessible.role: Accessible.Button
                                    Accessible.name: "Ma_winThumb_closeBtn_"+(multitaskingModel.currentIndex()+1)+"_"+currentScreen+"_"+ windowThumbnailitem.winId
                                    Accessible.description: "windowThumbnail_closeButton_desktop_screen_winId"
                                    Accessible.onPressAction: pressed()
                                    onClicked: {
                                        qmlRemoveWindowThumbnail(currentScreen,multitaskingModel.currentIndex()+1, windowThumbnailitem.winId)
                                    }
                                }
                            }

                            Rectangle {
                                id: stickedBtn;
                                x: (parent.width - parent.width*0.85)/2 - stickedBtn.width/2
                                y: (parent.height - parent.height*0.85)/2 - stickedBtn.height/2
//                                anchors.left: parent.left;
                                width: stickedBtnIcon.width;
                                height: stickedBtnIcon.height;
                                color: "transparent";
                                visible:false;

                                Image {
                                    id: stickedBtnIcon;
                                    source: "qrc:///icons/data/unsticked_normal.svg"
                                }

                                MouseArea {
                                    anchors.fill: stickedBtn;

                                    Accessible.role: Accessible.Button
                                    Accessible.name: "Ma_winThumb_stickedBtn_"+(multitaskingModel.currentIndex()+1)+"_"+currentScreen+"_" + windowThumbnailitem.winId
                                    Accessible.description: "windowThumbnai_stickedButton_desktop_screen_winId"
                                    Accessible.onPressAction: pressed()

                                    onClicked: {
                                        if (multitaskingModel.getWindowKeepAbove(modelData))
                                        {
                                            stickedBtnIcon.source = "qrc:///icons/data/unsticked_normal.svg"
                                        } else {
                                            stickedBtnIcon.source = "qrc:///icons/data/sticked_normal.svg"
                                        }
                                        multitaskingModel.setWindowKeepAbove(modelData);
                                    }
                                }

                            }

                            states: State {
                                name: "isHightlighted"
                                when: isHightlighted 
                                PropertyChanges {
                                    target: windowThumbnailitem
                                    scale: 1.02
                                    
                                }
                                PropertyChanges {
                                    target: backgroundrect
                                    border.width: 5;
                                }
                            }

                            Rectangle {
                                id: clientIcon;
//                                color: "red"
                                x:(plasmaCoreWindowThumbnail.width)*0.05;
                                y:(parent.height - plasmaCoreWindowThumbnail.height/0.85 - clientIcon.height/1.6);
                                width: clientIconImage.width;
                                height: clientIconImage.height;
                                color: "transparent";
                                Image {
                                    id: clientIconImage;
                                    source: "image://imageProvider/" + modelData ;
                                    cache : false
                                }
                            }
                            //caption
                            Rectangle {
                                id: clientCaption;
                                x:(plasmaCoreWindowThumbnail.width)*0.05 + clientIconImage.width;
                                y:(parent.height - plasmaCoreWindowThumbnail.height/0.85 - clientIcon.height/2);
                                width: 200;
                                height: clientIconImage.height;
                                color: "transparent";
                                Text {
                                    width: 200
                                    font.pointSize: 16
                                    font.bold: true
                                    elide: Text.ElideLeft
                                    color: "white"
                                    text: multitaskingModel.windowCaption(winId)
                                }
                            }
                        } //this
                    }
                Connections {
                    target: root
                    onResetModel: {
                        setGridviewData();
                        bigWindowThrumbContainer.curdesktop=multitaskingModel.currentIndex()+1
                    }
                }
                Component.onCompleted: {
                    setGridviewData();
                }
                }
            }
        }
    }

    Component.onCompleted: {
        var numScreens = 1;
        if (multitaskingModel.isExtensionMode) {
           numScreens = multitaskingModel.numScreens();
        }

        for (var i = 0; i < numScreens; ++i) {
            var geom = multitaskingModel.screenGeometry(i);
            var src =
                'import QtQuick 2.0;' +
                'Loader {' +
                '   x: ' + geom.x + ';' +
                '   y: ' + geom.y + ';' +
                '   property int screenWidth: ' + geom.width + ';' +
                '   property int screenHeight: '+ geom.height + ';'+
                '   height: '+ geom.height/5+';'+
                '   property int currentScreen: ' + i + ';' +
                '   sourceComponent: desktopThumbnailView;' +
                '   property var screenname: multitaskingModel.screenName(x,y);' +
                '   property int currentIndex: multitaskingModel.currentIndex();' +
                '}';
            Qt.createQmlObject(src, root, "dynamicSnippet");
        }
    }
}

