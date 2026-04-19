import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DevicePilotHMI

Page {
    id: root

    required property var logModel
    required property var filteredLogModel

    property string selectedLevel: ""

    background: Rectangle {
        color: "#0b1220"
    }

    Binding {
        target: root.filteredLogModel
        property: "sourceLogModel"
        value: root.logModel
    }

    Binding {
        target: root.filteredLogModel
        property: "levelFilter"
        value: root.selectedLevel
    }

    Binding {
        target: root.filteredLogModel
        property: "searchText"
        value: searchField.text
    }

    Binding {
        target: root.filteredLogModel
        property: "showOnlyUnacknowledged"
        value: unackOnlyCheck.checked
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Label {
            text: "Event Log"
            color: "white"
            font.pixelSize: 22
            font.bold: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "All"
                checkable: true
                checked: root.selectedLevel === ""
                onClicked: root.selectedLevel = ""
            }

            Button {
                text: "CONFIG"
                checkable: true
                checked: root.selectedLevel === "CONFIG"
                onClicked: root.selectedLevel = "CONFIG"
            }

            Button {
                text: "INFO"
                checkable: true
                checked: root.selectedLevel === "INFO"
                onClicked: root.selectedLevel = "INFO"
            }

            Button {
                text: "WARNING"
                checkable: true
                checked: root.selectedLevel === "WARNING"
                onClicked: root.selectedLevel = "WARNING"
            }

            Button {
                text: "FAULT"
                checkable: true
                checked: root.selectedLevel === "FAULT"
                onClicked: root.selectedLevel = "FAULT"
            }
        }

        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: "Search level, timestamp, or message"
        }

        CheckBox {
            id: unackOnlyCheck
            text: "Only unacknowledged"
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 10
            color: "#111827"
            border.width: 1
            border.color: "#374151"

            ListView {
                anchors.fill: parent
                anchors.margins: 8
                clip: true
                model: root.filteredLogModel
                spacing: 6

                delegate: Rectangle {
                    width: ListView.view.width
                    implicitHeight: 64
                    radius: 6
                    color: acknowledged ? "#18212b" : "#1f2937"
                    opacity: acknowledged ? 0.72 : 1.0

                    required property int index
                    required property string timestamp
                    required property string level
                    required property string message
                    required property bool acknowledged

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        CheckBox {
                            checked: acknowledged
                            onToggled: root.filteredLogModel.setAcknowledged(index, checked)
                        }

                        Label {
                            text: timestamp
                            color: "#9ca3af"
                            font.pixelSize: 12
                            Layout.preferredWidth: 150
                        }

                        Label {
                            text: level
                            color: level === "FAULT" ? "#ef4444"
                                  : level === "WARNING" ? "#f59e0b"
                                  : level === "CONFIG" ? "#34d399"
                                  : "#93c5fd"
                            font.bold: true
                            Layout.preferredWidth: 90
                        }

                        Label {
                            text: message
                            color: "white"
                            Layout.fillWidth: true
                            wrapMode: Text.WrapAnywhere
                            font.strikeout: acknowledged
                        }
                    }
                }
            }
        }
    }
}
