import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DevicePilotHMI

Page {
    id: root

    required property var session
    property int pageCornerRadius: 18
    property int contentMaxWidth: 750
    clip: true

    component FieldLabel : Label {
        color: "#94a3b8"
        font.pixelSize: 12
        font.weight: Font.Medium
    }

    component ActionButton : Button {
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

        ToolTip.visible: !control.enabled && disabledHover.hovered
            && control.disabledReason.length > 0
        ToolTip.delay: 300
        ToolTip.text: control.disabledReason
    }

    component StepperGlyph : Canvas {
        property bool plus: true
        property color strokeColor: "#d8e7f8"
        property color outlineColor: "#08101d"

        implicitWidth: 14
        implicitHeight: 14
        contextType: "2d"

        onPlusChanged: requestPaint()
        onStrokeColorChanged: requestPaint()
        onOutlineColorChanged: requestPaint()

        onPaint: {
            const ctx = getContext("2d")
            const horizontalY = Math.round(height / 2)
            const horizontalStart = 3
            const horizontalEnd = width - 3
            const verticalX = Math.round(width / 2)
            const verticalStart = 3
            const verticalEnd = height - 3

            function drawGlyph(color, lineWidth) {
                ctx.strokeStyle = color
                ctx.lineWidth = lineWidth
                ctx.lineCap = "round"
                ctx.beginPath()
                ctx.moveTo(horizontalStart, horizontalY)
                ctx.lineTo(horizontalEnd, horizontalY)
                if (plus) {
                    ctx.moveTo(verticalX, verticalStart)
                    ctx.lineTo(verticalX, verticalEnd)
                }
                ctx.stroke()
            }

            ctx.reset()
            drawGlyph(outlineColor, 3.6)
            drawGlyph(strokeColor, 2.2)
        }
    }

    component StyledSpinBox : SpinBox {
        id: control

        readonly property int indicatorWidth: 22
        readonly property int indicatorInset: 3
        readonly property int indicatorGap: 1

        implicitHeight: 27
        implicitWidth: 148
        editable: false
        hoverEnabled: true
        wheelEnabled: true
        leftPadding: 14
        rightPadding: indicatorWidth + 10
        topPadding: 4
        bottomPadding: 4
        font.pixelSize: 13
        font.weight: Font.Medium

        contentItem: Text {
            text: control.displayText
            color: control.enabled ? "#e5e7eb" : "#64748b"
            font: control.font
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            radius: 8
            color: !control.enabled
                ? "#0f172a"
                : (control.activeFocus
                    ? "#122030"
                    : (control.hovered ? "#172033" : "#111827"))
            border.width: 1
            border.color: control.activeFocus ? "#5f84a4" : "#475569"
        }

        up.indicator: Rectangle {
            id: upIndicator
            x: control.width - width - control.indicatorInset
            y: control.indicatorInset
            width: control.indicatorWidth
            height: Math.floor((control.height - (control.indicatorInset * 2) - control.indicatorGap) / 2)
            radius: 6
            color: !control.enabled
                ? "#0f172a"
                : (control.up.pressed ? "#18273a" : (upHover.hovered ? "#1c2d44" : "#162538"))
            border.width: 1
            border.color: control.enabled
                ? (control.up.pressed ? "#7d9bbd" : (upHover.hovered ? "#67819f" : "#4e627b"))
                : "#273244"

            HoverHandler {
                id: upHover
                enabled: control.enabled
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                radius: parent.radius - 1
                color: "transparent"
                border.width: upHover.hovered || control.up.pressed ? 1 : 0
                border.color: control.up.pressed ? "#b7d4f4" : "#8fb2d8"
                opacity: control.up.pressed ? 0.28 : 0.14
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 2
                height: Math.max(2, Math.floor(parent.height * 0.42))
                radius: parent.radius - 2
                color: "#f8fbff"
                opacity: control.enabled
                    ? (control.up.pressed ? 0.05 : (upHover.hovered ? 0.09 : 0.06))
                    : 0.02
            }

            StepperGlyph {
                anchors.centerIn: parent
                width: 10
                height: 10
                plus: true
                strokeColor: !control.enabled
                    ? "#64748b"
                    : (control.up.pressed ? "#ffffff" : (upHover.hovered ? "#f2f8ff" : "#deebfa"))
                outlineColor: !control.enabled ? "#0b1220" : "#091220"
            }
        }

        down.indicator: Rectangle {
            id: downIndicator
            x: control.width - width - control.indicatorInset
            y: upIndicator.y + upIndicator.height + control.indicatorGap
            width: control.indicatorWidth
            height: upIndicator.height
            radius: 6
            color: !control.enabled
                ? "#0f172a"
                : (control.down.pressed
                    ? "#18273a"
                    : (downHover.hovered ? "#1c2d44" : "#162538"))
            border.width: 1
            border.color: control.enabled
                ? (control.down.pressed ? "#7d9bbd" : (downHover.hovered ? "#67819f" : "#4e627b"))
                : "#273244"

            HoverHandler {
                id: downHover
                enabled: control.enabled
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                radius: parent.radius - 1
                color: "transparent"
                border.width: downHover.hovered || control.down.pressed ? 1 : 0
                border.color: control.down.pressed ? "#b7d4f4" : "#8fb2d8"
                opacity: control.down.pressed ? 0.28 : 0.14
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 2
                height: Math.max(2, Math.floor(parent.height * 0.42))
                radius: parent.radius - 2
                color: "#f8fbff"
                opacity: control.enabled
                    ? (control.down.pressed ? 0.04 : (downHover.hovered ? 0.08 : 0.05))
                    : 0.02
            }

            StepperGlyph {
                anchors.centerIn: parent
                width: 10
                height: 10
                plus: false
                strokeColor: !control.enabled
                    ? "#64748b"
                    : (control.down.pressed
                        ? "#ffffff"
                        : (downHover.hovered ? "#f2f8ff" : "#deebfa"))
                outlineColor: !control.enabled ? "#0b1220" : "#091220"
            }
        }
    }

    background: Rectangle {
        color: "#0b1220"
        radius: root.pageCornerRadius
        antialiasing: true
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: Math.max(pageContent.y + pageContent.height + 24, height)

        Item {
            id: pageContent
            readonly property int horizontalPadding: 16
            readonly property int availableContentWidth: Math.max(0, flickable.width - (horizontalPadding * 2))

            x: Math.max(horizontalPadding, Math.round((flickable.width - width) / 2))
            y: 16
            width: Math.min(availableContentWidth, root.contentMaxWidth)
            height: settingsCard.height

            Rectangle {
                id: settingsCard
                width: parent.width
                height: Math.max(settingsLayout.implicitHeight + 32, flickable.height - 32)
                radius: 12
                color: "#111827"
                border.width: 1
                border.color: "#314056"

                ColumnLayout {
                    id: settingsLayout
                    x: 16
                    y: 16
                    width: parent.width - 32
                    height: parent.height - 32
                    spacing: 14

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        FieldLabel {
                            text: "Warning Temperature"
                            Layout.preferredWidth: 180
                            Layout.alignment: Qt.AlignVCenter
                        }

                        StyledSpinBox {
                            Layout.alignment: Qt.AlignVCenter
                            from: 0
                            to: 199
                            value: session.draft.warningTemperature
                            onValueModified: session.draft.warningTemperature = value
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        FieldLabel {
                            text: "Fault Temperature"
                            Layout.preferredWidth: 180
                            Layout.alignment: Qt.AlignVCenter
                        }

                        StyledSpinBox {
                            Layout.alignment: Qt.AlignVCenter
                            from: 1
                            to: 200
                            value: session.draft.faultTemperature
                            onValueModified: session.draft.faultTemperature = value
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        FieldLabel {
                            text: "Warning Pressure"
                            Layout.preferredWidth: 180
                            Layout.alignment: Qt.AlignVCenter
                        }

                        StyledSpinBox {
                            Layout.alignment: Qt.AlignVCenter
                            from: 0
                            to: 149
                            value: session.draft.warningPressure
                            onValueModified: session.draft.warningPressure = value
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        FieldLabel {
                            text: "Fault Pressure"
                            Layout.preferredWidth: 180
                            Layout.alignment: Qt.AlignVCenter
                        }

                        StyledSpinBox {
                            Layout.alignment: Qt.AlignVCenter
                            from: 1
                            to: 150
                            value: session.draft.faultPressure
                            onValueModified: session.draft.faultPressure = value
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        FieldLabel {
                            text: "Update Interval (ms)"
                            Layout.preferredWidth: 180
                            Layout.alignment: Qt.AlignVCenter
                        }

                        StyledSpinBox {
                            Layout.alignment: Qt.AlignVCenter
                            from: 100
                            to: 5000
                            stepSize: 100
                            value: session.draft.updateIntervalMs
                            onValueModified: session.draft.updateIntervalMs = value
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Item {
                            Layout.fillWidth: true
                        }

                        ActionButton {
                            text: "Apply"
                            fillColor: "#112419"
                            hoverFillColor: "#173022"
                            downFillColor: "#0d1b14"
                            borderColor: "#3d7a5a"
                            foregroundColor: "#d9f4e5"
                            enabled: session.applyEnabled
                            disabledReason: session.applyRestrictionReason
                            onClicked: session.apply()
                        }

                        ActionButton {
                            text: "Revert"
                            enabled: session.draft.dirty
                            onClicked: session.reload()
                        }

                        ActionButton {
                            text: "Defaults"
                            borderColor: "#8b6b2f"
                            foregroundColor: "#e3cf9b"
                            onClicked: session.draft.resetDraftToDefaults()
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignRight
                        text: session.draft.validationMessage
                        visible: text.length > 0
                        color: "#f59e0b"
                        horizontalAlignment: Text.AlignRight
                        wrapMode: Text.Wrap
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignRight
                        text: session.applyRestrictionReason
                        visible: text.length > 0
                        color: "#9ca3af"
                        horizontalAlignment: Text.AlignRight
                        wrapMode: Text.Wrap
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignRight
                        text: session.draft.dirty ? "Draft has unapplied changes." : "Draft matches committed settings."
                        color: session.draft.dirty ? "#93c5fd" : "#6b7280"
                        horizontalAlignment: Text.AlignRight
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }
}
