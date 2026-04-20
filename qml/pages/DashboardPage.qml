import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components" as Components

ScrollView {
    id: root
    clip: true

    required property var runtime
    required property var alarm

    ColumnLayout {
        spacing: 16
        x: 16
        y: 16
        width: root.availableWidth - 32

        Components.StatusBanner {
            Layout.fillWidth: true
            alarmText: root.alarm.alarmText
            hasWarning: root.alarm.hasWarning
            isFault: root.alarm.isFault
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            Components.MetricCard {
                Layout.fillWidth: true
                title: "Temperature"
                valueText: Number(root.runtime.temperature).toFixed(1)
                unitText: "°C"
            }

            Components.MetricCard {
                Layout.fillWidth: true
                title: "Pressure"
                valueText: Number(root.runtime.pressure).toFixed(2)
                unitText: "bar"
            }

            Components.MetricCard {
                Layout.fillWidth: true
                title: "Speed"
                valueText: Number(root.runtime.speed).toFixed(0)
                unitText: "rpm"
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 140
                radius: 10
                color: "#111827"
                border.width: 1
                border.color: "#374151"

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    Label {
                        text: "Temperature Trend"
                        color: "#9ca3af"
                    }

                    Components.TrendChart {
                        width: parent.width
                        height: 80
                        samples: root.runtime.temperatureHistory
                        markers: root.runtime.historyMarkers
                        historyStartSampleIndex: root.runtime.historyStartSampleIndex
                        autoScale: true
                        minimumAutoRange: 10
                        valueDecimals: 1
                        valueSuffix: "°C"
                        lineColor: "#f97316"
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 140
                radius: 10
                color: "#111827"
                border.width: 1
                border.color: "#374151"

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    Label {
                        text: "Pressure Trend"
                        color: "#9ca3af"
                    }

                    Components.TrendChart {
                        width: parent.width
                        height: 80
                        samples: root.runtime.pressureHistory
                        markers: root.runtime.historyMarkers
                        historyStartSampleIndex: root.runtime.historyStartSampleIndex
                        autoScale: true
                        minimumAutoRange: 10
                        valueDecimals: 2
                        valueSuffix: "bar"
                        lineColor: "#38bdf8"
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 140
                radius: 10
                color: "#111827"
                border.width: 1
                border.color: "#374151"

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    Label {
                        text: "Speed Trend"
                        color: "#9ca3af"
                    }

                    Components.TrendChart {
                        width: parent.width
                        height: 80
                        samples: root.runtime.speedHistory
                        markers: root.runtime.historyMarkers
                        historyStartSampleIndex: root.runtime.historyStartSampleIndex
                        autoScale: true
                        minimumAutoRange: 200
                        valueDecimals: 0
                        valueSuffix: "rpm"
                        lineColor: "#22c55e"
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 120
            radius: 10
            color: "#111827"
            border.width: 1
            border.color: "#374151"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 8

                Label {
                    text: "Machine Status"
                    color: "#9ca3af"
                    font.pixelSize: 14
                }

                Label {
                    text: root.runtime.status
                    color: "white"
                    font.pixelSize: 28
                    font.bold: true
                }
            }
        }

        Components.ControlPanel {
            Layout.fillWidth: true
            canStart: root.runtime.canStart
            canStop: root.runtime.canStop
            canResetFault: root.runtime.canResetFault

            onStartRequested: root.runtime.start()
            onStopRequested: root.runtime.stop()
            onResetRequested: root.runtime.resetFault()
        }
    }
}
