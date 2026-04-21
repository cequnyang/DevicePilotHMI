import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DevicePilotHMI

Page {
    id: root

    required property var logModel
    required property var filteredLogModel
    property int pageCornerRadius: 18
    clip: true

    property string selectedLevel: ""
    property bool newestFirst: true
    readonly property var levelOptions: [
        { label: "All", value: "" },
        { label: "CONFIG", value: "CONFIG" },
        { label: "INFO", value: "INFO" },
        { label: "WARNING", value: "WARNING" },
        { label: "FAULT", value: "FAULT" }
    ]

    function levelOptionIndexFor(value) {
        for (let i = 0; i < levelOptions.length; ++i) {
            if (levelOptions[i].value === value)
                return i
        }
        return 0
    }

    component SearchField : TextField {
        id: control

        implicitHeight: 40
        leftPadding: 14
        rightPadding: 14
        topPadding: 9
        bottomPadding: 9
        hoverEnabled: true
        font.pixelSize: 14
        font.weight: Font.Medium
        color: "#e5e7eb"
        placeholderTextColor: "#64748b"
        selectedTextColor: "#f8fafc"
        selectionColor: "#37506f"

        background: Rectangle {
            radius: 10
            color: activeFocus
                ? "#122030"
                : (control.hovered ? "#172033" : "#111827")
            border.width: 1
            border.color: activeFocus ? "#5f84a4" : "#475569"
        }
    }

    component FilterComboBox : ComboBox {
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

    component LogCheckBox : AbstractButton {
        id: control

        property color accentColor: "#8cc7f2"
        property color boxColor: checked ? "#8cc7f2" : (hovered ? "#172033" : "#111827")
        property color boxBorderColor: checked ? "#b7e3ff" : (hovered ? "#5f748d" : "#475569")
        property color markColor: "#08111d"
        property color labelColor: enabled ? "#d1d5db" : "#64748b"
        property int boxSize: 18
        property int labelSpacing: text.length > 0 ? 10 : 0

        checkable: true
        hoverEnabled: true
        implicitWidth: contentRow.implicitWidth
        implicitHeight: Math.max(control.boxSize, contentRow.implicitHeight)
        focusPolicy: Qt.NoFocus
        padding: 0

        background: Item {}

        contentItem: Row {
            id: contentRow
            spacing: control.labelSpacing

            Rectangle {
                width: control.boxSize
                height: control.boxSize
                radius: 5
                color: control.boxColor
                border.width: 1
                border.color: control.boxBorderColor

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 1
                    radius: parent.radius - 1
                    color: "transparent"
                    border.width: control.hovered || control.checked ? 1 : 0
                    border.color: control.hovered && !control.checked ? "#26384d" : "#d8f0ff"
                    opacity: control.checked ? 0.18 : 0.35
                }

                Canvas {
                    anchors.centerIn: parent
                    width: 10
                    height: 8
                    visible: control.checked
                    contextType: "2d"

                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.reset()
                        ctx.strokeStyle = control.markColor
                        ctx.lineWidth = 1.9
                        ctx.lineCap = "round"
                        ctx.lineJoin = "round"
                        ctx.beginPath()
                        ctx.moveTo(1, height / 2)
                        ctx.lineTo(width / 2 - 1, height - 1)
                        ctx.lineTo(width - 1, 1)
                        ctx.stroke()
                    }
                }
            }

            Text {
                text: control.text
                visible: text.length > 0
                color: control.labelColor
                font.pixelSize: 13
                font.weight: Font.Medium
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    component SortOrderButton : AbstractButton {
        id: control

        property bool newestFirst: true
        readonly property color fillColor: pressed
            ? "#0d1422"
            : (hovered ? "#172033" : "#111827")
        readonly property color frameColor: hovered ? "#5f84a4" : "#475569"
        readonly property color glyphColor: "#d8e7f8"

        implicitWidth: 46
        implicitHeight: 32
        hoverEnabled: true
        focusPolicy: Qt.NoFocus
        padding: 0

        ToolTip.visible: hovered
        ToolTip.delay: 260
        ToolTip.text: control.newestFirst ? "Newest first" : "Oldest first"

        background: Rectangle {
            radius: 10
            color: control.fillColor
            border.width: 1
            border.color: control.frameColor
        }

        contentItem: Item {
            anchors.fill: parent

            Item {
                id: iconWrap
                anchors.centerIn: parent
                width: 24
                height: 20

                transform: Scale {
                    id: iconFlip
                    origin.x: iconWrap.width / 2
                    origin.y: iconWrap.height / 2
                    yScale: control.newestFirst ? 1 : -1
                    
                    Behavior on yScale {
                        NumberAnimation {
                            duration: 140
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                Canvas {
                    anchors.fill: parent
                    contextType: "2d"

                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.reset()
                        ctx.fillStyle = control.glyphColor
                        ctx.strokeStyle = control.glyphColor
                        ctx.lineWidth = 1.8
                        ctx.lineCap = "round"
                        ctx.lineJoin = "round"

                        function fillBar(x, y, w, h) {
                            ctx.fillRect(x, y, w, h)
                        }

                        ctx.beginPath()
                        ctx.moveTo(4.5, 17.0)
                        ctx.lineTo(4.5, 5.8)
                        ctx.stroke()

                        ctx.beginPath()
                        ctx.moveTo(4.5, 2.0)
                        ctx.lineTo(1.4, 6.0)
                        ctx.lineTo(7.6, 6.0)
                        ctx.closePath()
                        ctx.fill()

                        fillBar(11.0, 2.2, 5.0, 2.4)
                        fillBar(11.0, 6.1, 7.2, 2.4)
                        fillBar(11.0, 10.0, 9.4, 2.4)
                        fillBar(11.0, 13.9, 11.6, 2.4)
                        fillBar(11.0, 17.8, 13.0, 2.2)
                    }
                }
            }
        }
    }

    component ElidedToolTipLabel : Label {
        id: control

        property string toolTipText: text

        wrapMode: Text.NoWrap
        elide: Text.ElideRight
        maximumLineCount: 1

        HoverHandler {
            id: hover
        }

        ToolTip.visible: hover.hovered && control.toolTipText.length > 0
            && control.implicitWidth > control.width
        ToolTip.delay: 300
        ToolTip.text: control.toolTipText
    }

    function sourceColor(value) {
        switch (value) {
        case "alarm":
            return "#f59e0b"
        case "persistence":
            return "#34d399"
        case "settings":
            return "#60a5fa"
        case "runtime":
            return "#c084fc"
        default:
            return "#9ca3af"
        }
    }

    background: Rectangle {
        color: "#0b1220"
        radius: root.pageCornerRadius
        antialiasing: true
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

    Binding {
        target: root.filteredLogModel
        property: "newestFirst"
        value: root.newestFirst
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        SearchField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: "Search level, source, event type, timestamp, or message"
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            Label {
                text: "Level Filter"
                color: "#94a3b8"
                font.pixelSize: 12
                font.weight: Font.Medium
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 16

                FilterComboBox {
                    id: levelFilterCombo
                    Layout.preferredWidth: 240
                    model: root.levelOptions
                    textRole: "label"
                    currentIndex: root.levelOptionIndexFor(root.selectedLevel)

                    onActivated: {
                        root.selectedLevel = root.levelOptions[currentIndex].value
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                RowLayout {
                    spacing: 12
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                    LogCheckBox {
                        id: unackOnlyCheck
                        text: "Only unacknowledged"
                        Layout.alignment: Qt.AlignVCenter
                        labelColor: "#e5e7eb"
                    }

                    SortOrderButton {
                        id: sortOrderButton
                        newestFirst: root.newestFirst
                        Layout.alignment: Qt.AlignVCenter

                        onClicked: {
                            root.newestFirst = !root.newestFirst
                            logList.positionViewAtBeginning()
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: "#111827"
            border.width: 1
            border.color: "#314056"

            ListView {
                id: logList
                anchors.fill: parent
                anchors.margins: 8
                clip: true
                model: root.filteredLogModel
                spacing: 3

                delegate: Rectangle {
                    width: ListView.view.width
                    implicitHeight: contentRow.implicitHeight + 8
                    radius: 6
                    color: acknowledged ? "#18212b" : "#1f2937"
                    opacity: acknowledged ? 0.72 : 1.0

                    required property int index
                    required property string timestamp
                    required property string level
                    required property string source
                    required property string eventType
                    required property string message
                    required property bool acknowledged

                    RowLayout {
                        id: contentRow
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 8

                        LogCheckBox {
                            checked: acknowledged
                            text: ""
                            boxSize: 16
                            implicitWidth: 16
                            labelSpacing: 0
                            onToggled: root.filteredLogModel.setAcknowledged(index, checked)
                        }

                        Label {
                            text: timestamp
                            color: "#9ca3af"
                            font.pixelSize: 12
                            Layout.preferredWidth: 90
                        }

                        Rectangle {
                            radius: 999
                            color: Qt.rgba(0, 0, 0, 0)
                            border.width: 2
                            border.color: root.sourceColor(source)
                            implicitWidth: sourceLabel.implicitWidth + 16
                            implicitHeight: sourceLabel.implicitHeight + 6

                            Label {
                                id: sourceLabel
                                anchors.centerIn: parent
                                text: source
                                color: parent.border.color
                                font.pixelSize: 12
                                font.weight: Font.DemiBold
                            }
                        }

                        Label {
                            text: level
                            color: level === "FAULT" ? "#ef4444"
                                  : level === "WARNING" ? "#f59e0b"
                                  : level === "CONFIG" ? "#34d399"
                                  : "#93c5fd"
                            font.bold: true
                            Layout.preferredWidth: 78
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1

                            ElidedToolTipLabel {
                                text: message
                                toolTipText: message
                                color: "white"
                                Layout.fillWidth: true
                                font.strikeout: acknowledged
                            }

                            ElidedToolTipLabel {
                                text: eventType
                                toolTipText: eventType
                                color: "#9ca3af"
                                font.pixelSize: 11
                                Layout.fillWidth: true
                                visible: eventType.length > 0
                            }
                        }
                    }
                }
            }
        }
    }
}
