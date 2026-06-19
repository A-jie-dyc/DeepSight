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
    color: "#2c3e50"

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
        color: "#34495e"
        opacity: 0.9

        RowLayout {
            anchors.centerIn: parent
            spacing: 40

            Column {
                Layout.alignment: Qt.AlignCenter
                Text {
                    text: "累计进入"
                    color: "white"
                    font.pixelSize: 14
                    horizontalAlignment: Qt.AlignCenter
                }

                Text {
                    text: Controller.enterTotal
                    color: "#2ecc71"
                    font.pixelSize: 22
                    font.bold: true
                    horizontalAlignment: Qt.AlignCenter
                }
            }

            Column {
                Layout.alignment: Qt.AlignCenter
                Text {
                    text: "当前在场"
                    color: "white"
                    font.pixelSize: 14
                    horizontalAlignment: Qt.AlignCenter
                }

                Text {
                    text: Controller.currentPeople
                    color: "#3498db" //蓝色高亮
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
            text: Controller.running ? "停止" : "启动"
            font.pixelSize: 14
            width: 120
            height: 40
            onClicked: {
                if (Controller.running) {
                    Controller.stop()
                } else {
                    Controller.start()
                }
            }
            background: Rectangle {
                color: Controller.running ? "#e74c3c" : "#2ecc71"
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
                color: Controller.AIRunning ? "#e67e22" : "#3498db"
                radius: 5
            }
        }

        Button {
            text: "打开视频"
            font.pixelSize: 14
            width: 120
            height: 40
            onClicked: videoDialog.open()
        }

        Button {
            text: video_screen.drawMode ? "取消绘制" : "绘制统计线"
            font.pixelSize: 14
            width: 120
            height: 40
            onClicked: video_screen.drawMode = !video_screen.drawMode
            background: Rectangle {
                color: video_screen.drawMode ? "#e74c3c" : "#3498db"
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