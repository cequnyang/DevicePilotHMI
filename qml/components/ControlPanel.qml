import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property bool canStart
    required property bool canStop
    required property bool canResetFault
    required property var scenarioOptions
    required property int currentScenarioIndex
    required property bool scenarioSelectionEnabled
    required property string startDisabledReason
    required property string stopDisabledReason
    required property string resetDisabledReason

    signal startRequested()
    signal stopRequested()
    signal resetRequested()
    signal scenarioSelected(int index)

    radius: 12
    color: "#111827"
    border.width: 1
    border.color: "#314056"

    implicitHeight: 98

    component ControlButton : Button {
        id: control

        property color fillColor: "#111827"
        property color hoverFillColor: "#172033"
        property color downFillColor: "#0d1422"
        property color borderColor: "#475569"
        property color foregroundColor: "#e5e7eb"
        property string disabledReason: ""

        implicitHeight: 40
        implicitWidth: Math.max(116, contentItem.implicitWidth + leftPadding + rightPadding)
        leftPadding: 16
        rightPadding: 16
        topPadding: 9
        bottomPadding: 9
        hoverEnabled: true
        font.pixelSize: 14
        font.weight: Font.DemiBold

        HoverHandler {
            id: disabledHover
            enabled: !control.enabled
        }

        background: Rectangle {
            radius: 10
            color: !control.enabled
                ? "#0f172a"
                : (control.down
                    ? control.downFillColor
                    : (control.hovered ? control.hoverFillColor : control.fillColor))
            border.width: 1
            border.color: control.enabled ? control.borderColor : "#273244"
        }

        contentItem: Text {
            text: control.text
            color: control.enabled ? control.foregroundColor : "#64748b"
            font: control.font
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        ToolTip.visible: (control.enabled ? control.hovered : disabledHover.hovered)
            && !control.enabled
            && control.disabledReason.length > 0
        ToolTip.delay: 300
        ToolTip.text: control.disabledReason
    }

    component ScenarioComboBox : ComboBox {
        id: control

        implicitHeight: 40
        implicitWidth: 240
        leftPadding: 14
        rightPadding: 36
        topPadding: 9
        bottomPadding: 9
        hoverEnabled: true
        font.pixelSize: 14
        font.weight: Font.Medium

        delegate: ItemDelegate {
            width: control.width - 12
            height: 36
            leftPadding: 12
            rightPadding: 12
            highlighted: control.highlightedIndex === index

            contentItem: Text {
                text: modelData && modelData[control.textRole] !== undefined
                    ? modelData[control.textRole]
                    : modelData
                color: highlighted ? "#f8fafc" : "#d1d5db"
                font.pixelSize: 13
                font.weight: highlighted ? Font.DemiBold : Font.Normal
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }

            background: Rectangle {
                radius: 8
                color: highlighted
                    ? "#172033"
                    : (parent.hovered ? "#121a28" : "transparent")
                border.width: highlighted ? 1 : 0
                border.color: highlighted ? "#37506f" : "transparent"
            }
        }

        indicator: Canvas {
            id: indicatorCanvas
            x: control.width - width - 14
            y: (control.height - height) / 2
            width: 12
            height: 8
            contextType: "2d"

            onPaint: {
                const ctx = getContext("2d")
                ctx.reset()
                ctx.strokeStyle = control.enabled ? "#94a3b8" : "#64748b"
                ctx.lineWidth = 1.6
                ctx.lineCap = "round"
                ctx.beginPath()
                ctx.moveTo(1, 1)
                ctx.lineTo(width / 2, height - 1)
                ctx.lineTo(width - 1, 1)
                ctx.stroke()
            }
        }

        contentItem: Text {
            text: control.displayText
            color: control.enabled ? "#e5e7eb" : "#64748b"
            font: control.font
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            x: control.leftPadding
            width: control.availableWidth - control.leftPadding - control.rightPadding
        }

        background: Rectangle {
            radius: 10
            color: !control.enabled
                ? "#0f172a"
                : (control.pressed
                    ? "#0d1422"
                    : (control.hovered ? "#172033" : "#111827"))
            border.width: 1
            border.color: control.enabled ? "#475569" : "#273244"
        }

        popup: Popup {
            y: control.height + 6
            width: control.width
            padding: 6
            implicitHeight: Math.min(contentItem.implicitHeight + (padding * 2), 220)

            background: Rectangle {
                radius: 12
                color: "#0f172a"
                border.width: 1
                border.color: "#314056"
            }

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: control.popup.visible ? control.delegateModel : null
                currentIndex: control.highlightedIndex
                spacing: 4
                ScrollIndicator.vertical: ScrollIndicator {}
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 14

        ColumnLayout {
            spacing: 4

            Label {
                text: "Simulation Scenario"
                color: "#94a3b8"
                font.pixelSize: 12
                font.weight: Font.Medium
            }

            ScenarioComboBox {
                id: scenarioCombo
                Layout.preferredWidth: 240
                model: root.scenarioOptions
                textRole: "label"
                currentIndex: root.currentScenarioIndex
                enabled: root.scenarioSelectionEnabled

                onActivated: {
                    root.scenarioSelected(currentIndex)
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }

        RowLayout {
            spacing: 12

            ControlButton {
                text: "Start"
                fillColor: "#112419"
                hoverFillColor: "#173022"
                downFillColor: "#0d1b14"
                borderColor: "#3d7a5a"
                foregroundColor: "#d9f4e5"
                enabled: root.canStart
                disabledReason: root.startDisabledReason
                onClicked: root.startRequested()
            }

            ControlButton {
                text: "Stop"
                fillColor: "#111827"
                hoverFillColor: "#172033"
                downFillColor: "#0d1422"
                borderColor: "#475569"
                foregroundColor: "#e5e7eb"
                enabled: root.canStop
                disabledReason: root.stopDisabledReason
                onClicked: root.stopRequested()
            }
        }

        Rectangle {
            implicitWidth: 1
            implicitHeight: 26
            radius: 1
            color: "#243041"
            opacity: 0.9
        }

        ControlButton {
            text: "Reset Fault"
            fillColor: "#111827"
            hoverFillColor: "#161c2a"
            downFillColor: "#0d1320"
            borderColor: "#8b6b2f"
            foregroundColor: "#e3cf9b"
            enabled: root.canResetFault
            disabledReason: root.resetDisabledReason
            onClicked: root.resetRequested()
        }
    }
}
