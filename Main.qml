import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import DeepSight

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Image {
        id: video_screen
        anchors.fill: parent
        anchors.bottomMargin: 80
        fillMode: Image.PreserveAspectFit
        source: "image://frameProvider/frame"
        cache: false
    }

    Connections {
        target: Controller.getProvider()
        function onImageUpdate() {
            video_screen.source = ""
            video_screen.source = "image://frameProvider/frame?t=" + Date.now();
        }
    }

    RowLayout {
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 30
            anchors.bottomMargin: 20

            // 打开摄像头按钮
            Button {
                text: "打开摄像头"
                font.pixelSize: 14
                width: 120
                height: 40
                // 点击 → 调用总控类打开
                onClicked: Controller.start()
            }

            // 关闭摄像头按钮
            Button {
                text: "关闭摄像头"
                font.pixelSize: 14
                width: 120
                height: 40
                // 点击 → 调用总控类关闭
                onClicked: Controller.stop()
            }
        }
}