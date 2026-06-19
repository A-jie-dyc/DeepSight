import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: ""
    modal: true
    closePolicy: Dialog.CloseOnEscape | Dialog.CloseOnPressOutside
    standardButtons: Dialog.NoButton

    implicitWidth: 420

    //居中显示在父窗口
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0

    property int headerHeight: 44
    //对接ErrorInfo结构体
    property var errorInfo: ({
            type: 0,
            level: 0,
            title: "",
            message: "",
            suggestion: ""
        })


    function levelColor(level) {
        switch (level) {
            case ErrorDef.ErrorLevel_Critical: return "#e74c3c"         //红
            case ErrorDef.ErrorLevel_Warning: return "#f39c12"          //橙
            case ErrorDef.ErrorLevel_Info: return "#3498db"             //蓝
            default: return "#7f8c8d"
        }
    }

    function levelIcon(level) {
        switch (level) {
            case ErrorDef.ErrorLevel_Critical: return "⚠"
            case ErrorDef.ErrorLevel_Warning: return "⚡"
            case ErrorDef.ErrorLevel_Info: return "ℹ"
            default: return "❗"
        }
    }

    //整体内容容器
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        //顶部标题栏
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: root.headerHeight
            color: root.levelColor(root.errorInfo.level)
            radius: 6
            clip: true

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 12

                Text {
                    text: root.levelIcon(root.errorInfo.level)
                    font.pixelSize: 20
                    color: "#ffffff"
                }

                Text {
                    Layout.fillWidth: true
                    text: root.errorInfo.title || "系统提示"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#ffffff"
                    elide: Text.ElideRight
                }
            }
        }

        //内容区
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: 20
            Layout.leftMargin: 24
            Layout.rightMargin: 24
            Layout.bottomMargin: 16
            spacing: 16

            //错误详情文本
            Text {
                Layout.fillWidth: true
                text: root.errorInfo.message
                wrapMode: Text.WordWrap
                font.pixelSize: 14
                color: "#2c3e50"
                lineHeight: 1.5
            }

            //修复建议卡片
            Rectangle {
                Layout.fillWidth: true
                visible: root.errorInfo.suggestion && root.errorInfo.suggestion.length > 0
                color: "#f5f6fa"
                radius: 4
                //根据内容自适应高度，避免与按钮区重叠
                implicitHeight: suggestionContent.implicitHeight + 24

                Column {
                    id: suggestionContent
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: 12
                    spacing: 6

                    Text {
                        text: "修复建议"
                        font.pixelSize: 12
                        font.bold: true
                        color: "#7f8c8d"
                    }

                    Text {
                        width: parent.width
                        text: root.errorInfo.suggestion
                        wrapMode: Text.WordWrap
                        font.pixelSize: 12
                        color: "#7f8c8d"
                        lineHeight: 1.4
                    }
                }
            }
        }

        //底部按钮区
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: "transparent"

            Button {
                anchors.right: parent.right
                anchors.rightMargin: 24
                anchors.verticalCenter: parent.verticalCenter
                width: 88
                height: 34
                text: "确定"
                onClicked: root.close()

                background: Rectangle {
                    color: parent.down ? "#2980b9" : "#3498db"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: "#ffffff"
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
}

