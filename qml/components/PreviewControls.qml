import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

Rectangle {
    id: previewControls
    height: 44
    color: Theme.headerColor

    Rectangle {
        anchors.top: parent.top
        width: parent.width
        height: 1
        color: Theme.borderColor
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: 8

        // Play/Pause button
        Rectangle {
            width: 36
            height: 28
            radius: 4
            color: playHover.containsMouse ? Theme.primaryColor : Theme.surfaceColorLight

            Text {
                anchors.centerIn: parent
                text: AnimController.playing ? "⏸" : "▶"
                color: playHover.containsMouse ? "#ffffff" : Theme.textColor
                font.pixelSize: 14
            }

            MouseArea {
                id: playHover
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: AnimController.togglePlayPause()
            }

            ToolTip.visible: playHover.containsMouse
            ToolTip.text: AnimController.playing ? qsTr("Pause (Space)") : qsTr("Play (Space)")
        }

        // Stop button
        Rectangle {
            width: 36
            height: 28
            radius: 4
            color: stopHover.containsMouse ? Theme.surfaceColorLight : Theme.surfaceColor

            Text {
                anchors.centerIn: parent
                text: "⏹"
                color: Theme.textColorDim
                font.pixelSize: 14
            }

            MouseArea {
                id: stopHover
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: AnimController.stop()
            }

            ToolTip.visible: stopHover.containsMouse
            ToolTip.text: qsTr("Stop (Home)")
        }

        Rectangle {
            width: 1
            height: 20
            color: Theme.borderColor
        }

        // Previous keyframe
        Rectangle {
            width: 28
            height: 28
            radius: 4
            color: prevHover.containsMouse ? Theme.surfaceColorLight : "transparent"

            Text {
                anchors.centerIn: parent
                text: "⏮"
                color: Theme.textColorDim
                font.pixelSize: 12
            }

            MouseArea {
                id: prevHover
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: AnimController.stepBackward()
            }
        }

        // Next keyframe
        Rectangle {
            width: 28
            height: 28
            radius: 4
            color: nextHover.containsMouse ? Theme.surfaceColorLight : "transparent"

            Text {
                anchors.centerIn: parent
                text: "⏭"
                color: Theme.textColorDim
                font.pixelSize: 12
            }

            MouseArea {
                id: nextHover
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: AnimController.stepForward()
            }
        }

        Rectangle {
            width: 1
            height: 20
            color: Theme.borderColor
        }

        // Time display
        Rectangle {
            width: timeText.width + 16
            height: 28
            radius: 4
            color: Theme.surfaceColor

            Text {
                id: timeText
                anchors.centerIn: parent
                text: formatTime(AnimController.currentTime) + " / " + formatTime(Keyframes.totalDuration)
                color: Theme.textColor
                font.family: "Consolas"
                font.pixelSize: 11
            }
        }

        Item { Layout.fillWidth: true }

        // Playback speed
        Text {
            text: qsTr("Speed")
            color: Theme.textColorDim
            font.pixelSize: 10
        }

        Rectangle {
            width: speedCombo.width + 8
            height: 28
            radius: 4
            color: Theme.surfaceColor
            border.color: Theme.borderColor

            ComboBox {
                id: speedCombo
                anchors.centerIn: parent
                model: ["0.25x", "0.5x", "1x", "1.5x", "2x"]
                currentIndex: 2
                implicitWidth: 65
                flat: true

                onCurrentIndexChanged: {
                    let speeds = [0.25, 0.5, 1.0, 1.5, 2.0]
                    AnimController.playbackSpeed = speeds[currentIndex]
                }
            }
        }

        Rectangle {
            width: 1
            height: 20
            color: Theme.borderColor
        }

        // Loop toggle
        Rectangle {
            width: 28
            height: 28
            radius: 4
            color: AnimController.looping ? Theme.primaryColor : (loopHover.containsMouse ? Theme.surfaceColorLight : "transparent")

            Text {
                anchors.centerIn: parent
                text: "🔁"
                font.pixelSize: 12
            }

            MouseArea {
                id: loopHover
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: AnimController.looping = !AnimController.looping
            }

            ToolTip.visible: loopHover.containsMouse
            ToolTip.text: AnimController.looping ? qsTr("Loop ON") : qsTr("Loop OFF")
        }

        Rectangle {
            width: 1
            height: 20
            color: Theme.borderColor
        }

        // Keyframe count
        Text {
            text: qsTr("%1 keyframes").arg(Keyframes.count)
            color: Theme.textColorDim
            font.pixelSize: 10
        }
    }

    function formatTime(ms) {
        if (ms === undefined || ms === null || isNaN(ms)) ms = 0
        let totalSeconds = Math.floor(ms / 1000)
        let minutes = Math.floor(totalSeconds / 60)
        let seconds = totalSeconds % 60
        let millis = Math.floor((ms % 1000) / 10)
        return String(minutes).padStart(2, '0') + ":" +
               String(seconds).padStart(2, '0') + "." +
               String(millis).padStart(2, '0')
    }
}
