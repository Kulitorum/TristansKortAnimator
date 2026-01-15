import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

Rectangle {
    id: previewControls
    height: 50
    color: Theme.surfaceColor

    Rectangle {
        anchors.top: parent.top
        width: parent.width
        height: 1
        color: Theme.borderColor
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spacingNormal
        anchors.rightMargin: Theme.spacingNormal
        spacing: Theme.spacingNormal

        // Play/Pause button
        Button {
            id: playButton
            text: AnimController.playing ? "⏸" : "▶"
            font.pixelSize: 20
            implicitWidth: 50
            onClicked: AnimController.togglePlayPause()

            ToolTip.visible: hovered
            ToolTip.text: AnimController.playing ? qsTr("Pause (Space)") : qsTr("Play (Space)")
        }

        // Stop button
        Button {
            text: "⏹"
            font.pixelSize: 20
            implicitWidth: 50
            onClicked: AnimController.stop()

            ToolTip.visible: hovered
            ToolTip.text: qsTr("Stop (Home)")
        }

        // Previous keyframe
        Button {
            text: "⏮"
            font.pixelSize: 16
            implicitWidth: 40
            onClicked: AnimController.stepBackward()

            ToolTip.visible: hovered
            ToolTip.text: qsTr("Previous Keyframe (Page Up)")
        }

        // Next keyframe
        Button {
            text: "⏭"
            font.pixelSize: 16
            implicitWidth: 40
            onClicked: AnimController.stepForward()

            ToolTip.visible: hovered
            ToolTip.text: qsTr("Next Keyframe (Page Down)")
        }

        Item { Layout.preferredWidth: Theme.spacingLarge }

        // Time display
        Text {
            text: formatTime(AnimController.currentTime) + " / " + formatTime(Keyframes.totalDuration)
            color: Theme.textColor
            font.family: "Consolas"
            font.pixelSize: Theme.fontSizeNormal
        }

        Item { Layout.fillWidth: true }

        // Playback speed
        Text {
            text: qsTr("Speed:")
            color: Theme.textColorDim
        }

        ComboBox {
            id: speedCombo
            model: ["0.25x", "0.5x", "1x", "1.5x", "2x"]
            currentIndex: 2
            implicitWidth: 80

            onCurrentIndexChanged: {
                let speeds = [0.25, 0.5, 1.0, 1.5, 2.0]
                AnimController.playbackSpeed = speeds[currentIndex]
            }
        }

        // Loop toggle
        Button {
            text: "🔁"
            font.pixelSize: 16
            implicitWidth: 40
            checked: AnimController ? AnimController.looping : false
            checkable: true
            onClicked: if (AnimController) AnimController.looping = checked

            ToolTip.visible: hovered
            ToolTip.text: qsTr("Loop animation")

            background: Rectangle {
                color: parent.checked ? Theme.primaryColor : Theme.surfaceColorLight
                radius: Theme.radiusSmall
            }
        }

        Item { Layout.preferredWidth: Theme.spacingLarge }

        // Keyframe count
        Text {
            text: qsTr("%1 keyframes").arg(Keyframes.count)
            color: Theme.textColorDim
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
