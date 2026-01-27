import QtQuick
import QtQuick.Controls
import TristansKortAnimator

Item {
    id: marker
    width: 20
    height: 40

    property int keyframeIndex: 0
    property bool selected: false
    property bool multiSelected: false
    property real keyframeTime: 0
    property real pixelsPerSecond: 100
    property bool isCopying: false  // True when shift+dragging

    signal clicked(bool withCtrl)
    signal dragged(real newX)
    signal copied(real newX)  // New signal for shift+drag copy

    // Multi-selection highlight
    Rectangle {
        anchors.centerIn: parent
        width: 24
        height: 24
        radius: 12
        color: "transparent"
        border.color: Theme.accentColor
        border.width: 2
        visible: multiSelected && !selected
        opacity: 0.7
    }

    // Keyframe diamond
    Rectangle {
        anchors.centerIn: parent
        width: 16
        height: 16
        rotation: 45
        color: selected ? Theme.keyframeSelectedColor : (multiSelected ? Theme.accentColorLight : Theme.keyframeColor)
        border.color: selected ? Theme.primaryColorLight : (multiSelected ? Theme.accentColor : Theme.accentColorLight)
        border.width: 2

        Behavior on color {
            ColorAnimation { duration: Theme.animationFast }
        }
    }

    // Index label
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.bottom
        anchors.topMargin: 2
        text: keyframeIndex + 1
        color: selected ? Theme.textColor : (multiSelected ? Theme.accentColor : Theme.textColorDim)
        font.pixelSize: 10
        font.bold: selected || multiSelected
    }

    // Copy indicator (shows "+" when shift+dragging)
    Rectangle {
        visible: isCopying
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: -4
        anchors.topMargin: 2
        width: 14
        height: 14
        radius: 7
        color: Theme.primaryColor
        z: 10

        Text {
            anchors.centerIn: parent
            text: "+"
            color: "white"
            font.pixelSize: 11
            font.bold: true
        }
    }

    MouseArea {
        id: markerMouseArea
        anchors.fill: parent
        anchors.margins: -5
        cursorShape: marker.isCopying ? Qt.DragCopyCursor : Qt.PointingHandCursor

        property real startX: 0
        property bool wasDragged: false

        drag.target: marker
        drag.axis: Drag.XAxis

        onPressed: (mouse) => {
            marker.isCopying = (mouse.modifiers & Qt.ShiftModifier)
            startX = marker.x
            wasDragged = false
        }

        onPositionChanged: (mouse) => {
            if (pressed && Math.abs(marker.x - startX) > 3) {
                wasDragged = true
            }
            if (pressed) {
                marker.isCopying = (mouse.modifiers & Qt.ShiftModifier)
            }
        }

        onClicked: (mouse) => {
            if (!wasDragged) {
                marker.clicked(mouse.modifiers & Qt.ControlModifier)
            }
        }

        onReleased: (mouse) => {
            if (wasDragged) {
                let newX = marker.x + marker.width/2
                if (marker.isCopying) {
                    marker.x = startX
                    marker.copied(newX)
                } else {
                    marker.dragged(newX)
                }
            }
            marker.isCopying = false
            wasDragged = false
        }
    }
}
