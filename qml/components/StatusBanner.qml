import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DevicePilotHMI

Rectangle {
    id: root

    required property string alarmText
    required property bool hasWarning
    required property bool isFault
    property bool embedded: false
    readonly property bool alertActive: root.hasWarning || root.isFault
    readonly property string statusText: root.alertActive ? root.alarmText : "System normal"
    readonly property string stateLabel: root.isFault ? "Fault" : (root.hasWarning ? "Warning" : "Normal")
    readonly property color severityColor: root.isFault ? "#ef4444" : (root.hasWarning ? "#f59e0b" : "#22c55e")
    readonly property color embeddedBorderColor: root.isFault ? "#6b3941" : (root.hasWarning ? "#6a5a3b" : "#3f536a")
    readonly property int marqueeGap: 28

    implicitHeight: root.embedded ? 36 : 40
    radius: root.embedded ? 10 : 12
    color: root.embedded
        ? (root.isFault ? "#18141a" : (root.hasWarning ? "#1a1713" : "#0e18265e"))
        : (root.isFault ? "#2c1313" : (root.hasWarning ? "#2a1f0a" : "#0f172a"))
    border.width: 1
    border.color: root.embedded
        ? root.embeddedBorderColor
        : (root.isFault ? "#7f1d1d" : (root.hasWarning ? "#92400e" : "#2b3648"))

    Rectangle {
        visible: root.embedded && root.alertActive
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 1
        anchors.topMargin: 6
        anchors.bottomMargin: 6
        width: 3
        radius: 2
        color: root.severityColor
        opacity: 0.72
    }


    Rectangle {
        visible: root.embedded
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 1
        anchors.rightMargin: 1
        anchors.topMargin: 1
        height: 1
        radius: 1
        color: "#ecf5ff"
        opacity: root.alertActive ? 0.09 : 0.06
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: root.embedded ? 12 : 14
        anchors.rightMargin: root.embedded ? 10 : 12
        anchors.topMargin: root.embedded ? 5 : 6
        anchors.bottomMargin: root.embedded ? 5 : 6
        spacing: root.embedded ? 8 : 10

        Rectangle {
            implicitWidth: root.embedded ? 7 : 8
            implicitHeight: root.embedded ? 7 : 8
            radius: width / 2
            color: root.severityColor
            Layout.alignment: Qt.AlignVCenter
        }

        Item {
            id: marqueeViewport
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            implicitHeight: marqueeText.implicitHeight
            clip: true

            readonly property bool marqueeActive: marqueeText.implicitWidth > width
            onMarqueeActiveChanged: {
                if (!marqueeActive)
                    marqueeTrack.x = 0
            }

            Item {
                id: marqueeTrack
                width: marqueeActive
                    ? (marqueeText.implicitWidth * 2) + root.marqueeGap
                    : marqueeText.implicitWidth
                height: parent.height

                Text {
                    id: marqueeText
                    anchors.verticalCenter: parent.verticalCenter
                    text: root.statusText
                    color: root.alertActive ? "#f8fafc" : (root.embedded ? "#d4deea" : "#cbd5e1")
                    font.pixelSize: root.embedded ? 12 : 13
                    font.weight: root.embedded ? Font.Medium : Font.DemiBold
                }

                Text {
                    visible: parent.parent.marqueeActive
                    anchors.verticalCenter: parent.verticalCenter
                    x: marqueeText.implicitWidth + root.marqueeGap
                    text: root.statusText
                    color: marqueeText.color
                    font.pixelSize: marqueeText.font.pixelSize
                    font.weight: marqueeText.font.weight
                }
            }

            SequentialAnimation {
                running: marqueeViewport.marqueeActive
                loops: Animation.Infinite

                PauseAnimation {
                    duration: 1000
                }

                NumberAnimation {
                    target: marqueeTrack
                    property: "x"
                    from: 0
                    to: -(marqueeText.implicitWidth + root.marqueeGap)
                    duration: Math.max(3500, marqueeText.implicitWidth * 28)
                    easing.type: Easing.Linear
                }

                PauseAnimation {
                    duration: 700
                }
            }
        }

        Rectangle {
            radius: 999
            color: root.embedded
                ? (root.isFault ? "#4a1818a0" : (root.hasWarning ? "#4a3411a0" : "#13223480"))
                : (root.isFault ? "#3a1414" : (root.hasWarning ? "#3a2a0f" : "#132033"))
            border.width: 1
            border.color: root.embedded
                ? (root.isFault ? "#7d4a4ac0" : (root.hasWarning ? "#8a6a36c0" : "#4b627b"))
                : (root.isFault ? "#7f1d1d" : (root.hasWarning ? "#92400e" : "#334155"))
            implicitHeight: root.embedded ? 20 : 22
            implicitWidth: stateLabelText.implicitWidth + (root.embedded ? 12 : 14)
            Layout.alignment: Qt.AlignVCenter

            Label {
                id: stateLabelText
                anchors.centerIn: parent
                text: root.stateLabel
                color: root.isFault ? "#fecaca" : (root.hasWarning ? "#fcd34d" : "#a8d0f0")
                font.pixelSize: root.embedded ? 10 : 11
                font.weight: Font.DemiBold
            }
        }
    }
}
