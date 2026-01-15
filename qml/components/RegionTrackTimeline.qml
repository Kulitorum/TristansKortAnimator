import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

Item {
    id: regionTrackTimeline

    property real pixelsPerSecond: 100
    property real totalDuration: 60000  // Default 60 seconds

    // Height per track
    property int trackHeight: 30
    property int headerWidth: 120

    implicitHeight: Math.max(80, RegionTracks.count * trackHeight + 40)

    Rectangle {
        anchors.fill: parent
        color: Theme.timelineBackground
    }

    // Header column (track names)
    Rectangle {
        id: headerColumn
        width: headerWidth
        height: parent.height
        color: Theme.surfaceColor
        z: 10

        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height
            color: Theme.borderColor
        }

        // Add track button
        Button {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 4
            height: 28
            text: "+ Region"
            font.pixelSize: 11

            onClicked: regionPicker.open()
        }

        // Track labels
        Column {
            anchors.top: parent.top
            anchors.topMargin: 36
            anchors.left: parent.left
            anchors.right: parent.right

            Repeater {
                model: RegionTracks

                Rectangle {
                    width: headerColumn.width
                    height: trackHeight
                    color: trackMouseArea.containsMouse ? Theme.surfaceColorLight : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 4
                        anchors.rightMargin: 4
                        spacing: 2

                        // Color indicator
                        Rectangle {
                            width: 12
                            height: 12
                            radius: 2
                            color: model.fillColor
                            border.color: model.borderColor
                            border.width: 1
                        }

                        // Region name (truncated)
                        Text {
                            Layout.fillWidth: true
                            text: model.regionName
                            color: Theme.textColor
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }

                        // Delete button
                        Text {
                            text: "x"
                            color: deleteBtn.containsMouse ? Theme.errorColor : Theme.textColorDim
                            font.pixelSize: 12

                            MouseArea {
                                id: deleteBtn
                                anchors.fill: parent
                                anchors.margins: -4
                                hoverEnabled: true
                                onClicked: RegionTracks.removeTrack(index)
                            }
                        }
                    }

                    MouseArea {
                        id: trackMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.RightButton
                        onClicked: trackContextMenu.popup()
                    }

                    Menu {
                        id: trackContextMenu
                        MenuItem {
                            text: qsTr("Edit Colors...")
                            onTriggered: {
                                colorEditor.trackIndex = index
                                colorEditor.open()
                            }
                        }
                        MenuItem {
                            text: qsTr("Delete Track")
                            onTriggered: RegionTracks.removeTrack(index)
                        }
                    }
                }
            }
        }
    }

    // Timeline content area
    Flickable {
        id: trackFlickable
        anchors.left: headerColumn.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        contentWidth: Math.max(width, 40 + totalDuration * pixelsPerSecond / 1000 + 200)
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        // Sync scroll with main timeline (if parent provides contentX)
        onContentXChanged: {
            if (parent && parent.timelineContentX !== undefined) {
                parent.timelineContentX = contentX
            }
        }

        // Grid lines (same as main timeline)
        Repeater {
            model: Math.ceil(totalDuration / 1000) + 2

            Rectangle {
                x: 20 + index * pixelsPerSecond
                width: 1
                height: trackFlickable.height
                color: Theme.borderColor
                opacity: 0.2
            }
        }

        // Track bars
        Column {
            anchors.top: parent.top
            anchors.topMargin: 36

            Repeater {
                model: RegionTracks

                Item {
                    width: trackFlickable.contentWidth
                    height: trackHeight

                    // Track duration bar
                    Rectangle {
                        id: trackBar
                        x: 20 + model.startTime * pixelsPerSecond / 1000
                        y: 4
                        width: {
                            let endTime = model.endTime > 0 ? model.endTime : totalDuration
                            return (endTime - model.startTime + model.fadeOutDuration) * pixelsPerSecond / 1000
                        }
                        height: trackHeight - 8
                        radius: 4
                        color: Qt.rgba(model.fillColor.r, model.fillColor.g, model.fillColor.b, 0.6)
                        border.color: model.borderColor
                        border.width: 1

                        // Fade in indicator
                        Rectangle {
                            width: model.fadeInDuration * pixelsPerSecond / 1000
                            height: parent.height
                            radius: 4
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "transparent" }
                                GradientStop { position: 1.0; color: Qt.rgba(model.fillColor.r, model.fillColor.g, model.fillColor.b, 0.3) }
                            }
                        }

                        // Fade out indicator
                        Rectangle {
                            anchors.right: parent.right
                            width: model.fadeOutDuration * pixelsPerSecond / 1000
                            height: parent.height
                            radius: 4
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: Qt.rgba(model.fillColor.r, model.fillColor.g, model.fillColor.b, 0.3) }
                                GradientStop { position: 1.0; color: "transparent" }
                            }
                        }

                        // Drag to move
                        MouseArea {
                            id: barDragArea
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            cursorShape: Qt.SizeAllCursor
                            drag.target: trackBar
                            drag.axis: Drag.XAxis
                            drag.minimumX: 20

                            onReleased: {
                                // Update start time based on new position
                                let newStartTime = (trackBar.x - 20) / pixelsPerSecond * 1000
                                newStartTime = Math.max(0, newStartTime)
                                RegionTracks.updateTrack(index, {"startTime": newStartTime})
                            }
                        }

                        // Left edge - adjust start time
                        Rectangle {
                            width: 8
                            height: parent.height
                            color: leftEdge.containsMouse ? Theme.primaryColor : "transparent"
                            radius: 4

                            MouseArea {
                                id: leftEdge
                                anchors.fill: parent
                                cursorShape: Qt.SizeHorCursor
                                hoverEnabled: true

                                property real startX: 0
                                property real origStart: 0

                                onPressed: {
                                    startX = mouseX
                                    origStart = model.startTime
                                }

                                onPositionChanged: {
                                    if (pressed) {
                                        let delta = (mouseX - startX) / pixelsPerSecond * 1000
                                        let newStart = Math.max(0, origStart + delta)
                                        RegionTracks.updateTrack(index, {"startTime": newStart})
                                    }
                                }
                            }
                        }

                        // Right edge - adjust end time
                        Rectangle {
                            anchors.right: parent.right
                            width: 8
                            height: parent.height
                            color: rightEdge.containsMouse ? Theme.primaryColor : "transparent"
                            radius: 4

                            MouseArea {
                                id: rightEdge
                                anchors.fill: parent
                                cursorShape: Qt.SizeHorCursor
                                hoverEnabled: true

                                property real startX: 0
                                property real origEnd: 0

                                onPressed: {
                                    startX = mouseX
                                    origEnd = model.endTime > 0 ? model.endTime : totalDuration
                                }

                                onPositionChanged: {
                                    if (pressed) {
                                        let delta = (mouseX - startX) / pixelsPerSecond * 1000
                                        let newEnd = Math.max(model.startTime + 100, origEnd + delta)
                                        RegionTracks.updateTrack(index, {"endTime": newEnd})
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Scroll sync property
    property alias contentX: trackFlickable.contentX

    function setContentX(x) {
        trackFlickable.contentX = x
    }

    // Region picker dialog
    RegionPicker {
        id: regionPicker
        onRegionSelected: (code, name) => {
            // Add track at current animation time
            let startTime = AnimController ? AnimController.currentTime : 0
            RegionTracks.addTrack(code, name, "country", startTime)
        }
    }

    // Color editor dialog
    Dialog {
        id: colorEditor
        title: qsTr("Edit Track Colors")
        modal: true
        anchors.centerIn: parent
        width: 300

        property int trackIndex: -1

        contentItem: ColumnLayout {
            spacing: Theme.spacingNormal

            RowLayout {
                Label { text: qsTr("Fill Color:"); Layout.preferredWidth: 80 }
                Rectangle {
                    width: 40
                    height: 24
                    color: colorEditor.trackIndex >= 0 ? RegionTracks.getTrack(colorEditor.trackIndex).fillColor : "red"
                    border.color: Theme.borderColor
                    radius: 4

                    MouseArea {
                        anchors.fill: parent
                        onClicked: fillColorPicker.open()
                    }
                }
            }

            RowLayout {
                Label { text: qsTr("Border Color:"); Layout.preferredWidth: 80 }
                Rectangle {
                    width: 40
                    height: 24
                    color: colorEditor.trackIndex >= 0 ? RegionTracks.getTrack(colorEditor.trackIndex).borderColor : "red"
                    border.color: Theme.borderColor
                    radius: 4

                    MouseArea {
                        anchors.fill: parent
                        onClicked: borderColorPicker.open()
                    }
                }
            }

            RowLayout {
                Label { text: qsTr("Fade In (ms):"); Layout.preferredWidth: 80 }
                SpinBox {
                    id: fadeInSpinner
                    from: 0
                    to: 5000
                    stepSize: 100
                    value: colorEditor.trackIndex >= 0 ? RegionTracks.getTrack(colorEditor.trackIndex).fadeInDuration : 500
                    onValueModified: {
                        if (colorEditor.trackIndex >= 0) {
                            RegionTracks.updateTrack(colorEditor.trackIndex, {"fadeInDuration": value})
                        }
                    }
                }
            }

            RowLayout {
                Label { text: qsTr("Fade Out (ms):"); Layout.preferredWidth: 80 }
                SpinBox {
                    id: fadeOutSpinner
                    from: 0
                    to: 5000
                    stepSize: 100
                    value: colorEditor.trackIndex >= 0 ? RegionTracks.getTrack(colorEditor.trackIndex).fadeOutDuration : 500
                    onValueModified: {
                        if (colorEditor.trackIndex >= 0) {
                            RegionTracks.updateTrack(colorEditor.trackIndex, {"fadeOutDuration": value})
                        }
                    }
                }
            }
        }

        standardButtons: Dialog.Close
    }

    // Simple color picker dialog with presets
    Dialog {
        id: fillColorPicker
        title: qsTr("Select Fill Color")
        modal: true
        anchors.centerIn: parent

        contentItem: Grid {
            columns: 5
            spacing: 4
            Repeater {
                model: ["#80ff0000", "#80ff8000", "#80ffff00", "#8000ff00", "#8000ffff",
                        "#800000ff", "#80ff00ff", "#80ffffff", "#80808080", "#80000000"]
                Rectangle {
                    width: 32
                    height: 32
                    color: modelData
                    border.color: Theme.borderColor
                    radius: 4
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (colorEditor.trackIndex >= 0) {
                                RegionTracks.updateTrack(colorEditor.trackIndex, {"fillColor": modelData})
                            }
                            fillColorPicker.close()
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: borderColorPicker
        title: qsTr("Select Border Color")
        modal: true
        anchors.centerIn: parent

        contentItem: Grid {
            columns: 5
            spacing: 4
            Repeater {
                model: ["#ffff0000", "#ffff8000", "#ffffff00", "#ff00ff00", "#ff00ffff",
                        "#ff0000ff", "#ffff00ff", "#ffffffff", "#ff808080", "#ff000000"]
                Rectangle {
                    width: 32
                    height: 32
                    color: modelData
                    border.color: Theme.borderColor
                    radius: 4
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (colorEditor.trackIndex >= 0) {
                                RegionTracks.updateTrack(colorEditor.trackIndex, {"borderColor": modelData})
                            }
                            borderColorPicker.close()
                        }
                    }
                }
            }
        }
    }
}
