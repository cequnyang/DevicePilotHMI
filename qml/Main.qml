import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DevicePilotHMI

import "pages" as Pages

ApplicationWindow {
    id: root

    required property MachineRuntime runtime
    required property AlarmManager alarm
    required property LogModel logModel
    required property SettingsSession settingsSession

    width: 1100
    height: 760
    visible: true
    title: "DevicePilotHMI"

    property int currentPageIndex: 0

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
            }

            Pages.SettingsPage {
                session: root.settingsSession
            }
        }
    }
}
