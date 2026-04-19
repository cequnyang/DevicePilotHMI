import QtQuick

Item {
    id: root
    clip: true

    property var samples: []
    property color lineColor: "#93c5fd"
    property color gridColor: "#243041"
    property color labelColor: "#cbd5e1"
    property real minValue: 0
    property real maxValue: 100

    // marker example:
    // { sampleIndex: 12, kind: "warning", label: "Warning", color: "#f59e0b" }
    property var markers: []
    property int historyStartSampleIndex: 0

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

            const range = Math.max(1, root.maxValue - root.minValue)

            function clamp(value, low, high) {
                return Math.max(low, Math.min(high, value))
            }

            function sampleX(index, count) {
                if (count <= 1)
                    return w / 2
                return (index / (count - 1)) * (w - 1)
            }

            function sampleY(value) {
                const normalized = clamp((Number(value) - root.minValue) / range, 0, 1)
                return h - normalized * (h - 1)
            }

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

                    const x = sampleX(logicalIndex, values.length)
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
                        const boxX = clamp(x + 4, 0, w - boxWidth)
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
                const x = sampleX(i, values.length)
                const y = sampleY(values[i])

                if (i === 0)
                    ctx.moveTo(x, y)
                else
                    ctx.lineTo(x, y)
            }

            ctx.stroke()

            // latest point
            const lastIndex = values.length - 1
            const lastX = sampleX(lastIndex, values.length)
            const lastY = sampleY(values[lastIndex])

            ctx.fillStyle = root.lineColor
            ctx.beginPath()
            ctx.arc(lastX, lastY, 3, 0, Math.PI * 2)
            ctx.fill()
        }
    }

    onSamplesChanged: canvas.requestPaint()
    onMarkersChanged: canvas.requestPaint()
    onHistoryStartSampleIndexChanged: canvas.requestPaint()
    onMinValueChanged: canvas.requestPaint()
    onMaxValueChanged: canvas.requestPaint()
    onLineColorChanged: canvas.requestPaint()
    onGridColorChanged: canvas.requestPaint()
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()
}
