import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DevicePilotHMI

Page {
    id: root

    required property var session
    property int pageCornerRadius: 18
    property int contentMaxWidth: 980
    property int compareRowInnerMargin: 14
    property int compareColumnSpacing: 14
    property int compareParameterWidth: 220
    property int compareLiveWidth: 132
    property int compareDraftWidth: 160
    clip: true

    function formatSettingValue(value, unitLabel) {
        return unitLabel.length > 0 ? value + " " + unitLabel : String(value)
    }

    function hasPendingDelta(liveValue, draftValue) {
        return liveValue !== draftValue
    }

    function pendingDeltaText(liveValue, draftValue, unitLabel) {
        const diff = draftValue - liveValue
        if (diff === 0)
            return "Live"

        const prefix = diff > 0 ? "+" : ""
        const suffix = unitLabel.length > 0 ? " " + unitLabel : ""
        return prefix + diff + suffix + " pending"
    }

    function pendingCountText() {
        if (session.pendingChangeCount === 0)
            return "No pending changes"

        return session.pendingChangeCount + (session.pendingChangeCount === 1
            ? " pending change"
            : " pending changes")
    }

    function draftStatusText() {
        if (!session.draft.dirty)
            return "Draft matches live settings."
        if (session.draft.validationMessage.length > 0)
            return session.draft.validationMessage
        if (session.applyRestrictionReason.length > 0)
            return session.applyRestrictionReason
        if (session.applyEnabled)
            return "Draft is valid and ready to apply."
        return "Draft has unapplied changes."
    }

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

    component PresetButton : ActionButton {
        property bool selected: false

        fillColor: selected ? "#17304a" : "#111827"
        hoverFillColor: selected ? "#214465" : "#172033"
        downFillColor: selected ? "#11273d" : "#0d1422"
        borderColor: selected ? "#78a6cb" : "#475569"
        foregroundColor: selected ? "#f3f8fe" : "#dbe6f1"
    }

    component StatusPill : Rectangle {
        id: pill

        property string text: ""
        property color fillColor: "#101926"
        property color borderColor: "#334155"
        property color foregroundColor: "#e5e7eb"

        implicitHeight: 30
        implicitWidth: Math.max(86, statusText.implicitWidth + 20)
        radius: 15
        color: fillColor
        border.width: 1
        border.color: borderColor

        Text {
            id: statusText
            anchors.centerIn: parent
            text: pill.text
            color: pill.foregroundColor
            font.pixelSize: 12
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }
    }

    component SummaryTile : Item {
        id: tile

        required property string titleText
        required property string valueText
        property string noteText: ""
        property color accentColor: "#8cc7f2"
        property int accentWidth: 6
        property int cornerRadius: 12

        Layout.fillWidth: true
        implicitHeight: noteText.length > 0 ? 110 : 90

        Rectangle {
            anchors.fill: parent
            radius: tile.cornerRadius
            color: tile.accentColor
            opacity: 0.88
        }

        Rectangle {
            id: tileSurface
            anchors.fill: parent
            anchors.leftMargin: tile.accentWidth
            radius: tile.cornerRadius
            color: "#0d1522"
        }

        Rectangle {
            anchors.fill: parent
            radius: tile.cornerRadius
            color: "transparent"
            border.width: 1
            border.color: "#243447"
        }

        ColumnLayout {
            anchors.fill: tileSurface
            anchors.margins: 14
            spacing: 6

            FieldLabel {
                text: tile.titleText
                Layout.fillWidth: true
            }

            Label {
                Layout.fillWidth: true
                text: tile.valueText
                color: "#f3f8fe"
                font.pixelSize: 22
                font.weight: Font.DemiBold
                wrapMode: Text.Wrap
            }

            Label {
                Layout.fillWidth: true
                text: tile.noteText
                visible: text.length > 0
                color: "#9fb1c6"
                font.pixelSize: 12
                wrapMode: Text.Wrap
            }
        }
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

    component CompareRow : Rectangle {
        id: rowCard

        required property string labelText
        required property string liveValueText
        required property string statusText
        required property bool changed
        default property alias editorContent: editorSlot.data

        implicitHeight: 76
        radius: 12
        color: changed ? "#101b2a" : "#0d1522"
        border.width: 1
        border.color: changed ? "#365978" : "#243447"

        RowLayout {
            anchors.fill: parent
            anchors.margins: root.compareRowInnerMargin
            spacing: root.compareColumnSpacing

            ColumnLayout {
                Layout.preferredWidth: root.compareParameterWidth
                Layout.minimumWidth: root.compareParameterWidth
                Layout.maximumWidth: root.compareParameterWidth
                Layout.alignment: Qt.AlignVCenter
                spacing: 4

                Label {
                    text: rowCard.labelText
                    color: "#f3f8fe"
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }

                FieldLabel {
                    text: rowCard.changed ? "Draft diverges from live value." : "Draft matches live value."
                    Layout.fillWidth: true
                }
            }

            Item {
                Layout.fillWidth: true
            }

            ColumnLayout {
                Layout.preferredWidth: root.compareLiveWidth
                Layout.minimumWidth: root.compareLiveWidth
                Layout.maximumWidth: root.compareLiveWidth
                Layout.alignment: Qt.AlignVCenter

                FieldLabel {
                    text: "Live"
                    visible: false
                }

                StatusPill {
                    text: rowCard.liveValueText
                    fillColor: "#0f172a"
                    borderColor: "#334155"
                    foregroundColor: "#dbe6f1"
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            Item {
                Layout.fillWidth: true
            }

            ColumnLayout {
                Layout.preferredWidth: root.compareDraftWidth
                Layout.minimumWidth: root.compareDraftWidth
                Layout.maximumWidth: root.compareDraftWidth
                Layout.alignment: Qt.AlignVCenter

                FieldLabel {
                    text: "Draft"
                    visible: false
                }

                Item {
                    id: editorSlot
                    implicitWidth: childrenRect.width
                    implicitHeight: childrenRect.height
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            Item {
                Layout.fillWidth: true
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignVCenter

                FieldLabel {
                    text: "Pending"
                    visible: false
                }

                StatusPill {
                    text: rowCard.statusText
                    fillColor: rowCard.changed ? "#17304a" : "#101926"
                    borderColor: rowCard.changed ? "#78a6cb" : "#334155"
                    foregroundColor: rowCard.changed ? "#f3f8fe" : "#94a3b8"
                    Layout.alignment: Qt.AlignRight
                }
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
                    spacing: 14

                    Label {
                        Layout.fillWidth: true
                        text: "Settings Profiles"
                        color: "#f8fafc"
                        font.pixelSize: 26
                        font.weight: Font.Bold
                    }

                    Label {
                        Layout.fillWidth: true
                        text: "Load threshold presets, compare live values against your working draft, and apply only when the runtime policy allows it."
                        color: "#9fb1c6"
                        font.pixelSize: 13
                        wrapMode: Text.Wrap
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: settingsLayout.width >= 860 ? 3 : (settingsLayout.width >= 560 ? 2 : 1)
                        rowSpacing: 12
                        columnSpacing: 12

                        SummaryTile {
                            titleText: "Live Threshold Profile"
                            valueText: session.committedThresholdPresetName
                            noteText: "Applied thresholds currently driving warning and fault behavior."
                            accentColor: "#38bdf8"
                        }

                        SummaryTile {
                            titleText: "Draft Threshold Profile"
                            valueText: session.draftThresholdPresetName
                            noteText: "Preset buttons only reshape threshold bands. Update interval stays manual."
                            accentColor: "#22c55e"
                        }

                        SummaryTile {
                            titleText: "Pending Changes"
                            valueText: root.pendingCountText()
                            noteText: root.draftStatusText()
                            accentColor: session.pendingChangeCount > 0 ? "#f59e0b" : "#64748b"
                        }
                    }

                    Rectangle {
                        id: presetCard
                        Layout.fillWidth: true
                        implicitHeight: presetContent.implicitHeight + 28
                        radius: 12
                        color: "#0d1522"
                        border.width: 1
                        border.color: "#243447"
                        clip: true

                        ColumnLayout {
                            id: presetContent
                            x: 14
                            y: 14
                            width: parent.width - 28
                            spacing: 12

                            RowLayout {
                                Layout.fillWidth: true

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    Label {
                                        text: "Threshold Presets"
                                        color: "#f3f8fe"
                                        font.pixelSize: 16
                                        font.weight: Font.DemiBold
                                    }

                                    FieldLabel {
                                        Layout.fillWidth: true
                                        text: "Presets update only the threshold bands. Use Factory Defaults below to reset the entire settings block."
                                        wrapMode: Text.Wrap
                                    }
                                }
                            }

                            Flow {
                                Layout.fillWidth: true
                                spacing: 10

                                PresetButton {
                                    text: "Conservative"
                                    selected: session.draftThresholdPresetName === "Conservative"
                                    onClicked: session.loadConservativePreset()
                                }

                                PresetButton {
                                    text: "Balanced"
                                    selected: session.draftThresholdPresetName === "Balanced"
                                    onClicked: session.loadBalancedPreset()
                                }

                                PresetButton {
                                    text: "Aggressive"
                                    selected: session.draftThresholdPresetName === "Aggressive"
                                    onClicked: session.loadAggressivePreset()
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.topMargin: 6
                        implicitHeight: compareHeaderLayout.implicitHeight

                        RowLayout {
                            id: compareHeaderLayout
                            anchors.fill: parent
                            anchors.leftMargin: root.compareRowInnerMargin
                            anchors.rightMargin: root.compareRowInnerMargin
                            spacing: root.compareColumnSpacing

                            FieldLabel {
                                text: "Parameter"
                                Layout.preferredWidth: root.compareParameterWidth
                                Layout.minimumWidth: root.compareParameterWidth
                                Layout.maximumWidth: root.compareParameterWidth
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            Item {
                                Layout.preferredWidth: root.compareLiveWidth
                                Layout.minimumWidth: root.compareLiveWidth
                                Layout.maximumWidth: root.compareLiveWidth

                                FieldLabel {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: "Live"
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            Item {
                                Layout.preferredWidth: root.compareDraftWidth
                                Layout.minimumWidth: root.compareDraftWidth
                                Layout.maximumWidth: root.compareDraftWidth

                                FieldLabel {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: "Draft"
                                }
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            FieldLabel {
                                text: "Pending"
                            }
                        }
                    }

                    CompareRow {
                        Layout.fillWidth: true
                        labelText: "Warning Temperature"
                        liveValueText: root.formatSettingValue(session.committedWarningTemperature, "")
                        statusText: root.pendingDeltaText(session.committedWarningTemperature, session.draft.warningTemperature, "")
                        changed: root.hasPendingDelta(session.committedWarningTemperature, session.draft.warningTemperature)

                        StyledSpinBox {
                            from: 0
                            to: 199
                            value: session.draft.warningTemperature
                            onValueModified: session.draft.warningTemperature = value
                        }
                    }

                    CompareRow {
                        Layout.fillWidth: true
                        labelText: "Fault Temperature"
                        liveValueText: root.formatSettingValue(session.committedFaultTemperature, "")
                        statusText: root.pendingDeltaText(session.committedFaultTemperature, session.draft.faultTemperature, "")
                        changed: root.hasPendingDelta(session.committedFaultTemperature, session.draft.faultTemperature)

                        StyledSpinBox {
                            from: 1
                            to: 200
                            value: session.draft.faultTemperature
                            onValueModified: session.draft.faultTemperature = value
                        }
                    }

                    CompareRow {
                        Layout.fillWidth: true
                        labelText: "Warning Pressure"
                        liveValueText: root.formatSettingValue(session.committedWarningPressure, "")
                        statusText: root.pendingDeltaText(session.committedWarningPressure, session.draft.warningPressure, "")
                        changed: root.hasPendingDelta(session.committedWarningPressure, session.draft.warningPressure)

                        StyledSpinBox {
                            from: 0
                            to: 149
                            value: session.draft.warningPressure
                            onValueModified: session.draft.warningPressure = value
                        }
                    }

                    CompareRow {
                        Layout.fillWidth: true
                        labelText: "Fault Pressure"
                        liveValueText: root.formatSettingValue(session.committedFaultPressure, "")
                        statusText: root.pendingDeltaText(session.committedFaultPressure, session.draft.faultPressure, "")
                        changed: root.hasPendingDelta(session.committedFaultPressure, session.draft.faultPressure)

                        StyledSpinBox {
                            from: 1
                            to: 150
                            value: session.draft.faultPressure
                            onValueModified: session.draft.faultPressure = value
                        }
                    }

                    CompareRow {
                        Layout.fillWidth: true
                        labelText: "Update Interval (ms)"
                        liveValueText: root.formatSettingValue(session.committedUpdateIntervalMs, "ms")
                        statusText: root.pendingDeltaText(session.committedUpdateIntervalMs, session.draft.updateIntervalMs, "ms")
                        changed: root.hasPendingDelta(session.committedUpdateIntervalMs, session.draft.updateIntervalMs)

                        StyledSpinBox {
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
                            text: "Factory Defaults"
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
                        text: root.draftStatusText()
                        color: session.draft.dirty ? "#93c5fd" : "#6b7280"
                        horizontalAlignment: Text.AlignRight
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }
}
