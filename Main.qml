import QtQuick.Window
import QtQuick.Controls
import DeepSight

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    VideoCapture {
        id: cap
    }

    Button {
        text: "打开摄像头"
        onClicked: cap.openCamera()
    }

    Button {
        text: "关闭"
        onClicked: cap.stopCapture()
        anchors.left: parent.right
    }

}