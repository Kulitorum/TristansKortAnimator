import QtQuick
import QtQuick.Controls
import TristansKortAnimator

Rectangle {
    id: subTrack

    required property string propertyName
    required property string propertyKey
    required property int overlayIndex
    required property color trackColor
    property real pixelsPerSecond: 100
    property bool isColorTrack: false
    property real minValue: 0
    property real maxValue: 1

    color: "#0d1520"
    border.color: "#1a2a3a"
    border.width: 1

    // Track label
    Text {
        x: 22
        anchors.verticalCenter: parent.verticalCenter
        text: propertyName
        color: trackColor
        font.pixelSize: 9
        opacity: 0.8
    }

    // Track line
    Rectangle {
        x: 80
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - 90
        height: 1
        color: trackColor
        opacity: 0.3
    }

    // Keyframes on this track
    Repeater {
        model: GeoOverlays ? GeoOverlays.getPropertyKeyframes(overlayIndex, propertyKey) : []

        Rectangle {
            id: keyframeDiamond
            x: 80 + (modelData.time * pixelsPerSecond / 1000) - 4
            anchors.verticalCenter: parent.verticalCenter
            width: 8
            height: 8
            rotation: 45
            color: isColorTrack ? (modelData.color || trackColor) : trackColor
            border.color: "white"
            border.width: 1
            z: 10

            // Tooltip showing value
            ToolTip.visible: keyframeMouse.containsMouse
            ToolTip.delay: 300
            ToolTip.text: isColorTrack
                ? "Color at " + (modelData.time / 1000).toFixed(1) + "s"
                : propertyName + ": " + (modelData.value !== undefined ? modelData.value.toFixed(2) : "?") + " at " + (modelData.time / 1000).toFixed(1) + "s"

            MouseArea {
                id: keyframeMouse
                anchors.fill: parent
                anchors.margins: -4
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                property real dragStartX: 0
                property real origTime: 0

                onPressed: (mouse) => {
                    dragStartX = mouse.x
                    origTime = modelData.time
                }

                onPositionChanged: (mouse) => {
                    if (pressed) {
                        let delta = (mouse.x - dragStartX) / pixelsPerSecond * 1000
                        let newTime = Math.max(0, origTime + delta)
                        GeoOverlays.movePropertyKeyframe(overlayIndex, propertyKey, index, newTime)
                    }
                }

                onDoubleClicked: {
                    // Remove keyframe on double-click
                    GeoOverlays.removePropertyKeyframe(overlayIndex, propertyKey, index)
                }
            }
        }
    }

    // Add keyframe on double-click
    MouseArea {
        anchors.fill: parent
        anchors.leftMargin: 80
        z: -1

        onDoubleClicked: (mouse) => {
            let timeMs = (mouse.x / pixelsPerSecond) * 1000
            if (isColorTrack) {
                // For color tracks, use the current overlay color
                let overlay = GeoOverlays.getOverlay(overlayIndex)
                let color = propertyKey === "fillColor" ? overlay.fillColor : overlay.borderColor
                GeoOverlays.addColorKeyframe(overlayIndex, propertyKey, timeMs, color)
            } else {
                // For value tracks, interpolate or use default
                let defaultVal = (minValue + maxValue) / 2
                GeoOverlays.addPropertyKeyframe(overlayIndex, propertyKey, timeMs, defaultVal)
            }
        }
    }

    // Add keyframe button
    Text {
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        text: "+"
        color: addMouse.containsMouse ? Theme.primaryColor : Theme.textColorDim
        font.pixelSize: 12
        font.bold: true

        MouseArea {
            id: addMouse
            anchors.fill: parent
            anchors.margins: -4
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                // Add keyframe at current playhead position
                let timeMs = AnimController ? AnimController.currentTime : 0
                if (isColorTrack) {
                    let overlay = GeoOverlays.getOverlay(overlayIndex)
                    let color = propertyKey === "fillColor" ? overlay.fillColor : overlay.borderColor
                    GeoOverlays.addColorKeyframe(overlayIndex, propertyKey, timeMs, color)
                } else {
                    let defaultVal = (minValue + maxValue) / 2
                    GeoOverlays.addPropertyKeyframe(overlayIndex, propertyKey, timeMs, defaultVal)
                }
            }
        }
    }
}
