import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property string title
    required property string valueText
    required property string unitText

    radius: 10
    color: "#111827"
    border.width: 1
    border.color: "#374151"

    implicitWidth: 220
    implicitHeight: 140

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 8

        Label {
            text: root.title
            color: "#9ca3af"
            font.pixelSize: 14
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            spacing: 6

            Label {
                text: root.valueText
                color: "white"
                font.pixelSize: 34
                font.bold: true
            }

            Label {
                text: root.unitText
                color: "#d1d5db"
                font.pixelSize: 16
                Layout.alignment: Qt.AlignBottom
            }
        }
    }
}