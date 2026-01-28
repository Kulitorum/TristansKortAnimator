import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

Item {
    id: timeline

    property real pixelsPerSecond: 100 * (Settings ? Settings.timelineZoom : 1.0)
    property real totalDuration: AnimController ? AnimController.totalDuration : 60000
    property real currentTime: AnimController ? AnimController.currentTime : 0
    property bool useExplicitDuration: AnimController ? AnimController.useExplicitDuration : true
    property real explicitDuration: AnimController ? AnimController.explicitDuration : 60000

    // Effective timeline width - always use totalDuration (which now handles explicit mode)
    property real effectiveExtent: totalDuration

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

        // Ruler mouse area for scrubbing and panning
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            cursorShape: rulerPanning ? Qt.ClosedHandCursor : Qt.ArrowCursor

            property bool rulerPanning: false
            property real panStartX: 0
            property real panStartContentX: 0

            onPressed: (mouse) => {
                if (mouse.button === Qt.RightButton) {
                    rulerPanning = true
                    panStartX = mouse.x
                    panStartContentX = timelineFlickable.contentX
                    return
                }
                // Left click scrubs
                let time = (timelineFlickable.contentX + mouse.x - 20) / pixelsPerSecond * 1000
                AnimController.setCurrentTime(Math.max(0, time))
            }
            onPositionChanged: (mouse) => {
                if (rulerPanning) {
                    let deltaX = panStartX - mouse.x
                    let newContentX = panStartContentX + deltaX
                    newContentX = Math.max(0, Math.min(
                        timelineFlickable.contentWidth - timelineFlickable.width,
                        newContentX
                    ))
                    timelineFlickable.contentX = newContentX
                    return
                }
                if (pressed) {
                    let time = (timelineFlickable.contentX + mouse.x - 20) / pixelsPerSecond * 1000
                    AnimController.setCurrentTime(Math.max(0, time))
                }
            }
            onReleased: {
                rulerPanning = false
            }
        }
    }

    // Track heights
    property real cameraTrackHeight: 50
    property real overlayTrackHeight: 40
    property real totalTracksHeight: cameraTrackHeight + (GeoOverlays ? GeoOverlays.count * overlayTrackHeight : 0)

    // Timeline content
    Flickable {
        id: timelineFlickable
        anchors.top: ruler.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: durationBar.top
        contentWidth: Math.max(parent.width, 40 + effectiveExtent * pixelsPerSecond / 1000 + 200)
        contentHeight: Math.max(height, totalTracksHeight + 20)
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        interactive: false

        // Vertical scrollbar for when there are many overlay tracks
        ScrollBar.vertical: ScrollBar {
            id: verticalTimelineScrollBar
            policy: totalTracksHeight + 20 > timelineFlickable.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            width: 12

            contentItem: Rectangle {
                implicitWidth: 8
                radius: 4
                color: verticalTimelineScrollBar.pressed ? Theme.primaryColor :
                       verticalTimelineScrollBar.hovered ? Theme.textColorDim : Theme.borderColor
            }

            background: Rectangle {
                implicitWidth: 12
                color: Theme.surfaceColor
            }
        }

        // Track background with grid lines
        Rectangle {
            width: timelineFlickable.contentWidth
            height: Math.max(timelineFlickable.height, totalTracksHeight + 20)
            color: "transparent"

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

        // Camera keyframe track
        Rectangle {
            id: keyframeTrack
            x: 0
            y: 5
            width: timelineFlickable.contentWidth
            height: cameraTrackHeight - 10
            color: Theme.surfaceColorLight
            radius: Theme.radiusSmall

            Text {
                x: 8
                anchors.verticalCenter: parent.verticalCenter
                text: "Camera"
                color: Theme.textColorDim
                font.pixelSize: 10
            }
        }

        // Camera keyframe markers
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
                z: 50

                onClicked: (withCtrl) => {
                    if (withCtrl) {
                        if (Keyframes.isSelected(index)) {
                            Keyframes.deselectKeyframe(index)
                        } else {
                            Keyframes.selectKeyframe(index, true)
                        }
                    } else {
                        Keyframes.currentIndex = index
                        MainController.goToKeyframe(index)
                    }
                }

                onDragged: (newX) => {
                    let newTime = (newX - 20 + width/2) / pixelsPerSecond * 1000
                    newTime = Math.max(0, newTime)
                    if (Keyframes.selectedIndices.length > 1 && Keyframes.isSelected(index)) {
                        let oldTime = model.time
                        let deltaTime = newTime - oldTime
                        Keyframes.moveSelectedKeyframes(deltaTime)
                    } else {
                        Keyframes.setKeyframeTime(index, newTime)
                    }
                }

                onCopied: (newX) => {
                    // Shift+drag: duplicate keyframe at new time position
                    let newTime = (newX - 20 + width/2) / pixelsPerSecond * 1000
                    newTime = Math.max(0, newTime)
                    Keyframes.duplicateKeyframeAtTime(index, newTime)
                }
            }
        }

        // Overlay tracks area - click to scrub (behind track bars)
        MouseArea {
            id: overlaysScrubArea
            x: 0
            y: cameraTrackHeight
            width: timelineFlickable.contentWidth
            height: Math.max(0, GeoOverlays.count * overlayTrackHeight)
            z: -1  // Behind track bars
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            cursorShape: isPanning ? Qt.ClosedHandCursor : Qt.ArrowCursor

            property bool isPanning: false
            property real panStartX: 0
            property real panStartContentX: 0

            onPressed: (mouse) => {
                // Right-click: start panning
                if (mouse.button === Qt.RightButton) {
                    isPanning = true
                    panStartX = mouse.x
                    panStartContentX = timelineFlickable.contentX
                    return
                }

                Keyframes.editMode = false
                let time = (mouse.x - 20) / pixelsPerSecond * 1000
                AnimController.setCurrentTime(Math.max(0, time))
            }
            onPositionChanged: (mouse) => {
                if (isPanning) {
                    let deltaX = panStartX - mouse.x
                    let newContentX = panStartContentX + deltaX
                    newContentX = Math.max(0, Math.min(
                        timelineFlickable.contentWidth - timelineFlickable.width,
                        newContentX
                    ))
                    timelineFlickable.contentX = newContentX
                    return
                }

                if (pressed) {
                    let time = (mouse.x - 20) / pixelsPerSecond * 1000
                    AnimController.setCurrentTime(Math.max(0, time))
                }
            }
            onReleased: {
                isPanning = false
            }

            // Wheel for vertical scrolling in overlay tracks
            onWheel: (wheel) => {
                if (wheel.modifiers & Qt.ControlModifier) {
                    // Ctrl+wheel: vertical scroll
                    let scrollAmount = wheel.angleDelta.y > 0 ? -50 : 50
                    timelineFlickable.contentY = Math.max(0, Math.min(
                        timelineFlickable.contentHeight - timelineFlickable.height,
                        timelineFlickable.contentY + scrollAmount
                    ))
                    wheel.accepted = true
                } else {
                    // Pass through to parent for zoom/horizontal scroll
                    wheel.accepted = false
                }
            }
        }

        // Overlay tracks
        Repeater {
            model: GeoOverlays

            Rectangle {
                id: overlayTrackRow
                x: 0
                y: cameraTrackHeight + index * overlayTrackHeight
                width: timelineFlickable.contentWidth
                height: overlayTrackHeight
                color: index % 2 === 0 ? "#1a2a3a" : "#152535"

                property int overlayIdx: index
                // Store model values in properties for stable bindings
                property real trackStartTime: model.startTime
                property real trackEndTime: model.endTime
                property real trackFadeIn: model.fadeInDuration || 0
                property real trackFadeOut: model.fadeOutDuration || 0

                // Track label - integrated like Camera label
                Text {
                    x: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: model.name
                    color: Theme.textColorDim
                    font.pixelSize: 10
                    z: 1
                }

                // Track bar - positioned in timeline coordinates
                Rectangle {
                    id: overlayBar
                    x: 20 + trackStartTime * pixelsPerSecond / 1000
                    y: 4
                    width: Math.max(30, (trackEndTime - trackStartTime) * pixelsPerSecond / 1000)
                    height: parent.height - 8
                    radius: 3
                    color: Qt.rgba(model.borderColor.r, model.borderColor.g, model.borderColor.b, 0.3)
                    border.color: model.borderColor
                    border.width: 2
                    z: 5

                    // Fade in indicator
                    Rectangle {
                        visible: trackFadeIn > 0
                        width: Math.min(parent.width * 0.4, trackFadeIn * pixelsPerSecond / 1000)
                        height: parent.height
                        anchors.left: parent.left
                        radius: parent.radius
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: "transparent" }
                            GradientStop { position: 1.0; color: Qt.rgba(model.borderColor.r, model.borderColor.g, model.borderColor.b, 0.5) }
                        }
                    }

                    // Fade out indicator
                    Rectangle {
                        visible: trackFadeOut > 0
                        width: Math.min(parent.width * 0.4, trackFadeOut * pixelsPerSecond / 1000)
                        height: parent.height
                        anchors.right: parent.right
                        radius: parent.radius
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: Qt.rgba(model.borderColor.r, model.borderColor.g, model.borderColor.b, 0.5) }
                            GradientStop { position: 1.0; color: "transparent" }
                        }
                    }

                    // Left handle - trim start time
                    Rectangle {
                        id: leftHandle
                        width: 6
                        height: parent.height
                        anchors.left: parent.left
                        color: leftArea.containsMouse || leftArea.pressed ? Theme.primaryColor : "transparent"
                        z: 10

                        MouseArea {
                            id: leftArea
                            anchors.fill: parent
                            anchors.margins: -5
                            hoverEnabled: true
                            cursorShape: Qt.SizeHorCursor
                            property real dragStartGlobalX: 0
                            property real origStart: 0
                            property real origEnd: 0
                            property real origFadeIn: 0
                            property real origFadeOut: 0

                            onPressed: (mouse) => {
                                // Use global coordinates for stable tracking
                                let globalPos = mapToItem(overlayTrackRow, mouse.x, mouse.y)
                                dragStartGlobalX = globalPos.x
                                origStart = trackStartTime
                                origEnd = trackEndTime
                                origFadeIn = trackFadeIn
                                origFadeOut = trackFadeOut
                            }
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    let globalPos = mapToItem(overlayTrackRow, mouse.x, mouse.y)
                                    let delta = (globalPos.x - dragStartGlobalX) / pixelsPerSecond * 1000
                                    let newStart = Math.max(0, Math.min(origEnd - 500, origStart + delta))
                                    GeoOverlays.setOverlayTiming(overlayIdx, newStart, origFadeIn, origEnd, origFadeOut)
                                }
                            }
                        }
                    }

                    // Right handle - trim end time
                    Rectangle {
                        id: rightHandle
                        width: 6
                        height: parent.height
                        anchors.right: parent.right
                        color: rightArea.containsMouse || rightArea.pressed ? Theme.primaryColor : "transparent"
                        z: 10

                        MouseArea {
                            id: rightArea
                            anchors.fill: parent
                            anchors.margins: -5
                            hoverEnabled: true
                            cursorShape: Qt.SizeHorCursor
                            property real dragStartGlobalX: 0
                            property real origStart: 0
                            property real origEnd: 0
                            property real origFadeIn: 0
                            property real origFadeOut: 0

                            onPressed: (mouse) => {
                                let globalPos = mapToItem(overlayTrackRow, mouse.x, mouse.y)
                                dragStartGlobalX = globalPos.x
                                origStart = trackStartTime
                                origEnd = trackEndTime
                                origFadeIn = trackFadeIn
                                origFadeOut = trackFadeOut
                            }
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    let globalPos = mapToItem(overlayTrackRow, mouse.x, mouse.y)
                                    let delta = (globalPos.x - dragStartGlobalX) / pixelsPerSecond * 1000
                                    let newEnd = Math.max(origStart + 500, origEnd + delta)
                                    GeoOverlays.setOverlayTiming(overlayIdx, origStart, origFadeIn, newEnd, origFadeOut)
                                }
                            }
                        }
                    }

                    // Middle drag - move entire bar
                    MouseArea {
                        id: middleArea
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        property real dragStartGlobalX: 0
                        property real origStart: 0
                        property real origEnd: 0
                        property real origFadeIn: 0
                        property real origFadeOut: 0

                        onPressed: (mouse) => {
                            let globalPos = mapToItem(overlayTrackRow, mouse.x, mouse.y)
                            dragStartGlobalX = globalPos.x
                            origStart = trackStartTime
                            origEnd = trackEndTime
                            origFadeIn = trackFadeIn
                            origFadeOut = trackFadeOut
                        }
                        onPositionChanged: (mouse) => {
                            if (pressed) {
                                let globalPos = mapToItem(overlayTrackRow, mouse.x, mouse.y)
                                let delta = (globalPos.x - dragStartGlobalX) / pixelsPerSecond * 1000
                                let duration = origEnd - origStart
                                let newStart = Math.max(0, origStart + delta)
                                GeoOverlays.setOverlayTiming(overlayIdx, newStart, origFadeIn, newStart + duration, origFadeOut)
                            }
                        }
                    }
                }

                // Delete button
                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: "×"
                    color: delMouse.containsMouse ? Theme.primaryColor : Theme.textColorDim
                    font.pixelSize: 14
                    z: 20
                    MouseArea {
                        id: delMouse
                        anchors.fill: parent
                        anchors.margins: -4
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: GeoOverlays.removeOverlay(overlayIdx)
                    }
                }
            }
        }

        // Playhead - spans all tracks
        Rectangle {
            id: playhead
            x: 20 + currentTime * pixelsPerSecond / 1000
            width: 2
            height: Math.max(timelineFlickable.height, totalTracksHeight + 20)
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

    // SCRUB OVERLAY - Only covers camera track area
    MouseArea {
        id: scrubOverlay
        anchors.top: ruler.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: cameraTrackHeight  // Only camera track, not overlay tracks
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        cursorShape: isPanning ? Qt.ClosedHandCursor : Qt.ArrowCursor

        // Frame selection rectangle (rubber band)
        Rectangle {
            id: selectionRect
            visible: false
            color: Qt.rgba(Theme.accentColor.r, Theme.accentColor.g, Theme.accentColor.b, 0.2)
            border.color: Theme.accentColor
            border.width: 1
        }

        // Ghost marker for copy operation
        Rectangle {
            id: copyGhost
            visible: scrubOverlay.isCopyingKeyframe && scrubOverlay.isDraggingKeyframe
            width: 16
            height: 16
            rotation: 45
            color: Theme.primaryColor
            opacity: 0.6
            border.color: Theme.primaryColorLight
            border.width: 2
            x: scrubOverlay.mouseX - 8
            y: 25  // Center in keyframe track

            // "+" badge
            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.rightMargin: -12
                anchors.topMargin: -8
                rotation: -45  // Counter-rotate to be upright
                width: 14
                height: 14
                radius: 7
                color: Theme.primaryColor

                Text {
                    anchors.centerIn: parent
                    text: "+"
                    color: "white"
                    font.pixelSize: 11
                    font.bold: true
                }
            }
        }

        property bool isDraggingScrub: false
        property bool isDraggingKeyframe: false
        property bool isDraggingSelection: false
        property bool isPanning: false
        property bool isCopyingKeyframe: false  // Shift+drag to copy
        property int draggedKeyframeIndex: -1
        property real dragStartX: 0
        property real panStartX: 0
        property real panStartContentX: 0
        property real originalKeyframeTime: 0  // For copy operation

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

            // Right-click: start panning
            if (mouse.button === Qt.RightButton) {
                isPanning = true
                panStartX = mouse.x
                panStartContentX = timelineFlickable.contentX
                return
            }

            let kfIndex = keyframeIndexAt(mouse.x, mouse.y)

            if (mouse.modifiers & Qt.ShiftModifier) {
                if (kfIndex >= 0) {
                    // Shift+drag on keyframe: prepare to copy
                    isDraggingKeyframe = true
                    isCopyingKeyframe = true
                    draggedKeyframeIndex = kfIndex
                    dragStartX = mouse.x
                    let kf = Keyframes.getKeyframe(kfIndex)
                    originalKeyframeTime = kf.time
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
            if (isPanning) {
                let deltaX = panStartX - mouse.x
                let newContentX = panStartContentX + deltaX
                newContentX = Math.max(0, Math.min(
                    timelineFlickable.contentWidth - timelineFlickable.width,
                    newContentX
                ))
                timelineFlickable.contentX = newContentX
                return
            }

            if (isDraggingScrub) {
                seekToPosition(timelineFlickable.contentX + mouse.x)
            } else if (isDraggingKeyframe && draggedKeyframeIndex >= 0 && !isCopyingKeyframe) {
                // Dragging a keyframe to new time position (only if not copying)
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
            if (isPanning) {
                isPanning = false
                return
            }

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

            // Handle copy on shift+drag release
            if (isCopyingKeyframe && draggedKeyframeIndex >= 0) {
                let dragDistance = Math.abs(mouse.x - dragStartX)
                if (dragDistance > 5) {
                    // Actually dragged - create copy at drop position
                    let contentX = timelineFlickable.contentX + mouse.x
                    let newTime = (contentX - 20) / pixelsPerSecond * 1000
                    newTime = Math.max(0, newTime)
                    Keyframes.duplicateKeyframeAtTime(draggedKeyframeIndex, newTime)
                }
            }

            isDraggingScrub = false
            isDraggingKeyframe = false
            isDraggingSelection = false
            isCopyingKeyframe = false
            isPanning = false
            draggedKeyframeIndex = -1
        }

        // Mousewheel handler: zoom (normal), horizontal scroll (shift), vertical scroll (ctrl)
        onWheel: (wheel) => {
            if (wheel.modifiers & Qt.ControlModifier) {
                // Ctrl+wheel: vertical scroll (for many overlay tracks)
                let scrollAmount = wheel.angleDelta.y > 0 ? -50 : 50
                timelineFlickable.contentY = Math.max(0, Math.min(
                    timelineFlickable.contentHeight - timelineFlickable.height,
                    timelineFlickable.contentY + scrollAmount
                ))
            } else if (wheel.modifiers & Qt.ShiftModifier) {
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

    // Duration control bar at the bottom
    Rectangle {
        id: durationBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        color: Theme.surfaceColor
        z: 200
        clip: true

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
            anchors.topMargin: 4
            anchors.bottomMargin: 4
            spacing: 6

            // Duration label and input
            Text {
                text: "Duration:"
                color: Theme.textColorDim
                font.pixelSize: 11
                Layout.alignment: Qt.AlignVCenter
            }

            Rectangle {
                implicitWidth: 50
                implicitHeight: 22
                color: Theme.surfaceColorLight
                border.color: durationInput.activeFocus ? Theme.primaryColor : Theme.borderColor
                border.width: 1
                radius: 3

                TextInput {
                    id: durationInput
                    anchors.fill: parent
                    anchors.margins: 4
                    text: Math.round(explicitDuration / 1000).toString()
                    horizontalAlignment: TextInput.AlignHCenter
                    verticalAlignment: TextInput.AlignVCenter
                    color: Theme.textColor
                    font.pixelSize: 11
                    selectByMouse: true
                    validator: IntValidator { bottom: 1; top: 3600 }

                    onEditingFinished: {
                        let val = parseInt(text) || 60
                        val = Math.max(1, Math.min(3600, val))
                        if (AnimController) {
                            AnimController.explicitDuration = val * 1000
                        }
                    }
                }
            }

            Text {
                text: "s"
                color: Theme.textColorDim
                font.pixelSize: 11
                Layout.alignment: Qt.AlignVCenter
            }

            Item { Layout.fillWidth: true }

            // Current time / total
            Text {
                text: formatTime(currentTime) + " / " + formatTime(totalDuration)
                color: Theme.textColor
                font.pixelSize: 11
                font.family: "Consolas"
                Layout.alignment: Qt.AlignVCenter
            }

            // Zoom level
            Text {
                text: (Settings.timelineZoom * 100).toFixed(0) + "%"
                color: Theme.textColorDim
                font.pixelSize: 11
                Layout.alignment: Qt.AlignVCenter
            }
        }
    }
}
