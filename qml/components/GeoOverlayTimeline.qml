import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

Item {
    id: geoOverlayTimeline

    property real pixelsPerSecond: 100
    property real totalDuration: 60000  // Default 60 seconds

    // Height per track
    property int trackHeight: 30
    property int headerWidth: 150

    // Always show at least the header, expand for tracks
    implicitHeight: Math.max(60, GeoOverlays.count * trackHeight + 50)

    Rectangle {
        anchors.fill: parent
        color: Theme.timelineBackground
    }

    // Header column (overlay names)
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

        // Header label
        Text {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 8
            height: 28
            text: qsTr("Geo Overlays")
            color: Theme.textColorDim
            font.pixelSize: 11
            font.bold: true
            verticalAlignment: Text.AlignVCenter
        }

        // Overlay labels
        Column {
            anchors.top: parent.top
            anchors.topMargin: 40
            anchors.left: parent.left
            anchors.right: parent.right

            Repeater {
                model: GeoOverlays

                Rectangle {
                    width: headerColumn.width
                    height: trackHeight
                    color: trackMouseArea.containsMouse ? Theme.surfaceColorLight : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 4
                        anchors.rightMargin: 4
                        spacing: 2

                        // Type indicator (color-coded)
                        Rectangle {
                            width: 14
                            height: 14
                            radius: model.overlayType === 2 ? 7 : 2  // Circle for cities
                            color: model.fillColor
                            border.color: model.borderColor
                            border.width: 1
                        }

                        // Name and type
                        Column {
                            Layout.fillWidth: true
                            spacing: 0

                            Text {
                                width: parent.width
                                text: model.name
                                color: Theme.textColor
                                font.pixelSize: 10
                                elide: Text.ElideRight
                            }

                            Text {
                                width: parent.width
                                text: model.typeString + (model.parentName ? " - " + model.parentName : "")
                                color: Theme.textColorDim
                                font.pixelSize: 8
                                elide: Text.ElideRight
                            }
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
                                onClicked: GeoOverlays.removeOverlay(index)
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
                            text: qsTr("Edit Appearance...")
                            onTriggered: {
                                overlayEditor.overlayIndex = index
                                overlayEditor.open()
                            }
                        }
                        MenuItem {
                            text: qsTr("Delete")
                            onTriggered: GeoOverlays.removeOverlay(index)
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

        // Grid lines
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
            anchors.topMargin: 40

            Repeater {
                model: GeoOverlays

                Item {
                    width: trackFlickable.contentWidth
                    height: trackHeight

                    property int overlayIndex: index

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
                        radius: model.overlayType === 2 ? height/2 : 4  // Rounded for cities
                        color: Qt.rgba(model.fillColor.r, model.fillColor.g, model.fillColor.b, 0.6)
                        border.color: model.borderColor
                        border.width: 1

                        // Fade in indicator
                        Rectangle {
                            width: model.fadeInDuration * pixelsPerSecond / 1000
                            height: parent.height
                            radius: parent.radius
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
                            radius: parent.radius
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: Qt.rgba(model.fillColor.r, model.fillColor.g, model.fillColor.b, 0.3) }
                                GradientStop { position: 1.0; color: "transparent" }
                            }
                        }

                        // Keyframe markers (diamonds)
                        Repeater {
                            model: GeoOverlays.getAllKeyframes(overlayIndex)

                            Rectangle {
                                id: keyframeDiamond
                                property var kfData: modelData
                                x: (kfData.timeMs - model.startTime) * pixelsPerSecond / 1000 - width/2
                                y: (trackBar.height - height) / 2
                                width: 12
                                height: 12
                                rotation: 45
                                color: keyframeSelected ? Theme.primaryColor : "#ffffff"
                                border.color: kfData.extrusion > 0 ? "#ffcc00" : "#888888"
                                border.width: 2

                                property bool keyframeSelected: false

                                MouseArea {
                                    anchors.fill: parent
                                    anchors.margins: -4
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor

                                    onClicked: (mouse) => {
                                        if (mouse.button === Qt.LeftButton) {
                                            // Select this keyframe
                                            keyframeDiamond.keyframeSelected = true
                                            keyframeEditor.overlayIndex = overlayIndex
                                            keyframeEditor.keyframeIndex = index
                                            keyframeEditor.open()
                                        }
                                    }

                                    onDoubleClicked: {
                                        // Delete keyframe on double-click
                                        GeoOverlays.removeKeyframe(overlayIndex, index)
                                    }

                                    ToolTip.visible: containsMouse
                                    ToolTip.text: qsTr("Extrusion: %1\nOpacity: %2\nScale: %3\nClick to edit, double-click to delete")
                                        .arg(kfData.extrusion.toFixed(1))
                                        .arg(kfData.opacity.toFixed(2))
                                        .arg(kfData.scale.toFixed(2))
                                }
                            }
                        }

                        // Drag to move bar
                        MouseArea {
                            id: barDragArea
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            cursorShape: Qt.SizeAllCursor
                            drag.target: trackBar
                            drag.axis: Drag.XAxis
                            drag.minimumX: 20
                            z: -1  // Below keyframe markers

                            onReleased: {
                                let newStartTime = (trackBar.x - 20) / pixelsPerSecond * 1000
                                newStartTime = Math.max(0, newStartTime)
                                GeoOverlays.updateOverlay(overlayIndex, {"startTime": newStartTime})
                            }

                            onDoubleClicked: (mouse) => {
                                // Add keyframe at clicked position
                                let localX = mouse.x
                                let timeMs = model.startTime + localX / pixelsPerSecond * 1000
                                GeoOverlays.addKeyframe(overlayIndex, timeMs)
                            }
                        }

                        // Left edge - adjust start time
                        Rectangle {
                            width: 8
                            height: parent.height
                            color: leftEdge.containsMouse ? Theme.primaryColor : "transparent"
                            radius: parent.radius

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
                                        GeoOverlays.updateOverlay(overlayIndex, {"startTime": newStart})
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
                            radius: parent.radius

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
                                        GeoOverlays.updateOverlay(overlayIndex, {"endTime": newEnd})
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

    // Overlay editor dialog
    Dialog {
        id: overlayEditor
        title: qsTr("Edit Overlay Appearance")
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 350

        property int overlayIndex: -1

        contentItem: ColumnLayout {
            spacing: Theme.spacingNormal

            RowLayout {
                Label { text: qsTr("Fill Color:"); Layout.preferredWidth: 100 }
                Rectangle {
                    width: 40
                    height: 24
                    color: overlayEditor.overlayIndex >= 0 ? GeoOverlays.getOverlay(overlayEditor.overlayIndex).fillColor : "red"
                    border.color: Theme.borderColor
                    radius: 4

                    MouseArea {
                        anchors.fill: parent
                        onClicked: fillColorPicker.open()
                    }
                }
            }

            RowLayout {
                Label { text: qsTr("Border Color:"); Layout.preferredWidth: 100 }
                Rectangle {
                    width: 40
                    height: 24
                    color: overlayEditor.overlayIndex >= 0 ? GeoOverlays.getOverlay(overlayEditor.overlayIndex).borderColor : "red"
                    border.color: Theme.borderColor
                    radius: 4

                    MouseArea {
                        anchors.fill: parent
                        onClicked: borderColorPicker.open()
                    }
                }
            }

            RowLayout {
                Label { text: qsTr("Fade In (ms):"); Layout.preferredWidth: 100 }
                SpinBox {
                    from: 0
                    to: 5000
                    stepSize: 100
                    value: overlayEditor.overlayIndex >= 0 ? GeoOverlays.getOverlay(overlayEditor.overlayIndex).fadeInDuration : 500
                    onValueModified: {
                        if (overlayEditor.overlayIndex >= 0) {
                            GeoOverlays.updateOverlay(overlayEditor.overlayIndex, {"fadeInDuration": value})
                        }
                    }
                }
            }

            RowLayout {
                Label { text: qsTr("Fade Out (ms):"); Layout.preferredWidth: 100 }
                SpinBox {
                    from: 0
                    to: 5000
                    stepSize: 100
                    value: overlayEditor.overlayIndex >= 0 ? GeoOverlays.getOverlay(overlayEditor.overlayIndex).fadeOutDuration : 500
                    onValueModified: {
                        if (overlayEditor.overlayIndex >= 0) {
                            GeoOverlays.updateOverlay(overlayEditor.overlayIndex, {"fadeOutDuration": value})
                        }
                    }
                }
            }
        }

        standardButtons: Dialog.Close
    }

    // Color picker dialogs
    Dialog {
        id: fillColorPicker
        title: qsTr("Select Fill Color")
        modal: true
        parent: Overlay.overlay
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
                            if (overlayEditor.overlayIndex >= 0) {
                                GeoOverlays.updateOverlay(overlayEditor.overlayIndex, {"fillColor": modelData})
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
        parent: Overlay.overlay
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
                            if (overlayEditor.overlayIndex >= 0) {
                                GeoOverlays.updateOverlay(overlayEditor.overlayIndex, {"borderColor": modelData})
                            }
                            borderColorPicker.close()
                        }
                    }
                }
            }
        }
    }

    // Keyframe editor dialog - edit the 5 animatable properties
    Dialog {
        id: keyframeEditor
        title: qsTr("Edit Keyframe Properties")
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 380

        property int overlayIndex: -1
        property int keyframeIndex: -1
        property var keyframeData: overlayIndex >= 0 && keyframeIndex >= 0 ?
            GeoOverlays.getKeyframe(overlayIndex, keyframeIndex) : {}

        onOpened: {
            if (overlayIndex >= 0 && keyframeIndex >= 0) {
                keyframeData = GeoOverlays.getKeyframe(overlayIndex, keyframeIndex)
                extrusionSlider.value = keyframeData.extrusion || 0
                opacitySlider.value = (keyframeData.opacity || 1) * 100
                scaleSlider.value = (keyframeData.scale || 1) * 100
            }
        }

        contentItem: ColumnLayout {
            spacing: Theme.spacingNormal

            // Time position
            RowLayout {
                Label { text: qsTr("Time (ms):"); Layout.preferredWidth: 100 }
                SpinBox {
                    id: timeSpinBox
                    from: 0
                    to: 999999
                    stepSize: 100
                    value: keyframeEditor.keyframeData.timeMs || 0
                    editable: true
                    onValueModified: {
                        if (keyframeEditor.overlayIndex >= 0 && keyframeEditor.keyframeIndex >= 0) {
                            GeoOverlays.moveKeyframe(keyframeEditor.overlayIndex, keyframeEditor.keyframeIndex, value)
                        }
                    }
                }
            }

            // Extrusion (0-100)
            RowLayout {
                Label { text: qsTr("Extrusion:"); Layout.preferredWidth: 100 }
                Slider {
                    id: extrusionSlider
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    stepSize: 1
                    value: keyframeEditor.keyframeData.extrusion || 0
                    onMoved: {
                        if (keyframeEditor.overlayIndex >= 0 && keyframeEditor.keyframeIndex >= 0) {
                            GeoOverlays.updateKeyframe(keyframeEditor.overlayIndex, keyframeEditor.keyframeIndex,
                                {"extrusion": value})
                        }
                    }
                }
                Label {
                    text: extrusionSlider.value.toFixed(0)
                    Layout.preferredWidth: 30
                }
            }

            // Fill Color
            RowLayout {
                Label { text: qsTr("Fill Color:"); Layout.preferredWidth: 100 }
                Rectangle {
                    width: 40
                    height: 24
                    color: keyframeEditor.keyframeData.fillColor || "#ff0000"
                    border.color: Theme.borderColor
                    radius: 4

                    MouseArea {
                        anchors.fill: parent
                        onClicked: kfFillColorPicker.open()
                    }
                }
            }

            // Border Color
            RowLayout {
                Label { text: qsTr("Border Color:"); Layout.preferredWidth: 100 }
                Rectangle {
                    width: 40
                    height: 24
                    color: keyframeEditor.keyframeData.borderColor || "#ff0000"
                    border.color: Theme.borderColor
                    radius: 4

                    MouseArea {
                        anchors.fill: parent
                        onClicked: kfBorderColorPicker.open()
                    }
                }
            }

            // Opacity (0-100%)
            RowLayout {
                Label { text: qsTr("Opacity:"); Layout.preferredWidth: 100 }
                Slider {
                    id: opacitySlider
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    stepSize: 1
                    value: (keyframeEditor.keyframeData.opacity || 1) * 100
                    onMoved: {
                        if (keyframeEditor.overlayIndex >= 0 && keyframeEditor.keyframeIndex >= 0) {
                            GeoOverlays.updateKeyframe(keyframeEditor.overlayIndex, keyframeEditor.keyframeIndex,
                                {"opacity": value / 100})
                        }
                    }
                }
                Label {
                    text: (opacitySlider.value).toFixed(0) + "%"
                    Layout.preferredWidth: 40
                }
            }

            // Scale (80-150%)
            RowLayout {
                Label { text: qsTr("Scale:"); Layout.preferredWidth: 100 }
                Slider {
                    id: scaleSlider
                    Layout.fillWidth: true
                    from: 80
                    to: 150
                    stepSize: 1
                    value: (keyframeEditor.keyframeData.scale || 1) * 100
                    onMoved: {
                        if (keyframeEditor.overlayIndex >= 0 && keyframeEditor.keyframeIndex >= 0) {
                            GeoOverlays.updateKeyframe(keyframeEditor.overlayIndex, keyframeEditor.keyframeIndex,
                                {"scale": value / 100})
                        }
                    }
                }
                Label {
                    text: (scaleSlider.value).toFixed(0) + "%"
                    Layout.preferredWidth: 40
                }
            }

            // Easing type
            RowLayout {
                Label { text: qsTr("Easing:"); Layout.preferredWidth: 100 }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["Linear", "Ease In/Out", "Ease In", "Ease Out", "Ease In/Out Cubic", "Ease In/Out Quint"]
                    currentIndex: keyframeEditor.keyframeData.easingType || 1
                    onCurrentIndexChanged: {
                        if (keyframeEditor.overlayIndex >= 0 && keyframeEditor.keyframeIndex >= 0) {
                            GeoOverlays.updateKeyframe(keyframeEditor.overlayIndex, keyframeEditor.keyframeIndex,
                                {"easingType": currentIndex})
                        }
                    }
                }
            }

            // Delete button
            Button {
                text: qsTr("Delete Keyframe")
                Layout.fillWidth: true
                Material.background: Theme.errorColor
                onClicked: {
                    if (keyframeEditor.overlayIndex >= 0 && keyframeEditor.keyframeIndex >= 0) {
                        GeoOverlays.removeKeyframe(keyframeEditor.overlayIndex, keyframeEditor.keyframeIndex)
                        keyframeEditor.close()
                    }
                }
            }
        }

        standardButtons: Dialog.Close
    }

    // Keyframe fill color picker
    Dialog {
        id: kfFillColorPicker
        title: qsTr("Select Fill Color")
        modal: true
        parent: Overlay.overlay
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
                            if (keyframeEditor.overlayIndex >= 0 && keyframeEditor.keyframeIndex >= 0) {
                                GeoOverlays.updateKeyframe(keyframeEditor.overlayIndex, keyframeEditor.keyframeIndex,
                                    {"fillColor": modelData})
                                keyframeEditor.keyframeData = GeoOverlays.getKeyframe(
                                    keyframeEditor.overlayIndex, keyframeEditor.keyframeIndex)
                            }
                            kfFillColorPicker.close()
                        }
                    }
                }
            }
        }
    }

    // Keyframe border color picker
    Dialog {
        id: kfBorderColorPicker
        title: qsTr("Select Border Color")
        modal: true
        parent: Overlay.overlay
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
                            if (keyframeEditor.overlayIndex >= 0 && keyframeEditor.keyframeIndex >= 0) {
                                GeoOverlays.updateKeyframe(keyframeEditor.overlayIndex, keyframeEditor.keyframeIndex,
                                    {"borderColor": modelData})
                                keyframeEditor.keyframeData = GeoOverlays.getKeyframe(
                                    keyframeEditor.overlayIndex, keyframeEditor.keyframeIndex)
                            }
                            kfBorderColorPicker.close()
                        }
                    }
                }
            }
        }
    }
}
