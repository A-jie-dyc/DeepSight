import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import DeepSight

Window {
    id: mainWindow
    width: 800
    height: 720
    visible: true
    title: qsTr("智能人流统计系统")
    color: "#2c3e50"        //深灰蓝

    ErrorDialog {
        id: errorDialog
        parent: mainWindow
    }

    Connections {
        target: Controller
        function onErrorHappened(errorInfo) {
            errorDialog.errorInfo = errorInfo
            errorDialog.open()
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 60
        color: "#34495e"        //浅灰蓝
        opacity: 0.9

        RowLayout {
            anchors.centerIn: parent
            spacing: 40

            Column {
                Layout.alignment: Qt.AlignCenter
                Text {
                    text: "累计进入"
                    color: "#ffffff"
                    font.pixelSize: 14
                    horizontalAlignment: Qt.AlignCenter
                }

                Text {
                    text: Controller.enterTotal
                    color: "#2ecc71"        //高亮绿
                    font.pixelSize: 22
                    font.bold: true
                    horizontalAlignment: Qt.AlignCenter
                }
            }

            Column {
                Layout.alignment: Qt.AlignCenter
                Text {
                    text: "当前在场"
                    color: "#ffffff"
                    font.pixelSize: 14
                    horizontalAlignment: Qt.AlignCenter
                }

                Text {
                    text: Controller.currentPeople
                    color: "#3498db"        //高亮蓝
                    font.pixelSize: 22
                    font.bold: true
                    horizontalAlignment: Qt.AlignCenter
                }
            }
        }
    }

    FrameDisplay {
        id: video_screen
        objectName: "videoScreen"
        anchors.top: parent.top
        anchors.topMargin: 60
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: topBtnLayout.top
        anchors.margins: 5
    }

    Rectangle {
        anchors.top: videoScreen.top
        anchors.right: videoScreen.right
        anchors.topMargin: 10
        anchors.rightMargin: 10
        color: "#000000"        //黑
        opacity: 0.5            //50%透明度
        radius: 4
        implicitWidth: fpsText.implicitWidth + 12
        implicitHeight: fpsText.implicitHeight + 16

        Column {
            id: fpsColumn
            anchors.left: parent
            spacing: 2

            Text {
                text: "推理帧率: " + Controller.inferFps.toFixed(1) + " FPS"
                color: "#2ecc71"    //绿
                font.pixelSize: 13
                font.bold: true
            }

            Text {
                text: "输入帧率: " + Controller.totalFps.toFixed(1) + " FPS"
                color: "#ffffff"    //白
                font.pixelSize: 13
            }

            Text {
                text: "丢帧率: " + Controller.dropRate.toFixed(1) + " %"
                color: "#e74c3c"    //红
                font.pixelSize: 13
            }
        }
        visible: Controller.running
    }

    RowLayout {
        id: topBtnLayout
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 20
        anchors.bottomMargin: 20
        height: 40

        Button {
            text: "打开摄像头"
            font.pixelSize: 14
            width: 120
            height: 40
            onClicked: Controller.openCamera()
        }

        Button {
            text: "打开视频"
            font.pixelSize: 14
            width: 120
            height: 40
            onClicked: videoDialog.open()
        }

        Button {
            text: "关闭媒体"
            font.pixelSize: 14
            width: 120
            height: 40
            enabled: Controller.mediaOpened
            onClicked: Controller.closeMedia()
            background: Rectangle {
                    color: parent.enabled ? "#e67e22" : "#95a5a6"       //橙|灰
                    radius: 5
                }
        }

        Button {
            text: Controller.running ? "停止" : "启动"
            font.pixelSize: 14
            width: 120
            height: 40
            enabled: Controller.mediaOpened
            onClicked: {
                if (Controller.running) {
                    Controller.stop()
                } else {
                    Controller.start()
                }
            }
            background: Rectangle {
                color: !parent.enabled ? "#95a5a6" : (Controller.running ? "#e74c3c" : "#2ecc71")      //灰|红|绿
                radius: 5
            }
        }

        Button {
            text: Controller.AIRunning ? "暂停检测" : "开始检测"
            font.pixelSize: 14
            width: 120
            height: 40
            enabled: Controller.running
            onClicked: {
                if (Controller.AIRunning) {
                    Controller.stopAI()
                } else {
                    Controller.startAI()
                }
            }
            background: Rectangle {
                color: !parent.enabled ? "#95a5a6" : (Controller.AIRunning ? "#e67e22" : "#3498db")     //灰|橙|蓝
                radius: 5
            }
        }

        Button {
            text: video_screen.drawMode ? "取消绘制" : "绘制统计线"
            font.pixelSize: 14
            width: 120
            height: 40
            enabled: Controller.mediaOpened
            onClicked: video_screen.drawMode = !video_screen.drawMode
            background: Rectangle {
                color: !parent.enabled ? "#95a5a6" : (video_screen.drawMode ? "#e74c3c" : "#ffffff")    //灰|红
                radius: 5
            }
        }

        Button {
            text: "清空统计"
            font.pixelSize: 14
            width: 120
            height: 40
            onClicked: Controller.resetFlowCount()
        }
    }

    FileDialog {
        id:videoDialog
        title: "选择视频文件"
        nameFilters: ["视频文件(*.mp4 *.avi *.mov *.mkv)","所有文件(*)"]
        currentFolder: ""
        fileMode: FileDialog.OpenFile

        onAccepted: {
            console.log("选中的视频路径:", selectedFile)
            if(selectedFile.toString() !== "") {
                Controller.openVideo(selectedFile)
            }
        }
    }
}