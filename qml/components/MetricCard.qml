import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property string title
    required property string valueText
    required property string unitText
    property color titleColor: "#9ca3af"
    property color valueColor: "white"
    property color unitColor: "#d1d5db"
    property string statusText: ""
    property color statusColor: "#94a3b8"
    property color statusBackgroundColor: "#0f172a"
    property color statusBorderColor: "#334155"

    radius: 12
    color: "#111827"
    border.width: 1
    border.color: "#314056"

    implicitWidth: 220
    implicitHeight: 136

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: root.title
                color: root.titleColor
                font.pixelSize: 13
                font.weight: Font.Medium
            }

            Item {
                Layout.fillWidth: true
            }

            Rectangle {
                visible: root.statusText.length > 0
                radius: 999
                color: root.statusBackgroundColor
                border.width: 1
                border.color: root.statusBorderColor
                implicitHeight: 24
                implicitWidth: statusLayout.implicitWidth + 14

                RowLayout {
                    id: statusLayout
                    anchors.centerIn: parent
                    spacing: 6

                    Rectangle {
                        implicitWidth: 7
                        implicitHeight: 7
                        radius: width / 2
                        color: root.statusColor
                    }

                    Label {
                        text: root.statusText
                        color: root.statusColor
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            spacing: 6

            Label {
                text: root.valueText
                color: root.valueColor
                font.pixelSize: 48
                font.bold: true
            }

            Label {
                text: root.unitText
                color: root.unitColor
                font.pixelSize: 24
                Layout.alignment: Qt.AlignBottom
            }
        }
    }
}
