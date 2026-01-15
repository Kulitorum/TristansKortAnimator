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

    signal clicked(bool withCtrl)
    signal dragged(real newX)

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

    MouseArea {
        anchors.fill: parent
        anchors.margins: -5
        cursorShape: Qt.PointingHandCursor

        drag.target: marker
        drag.axis: Drag.XAxis

        onClicked: (mouse) => {
            marker.clicked(mouse.modifiers & Qt.ControlModifier)
        }

        onReleased: {
            if (drag.active) {
                marker.dragged(marker.x + marker.width/2)
            }
        }
    }
}
