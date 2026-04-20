import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DevicePilotHMI

import "pages" as Pages

ApplicationWindow {
    id: root

    required property var simulCtrl
    required property var runtime
    required property var alarm
    required property var logModel
    required property var filteredLogModel
    required property var settingsSession

    width: 1100
    height: 760
    visible: true
    title: "DevicePilotHMI"

    property int currentPageIndex: 0
    property var scenarioOptions: [
        { label: "Normal Ramp", scenario: SimulationScenario.NormalRamp },
        { label: "Overload", scenario: SimulationScenario.Overload },
        { label: "Cooling Failure", scenario: SimulationScenario.CoolingFailure },
        { label: "Load Step Response", scenario: SimulationScenario.LoadStepResponse }
    ]

    function scenarioIndexFor(value) {
        for (let i = 0; i < scenarioOptions.length; ++i) {
            if (scenarioOptions[i].scenario === value)
                return i
        }
        return 0
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 56
            color: "#1f2937"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16

                Label {
                    text: "DevicePilotHMI"
                    color: "white"
                    font.pixelSize: 22
                    font.bold: true
                }

                Item { Layout.fillWidth: true }

                Label {
                    text: "Scenario:"
                    color: "#d1d5db"
                    font.pixelSize: 14
                }

                ComboBox {
                    id: scenarioCombo
                    Layout.preferredWidth: 200
                    model: root.scenarioOptions
                    textRole: "label"
                    currentIndex: root.scenarioIndexFor(root.simulCtrl.scenario)
                    enabled: root.runtime.canStart
                    onActivated: {
                        root.simulCtrl.scenario = root.scenarioOptions[currentIndex].scenario
                    }
                }

                Label {
                    text: root.runtime.status
                    color: "#d1d5db"
                    font.pixelSize: 14
                }
            }
        }

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            currentIndex: root.currentPageIndex
            onCurrentIndexChanged: root.currentPageIndex = currentIndex

            TabButton { text: "Dashboard" }
            TabButton { text: "Event Log" }
            TabButton { text: "Settings" }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.currentPageIndex

            Pages.DashboardPage {
                runtime: root.runtime
                alarm: root.alarm
            }

            Pages.LogPage {
                logModel: root.logModel
                filteredLogModel: root.filteredLogModel
            }

            Pages.SettingsPage {
                session: root.settingsSession
            }
        }
    }
}
