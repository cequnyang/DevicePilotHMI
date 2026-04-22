import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DevicePilotHMI

import "components" as Components
import "pages" as Pages

ApplicationWindow {
    id: root

    required property var simulCtrl
    required property var runtime
    required property var alarm
    required property var logModel
    required property var filteredLogModel
    required property var settingsManager
    required property var settingsSession

    width: 1100
    minimumWidth: 800
    height: 760
    minimumHeight: 420
    visible: true
    title: "DevicePilotHMI"
    flags: root.useCustomWindowChrome ? (Qt.Window | Qt.FramelessWindowHint) : Qt.Window
    color: root.useCustomWindowChrome ? "transparent" : "#1f2937"

    property int currentPageIndex: 0
    readonly property bool useCustomWindowChrome: Qt.platform.os === "windows"
    readonly property bool windowIsMaximized: root.visibility === Window.Maximized
    readonly property bool windowIsExpanded: root.windowIsMaximized
        || root.visibility === Window.FullScreen
    readonly property int windowFrameMargin: root.useCustomWindowChrome && !root.windowIsExpanded ? 8 : 0
    readonly property int windowCornerRadius: root.useCustomWindowChrome && !root.windowIsExpanded ? 18 : 0
    readonly property int headerBannerPreferredWidth: 300
    readonly property color deviceStatusColor: root.alarm.isFault
        ? "#ef4444"
        : (root.alarm.hasWarning ? "#f59e0b" : "#22c55e")
    readonly property color deviceStatusBorderColor: root.alarm.isFault
        ? "#7f1d1d"
        : (root.alarm.hasWarning ? "#92400e" : "#2d6a4f")
    readonly property string deviceStatusLabel: root.alarm.stateLabel
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

    function toggleWindowMaximize() {
        if (root.windowIsExpanded)
            root.showNormal()
        else
            root.showMaximized()
    }

    component TitleDragRegion : Item {
        DragHandler {
            target: null
            acceptedButtons: Qt.LeftButton

            onActiveChanged: {
                if (active)
                    root.startSystemMove()
            }
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            gesturePolicy: TapHandler.ReleaseWithinBounds

            onDoubleTapped: {
                root.toggleWindowMaximize()
            }
        }
    }

    component WindowControlButton : Button {
        id: control

        required property string buttonType

        implicitWidth: 40
        implicitHeight: 28
        padding: 0
        hoverEnabled: true
        onHoveredChanged: iconCanvas.requestPaint()
        onDownChanged: iconCanvas.requestPaint()
        onButtonTypeChanged: iconCanvas.requestPaint()

        background: Rectangle {
            radius: 8
            color: control.down
                ? (control.buttonType === "close" ? "#8b1d1d" : "#162231")
                : (control.hovered
                    ? (control.buttonType === "close" ? "#aa2a2a" : "#213145")
                    : "transparent")
            border.width: control.hovered && control.buttonType !== "close" ? 1 : 0
            border.color: "#3d5874"
        }

        contentItem: Item {
            Canvas {
                id: iconCanvas
                anchors.centerIn: parent
                width: 14
                height: 14
                contextType: "2d"

                function strokeForButton() {
                    if (control.buttonType === "close" && (control.hovered || control.down))
                        return "#fef2f2"
                    return "#d6e2f0"
                }

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    ctx.strokeStyle = strokeForButton()
                    ctx.fillStyle = strokeForButton()
                    ctx.lineWidth = 1.5
                    ctx.lineCap = "square"
                    ctx.lineJoin = "miter"

                    if (control.buttonType === "minimize") {
                        ctx.beginPath()
                        ctx.moveTo(1.5, height - 3.0)
                        ctx.lineTo(width - 1.5, height - 3.0)
                        ctx.stroke()
                    } else if (control.buttonType === "maximize") {
                        ctx.strokeRect(2.0, 2.0, width - 4.0, height - 4.0)
                    } else if (control.buttonType === "restore") {
                        ctx.strokeRect(4.5, 1.5, width - 6.0, height - 6.0)
                        ctx.clearRect(1.0, 4.0, width - 5.0, height - 5.0)
                        ctx.strokeStyle = strokeForButton()
                        ctx.strokeRect(1.5, 4.5, width - 6.0, height - 6.0)
                    } else if (control.buttonType === "close") {
                        ctx.beginPath()
                        ctx.moveTo(2, 2)
                        ctx.lineTo(width - 2, height - 2)
                        ctx.moveTo(width - 2, 2)
                        ctx.lineTo(2, height - 2)
                        ctx.stroke()
                    }
                }
            }
        }
    }

    component ConsoleTabButton : TabButton {
        id: control

        readonly property real segmentWidth: (tabBar.availableWidth
                                              - (tabBar.spacing * Math.max(0, tabBar.count - 1)))
                                             / Math.max(1, tabBar.count)

        width: segmentWidth
        height: 44
        implicitHeight: 44
        padding: 0
        hoverEnabled: true

        background: Item {
            anchors.fill: parent

            Rectangle {
                anchors.fill: parent
                radius: 12
                color: control.checked
                    ? "#162334"
                    : (control.hovered ? "#131d2b" : "transparent")
                border.width: 1
                border.color: control.checked ? "#5f84a4" : "#243447"

                Behavior on color {
                    ColorAnimation {
                        duration: 120
                    }
                }

                Behavior on border.color {
                    ColorAnimation {
                        duration: 120
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: 14
                anchors.rightMargin: 14
                anchors.bottomMargin: 3
                height: 2
                radius: 1
                color: "#8cc7f2"
                opacity: control.checked ? 1.0 : 0.0

                Behavior on opacity {
                    NumberAnimation {
                        duration: 120
                    }
                }
            }
        }

        contentItem: Text {
            text: control.text
            font.pixelSize: 15
            font.weight: control.checked ? Font.DemiBold : Font.Medium
            color: control.checked ? "#f3f8fe" : (control.hovered ? "#c4d3e3" : "#9fb1c6")
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    component TitleStatusIndicator : Item {
        id: indicator

        required property color accentColor
        required property color frameColor
        property string statusLabel: ""
        property real pulseLevel: 1.0

        implicitWidth: 18
        implicitHeight: 18

        SequentialAnimation on pulseLevel {
            running: indicator.visible
            loops: Animation.Infinite

            NumberAnimation {
                to: 0.60
                duration: 1400
                easing.type: Easing.InOutSine
            }

            NumberAnimation {
                to: 1.0
                duration: 1400
                easing.type: Easing.InOutSine
            }
        }

        ToolTip.visible: hover.hovered
        ToolTip.delay: 250
        ToolTip.text: indicator.statusLabel.length > 0
            ? "Device status: " + indicator.statusLabel
            : ""

        HoverHandler {
            id: hover
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width + 4
            height: parent.height + 4
            radius: 7
            color: indicator.accentColor
            opacity: 0.10 + (indicator.pulseLevel * 0.12)
        }

        Rectangle {
            anchors.fill: parent
            radius: 5
            color: "#0f1725"
            border.width: 1
            border.color: indicator.frameColor
            opacity: 0.88
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: 4
            color: "#111c2b"
            border.width: 1
            border.color: "#dbeafe"
            opacity: 0.06 + (indicator.pulseLevel * 0.08)
        }

        Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            radius: 3
            color: indicator.accentColor
            opacity: indicator.pulseLevel
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 3
            height: Math.max(2, Math.floor(parent.height * 0.28))
            radius: 2
            color: "#f8fbff"
            opacity: 0.04 + (indicator.pulseLevel * 0.08)
        }
    }

    Rectangle {
        id: windowSurface
        anchors.fill: parent
        anchors.margins: root.windowFrameMargin
        radius: root.windowCornerRadius
        antialiasing: true
        color: "#1f2937"
        border.width: root.useCustomWindowChrome ? 1 : 0
        border.color: "#16202d"
        clip: true

        ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: root.useCustomWindowChrome ? 52 : 64
            radius: root.windowCornerRadius
            color: "#1f2937"
            border.width: 0

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: root.useCustomWindowChrome ? "#32465f" : "transparent"
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: root.useCustomWindowChrome ? 8 : 16
                anchors.topMargin: root.useCustomWindowChrome ? 6 : 10
                anchors.bottomMargin: root.useCustomWindowChrome ? 6 : 10
                spacing: root.useCustomWindowChrome ? 12 : 16

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignVCenter

                    TitleDragRegion {
                        anchors.fill: parent
                        visible: root.useCustomWindowChrome
                    }

                    RowLayout {
                        anchors.fill: parent
                        spacing: root.useCustomWindowChrome ? 12 : 16

                        RowLayout {
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 10

                            Item {
                                visible: root.useCustomWindowChrome
                                implicitWidth: 18
                                implicitHeight: 18
                                Layout.alignment: Qt.AlignVCenter

                                TitleStatusIndicator {
                                    anchors.centerIn: parent
                                    accentColor: root.deviceStatusColor
                                    frameColor: root.deviceStatusBorderColor
                                    statusLabel: root.deviceStatusLabel
                                }
                            }

                            Label {
                                text: "DevicePilotHMI"
                                color: "white"
                                font.pixelSize: root.useCustomWindowChrome ? 16 : 22
                                font.bold: true
                                Layout.alignment: Qt.AlignVCenter
                            }
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.minimumWidth: 24
                        }

                        Components.StatusBanner {
                            id: headerStatusBanner
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: root.headerBannerPreferredWidth
                            Layout.maximumWidth: root.headerBannerPreferredWidth
                            embedded: true
                            headline: root.alarm.headline
                            detail: root.alarm.detail
                            operatorHint: root.alarm.operatorHint
                            stateLabel: root.alarm.stateLabel
                            hasWarning: root.alarm.hasWarning
                            isFault: root.alarm.isFault
                            recoveryActive: root.alarm.recoveryActive
                        }

                        Label {
                            text: root.runtime.status
                            color: "#d1d5db"
                            font.pixelSize: 14
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }
                }

                RowLayout {
                    visible: root.useCustomWindowChrome
                    spacing: 4
                    Layout.alignment: Qt.AlignVCenter

                    WindowControlButton {
                        buttonType: "minimize"
                        onClicked: root.showMinimized()
                    }

                    WindowControlButton {
                        buttonType: root.windowIsExpanded ? "restore" : "maximize"
                        onClicked: root.toggleWindowMaximize()
                    }

                    WindowControlButton {
                        buttonType: "close"
                        onClicked: root.close()
                    }
                }
            }
        }

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            currentIndex: root.currentPageIndex
            onCurrentIndexChanged: root.currentPageIndex = currentIndex
            implicitHeight: 62
            spacing: 8
            leftPadding: 8
            rightPadding: 8
            topPadding: 8
            bottomPadding: 8
            clip: true

            background: Rectangle {
                radius: 14
                color: "#101926"
                border.width: 1
                border.color: "#28384b"
            }

            ConsoleTabButton { text: "Dashboard" }
            ConsoleTabButton { text: "Event Log" }
            ConsoleTabButton { text: "Settings" }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.currentPageIndex

            Pages.DashboardPage {
                runtime: root.runtime
                alarm: root.alarm
                simulCtrl: root.simulCtrl
                settingsSession: root.settingsSession
                scenarioOptions: root.scenarioOptions
                currentScenarioIndex: root.scenarioIndexFor(root.simulCtrl.scenario)
            }

            Pages.LogPage {
                logModel: root.logModel
                filteredLogModel: root.filteredLogModel
                settingsManager: root.settingsManager
                pageCornerRadius: root.windowCornerRadius
            }

            Pages.SettingsPage {
                session: root.settingsSession
                pageCornerRadius: root.windowCornerRadius
            }
        }
        }
    }

    MouseArea {
        visible: root.useCustomWindowChrome && !root.windowIsExpanded
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 6
        hoverEnabled: true
        cursorShape: Qt.SizeHorCursor
        onPressed: mouse => root.startSystemResize(Qt.LeftEdge)
    }

    MouseArea {
        visible: root.useCustomWindowChrome && !root.windowIsExpanded
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 6
        hoverEnabled: true
        cursorShape: Qt.SizeHorCursor
        onPressed: mouse => root.startSystemResize(Qt.RightEdge)
    }

    MouseArea {
        visible: root.useCustomWindowChrome && !root.windowIsExpanded
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 6
        hoverEnabled: true
        cursorShape: Qt.SizeVerCursor
        onPressed: mouse => root.startSystemResize(Qt.TopEdge)
    }

    MouseArea {
        visible: root.useCustomWindowChrome && !root.windowIsExpanded
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 6
        hoverEnabled: true
        cursorShape: Qt.SizeVerCursor
        onPressed: mouse => root.startSystemResize(Qt.BottomEdge)
    }

    MouseArea {
        visible: root.useCustomWindowChrome && !root.windowIsExpanded
        anchors.left: parent.left
        anchors.top: parent.top
        width: 10
        height: 10
        hoverEnabled: true
        cursorShape: Qt.SizeFDiagCursor
        onPressed: mouse => root.startSystemResize(Qt.LeftEdge | Qt.TopEdge)
    }

    MouseArea {
        visible: root.useCustomWindowChrome && !root.windowIsExpanded
        anchors.right: parent.right
        anchors.top: parent.top
        width: 10
        height: 10
        hoverEnabled: true
        cursorShape: Qt.SizeBDiagCursor
        onPressed: mouse => root.startSystemResize(Qt.RightEdge | Qt.TopEdge)
    }

    MouseArea {
        visible: root.useCustomWindowChrome && !root.windowIsExpanded
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        width: 10
        height: 10
        hoverEnabled: true
        cursorShape: Qt.SizeBDiagCursor
        onPressed: mouse => root.startSystemResize(Qt.LeftEdge | Qt.BottomEdge)
    }

    MouseArea {
        visible: root.useCustomWindowChrome && !root.windowIsExpanded
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        width: 10
        height: 10
        hoverEnabled: true
        cursorShape: Qt.SizeFDiagCursor
        onPressed: mouse => root.startSystemResize(Qt.RightEdge | Qt.BottomEdge)
    }
}
