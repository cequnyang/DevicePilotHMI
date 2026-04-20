import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../components" as Components

Item {
    id: root
    clip: true

    required property var runtime
    required property var alarm
    required property var simulCtrl
    required property var settingsSession
    required property var scenarioOptions
    required property int currentScenarioIndex

    readonly property int pageMargin: 16
    readonly property int cardRadius: 12
    readonly property int cardInnerMargin: 14
    readonly property color cardBorderColor: "#314056"
    readonly property string riskLabel: root.alarm.isFault
        ? "Critical"
        : (root.alarm.hasWarning ? "Warning" : "Healthy")
    readonly property color riskColor: root.alarm.isFault
        ? "#ef4444"
        : (root.alarm.hasWarning ? "#f59e0b" : "#22c55e")
    readonly property string riskDetailText: (root.alarm.isFault || root.alarm.hasWarning)
        ? root.alarm.alarmText
        : ""
    readonly property string activeAlarmMetric: root.alarm.activeMetric
    readonly property bool headerDetached: scrollArea.contentItem
        ? scrollArea.contentItem.contentY > 1
        : false

    function metricSeverity(metricKey) {
        if (root.activeAlarmMetric !== metricKey)
            return "normal"
        if (root.alarm.isFault)
            return "fault"
        if (root.alarm.hasWarning)
            return "warning"
        return "normal"
    }

    function metricTitleColor(metricKey) {
        return root.activeAlarmMetric === metricKey ? root.riskColor : "#9ca3af"
    }

    function metricValueColor(metricKey) {
        return root.activeAlarmMetric === metricKey ? root.riskColor : "white"
    }

    function metricUnitColor(metricKey) {
        return root.activeAlarmMetric === metricKey ? root.riskColor : "#d1d5db"
    }

    function metricStatusText(metricKey) {
        const severity = root.metricSeverity(metricKey)
        if (severity === "fault")
            return "Fault"
        if (severity === "warning")
            return "Warning"
        return "Normal"
    }

    function metricStatusColor(metricKey) {
        const severity = root.metricSeverity(metricKey)
        if (severity === "fault")
            return "#ef4444"
        if (severity === "warning")
            return "#f59e0b"
        return "#94a3b8"
    }

    function metricStatusBackgroundColor(metricKey) {
        const severity = root.metricSeverity(metricKey)
        if (severity === "fault")
            return "#261313"
        if (severity === "warning")
            return "#2a1f0a"
        return "#0f172a"
    }

    function metricStatusBorderColor(metricKey) {
        const severity = root.metricSeverity(metricKey)
        if (severity === "fault")
            return "#7f1d1d"
        if (severity === "warning")
            return "#92400e"
        return "#334155"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.topMargin: root.pageMargin
            Layout.leftMargin: root.pageMargin
            Layout.rightMargin: root.pageMargin
            implicitHeight: pinnedContent.implicitHeight

            ColumnLayout {
                id: pinnedContent
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                spacing: 16

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Components.MetricCard {
                        Layout.fillWidth: true
                        title: "Temperature"
                        valueText: Number(root.runtime.temperature).toFixed(1)
                        unitText: "°C"
                        titleColor: root.metricTitleColor("temperature")
                        valueColor: root.metricValueColor("temperature")
                        unitColor: root.metricUnitColor("temperature")
                        statusText: root.metricStatusText("temperature")
                        statusColor: root.metricStatusColor("temperature")
                        statusBackgroundColor: root.metricStatusBackgroundColor("temperature")
                        statusBorderColor: root.metricStatusBorderColor("temperature")
                    }

                    Components.MetricCard {
                        Layout.fillWidth: true
                        title: "Pressure"
                        valueText: Number(root.runtime.pressure).toFixed(2)
                        unitText: "bar"
                        titleColor: root.metricTitleColor("pressure")
                        valueColor: root.metricValueColor("pressure")
                        unitColor: root.metricUnitColor("pressure")
                        statusText: root.metricStatusText("pressure")
                        statusColor: root.metricStatusColor("pressure")
                        statusBackgroundColor: root.metricStatusBackgroundColor("pressure")
                        statusBorderColor: root.metricStatusBorderColor("pressure")
                    }

                    Components.MetricCard {
                        Layout.fillWidth: true
                        title: "Speed"
                        valueText: Number(root.runtime.speed).toFixed(0)
                        unitText: "rpm"
                        titleColor: root.metricTitleColor("speed")
                        valueColor: root.metricValueColor("speed")
                        unitColor: root.metricUnitColor("speed")
                        statusText: root.metricStatusText("speed")
                        statusColor: root.metricStatusColor("speed")
                        statusBackgroundColor: root.metricStatusBackgroundColor("speed")
                        statusBorderColor: root.metricStatusBorderColor("speed")
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: "#334155"
                opacity: root.headerDetached ? 1.0 : 0.0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 140
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 16
                opacity: root.headerDetached ? 0.8 : 0.0
                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: "#1b2535"
                    }
                    GradientStop {
                        position: 1.0
                        color: "#001b2535"
                    }
                }

                Behavior on opacity {
                    NumberAnimation {
                        duration: 140
                    }
                }
            }
        }

        ScrollView {
            id: scrollArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.bottomMargin: root.pageMargin
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 16
                x: root.pageMargin
                y: 12
                width: Math.max(0, scrollArea.availableWidth - (root.pageMargin * 2))

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 140
                        radius: root.cardRadius
                        color: "#111827"
                        border.width: 1
                        border.color: root.cardBorderColor

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.cardInnerMargin
                            spacing: 8

                            Label {
                                text: "Temperature Trend"
                                color: "#9ca3af"
                                font.pixelSize: 13
                                font.weight: Font.Medium
                            }

                            Components.TrendChart {
                                width: parent.width
                                height: 80
                                samples: root.runtime.temperatureHistory
                                markers: root.runtime.historyMarkers
                                thresholds: [
                                    {
                                        value: root.settingsSession.committedWarningTemperature,
                                        label: "Warn limit",
                                        color: "#f59e0b"
                                    },
                                    {
                                        value: root.settingsSession.committedFaultTemperature,
                                        label: "Fault limit",
                                        color: "#ef4444"
                                    }
                                ]
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
                        radius: root.cardRadius
                        color: "#111827"
                        border.width: 1
                        border.color: root.cardBorderColor

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.cardInnerMargin
                            spacing: 8

                            Label {
                                text: "Pressure Trend"
                                color: "#9ca3af"
                                font.pixelSize: 13
                                font.weight: Font.Medium
                            }

                            Components.TrendChart {
                                width: parent.width
                                height: 80
                                samples: root.runtime.pressureHistory
                                markers: root.runtime.historyMarkers
                                thresholds: [
                                    {
                                        value: root.settingsSession.committedWarningPressure,
                                        label: "Warn limit",
                                        color: "#f59e0b"
                                    },
                                    {
                                        value: root.settingsSession.committedFaultPressure,
                                        label: "Fault limit",
                                        color: "#ef4444"
                                    }
                                ]
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
                        radius: root.cardRadius
                        color: "#111827"
                        border.width: 1
                        border.color: root.cardBorderColor

                        Column {
                            anchors.fill: parent
                            anchors.margins: root.cardInnerMargin
                            spacing: 8

                            Label {
                                text: "Speed Trend"
                                color: "#9ca3af"
                                font.pixelSize: 13
                                font.weight: Font.Medium
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

                Components.StateContextCard {
                    Layout.fillWidth: true
                    stateText: root.runtime.status
                    durationText: root.runtime.currentStateDurationText
                    lastTransitionText: root.runtime.lastTransitionTimeText
                    scenarioText: root.simulCtrl.scenarioName
                    riskText: root.riskLabel
                    riskColor: root.riskColor
                    riskDetailText: root.riskDetailText
                }

                Components.ControlPanel {
                    Layout.fillWidth: true
                    canStart: root.runtime.canStart
                    canStop: root.runtime.canStop
                    canResetFault: root.runtime.canResetFault
                    scenarioOptions: root.scenarioOptions
                    currentScenarioIndex: root.currentScenarioIndex
                    scenarioSelectionEnabled: root.runtime.canStart
                    startDisabledReason: root.runtime.startDisabledReason
                    stopDisabledReason: root.runtime.stopDisabledReason
                    resetDisabledReason: root.runtime.resetDisabledReason

                    onScenarioSelected: index => {
                        root.simulCtrl.scenario = root.scenarioOptions[index].scenario
                    }
                    onStartRequested: root.runtime.start()
                    onStopRequested: root.runtime.stop()
                    onResetRequested: root.runtime.resetFault()
                }
            }
        }
    }
}
