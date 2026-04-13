import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    required property string alarmText
    required property bool hasWarning
    required property bool isFault

    implicitHeight: 48
    radius: 8
    visible: root.hasWarning || root.isFault
    color: isFault ? "#7f1d1d" : "#78350f"
    border.width: 1
    border.color: isFault ? "#ef4444" : "#f59e0b"

    Label {
        anchors.centerIn: parent
        text: root.alarmText
        color: "white"
        font.pixelSize: 15
        font.bold: true
    }
}