import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DevicePilotHMI

Page {
    id: root

    required property SettingsSession session

    background: Rectangle {
        color: "#0b1220"
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        clip: true

        Item {
            width: scrollView.availableWidth
            implicitHeight: pageContent.implicitHeight + 32

            ColumnLayout {
                id: pageContent
                x: 16
                y: 16
                width: parent.width - 32
                spacing: 16

                Label {
                    text: "Settings"
                    color: "white"
                    font.pixelSize: 22
                    font.bold: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: settingsLayout.implicitHeight + 32
                    radius: 10
                    color: "#111827"
                    border.width: 1
                    border.color: "#374151"

                    ColumnLayout {
                        id: settingsLayout
                        x: 16
                        y: 16
                        width: parent.width - 32
                        spacing: 12

                        Label { text: "Warning Temperature"; color: "white" }
                        SpinBox {
                            from: 0
                            to: 199
                            value: session.draft.warningTemperature
                            onValueModified: session.draft.warningTemperature = value
                        }

                        Label { text: "Fault Temperature"; color: "white" }
                        SpinBox {
                            from: 1
                            to: 200
                            value: session.draft.faultTemperature
                            onValueModified: session.draft.faultTemperature = value
                        }

                        Label { text: "Warning Pressure"; color: "white" }
                        SpinBox {
                            from: 0
                            to: 149
                            value: session.draft.warningPressure
                            onValueModified: session.draft.warningPressure = value
                        }

                        Label { text: "Fault Pressure"; color: "white" }
                        SpinBox {
                            from: 1
                            to: 150
                            value: session.draft.faultPressure
                            onValueModified: session.draft.faultPressure = value
                        }

                        Label { text: "Update Interval (ms)"; color: "white" }
                        SpinBox {
                            from: 100
                            to: 5000
                            stepSize: 100
                            value: session.draft.updateIntervalMs
                            onValueModified: session.draft.updateIntervalMs = value
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Button {
                                text: "Apply"
                                enabled: session.applyEnabled
                                onClicked: session.apply()
                            }

                            Button {
                                text: "Revert"
                                enabled: session.draft.dirty
                                onClicked: session.reload()
                            }

                            Button {
                                text: "Defaults"
                                onClicked: session.draft.resetDraftToDefaults()
                            }
                        }

                        Label {
                            Layout.fillWidth: true
                            text: session.draft.validationMessage
                            visible: text.length > 0
                            color: "#f59e0b"
                            wrapMode: Text.Wrap
                        }

                        Label {
                            Layout.fillWidth: true
                            text: session.applyRestrictionReason
                            visible: text.length > 0
                            color: "#9ca3af"
                            wrapMode: Text.Wrap
                        }

                        Label {
                            Layout.fillWidth: true
                            text: session.draft.dirty ? "Draft has unapplied changes." : "Draft matches committed settings."
                            color: session.draft.dirty ? "#93c5fd" : "#6b7280"
                            wrapMode: Text.Wrap
                        }
                    }
                }
            }
        }
    }
}
