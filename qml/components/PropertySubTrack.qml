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

    // Bar timing - defaults to parent overlay timing
    property real barStartTime: getBarStart()
    property real barEndTime: getBarEnd()
    property real barValue: (minValue + maxValue) / 2

    function getBarStart() {
        if (!GeoOverlays) return 0
        let overlay = GeoOverlays.getOverlay(overlayIndex)
        return overlay ? overlay.startTime : 0
    }

    function getBarEnd() {
        if (!GeoOverlays) return 10000
        let overlay = GeoOverlays.getOverlay(overlayIndex)
        return overlay ? overlay.endTime : 10000
    }

    color: "#0d1520"
    border.color: "#1a2a3a"
    border.width: 1

    // Refresh when overlay changes
    Connections {
        target: GeoOverlays
        function onOverlayModified(idx) {
            if (idx === overlayIndex) {
                barStartTime = getBarStart()
                barEndTime = getBarEnd()
            }
        }
    }

    // Track label
    Text {
        x: 8
        anchors.verticalCenter: parent.verticalCenter
        text: propertyName
        color: trackColor
        font.pixelSize: 9
        opacity: 0.8
        z: 5
    }

    // Value input (small number box)
    Rectangle {
        x: 55
        anchors.verticalCenter: parent.verticalCenter
        width: 32
        height: 14
        radius: 2
        color: "#1a2535"
        border.color: valueInput.activeFocus ? trackColor : "#2a3a4a"
        visible: !isColorTrack

        TextInput {
            id: valueInput
            anchors.fill: parent
            anchors.margins: 2
            text: barValue.toFixed(1)
            color: trackColor
            font.pixelSize: 9
            horizontalAlignment: TextInput.AlignHCenter
            selectByMouse: true

            onEditingFinished: {
                let val = parseFloat(text) || 0
                val = Math.max(minValue, Math.min(maxValue, val))
                barValue = val
                text = val.toFixed(1)
            }
        }
    }

    // Color swatch for color tracks
    Rectangle {
        visible: isColorTrack
        x: 55
        anchors.verticalCenter: parent.verticalCenter
        width: 32
        height: 14
        radius: 2
        color: {
            if (!GeoOverlays) return trackColor
            let overlay = GeoOverlays.getOverlay(overlayIndex)
            if (!overlay) return trackColor
            return propertyKey === "fillColor" ? overlay.fillColor : overlay.borderColor
        }
        border.color: "#2a3a4a"
    }

    // Horizontal bar (draggable)
    Rectangle {
        id: propertyBar
        x: 95 + barStartTime * pixelsPerSecond / 1000
        y: 3
        width: Math.max(20, (barEndTime - barStartTime) * pixelsPerSecond / 1000)
        height: parent.height - 6
        radius: 3
        color: Qt.rgba(trackColor.r, trackColor.g, trackColor.b, 0.4)
        border.color: trackColor
        border.width: 1
        z: 2

        // Left trim handle
        Rectangle {
            id: leftHandle
            width: 4
            height: parent.height
            anchors.left: parent.left
            color: leftHandleMouse.containsMouse || leftHandleMouse.pressed ? "white" : "transparent"
            radius: 2

            MouseArea {
                id: leftHandleMouse
                anchors.fill: parent
                anchors.margins: -3
                hoverEnabled: true
                cursorShape: Qt.SizeHorCursor

                property real dragStartX: 0
                property real origStart: 0

                onPressed: (mouse) => {
                    dragStartX = mapToItem(subTrack, mouse.x, 0).x
                    origStart = barStartTime
                }

                onPositionChanged: (mouse) => {
                    if (pressed) {
                        let currentX = mapToItem(subTrack, mouse.x, 0).x
                        let delta = (currentX - dragStartX) / pixelsPerSecond * 1000
                        barStartTime = Math.max(0, Math.min(barEndTime - 100, origStart + delta))
                    }
                }
            }
        }

        // Right trim handle
        Rectangle {
            id: rightHandle
            width: 4
            height: parent.height
            anchors.right: parent.right
            color: rightHandleMouse.containsMouse || rightHandleMouse.pressed ? "white" : "transparent"
            radius: 2

            MouseArea {
                id: rightHandleMouse
                anchors.fill: parent
                anchors.margins: -3
                hoverEnabled: true
                cursorShape: Qt.SizeHorCursor

                property real dragStartX: 0
                property real origEnd: 0

                onPressed: (mouse) => {
                    dragStartX = mapToItem(subTrack, mouse.x, 0).x
                    origEnd = barEndTime
                }

                onPositionChanged: (mouse) => {
                    if (pressed) {
                        let currentX = mapToItem(subTrack, mouse.x, 0).x
                        let delta = (currentX - dragStartX) / pixelsPerSecond * 1000
                        barEndTime = Math.max(barStartTime + 100, origEnd + delta)
                    }
                }
            }
        }

        // Middle drag area (move whole bar)
        MouseArea {
            id: middleDragMouse
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            hoverEnabled: true
            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor

            property real dragStartX: 0
            property real origStart: 0
            property real origEnd: 0

            onPressed: (mouse) => {
                dragStartX = mapToItem(subTrack, mouse.x, 0).x
                origStart = barStartTime
                origEnd = barEndTime
            }

            onPositionChanged: (mouse) => {
                if (pressed) {
                    let currentX = mapToItem(subTrack, mouse.x, 0).x
                    let delta = (currentX - dragStartX) / pixelsPerSecond * 1000
                    let duration = origEnd - origStart
                    let newStart = Math.max(0, origStart + delta)
                    barStartTime = newStart
                    barEndTime = newStart + duration
                }
            }
        }

        // Value label on bar
        Text {
            visible: !isColorTrack && parent.width > 40
            anchors.centerIn: parent
            text: barValue.toFixed(1)
            color: "white"
            font.pixelSize: 9
            font.bold: true
            opacity: 0.8
        }
    }
}
