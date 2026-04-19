import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DevicePilotHMI

Rectangle {
    id: root

    required property string alarmText
    required property bool hasWarning
    required property bool isFault

    implicitHeight: 48
    radius: 8
    visible: root.hasWarning || root.isFault
    color: root.isFault ? "#7f1d1d" : "#78350f"
    border.width: 1
    border.color: root.isFault ? "#ef4444" : "#f59e0b"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 0

        Label {
            text: root.alarmText
            visible: root.hasWarning || root.isFault
            color: "white"
            font.pixelSize: 15
            font.bold: true
        }
    }
}
