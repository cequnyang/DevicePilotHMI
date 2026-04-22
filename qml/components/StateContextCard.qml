import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property string stateText
    required property string durationText
    required property string lastTransitionText
    required property string scenarioText
    required property string riskText
    required property color riskColor
    property string riskHeadlineText: ""
    property string riskDetailText: ""
    property string riskHintText: ""

    radius: 12
    color: "#111827"
    border.width: 1
    border.color: "#314056"
    implicitHeight: content.implicitHeight + 32

    readonly property int featuredColumnCount: width >= 960 ? 2 : 1
    readonly property int detailColumnCount: width >= 1240 ? 3 : (width >= 760 ? 2 : 1)
    readonly property color surfaceColor: "#0f172a"
    readonly property color surfaceBorderColor: "#233044"

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: 14
        spacing: 14

        Label {
            text: "State Context"
            color: "#9ca3af"
            font.pixelSize: 13
            font.weight: Font.Medium
        }

        GridLayout {
            Layout.fillWidth: true
            columns: root.featuredColumnCount
            columnSpacing: 12
            rowSpacing: 12

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 84
                radius: 10
                color: "#101a2c"
                border.width: 1
                border.color: "#31506e"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 16

                    ColumnLayout {
                        spacing: 4

                        Label {
                            text: "Current state"
                            color: "#9ca3af"
                            font.pixelSize: 13
                        }

                        Label {
                            text: "Active machine mode"
                            color: "#64748b"
                            font.pixelSize: 11
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: root.stateText
                        color: "white"
                        font.pixelSize: 24
                        font.bold: true
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 84
                radius: 10
                color: "#10161f"
                border.width: 1
                border.color: root.riskColor

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 16

                    ColumnLayout {
                        spacing: 4

                        Label {
                            text: "Health / Risk"
                            color: "#9ca3af"
                            font.pixelSize: 13
                        }

                        Label {
                            text: "Current operational assessment"
                            color: "#64748b"
                            font.pixelSize: 11
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Rectangle {
                        radius: 999
                        color: root.riskColor
                        implicitWidth: riskLabel.implicitWidth + 24
                        implicitHeight: 32

                        Label {
                            id: riskLabel
                            anchors.centerIn: parent
                            text: root.riskText
                            color: "white"
                            font.pixelSize: 14
                            font.bold: true
                        }
                    }
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: root.detailColumnCount
            columnSpacing: 12
            rowSpacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredWidth: 240
                implicitHeight: 60
                radius: 10
                color: root.surfaceColor
                border.width: 1
                border.color: root.surfaceBorderColor

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 12

                    Label {
                        text: "In current state"
                        color: "#9ca3af"
                        font.pixelSize: 13
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: root.durationText
                        color: "white"
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredWidth: 240
                implicitHeight: 60
                radius: 10
                color: root.surfaceColor
                border.width: 1
                border.color: root.surfaceBorderColor

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 12

                    Label {
                        text: "Last transition"
                        color: "#9ca3af"
                        font.pixelSize: 13
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: root.lastTransitionText
                        color: "white"
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideLeft
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredWidth: 240
                implicitHeight: 60
                radius: 10
                color: root.surfaceColor
                border.width: 1
                border.color: root.surfaceBorderColor

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 12

                    Label {
                        text: "Scenario"
                        color: "#9ca3af"
                        font.pixelSize: 13
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: root.scenarioText
                        color: "white"
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                    }
                }
            }
        }

        Rectangle {
            visible: root.riskHeadlineText.length > 0 || root.riskDetailText.length > 0
                || root.riskHintText.length > 0
            Layout.fillWidth: true
            radius: 10
            color: root.surfaceColor
            border.width: 1
            border.color: root.surfaceBorderColor
            implicitHeight: riskDetailColumn.implicitHeight + 24

            ColumnLayout {
                id: riskDetailColumn
                anchors.fill: parent
                anchors.margins: 12
                spacing: 6

                Label {
                    visible: root.riskHeadlineText.length > 0
                    Layout.fillWidth: true
                    text: root.riskHeadlineText
                    color: root.riskColor
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                }

                Label {
                    visible: root.riskDetailText.length > 0
                    Layout.fillWidth: true
                    text: root.riskDetailText
                    color: "#d1d5db"
                    wrapMode: Text.WordWrap
                    font.pixelSize: 13
                }

                Label {
                    visible: root.riskHintText.length > 0
                    Layout.fillWidth: true
                    text: root.riskHintText
                    color: "#9fb7cf"
                    wrapMode: Text.WordWrap
                    font.pixelSize: 12
                }
            }
        }
    }
}
