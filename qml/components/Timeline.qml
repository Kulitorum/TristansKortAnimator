import QtQuick
import QtQuick.Controls
import TristansKortAnimator

Item {
    id: timeline

    property real pixelsPerSecond: 100 * (Settings ? Settings.timelineZoom : 1.0)
    property real totalDuration: Keyframes ? Keyframes.totalDuration : 0
    property real currentTime: AnimController ? AnimController.currentTime : 0

    // Minimum timeline extent in milliseconds (always show at least 60 seconds)
    property real minTimelineExtent: 60000
    // Effective timeline width - max of content duration or minimum extent
    property real effectiveExtent: Math.max(totalDuration, minTimelineExtent)

    // Enable keyboard focus for arrow key navigation
    focus: true
    Keys.onLeftPressed: {
        Keyframes.goToPreviousKeyframe()
        if (Keyframes.currentIndex >= 0) {
            MainController.goToKeyframe(Keyframes.currentIndex)
        }
    }
    Keys.onRightPressed: {
        Keyframes.goToNextKeyframe()
        if (Keyframes.currentIndex >= 0) {
            MainController.goToKeyframe(Keyframes.currentIndex)
        }
    }


    Rectangle {
        anchors.fill: parent
        color: Theme.timelineBackground
    }

    // Time ruler at top
    Rectangle {
        id: ruler
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        color: Theme.timelineRuler

        Flickable {
            id: rulerFlickable
            anchors.fill: parent
            contentWidth: Math.max(parent.width, 40 + effectiveExtent * pixelsPerSecond / 1000 + 200)
            clip: true
            interactive: false
            contentX: timelineFlickable.contentX

            // Time markers
            Repeater {
                model: Math.ceil(effectiveExtent / 1000) + 2

                Item {
                    x: 20 + index * pixelsPerSecond
                    height: parent.height

                    Rectangle {
                        width: 1
                        height: 10
                        color: Theme.textColorDim
                        anchors.bottom: parent.bottom
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 2
                        text: formatTime(index * 1000)
                        color: Theme.textColorDim
                        font.pixelSize: 10
                    }
                }
            }
        }
    }

    // Timeline content
    Flickable {
        id: timelineFlickable
        anchors.top: ruler.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 80  // Fixed height for keyframe track area
        contentWidth: Math.max(parent.width, 40 + effectiveExtent * pixelsPerSecond / 1000 + 200)
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        interactive: false  // Disable drag-to-scroll, use mousewheel instead

        // Track background
        Rectangle {
            anchors.fill: parent
            color: "transparent"

            // Grid lines
            Repeater {
                model: Math.ceil(effectiveExtent / 1000) + 2

                Rectangle {
                    x: 20 + index * pixelsPerSecond
                    width: 1
                    height: parent.height
                    color: Theme.borderColor
                    opacity: 0.3
                }
            }
        }

        // Keyframe track (visual background only)
        Rectangle {
            id: keyframeTrack
            anchors.top: parent.top
            anchors.topMargin: 20
            x: 0
            width: timelineFlickable.contentWidth
            height: 40
            color: Theme.surfaceColorLight
            radius: Theme.radiusSmall
        }

        // Keyframe markers - direct children of Flickable for proper z-ordering
        Repeater {
            model: Keyframes

            KeyframeMarker {
                keyframeIndex: index
                x: 20 + model.time * pixelsPerSecond / 1000 - width/2
                y: keyframeTrack.y + keyframeTrack.height/2 - height/2
                selected: Keyframes.currentIndex === index
                multiSelected: Keyframes.isSelected(index)
                keyframeTime: model.time
                pixelsPerSecond: timeline.pixelsPerSecond
                z: 50  // Above playhead drag area (z:10)

                onClicked: (withCtrl) => {
                    if (withCtrl) {
                        // Ctrl+click: toggle selection
                        if (Keyframes.isSelected(index)) {
                            Keyframes.deselectKeyframe(index)
                        } else {
                            Keyframes.selectKeyframe(index, true)
                        }
                    } else {
                        // Regular click: select single and navigate
                        Keyframes.currentIndex = index
                        MainController.goToKeyframe(index)
                    }
                }

                onDragged: (newX) => {
                    // Convert x position back to time and update keyframe
                    let newTime = (newX - 20 + width/2) / pixelsPerSecond * 1000
                    newTime = Math.max(0, newTime)

                    // If this keyframe is part of multi-selection, move all selected
                    if (Keyframes.selectedIndices.length > 1 && Keyframes.isSelected(index)) {
                        let oldTime = model.time
                        let deltaTime = newTime - oldTime
                        Keyframes.moveSelectedKeyframes(deltaTime)
                    } else {
                        Keyframes.setKeyframeTime(index, newTime)
                    }
                }
            }
        }

        // Playhead
        Rectangle {
            id: playhead
            x: 20 + currentTime * pixelsPerSecond / 1000
            width: 2
            height: parent.height
            color: Theme.playheadColor
            z: 100

            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: Theme.playheadColor
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.top
            }
        }

    }

    // SCRUB OVERLAY - Outside Flickable for guaranteed event handling
    // Handles ALL timeline mouse interactions: scrubbing, keyframe selection, dragging
    MouseArea {
        id: scrubOverlay
        anchors.top: ruler.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 80  // Fixed height for keyframe track area

        // Frame selection rectangle (rubber band)
        Rectangle {
            id: selectionRect
            visible: false
            color: Qt.rgba(Theme.accentColor.r, Theme.accentColor.g, Theme.accentColor.b, 0.2)
            border.color: Theme.accentColor
            border.width: 1
        }

        property bool isDraggingScrub: false
        property bool isDraggingKeyframe: false
        property bool isDraggingSelection: false
        property int draggedKeyframeIndex: -1
        property real dragStartX: 0

        // Find keyframe index at position, returns -1 if none
        function keyframeIndexAt(mouseX, mouseY) {
            let contentX = timelineFlickable.contentX + mouseX
            for (let i = 0; i < Keyframes.count; i++) {
                let kf = Keyframes.getKeyframe(i)
                let markerX = 20 + kf.time * pixelsPerSecond / 1000
                // Marker is ~20px wide with some margin
                if (Math.abs(contentX - markerX) < 15) {
                    // Check Y is in keyframe track area
                    if (mouseY >= 10 && mouseY <= 70) {
                        return i
                    }
                }
            }
            return -1
        }

        onPressed: (mouse) => {
            // Take keyboard focus for arrow key navigation
            timeline.forceActiveFocus()

            let kfIndex = keyframeIndexAt(mouse.x, mouse.y)

            if (mouse.modifiers & Qt.ShiftModifier) {
                if (kfIndex >= 0) {
                    // Shift+click on keyframe: add to selection
                    Keyframes.selectKeyframe(kfIndex, true)
                } else {
                    // Shift+drag on empty space: frame selection (rubber band)
                    isDraggingSelection = true
                    dragStartX = mouse.x
                    selectionRect.x = mouse.x
                    selectionRect.y = 20  // Keyframe track Y offset
                    selectionRect.width = 0
                    selectionRect.height = 40
                    selectionRect.visible = true
                }
            } else if (mouse.modifiers & Qt.ControlModifier && kfIndex >= 0) {
                // Ctrl+click on keyframe: toggle selection
                if (Keyframes.isSelected(kfIndex)) {
                    Keyframes.deselectKeyframe(kfIndex)
                } else {
                    Keyframes.selectKeyframe(kfIndex, true)
                }
            } else if (kfIndex >= 0) {
                // Regular click on keyframe: select and prepare for drag
                Keyframes.currentIndex = kfIndex
                MainController.goToKeyframe(kfIndex)
                isDraggingKeyframe = true
                draggedKeyframeIndex = kfIndex
                dragStartX = mouse.x
            } else {
                // Clicked on empty space - scrub
                // Disable edit mode so camera changes don't update keyframes while scrubbing
                Keyframes.editMode = false
                isDraggingScrub = true
                seekToPosition(timelineFlickable.contentX + mouse.x)
            }
        }

        onPositionChanged: (mouse) => {
            if (isDraggingScrub) {
                seekToPosition(timelineFlickable.contentX + mouse.x)
            } else if (isDraggingKeyframe && draggedKeyframeIndex >= 0) {
                // Dragging a keyframe to new time position
                let contentX = timelineFlickable.contentX + mouse.x
                let newTime = (contentX - 20) / pixelsPerSecond * 1000
                newTime = Math.max(0, newTime)

                // If multi-selection, move all selected
                if (Keyframes.selectedIndices.length > 1 && Keyframes.isSelected(draggedKeyframeIndex)) {
                    let kf = Keyframes.getKeyframe(draggedKeyframeIndex)
                    let deltaTime = newTime - kf.time
                    Keyframes.moveSelectedKeyframes(deltaTime)
                } else {
                    Keyframes.setKeyframeTime(draggedKeyframeIndex, newTime)
                }
            } else if (isDraggingSelection) {
                // Update selection rectangle
                let minX = Math.min(dragStartX, mouse.x)
                let maxX = Math.max(dragStartX, mouse.x)
                selectionRect.x = minX
                selectionRect.width = maxX - minX
            }
        }

        onReleased: (mouse) => {
            if (isDraggingSelection) {
                selectionRect.visible = false

                // Find keyframes within selection rectangle
                let minX = Math.min(dragStartX, mouse.x)
                let maxX = Math.max(dragStartX, mouse.x)

                // Convert to content coordinates and then to time
                let minContentX = timelineFlickable.contentX + minX
                let maxContentX = timelineFlickable.contentX + maxX
                let minTime = (minContentX - 20) / pixelsPerSecond * 1000
                let maxTime = (maxContentX - 20) / pixelsPerSecond * 1000

                // Find keyframe indices within this time range
                let firstIdx = -1
                let lastIdx = -1
                for (let i = 0; i < Keyframes.count; i++) {
                    let kf = Keyframes.getKeyframe(i)
                    if (kf.time >= minTime && kf.time <= maxTime) {
                        if (firstIdx < 0) firstIdx = i
                        lastIdx = i
                    }
                }

                if (firstIdx >= 0 && lastIdx >= 0) {
                    Keyframes.selectRange(firstIdx, lastIdx)
                }
            }

            isDraggingScrub = false
            isDraggingKeyframe = false
            isDraggingSelection = false
            draggedKeyframeIndex = -1
        }

        // Mousewheel handler: zoom (normal) or scroll (shift)
        onWheel: (wheel) => {
            if (wheel.modifiers & Qt.ShiftModifier) {
                // Shift+wheel: horizontal scroll
                let scrollAmount = wheel.angleDelta.y > 0 ? -100 : 100
                timelineFlickable.contentX = Math.max(0, Math.min(
                    timelineFlickable.contentWidth - timelineFlickable.width,
                    timelineFlickable.contentX + scrollAmount
                ))
            } else {
                // Normal wheel: zoom
                let zoomFactor = wheel.angleDelta.y > 0 ? 1.15 : 0.87
                let newZoom = Settings.timelineZoom * zoomFactor

                // Clamp zoom to valid range
                newZoom = Math.max(0.1, Math.min(8.0, newZoom))

                // Calculate mouse position in content coordinates
                let mouseContentX = timelineFlickable.contentX + wheel.x

                // Calculate time at mouse position before zoom
                let timeAtMouse = (mouseContentX - 20) / pixelsPerSecond * 1000

                // Apply new zoom
                Settings.timelineZoom = newZoom

                // After zoom, adjust scroll to keep the same time position under the mouse
                let newPPS = 100 * newZoom
                let newMouseContentX = 20 + timeAtMouse * newPPS / 1000
                let newContentX = newMouseContentX - wheel.x

                // Clamp to valid scroll range
                timelineFlickable.contentX = Math.max(0, Math.min(
                    timelineFlickable.contentWidth - timelineFlickable.width,
                    newContentX
                ))
            }
            wheel.accepted = true
        }
    }

    // Region tracks section (below keyframes)
    RegionTrackTimeline {
        id: regionTrackTimeline
        anchors.top: scrubOverlay.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: zoomControls.top
        anchors.bottomMargin: Theme.spacingSmall
        pixelsPerSecond: timeline.pixelsPerSecond
        totalDuration: timeline.effectiveExtent
        visible: RegionTracks.count > 0 || regionTrackTimeline.height > 40

        // Sync scroll with main timeline
        contentX: timelineFlickable.contentX
        onContentXChanged: {
            if (contentX !== timelineFlickable.contentX) {
                timelineFlickable.contentX = contentX
            }
        }
    }

    // Timeline zoom control
    Row {
        id: zoomControls
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Theme.spacingSmall
        spacing: Theme.spacingSmall

        // Add region track button
        Button {
            text: "+ Region"
            font.pixelSize: 11
            onClicked: regionPickerDialog.open()
        }

        Item { width: Theme.spacingLarge }

        Text {
            text: qsTr("Zoom:")
            color: Theme.textColorDim
            anchors.verticalCenter: parent.verticalCenter
        }

        Slider {
            width: 100
            from: 0.1
            to: 8.0
            value: Settings.timelineZoom
            onMoved: Settings.timelineZoom = value
        }
    }

    // Region picker for adding tracks
    RegionPicker {
        id: regionPickerDialog
        onRegionSelected: (code, name) => {
            let startTime = AnimController ? AnimController.currentTime : 0
            RegionTracks.addTrack(code, name, "country", startTime)
        }
    }

    function seekToPosition(x) {
        let time = (x - 20) / pixelsPerSecond * 1000
        // Only clamp to >= 0, allow seeking beyond keyframes to add new ones
        time = Math.max(0, time)
        AnimController.setCurrentTime(time)
    }

    function formatTime(ms) {
        let seconds = Math.floor(ms / 1000)
        let minutes = Math.floor(seconds / 60)
        seconds = seconds % 60
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }
}
