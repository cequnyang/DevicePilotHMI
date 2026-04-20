import QtQuick
import QtQuick.Controls

Item {
    id: root
    clip: true

    property var samples: []
    property color lineColor: "#93c5fd"
    property color gridColor: "#243041"
    property color labelColor: "#cbd5e1"
    property real minValue: 0
    property real maxValue: 100
    property bool autoScale: false
    property real autoScalePaddingRatio: 0.08
    property real minimumAutoRange: 1
    property int valueDecimals: 1
    property string valueSuffix: ""
    property color hoverGuideColor: "#64748b"
    property color hoverLabelBackgroundColor: "#0f172a"
    property color hoverLabelBorderColor: "#334155"
    property color hoverLabelTextColor: "#f8fafc"
    property bool hoverPointerActive: false
    property real hoverPointerX: -1
    property int hoveredSampleIndex: -1

    // marker example:
    // { sampleIndex: 12, kind: "warning", label: "Warning", color: "#f59e0b" }
    property var markers: []
    property int historyStartSampleIndex: 0

    readonly property int sampleCount: root.samples ? root.samples.length : 0
    readonly property bool hoverActive: root.hoveredSampleIndex >= 0
        && root.hoveredSampleIndex < root.sampleCount
    readonly property real hoveredValue: root.hoverActive
        ? Number(root.samples[root.hoveredSampleIndex])
        : 0
    readonly property string hoveredValueText: root.hoverActive
        ? root.formatValue(root.hoveredValue)
        : ""
    readonly property real effectiveMinValue: {
        let result = root.minValue
        if (!root.autoScale || !root.samples || root.samples.length === 0)
            return result

        let sampleMin = Infinity
        let sampleMax = -Infinity
        for (let i = 0; i < root.samples.length; ++i) {
            const numericValue = Number(root.samples[i])
            if (!isFinite(numericValue))
                continue
            sampleMin = Math.min(sampleMin, numericValue)
            sampleMax = Math.max(sampleMax, numericValue)
        }

        if (sampleMin === Infinity || sampleMax === -Infinity)
            return result

        const sampleRange = sampleMax - sampleMin
        const padding = sampleRange > 0
            ? Math.max(sampleRange * root.autoScalePaddingRatio, root.minimumAutoRange * 0.5)
            : Math.max(Math.abs(sampleMin) * root.autoScalePaddingRatio, root.minimumAutoRange * 0.5)
        return sampleMin - padding
    }
    readonly property real effectiveMaxValue: {
        let result = root.maxValue
        if (!root.autoScale || !root.samples || root.samples.length === 0)
            return result

        let sampleMin = Infinity
        let sampleMax = -Infinity
        for (let i = 0; i < root.samples.length; ++i) {
            const numericValue = Number(root.samples[i])
            if (!isFinite(numericValue))
                continue
            sampleMin = Math.min(sampleMin, numericValue)
            sampleMax = Math.max(sampleMax, numericValue)
        }

        if (sampleMin === Infinity || sampleMax === -Infinity)
            return result

        const sampleRange = sampleMax - sampleMin
        const padding = sampleRange > 0
            ? Math.max(sampleRange * root.autoScalePaddingRatio, root.minimumAutoRange * 0.5)
            : Math.max(Math.abs(sampleMin) * root.autoScalePaddingRatio, root.minimumAutoRange * 0.5)
        return sampleMax + padding
    }
    readonly property real effectiveRange: Math.max(root.minimumAutoRange,
                                                    root.effectiveMaxValue - root.effectiveMinValue)

    function clamp(value, low, high) {
        return Math.max(low, Math.min(high, value))
    }

    function sampleX(index, widthValue) {
        if (root.sampleCount <= 1)
            return widthValue / 2
        return (index / (root.sampleCount - 1)) * (widthValue - 1)
    }

    function sampleY(value, heightValue) {
        const normalized = root.clamp((Number(value) - root.effectiveMinValue) / root.effectiveRange, 0, 1)
        return heightValue - normalized * (heightValue - 1)
    }

    function sampleIndexForX(x, widthValue) {
        if (root.sampleCount <= 0)
            return -1
        if (root.sampleCount === 1)
            return 0

        const ratio = root.clamp(x / Math.max(1, widthValue - 1), 0, 1)
        return Math.round(ratio * (root.sampleCount - 1))
    }

    function formatValue(value) {
        const numericValue = Number(value)
        if (!isFinite(numericValue))
            return "--"
        return numericValue.toFixed(root.valueDecimals)
            + (root.valueSuffix.length > 0 ? " " + root.valueSuffix : "")
    }

    function updateHoveredSampleIndex(widthValue) {
        if (!root.hoverPointerActive) {
            root.hoveredSampleIndex = -1
            return
        }

        const targetWidth = widthValue !== undefined ? widthValue : root.width
        root.hoveredSampleIndex = root.sampleIndexForX(root.hoverPointerX, targetWidth)
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()

            const values = root.samples
            const w = width
            const h = height

            if (w <= 0 || h <= 0)
                return

            // background grid
            ctx.strokeStyle = root.gridColor
            ctx.lineWidth = 1

            for (let i = 0; i <= 3; ++i) {
                const y = i * (h - 1) / 3
                ctx.beginPath()
                ctx.moveTo(0, y)
                ctx.lineTo(w, y)
                ctx.stroke()
            }

            // event markers
            if (root.markers && values && values.length > 0) {
                for (let i = 0; i < root.markers.length; ++i) {
                    const marker = root.markers[i]
                    if (!marker || marker.sampleIndex === undefined)
                        continue

                    const logicalIndex = Number(marker.sampleIndex) - root.historyStartSampleIndex
                    if (logicalIndex < 0 || logicalIndex >= values.length)
                        continue

                    const x = root.sampleX(logicalIndex, w)
                    const markerColor = marker.color || "#f8fafc"
                    const label = marker.label || ""

                    ctx.strokeStyle = markerColor
                    ctx.lineWidth = 1
                    ctx.beginPath()
                    ctx.moveTo(x, 0)
                    ctx.lineTo(x, h)
                    ctx.stroke()

                    if (label.length > 0) {
                        ctx.font = "11px sans-serif"
                        const textWidth = ctx.measureText(label).width
                        const boxWidth = textWidth + 8
                        const boxHeight = 16
                        const boxX = root.clamp(x + 4, 0, w - boxWidth)
                        const boxY = 2

                        ctx.fillStyle = "#0f172a"
                        ctx.fillRect(boxX, boxY, boxWidth, boxHeight)

                        ctx.strokeStyle = markerColor
                        ctx.strokeRect(boxX, boxY, boxWidth, boxHeight)

                        ctx.fillStyle = root.labelColor
                        ctx.fillText(label, boxX + 4, boxY + 12)
                    }
                }
            }

            // trend line
            if (!values || values.length < 2)
                return

            ctx.strokeStyle = root.lineColor
            ctx.lineWidth = 2
            ctx.beginPath()

            for (let i = 0; i < values.length; ++i) {
                const x = root.sampleX(i, w)
                const y = root.sampleY(values[i], h)

                if (i === 0)
                    ctx.moveTo(x, y)
                else
                    ctx.lineTo(x, y)
            }

            ctx.stroke()

            // latest point
            const lastIndex = values.length - 1
            const lastX = root.sampleX(lastIndex, w)
            const lastY = root.sampleY(values[lastIndex], h)

            ctx.fillStyle = root.lineColor
            ctx.beginPath()
            ctx.arc(lastX, lastY, 3, 0, Math.PI * 2)
            ctx.fill()

            if (root.hoverActive) {
                const hoverX = root.sampleX(root.hoveredSampleIndex, w)
                const hoverY = root.sampleY(root.hoveredValue, h)

                ctx.strokeStyle = root.hoverGuideColor
                ctx.lineWidth = 1
                ctx.beginPath()
                ctx.moveTo(hoverX, 0)
                ctx.lineTo(hoverX, h)
                ctx.stroke()

                ctx.fillStyle = root.lineColor
                ctx.beginPath()
                ctx.arc(hoverX, hoverY, 4, 0, Math.PI * 2)
                ctx.fill()

                ctx.strokeStyle = "#f8fafc"
                ctx.lineWidth = 2
                ctx.beginPath()
                ctx.arc(hoverX, hoverY, 6, 0, Math.PI * 2)
                ctx.stroke()
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
        cursorShape: root.sampleCount > 0 ? Qt.CrossCursor : Qt.ArrowCursor

        onPositionChanged: mouse => {
            root.hoverPointerActive = true
            root.hoverPointerX = mouse.x
            root.updateHoveredSampleIndex(width)
        }
        onExited: {
            root.hoverPointerActive = false
            root.hoverPointerX = -1
            root.hoveredSampleIndex = -1
        }
    }

    Rectangle {
        visible: root.hoverActive
        z: 1
        radius: 6
        color: root.hoverLabelBackgroundColor
        border.width: 1
        border.color: root.hoverLabelBorderColor
        x: root.clamp(root.sampleX(root.hoveredSampleIndex, root.width) - width / 2,
                      0,
                      root.width - width)
        y: {
            const pointY = root.sampleY(root.hoveredValue, root.height)
            const desiredAboveY = pointY - height - 8
            if (desiredAboveY >= 0)
                return desiredAboveY
            return root.clamp(pointY + 8, 0, root.height - height)
        }
        implicitWidth: hoverLabel.implicitWidth + 12
        implicitHeight: hoverLabel.implicitHeight + 8

        Label {
            id: hoverLabel
            anchors.centerIn: parent
            text: root.hoveredValueText
            color: root.hoverLabelTextColor
            font.pixelSize: 12
            font.bold: true
        }
    }

    onSamplesChanged: {
        root.updateHoveredSampleIndex()
        canvas.requestPaint()
    }
    onMarkersChanged: canvas.requestPaint()
    onHistoryStartSampleIndexChanged: canvas.requestPaint()
    onMinValueChanged: canvas.requestPaint()
    onMaxValueChanged: canvas.requestPaint()
    onAutoScaleChanged: canvas.requestPaint()
    onAutoScalePaddingRatioChanged: canvas.requestPaint()
    onMinimumAutoRangeChanged: canvas.requestPaint()
    onHoveredSampleIndexChanged: canvas.requestPaint()
    onLineColorChanged: canvas.requestPaint()
    onGridColorChanged: canvas.requestPaint()
    onWidthChanged: {
        root.updateHoveredSampleIndex()
        canvas.requestPaint()
    }
    onHeightChanged: canvas.requestPaint()
}
