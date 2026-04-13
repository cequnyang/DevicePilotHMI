import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property bool canStart
    required property bool canStop
    required property bool canResetFault

    signal startRequested()
    signal stopRequested()
    signal resetRequested()

    radius: 10
    color: "#111827"
    border.width: 1
    border.color: "#374151"

    implicitHeight: 100

    RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Button {
            text: "Start"
            enabled: root.canStart
            onClicked: root.startRequested()
        }

        Button {
            text: "Stop"
            enabled: root.canStop
            onClicked: root.stopRequested()
        }

        Button {
            text: "Reset Fault"
            enabled: root.canResetFault
            onClicked: root.resetRequested()
        }

        Item { Layout.fillWidth: true }
    }
}