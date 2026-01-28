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
    property real effectiveExtent: totalDuration

    // Signal when effect is selected
    signal effectSelected(int overlayIndex, int effectIndex)

    // Layout constants
    property real trackHeaderWidth: 140
    property real rulerHeight: 28
    property real cameraTrackHeight: 44
    property real overlayTrackHeight: 36
    property real effectBarHeight: 24
    property real bottomBarHeight: 32

    // Colors - sleek dark theme
    property color bgDark: "#0a0e14"
    property color bgCamera: "#111a24"
    property color bgOverlay1: "#0d151e"
    property color bgOverlay2: "#0f1820"
    property color bgHeader: "#080c12"
    property color borderSubtle: "#1a2a3a"
    property color textPrimary: "#e0e6ed"
    property color textSecondary: "#6b7d8f"
    property color accentColor: "#3d9cf0"

    // Keyboard navigation
    focus: true
    Keys.onLeftPressed: {
        Keyframes.goToPreviousKeyframe()
        if (Keyframes.currentIndex >= 0) MainController.goToKeyframe(Keyframes.currentIndex)
    }
    Keys.onRightPressed: {
        Keyframes.goToNextKeyframe()
        if (Keyframes.currentIndex >= 0) MainController.goToKeyframe(Keyframes.currentIndex)
    }

    // Calculate total height
    function calculateTotalTracksHeight() {
        var height = cameraTrackHeight
        if (GeoOverlays) {
            for (var i = 0; i < GeoOverlays.count; i++) {
                height += overlayTrackHeight
                if (GeoOverlays.isExpanded(i)) {
                    var effectCount = GeoOverlays.effectCount(i)
                    height += effectBarHeight * Math.max(1, effectCount)
                }
            }
        }
        return height
    }
    property real totalTracksHeight: calculateTotalTracksHeight()

    function getOverlayTrackY(overlayIndex) {
        var y = cameraTrackHeight
        for (var i = 0; i < overlayIndex; i++) {
            y += overlayTrackHeight
            if (GeoOverlays && GeoOverlays.isExpanded(i)) {
                var effectCount = GeoOverlays.effectCount(i)
                y += effectBarHeight * Math.max(1, effectCount)
            }
        }
        return y
    }

    // Background
    Rectangle {
        anchors.fill: parent
        color: bgDark
    }

    // ═══════════════════════════════════════════════════════════════
    // TOP LEFT CORNER - Empty header cell
    // ═══════════════════════════════════════════════════════════════
    Rectangle {
        id: headerCorner
        x: 0
        y: 0
        width: trackHeaderWidth
        height: rulerHeight
        color: bgHeader

        // Subtle bottom border
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: borderSubtle
        }

        // Right border
        Rectangle {
            anchors.right: parent.right
            width: 1
            height: parent.height
            color: borderSubtle
        }

        Text {
            anchors.centerIn: parent
            text: "Tracks"
            color: textSecondary
            font.pixelSize: 11
            font.weight: Font.Medium
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // TIME RULER (top, scrolls horizontally)
    // ═══════════════════════════════════════════════════════════════
    Rectangle {
        id: ruler
        x: trackHeaderWidth
        y: 0
        width: parent.width - trackHeaderWidth
        height: rulerHeight
        color: bgHeader
        clip: true

        // Bottom border
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 1
            color: borderSubtle
        }

        // Scrollable ruler content
        Item {
            x: -timelineContent.contentX
            width: timelineContent.contentWidth
            height: parent.height

            // Time markers
            Repeater {
                model: Math.ceil(effectiveExtent / 1000) + 2

                Item {
                    x: index * pixelsPerSecond
                    height: parent.height

                    // Major tick
                    Rectangle {
                        width: 1
                        height: 8
                        anchors.bottom: parent.bottom
                        color: index % 5 === 0 ? textSecondary : borderSubtle
                    }

                    // Time label (every 5 seconds)
                    Text {
                        visible: index % 5 === 0
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: 4
                        text: formatTime(index * 1000)
                        color: textSecondary
                        font.pixelSize: 10
                        font.family: "Segoe UI"
                    }
                }
            }
        }

        // Ruler scrub interaction
        MouseArea {
            anchors.fill: parent
            onPressed: (mouse) => {
                let time = (timelineContent.contentX + mouse.x) / pixelsPerSecond * 1000
                AnimController.setCurrentTime(Math.max(0, time))
            }
            onPositionChanged: (mouse) => {
                if (pressed) {
                    let time = (timelineContent.contentX + mouse.x) / pixelsPerSecond * 1000
                    AnimController.setCurrentTime(Math.max(0, time))
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // TRACK HEADERS (left side, scrolls vertically only)
    // ═══════════════════════════════════════════════════════════════
    Flickable {
        id: trackHeaders
        x: 0
        y: rulerHeight
        width: trackHeaderWidth
        height: parent.height - rulerHeight - bottomBarHeight
        contentHeight: Math.max(height, totalTracksHeight)
        clip: true
        interactive: false
        contentY: timelineContent.contentY

        // Camera track header
        Rectangle {
            id: cameraHeader
            x: 0
            y: 0
            width: trackHeaderWidth
            height: cameraTrackHeight
            color: bgCamera

            // Right border
            Rectangle {
                anchors.right: parent.right
                width: 1
                height: parent.height
                color: borderSubtle
            }

            // Bottom border
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: borderSubtle
                opacity: 0.5
            }

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                // Camera icon (simple)
                Rectangle {
                    width: 16
                    height: 12
                    radius: 2
                    color: "transparent"
                    border.color: accentColor
                    border.width: 1.5
                    anchors.verticalCenter: parent.verticalCenter

                    Rectangle {
                        x: parent.width - 2
                        anchors.verticalCenter: parent.verticalCenter
                        width: 6
                        height: 8
                        radius: 1
                        color: "transparent"
                        border.color: accentColor
                        border.width: 1.5
                    }
                }

                Text {
                    text: "Camera"
                    color: textPrimary
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Overlay track headers
        Repeater {
            model: GeoOverlays

            Rectangle {
                id: overlayHeader
                x: 0
                y: totalTracksHeight >= 0 ? getOverlayTrackY(index) : 0
                width: trackHeaderWidth
                height: {
                    if (!isExpanded) return overlayTrackHeight
                    var effectCount = GeoOverlays ? GeoOverlays.effectCount(index) : 0
                    return overlayTrackHeight + effectBarHeight * Math.max(1, effectCount)
                }
                color: index % 2 === 0 ? bgOverlay1 : bgOverlay2

                property bool isExpanded: false
                Component.onCompleted: isExpanded = GeoOverlays.isExpanded(index)

                Connections {
                    target: GeoOverlays
                    function onOverlayModified(idx) {
                        if (idx === index) totalTracksHeight = calculateTotalTracksHeight()
                    }
                    function onDataModified() {
                        totalTracksHeight = calculateTotalTracksHeight()
                    }
                }

                // Right border
                Rectangle {
                    anchors.right: parent.right
                    width: 1
                    height: parent.height
                    color: borderSubtle
                }

                // Bottom border
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: borderSubtle
                    opacity: 0.3
                }

                // Selection highlight
                Rectangle {
                    visible: GeoOverlays.selectedIndex === index
                    anchors.left: parent.left
                    width: 3
                    height: overlayTrackHeight
                    color: accentColor
                }

                // Main header row
                Item {
                    width: parent.width
                    height: overlayTrackHeight

                    // Expand button
                    Text {
                        x: 8
                        anchors.verticalCenter: parent.verticalCenter
                        text: overlayHeader.isExpanded ? "▼" : "▶"
                        color: expandMouse.containsMouse ? accentColor : textSecondary
                        font.pixelSize: 8

                        MouseArea {
                            id: expandMouse
                            anchors.fill: parent
                            anchors.margins: -6
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                overlayHeader.isExpanded = !overlayHeader.isExpanded
                                GeoOverlays.setExpanded(index, overlayHeader.isExpanded)
                                totalTracksHeight = calculateTotalTracksHeight()
                            }
                        }
                    }

                    // Track name
                    Text {
                        x: 24
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 32
                        text: model.name
                        color: GeoOverlays.selectedIndex === index ? textPrimary : textSecondary
                        font.pixelSize: 11
                        font.weight: GeoOverlays.selectedIndex === index ? Font.Medium : Font.Normal
                        elide: Text.ElideRight
                    }

                    MouseArea {
                        anchors.fill: parent
                        anchors.leftMargin: 20
                        onClicked: GeoOverlays.setSelectedIndex(index)
                    }
                }

                // Effect sub-headers (when expanded)
                Column {
                    visible: overlayHeader.isExpanded
                    y: overlayTrackHeight
                    width: parent.width

                    Repeater {
                        model: GeoOverlays ? GeoOverlays.getEffects(index) : []

                        Rectangle {
                            width: parent.width
                            height: effectBarHeight
                            color: "transparent"

                            Text {
                                x: 28
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.type || "Effect"
                                color: textSecondary
                                font.pixelSize: 9
                                opacity: 0.8
                            }
                        }
                    }

                    // Empty placeholder
                    Rectangle {
                        visible: GeoOverlays && GeoOverlays.effectCount(index) === 0
                        width: parent.width
                        height: effectBarHeight
                        color: "transparent"

                        Text {
                            x: 28
                            anchors.verticalCenter: parent.verticalCenter
                            text: "No effects"
                            color: textSecondary
                            font.pixelSize: 9
                            font.italic: true
                            opacity: 0.5
                        }
                    }
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // TIMELINE CONTENT (right side, scrolls both ways)
    // ═══════════════════════════════════════════════════════════════
    Flickable {
        id: timelineContent
        x: trackHeaderWidth
        y: rulerHeight
        width: parent.width - trackHeaderWidth
        height: parent.height - rulerHeight - bottomBarHeight
        contentWidth: Math.max(width, effectiveExtent * pixelsPerSecond / 1000 + 200)
        contentHeight: Math.max(height, totalTracksHeight)
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        interactive: false  // Disable default drag - we handle it manually

        ScrollBar.vertical: ScrollBar {
            policy: totalTracksHeight > timelineContent.height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            width: 10
        }

        ScrollBar.horizontal: ScrollBar {
            policy: timelineContent.contentWidth > timelineContent.width ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            height: 10
        }

        // Grid lines background
        Item {
            width: timelineContent.contentWidth
            height: Math.max(timelineContent.height, totalTracksHeight)

            // Vertical grid lines
            Repeater {
                model: Math.ceil(effectiveExtent / 1000) + 2
                Rectangle {
                    x: index * pixelsPerSecond
                    width: 1
                    height: parent.height
                    color: borderSubtle
                    opacity: index % 5 === 0 ? 0.4 : 0.15
                }
            }
        }

        // Camera track content
        Rectangle {
            id: cameraTrackContent
            x: 0
            y: 0
            width: timelineContent.contentWidth
            height: cameraTrackHeight
            color: bgCamera

            // Bottom border
            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: borderSubtle
                opacity: 0.5
            }

            // Speed curve editor - synced with C++ AnimController
            Item {
                id: speedCurveEditor
                anchors.fill: parent
                anchors.topMargin: 4
                anchors.bottomMargin: 4

                // Get curve points from C++ AnimController
                property var curvePoints: AnimController ? AnimController.getSpeedCurve() : [{time: 0, speed: 0.5}]
                property int selectedPoint: -1
                property int refreshCounter: 0

                // Sync with C++ when curve changes
                Connections {
                    target: AnimController
                    function onSpeedCurveChanged() {
                        speedCurveEditor.curvePoints = AnimController.getSpeedCurve()
                        speedCurveEditor.refreshCounter++
                        speedCanvas.requestPaint()
                    }
                }

                function addPoint(timeMs, speed) {
                    AnimController.addSpeedPoint(timeMs, speed)
                    // Find index of newly added point
                    let points = AnimController.getSpeedCurve()
                    for (let i = 0; i < points.length; i++) {
                        if (Math.abs(points[i].time - timeMs) < 100) return i
                    }
                    return 0
                }

                function removePoint(idx) {
                    AnimController.removeSpeedPoint(idx)
                    selectedPoint = -1
                }

                function updatePoint(idx, timeMs, speed) {
                    AnimController.updateSpeedPoint(idx, timeMs, speed)
                }

                function getSpeedAtTime(timeMs) {
                    return AnimController ? AnimController.getSpeedAtTime(timeMs) : 0.5
                }

                // Canvas for drawing the speed curve
                Canvas {
                    id: speedCanvas
                    anchors.fill: parent

                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.clearRect(0, 0, width, height)

                        let points = speedCurveEditor.curvePoints
                        if (points.length === 0) return

                        // Draw horizontal grid lines
                        ctx.strokeStyle = Qt.rgba(1, 1, 1, 0.05)
                        ctx.lineWidth = 1
                        for (let i = 1; i < 4; i++) {
                            let y = height * i / 4
                            ctx.beginPath()
                            ctx.moveTo(0, y)
                            ctx.lineTo(width, y)
                            ctx.stroke()
                        }

                        // Draw center line (normal speed)
                        ctx.strokeStyle = Qt.rgba(1, 1, 1, 0.15)
                        ctx.beginPath()
                        ctx.moveTo(0, height / 2)
                        ctx.lineTo(width, height / 2)
                        ctx.stroke()

                        // Draw the speed curve
                        ctx.strokeStyle = "#3d9cf0"
                        ctx.lineWidth = 2
                        ctx.beginPath()

                        // Start from left edge
                        let firstX = points[0].time * pixelsPerSecond / 1000
                        let firstY = height - points[0].speed * height
                        ctx.moveTo(0, firstY)
                        ctx.lineTo(firstX, firstY)

                        // Draw through all points
                        for (let i = 0; i < points.length; i++) {
                            let x = points[i].time * pixelsPerSecond / 1000
                            let y = height - points[i].speed * height
                            ctx.lineTo(x, y)
                        }

                        // Extend to right edge
                        let lastY = height - points[points.length - 1].speed * height
                        ctx.lineTo(width, lastY)
                        ctx.stroke()

                        // Draw fill under curve
                        ctx.fillStyle = Qt.rgba(0.24, 0.61, 0.94, 0.1)
                        ctx.beginPath()
                        ctx.moveTo(0, height)
                        ctx.lineTo(0, firstY)
                        ctx.lineTo(firstX, firstY)
                        for (let i = 0; i < points.length; i++) {
                            let x = points[i].time * pixelsPerSecond / 1000
                            let y = height - points[i].speed * height
                            ctx.lineTo(x, y)
                        }
                        ctx.lineTo(width, lastY)
                        ctx.lineTo(width, height)
                        ctx.closePath()
                        ctx.fill()
                    }
                }

                // Click to add points
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    z: -1

                    onDoubleClicked: (mouse) => {
                        let timeMs = mouse.x / pixelsPerSecond * 1000
                        let speed = 1 - mouse.y / height
                        let idx = speedCurveEditor.addPoint(timeMs, speed)
                        speedCurveEditor.selectedPoint = idx
                    }
                }

                // Control points
                Repeater {
                    model: speedCurveEditor.refreshCounter, speedCurveEditor.curvePoints.length

                    Rectangle {
                        id: pointHandle
                        property int pointIdx: index
                        property var pointData: speedCurveEditor.curvePoints[index] || {time: 0, speed: 0.5}

                        x: pointData.time * pixelsPerSecond / 1000 - 6
                        y: speedCurveEditor.height - pointData.speed * speedCurveEditor.height - 6
                        width: 12
                        height: 12
                        radius: 6
                        color: speedCurveEditor.selectedPoint === index ? "#ffffff" : "#3d9cf0"
                        border.color: "#ffffff"
                        border.width: speedCurveEditor.selectedPoint === index ? 2 : 1
                        z: 10

                        MouseArea {
                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            cursorShape: Qt.SizeAllCursor
                            acceptedButtons: Qt.LeftButton | Qt.RightButton

                            property real dragStartX: 0
                            property real dragStartY: 0
                            property real origTime: 0
                            property real origSpeed: 0

                            onPressed: (mouse) => {
                                speedCurveEditor.selectedPoint = pointIdx
                                dragStartX = mouse.x
                                dragStartY = mouse.y
                                origTime = pointData.time
                                origSpeed = pointData.speed
                            }

                            onPositionChanged: (mouse) => {
                                if (pressed && (pressedButtons & Qt.LeftButton)) {
                                    let deltaTime = (mouse.x - dragStartX) / pixelsPerSecond * 1000
                                    let deltaSpeed = -(mouse.y - dragStartY) / speedCurveEditor.height
                                    speedCurveEditor.updatePoint(pointIdx, origTime + deltaTime, origSpeed + deltaSpeed)
                                }
                            }

                            onClicked: (mouse) => {
                                if (mouse.button === Qt.RightButton) {
                                    speedCurveEditor.removePoint(pointIdx)
                                }
                            }
                        }
                    }
                }

                // Labels
                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 4
                    anchors.top: parent.top
                    text: "Fast"
                    color: textSecondary
                    font.pixelSize: 8
                    opacity: 0.5
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 4
                    anchors.bottom: parent.bottom
                    text: "Slow"
                    color: textSecondary
                    font.pixelSize: 8
                    opacity: 0.5
                }
            }

            // Camera keyframe markers - vertical lines
            Repeater {
                model: Keyframes

                Rectangle {
                    id: keyframeMarker
                    x: model.time * pixelsPerSecond / 1000
                    y: 0
                    width: 2
                    height: parent.height
                    color: Keyframes.currentIndex === index ? accentColor : "#4a7aaa"
                    z: 20

                    // Top handle
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: 4
                        width: 8
                        height: 8
                        radius: 2
                        color: parent.color
                    }

                    // Bottom handle
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 4
                        width: 8
                        height: 8
                        radius: 2
                        color: parent.color
                    }

                    MouseArea {
                        anchors.fill: parent
                        anchors.margins: -6
                        hoverEnabled: true
                        cursorShape: Qt.SizeHorCursor
                        acceptedButtons: Qt.LeftButton

                        property real dragStartX: 0
                        property real origTime: 0

                        onPressed: (mouse) => {
                            dragStartX = mouse.x
                            origTime = model.time
                            Keyframes.currentIndex = index
                            MainController.goToKeyframe(index)
                        }

                        onPositionChanged: (mouse) => {
                            if (pressed) {
                                let delta = (mouse.x - dragStartX) / pixelsPerSecond * 1000
                                let newTime = Math.max(0, origTime + delta)
                                Keyframes.setKeyframeTime(index, newTime)
                            }
                        }
                    }
                }
            }
        }

        // Overlay track contents
        Repeater {
            model: GeoOverlays

            Item {
                id: overlayTrackContent
                x: 0
                y: totalTracksHeight >= 0 ? getOverlayTrackY(index) : 0
                width: timelineContent.contentWidth
                height: {
                    if (!isExpanded) return overlayTrackHeight
                    var effectCount = GeoOverlays ? GeoOverlays.effectCount(index) : 0
                    return overlayTrackHeight + effectBarHeight * Math.max(1, effectCount)
                }

                property int overlayIdx: index
                property bool isExpanded: false
                property real trackStartTime: model.startTime
                property real trackEndTime: model.endTime
                property real trackFadeIn: model.fadeInDuration || 0
                property real trackFadeOut: model.fadeOutDuration || 0

                Component.onCompleted: isExpanded = GeoOverlays.isExpanded(index)

                Connections {
                    target: GeoOverlays
                    function onOverlayModified(idx) {
                        if (idx === overlayTrackContent.overlayIdx) {
                            totalTracksHeight = calculateTotalTracksHeight()
                        }
                    }
                    function onDataModified() {
                        totalTracksHeight = calculateTotalTracksHeight()
                    }
                }

                // Track background
                Rectangle {
                    anchors.fill: parent
                    color: index % 2 === 0 ? bgOverlay1 : bgOverlay2

                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: borderSubtle
                        opacity: 0.3
                    }
                }

                // Main overlay bar
                Rectangle {
                    id: overlayBar
                    x: trackStartTime * pixelsPerSecond / 1000
                    y: 4
                    width: Math.max(30, (trackEndTime - trackStartTime) * pixelsPerSecond / 1000)
                    height: overlayTrackHeight - 8
                    radius: 4
                    color: Qt.rgba(model.borderColor.r, model.borderColor.g, model.borderColor.b, 0.35)
                    border.color: model.borderColor
                    border.width: GeoOverlays.selectedIndex === index ? 2 : 1

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
                            GradientStop { position: 1.0; color: Qt.rgba(model.borderColor.r, model.borderColor.g, model.borderColor.b, 0.4) }
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
                            GradientStop { position: 0.0; color: Qt.rgba(model.borderColor.r, model.borderColor.g, model.borderColor.b, 0.4) }
                            GradientStop { position: 1.0; color: "transparent" }
                        }
                    }

                    // Fade in duration handle (internal)
                    Rectangle {
                        z: 5
                        visible: overlayBar.width > 40
                        x: Math.min(overlayBar.width * 0.4, trackFadeIn * pixelsPerSecond / 1000) - 1
                        y: 0
                        width: 2
                        height: parent.height
                        color: fadeInMouse.containsMouse || fadeInMouse.pressed ? "#ffffff" : (trackFadeIn > 0 ? Qt.rgba(1, 1, 1, 0.4) : "transparent")

                        MouseArea {
                            id: fadeInMouse
                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            cursorShape: Qt.SizeHorCursor
                            acceptedButtons: Qt.LeftButton
                            property real dragStartX: 0
                            property real origFadeIn: 0

                            onPressed: (mouse) => {
                                let globalPos = mapToItem(overlayTrackContent, mouse.x, 0)
                                dragStartX = globalPos.x
                                origFadeIn = trackFadeIn
                            }
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    let globalPos = mapToItem(overlayTrackContent, mouse.x, 0)
                                    let delta = (globalPos.x - dragStartX) / pixelsPerSecond * 1000
                                    let maxFadeIn = trackEndTime - trackStartTime - trackFadeOut
                                    let newFadeIn = Math.max(0, Math.min(maxFadeIn, origFadeIn + delta))
                                    GeoOverlays.setOverlayTiming(overlayIdx, trackStartTime, newFadeIn, trackEndTime, trackFadeOut)
                                }
                            }
                        }
                    }

                    // Fade out duration handle (internal)
                    Rectangle {
                        z: 5
                        visible: overlayBar.width > 40
                        x: parent.width - Math.min(overlayBar.width * 0.4, trackFadeOut * pixelsPerSecond / 1000) - 1
                        y: 0
                        width: 2
                        height: parent.height
                        color: fadeOutMouse.containsMouse || fadeOutMouse.pressed ? "#ffffff" : (trackFadeOut > 0 ? Qt.rgba(1, 1, 1, 0.4) : "transparent")

                        MouseArea {
                            id: fadeOutMouse
                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            cursorShape: Qt.SizeHorCursor
                            acceptedButtons: Qt.LeftButton
                            property real dragStartX: 0
                            property real origFadeOut: 0

                            onPressed: (mouse) => {
                                let globalPos = mapToItem(overlayTrackContent, mouse.x, 0)
                                dragStartX = globalPos.x
                                origFadeOut = trackFadeOut
                            }
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    let globalPos = mapToItem(overlayTrackContent, mouse.x, 0)
                                    let delta = (globalPos.x - dragStartX) / pixelsPerSecond * 1000
                                    let maxFadeOut = trackEndTime - trackStartTime - trackFadeIn
                                    let newFadeOut = Math.max(0, Math.min(maxFadeOut, origFadeOut - delta))
                                    GeoOverlays.setOverlayTiming(overlayIdx, trackStartTime, trackFadeIn, trackEndTime, newFadeOut)
                                }
                            }
                        }
                    }

                    // Overlay name on bar
                    Text {
                        visible: parent.width > 60
                        anchors.centerIn: parent
                        text: model.name
                        color: "#ffffff"
                        font.pixelSize: 10
                        font.weight: Font.Medium
                        opacity: 0.9
                        elide: Text.ElideRight
                        width: parent.width - 20
                        horizontalAlignment: Text.AlignHCenter
                    }

                    // Left trim handle
                    Rectangle {
                        z: 6
                        width: 5
                        height: parent.height
                        anchors.left: parent.left
                        radius: parent.radius
                        color: leftMouse.containsMouse || leftMouse.pressed ? "#ffffff" : "transparent"
                        opacity: 0.8

                        MouseArea {
                            id: leftMouse
                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            cursorShape: Qt.SizeHorCursor
                            acceptedButtons: Qt.LeftButton
                            property real dragStartX: 0
                            property real origStart: 0

                            onPressed: (mouse) => {
                                let globalPos = mapToItem(overlayTrackContent, mouse.x, 0)
                                dragStartX = globalPos.x
                                origStart = trackStartTime
                            }
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    let globalPos = mapToItem(overlayTrackContent, mouse.x, 0)
                                    let delta = (globalPos.x - dragStartX) / pixelsPerSecond * 1000
                                    let newStart = Math.max(0, Math.min(trackEndTime - 500, origStart + delta))
                                    GeoOverlays.setOverlayTiming(overlayIdx, newStart, trackFadeIn, trackEndTime, trackFadeOut)
                                }
                            }
                        }
                    }

                    // Right trim handle
                    Rectangle {
                        z: 6
                        width: 5
                        height: parent.height
                        anchors.right: parent.right
                        radius: parent.radius
                        color: rightMouse.containsMouse || rightMouse.pressed ? "#ffffff" : "transparent"
                        opacity: 0.8

                        MouseArea {
                            id: rightMouse
                            anchors.fill: parent
                            anchors.margins: -4
                            hoverEnabled: true
                            cursorShape: Qt.SizeHorCursor
                            acceptedButtons: Qt.LeftButton
                            property real dragStartX: 0
                            property real origEnd: 0

                            onPressed: (mouse) => {
                                let globalPos = mapToItem(overlayTrackContent, mouse.x, 0)
                                dragStartX = globalPos.x
                                origEnd = trackEndTime
                            }
                            onPositionChanged: (mouse) => {
                                if (pressed) {
                                    let globalPos = mapToItem(overlayTrackContent, mouse.x, 0)
                                    let delta = (globalPos.x - dragStartX) / pixelsPerSecond * 1000
                                    let newEnd = Math.max(trackStartTime + 500, origEnd + delta)
                                    GeoOverlays.setOverlayTiming(overlayIdx, trackStartTime, trackFadeIn, newEnd, trackFadeOut)
                                }
                            }
                        }
                    }

                    // Middle drag (left click only)
                    MouseArea {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        acceptedButtons: Qt.LeftButton
                        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                        property real dragStartX: 0
                        property real origStart: 0
                        property real origEnd: 0

                        onPressed: (mouse) => {
                            let globalPos = mapToItem(overlayTrackContent, mouse.x, 0)
                            dragStartX = globalPos.x
                            origStart = trackStartTime
                            origEnd = trackEndTime
                            GeoOverlays.setSelectedIndex(overlayIdx)
                        }
                        onPositionChanged: (mouse) => {
                            if (pressed) {
                                let globalPos = mapToItem(overlayTrackContent, mouse.x, 0)
                                let delta = (globalPos.x - dragStartX) / pixelsPerSecond * 1000
                                let duration = origEnd - origStart
                                let newStart = Math.max(0, origStart + delta)
                                GeoOverlays.setOverlayTiming(overlayIdx, newStart, trackFadeIn, newStart + duration, trackFadeOut)
                            }
                        }
                    }
                }

                // Effect bars (when expanded)
                Column {
                    visible: overlayTrackContent.isExpanded
                    y: overlayTrackHeight
                    width: parent.width

                    Repeater {
                        model: GeoOverlays ? GeoOverlays.getEffects(overlayTrackContent.overlayIdx) : []

                        Rectangle {
                            width: parent.width
                            height: effectBarHeight
                            color: "transparent"

                            property int effectIdx: index
                            property color effectColor: {
                                switch(modelData.type) {
                                    case "opacity": return "#4CAF50"
                                    case "extrusion": return "#2196F3"
                                    case "scale": return "#FF9800"
                                    case "fillColor": return "#E91E63"
                                    case "borderColor": return "#9C27B0"
                                    default: return accentColor
                                }
                            }

                            // Effect bar
                            Rectangle {
                                x: modelData.startTime * pixelsPerSecond / 1000
                                y: 3
                                width: Math.max(20, (modelData.endTime - modelData.startTime) * pixelsPerSecond / 1000)
                                height: parent.height - 6
                                radius: 3
                                color: Qt.rgba(effectColor.r, effectColor.g, effectColor.b, 0.3)
                                border.color: effectColor
                                border.width: 1

                                Text {
                                    visible: parent.width > 40
                                    anchors.centerIn: parent
                                    text: modelData.value !== undefined ? modelData.value.toFixed(1) : ""
                                    color: "#ffffff"
                                    font.pixelSize: 9
                                    opacity: 0.8
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    acceptedButtons: Qt.LeftButton
                                    onClicked: timeline.effectSelected(overlayTrackContent.overlayIdx, effectIdx)
                                }
                            }
                        }
                    }

                    // Empty state
                    Rectangle {
                        visible: GeoOverlays && GeoOverlays.effectCount(overlayTrackContent.overlayIdx) === 0
                        width: parent.width
                        height: effectBarHeight
                        color: "transparent"

                        Text {
                            anchors.centerIn: parent
                            text: "Click + in Overlay panel to add effects"
                            color: textSecondary
                            font.pixelSize: 9
                            font.italic: true
                            opacity: 0.5
                        }
                    }
                }
            }
        }

        // Playhead
        Rectangle {
            id: playhead
            x: currentTime * pixelsPerSecond / 1000
            y: 0
            width: 2
            height: Math.max(timelineContent.height, totalTracksHeight)
            color: "#ff4757"
            z: 100

            // Playhead top marker
            Rectangle {
                width: 10
                height: 10
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: -5
                rotation: 45
                color: "#ff4757"
            }
        }

        // Click to scrub, drag to pan
        MouseArea {
            anchors.fill: parent
            z: -1
            acceptedButtons: Qt.LeftButton

            onPressed: (mouse) => {
                timeline.forceActiveFocus()
                Keyframes.editMode = false

                // Set playhead on click
                let time = mouse.x / pixelsPerSecond * 1000
                AnimController.setCurrentTime(Math.max(0, time))
            }
            onPositionChanged: (mouse) => {
                if (pressed) {
                    let time = mouse.x / pixelsPerSecond * 1000
                    AnimController.setCurrentTime(Math.max(0, time))
                }
            }

            onWheel: (wheel) => {
                if (wheel.modifiers & Qt.ControlModifier) {
                    // Ctrl+wheel = zoom
                    let zoomFactor = wheel.angleDelta.y > 0 ? 1.15 : 0.87
                    let newZoom = Math.max(0.1, Math.min(8.0, Settings.timelineZoom * zoomFactor))
                    Settings.timelineZoom = newZoom
                } else if (wheel.modifiers & Qt.ShiftModifier) {
                    // Shift+wheel = horizontal scroll
                    let scrollAmount = wheel.angleDelta.y > 0 ? -60 : 60
                    timelineContent.contentX = Math.max(0, Math.min(
                        timelineContent.contentWidth - timelineContent.width,
                        timelineContent.contentX + scrollAmount
                    ))
                } else {
                    // Regular wheel = vertical scroll
                    let scrollAmount = wheel.angleDelta.y > 0 ? -40 : 40
                    timelineContent.contentY = Math.max(0, Math.min(
                        timelineContent.contentHeight - timelineContent.height,
                        timelineContent.contentY + scrollAmount
                    ))
                }
                wheel.accepted = true
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════
    // BOTTOM BAR
    // ═══════════════════════════════════════════════════════════════
    Rectangle {
        id: bottomBar
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: bottomBarHeight
        color: bgHeader

        // Top border
        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: borderSubtle
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 12

            Text {
                text: "Duration:"
                color: textSecondary
                font.pixelSize: 11
            }

            Rectangle {
                implicitWidth: 50
                implicitHeight: 22
                color: bgOverlay1
                border.color: durationInput.activeFocus ? accentColor : borderSubtle
                radius: 3

                TextInput {
                    id: durationInput
                    anchors.fill: parent
                    anchors.margins: 4
                    text: Math.round(explicitDuration / 1000).toString()
                    horizontalAlignment: TextInput.AlignHCenter
                    verticalAlignment: TextInput.AlignVCenter
                    color: textPrimary
                    font.pixelSize: 11
                    selectByMouse: true
                    validator: IntValidator { bottom: 1; top: 3600 }

                    onEditingFinished: {
                        let val = parseInt(text) || 60
                        val = Math.max(1, Math.min(3600, val))
                        if (AnimController) AnimController.explicitDuration = val * 1000
                    }
                }
            }

            Text {
                text: "s"
                color: textSecondary
                font.pixelSize: 11
            }

            Item { Layout.fillWidth: true }

            Text {
                text: formatTime(currentTime) + " / " + formatTime(totalDuration)
                color: textPrimary
                font.pixelSize: 11
                font.family: "Consolas"
            }

            Rectangle {
                width: 1
                height: 16
                color: borderSubtle
            }

            Text {
                text: (Settings.timelineZoom * 100).toFixed(0) + "%"
                color: textSecondary
                font.pixelSize: 11
            }
        }
    }

    function formatTime(ms) {
        let seconds = Math.floor(ms / 1000)
        let minutes = Math.floor(seconds / 60)
        seconds = seconds % 60
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }
}
