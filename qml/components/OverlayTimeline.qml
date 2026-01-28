import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

// Standalone overlay timeline
Rectangle {
    id: root
    color: Theme.surfaceColor

    property real pixelsPerSecond: 50
    property real totalDuration: 60000

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header with add buttons
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            color: Theme.surfaceColorLight

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Text {
                    text: "Overlay Timeline"
                    color: Theme.textColor
                    font.bold: true
                    font.pixelSize: 14
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "+ Country"
                    onClicked: countryDialog.open()
                }
                Button {
                    text: "+ Region"
                    onClicked: regionDialog.open()
                }
                Button {
                    text: "+ City"
                    onClicked: cityDialog.open()
                }
            }
        }

        // Tracks area with time ruler
        Rectangle {
            id: tracksArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.timelineBackground

            // Time ruler at top
            Rectangle {
                id: timeRuler
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 24
                color: Theme.surfaceColorLight
                z: 10

                Flickable {
                    id: rulerFlickable
                    anchors.fill: parent
                    anchors.leftMargin: 150  // Match label column width
                    contentWidth: Math.max(width, totalDuration * pixelsPerSecond / 1000 + 100)
                    clip: true
                    interactive: false
                    contentX: tracksFlickable.contentX

                    // Time markers
                    Repeater {
                        model: Math.ceil(totalDuration / 1000) + 2

                        Item {
                            x: index * pixelsPerSecond
                            height: parent.height

                            Rectangle {
                                width: 1
                                height: 8
                                color: Theme.textColorDim
                                anchors.bottom: parent.bottom
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.top: parent.top
                                anchors.topMargin: 2
                                text: {
                                    let sec = index
                                    let min = Math.floor(sec / 60)
                                    sec = sec % 60
                                    return min + ":" + (sec < 10 ? "0" : "") + sec
                                }
                                color: Theme.textColorDim
                                font.pixelSize: 9
                            }
                        }
                    }
                }
            }

            // Empty state
            Text {
                anchors.centerIn: parent
                text: "Click + Country, + Region, or + City to add overlays"
                color: Theme.textColorDim
                font.pixelSize: 14
                visible: GeoOverlays.count === 0
            }

            // Scrollable track list
            Flickable {
                id: tracksFlickable
                anchors.top: timeRuler.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                contentWidth: Math.max(width, totalDuration * pixelsPerSecond / 1000 + 100)
                contentHeight: trackColumn.height
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                // Scroll with shift+wheel, zoom with wheel
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                    onWheel: (wheel) => {
                        if (wheel.modifiers & Qt.ShiftModifier) {
                            // Shift+wheel: horizontal scroll
                            let scrollAmount = wheel.angleDelta.y > 0 ? -80 : 80
                            tracksFlickable.contentX = Math.max(0, Math.min(
                                tracksFlickable.contentWidth - tracksFlickable.width,
                                tracksFlickable.contentX + scrollAmount
                            ))
                        } else {
                            // Normal wheel: zoom
                            let zoomFactor = wheel.angleDelta.y > 0 ? 1.2 : 0.83
                            let newPPS = Math.max(10, Math.min(200, pixelsPerSecond * zoomFactor))
                            pixelsPerSecond = newPPS
                        }
                        wheel.accepted = true
                    }
                }

                Column {
                    id: trackColumn
                    width: tracksFlickable.contentWidth
                    visible: GeoOverlays.count > 0

                    Repeater {
                        id: trackRepeater
                        model: GeoOverlays

                        delegate: Rectangle {
                            width: trackColumn.width
                            height: 50
                            color: index % 2 === 0 ? "#1a2a3a" : "#152535"
                            property int overlayIdx: index

                            RowLayout {
                                anchors.fill: parent
                                spacing: 0

                                // Label column
                                Rectangle {
                                    Layout.preferredWidth: 150
                                    Layout.fillHeight: true
                                    color: Theme.surfaceColor

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 4
                                        spacing: 4

                                        Rectangle {
                                            width: 16
                                            height: 16
                                            radius: model.overlayType === 2 ? 8 : 2
                                            color: model.fillColor
                                            border.color: model.borderColor
                                        }

                                        Text {
                                            text: model.name
                                            color: Theme.textColor
                                            font.pixelSize: 11
                                            elide: Text.ElideRight
                                            Layout.fillWidth: true
                                        }

                                        Text {
                                            text: "×"
                                            color: delArea.containsMouse ? Theme.primaryColor : Theme.textColorDim
                                            font.pixelSize: 16
                                            font.bold: true
                                            MouseArea {
                                                id: delArea
                                                anchors.fill: parent
                                                anchors.margins: -4
                                                hoverEnabled: true
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: GeoOverlays.removeOverlay(overlayIdx)
                                            }
                                        }
                                    }
                                }

                                // Track content
                                Rectangle {
                                    id: trackContent
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    color: "transparent"

                                    property real startTimeMs: model.startTime || 0
                                    property real endTimeMs: model.endTime > 0 ? model.endTime : 10000

                                    Rectangle {
                                        id: trackBar
                                        x: 10 + trackContent.startTimeMs * pixelsPerSecond / 1000
                                        y: 8
                                        width: Math.max(50, (trackContent.endTimeMs - trackContent.startTimeMs) * pixelsPerSecond / 1000)
                                        height: parent.height - 16
                                        radius: 4
                                        color: Qt.rgba(model.fillColor.r, model.fillColor.g, model.fillColor.b, 0.7)
                                        border.color: model.borderColor

                                        // Left drag handle
                                        Rectangle {
                                            id: leftHandle
                                            width: 8
                                            height: parent.height
                                            anchors.left: parent.left
                                            anchors.verticalCenter: parent.verticalCenter
                                            color: leftHandleArea.containsMouse || leftHandleArea.pressed ? Theme.primaryColor : "transparent"
                                            radius: 2

                                            MouseArea {
                                                id: leftHandleArea
                                                anchors.fill: parent
                                                anchors.margins: -4
                                                hoverEnabled: true
                                                cursorShape: Qt.SizeHorCursor
                                                property real dragStartX: 0
                                                property real originalStartTime: 0

                                                onPressed: (mouse) => {
                                                    dragStartX = mouse.x + trackBar.x
                                                    originalStartTime = trackContent.startTimeMs
                                                }
                                                onPositionChanged: (mouse) => {
                                                    if (pressed) {
                                                        let deltaX = (mouse.x + trackBar.x) - dragStartX
                                                        let deltaMs = deltaX / pixelsPerSecond * 1000
                                                        let newStart = Math.max(0, originalStartTime + deltaMs)
                                                        newStart = Math.min(newStart, trackContent.endTimeMs - 500)
                                                        GeoOverlays.setOverlayTiming(overlayIdx, newStart, 0, trackContent.endTimeMs, 0)
                                                    }
                                                }
                                            }
                                        }

                                        // Right drag handle
                                        Rectangle {
                                            id: rightHandle
                                            width: 8
                                            height: parent.height
                                            anchors.right: parent.right
                                            anchors.verticalCenter: parent.verticalCenter
                                            color: rightHandleArea.containsMouse || rightHandleArea.pressed ? Theme.primaryColor : "transparent"
                                            radius: 2

                                            MouseArea {
                                                id: rightHandleArea
                                                anchors.fill: parent
                                                anchors.margins: -4
                                                hoverEnabled: true
                                                cursorShape: Qt.SizeHorCursor
                                                property real dragStartX: 0
                                                property real originalEndTime: 0

                                                onPressed: (mouse) => {
                                                    dragStartX = mouse.x + trackBar.x + trackBar.width
                                                    originalEndTime = trackContent.endTimeMs
                                                }
                                                onPositionChanged: (mouse) => {
                                                    if (pressed) {
                                                        let deltaX = (mouse.x + trackBar.x + trackBar.width - rightHandle.width) - dragStartX
                                                        let deltaMs = deltaX / pixelsPerSecond * 1000
                                                        let newEnd = Math.max(trackContent.startTimeMs + 500, originalEndTime + deltaMs)
                                                        GeoOverlays.setOverlayTiming(overlayIdx, trackContent.startTimeMs, 0, newEnd, 0)
                                                    }
                                                }
                                            }
                                        }

                                        // Middle drag (move track)
                                        MouseArea {
                                            id: trackDragArea
                                            anchors.fill: parent
                                            anchors.leftMargin: 12
                                            anchors.rightMargin: 12
                                            hoverEnabled: true
                                            cursorShape: Qt.OpenHandCursor
                                            z: -1
                                            property real dragStartX: 0
                                            property real originalStartTime: 0
                                            property real originalEndTime: 0

                                            onPressed: (mouse) => {
                                                dragStartX = mapToItem(trackContent, mouse.x, mouse.y).x
                                                originalStartTime = trackContent.startTimeMs
                                                originalEndTime = trackContent.endTimeMs
                                                cursorShape = Qt.ClosedHandCursor
                                            }
                                            onReleased: cursorShape = Qt.OpenHandCursor
                                            onPositionChanged: (mouse) => {
                                                if (pressed) {
                                                    let currentX = mapToItem(trackContent, mouse.x, mouse.y).x
                                                    let deltaX = currentX - dragStartX
                                                    let deltaMs = deltaX / pixelsPerSecond * 1000
                                                    let duration = originalEndTime - originalStartTime
                                                    let newStart = Math.max(0, originalStartTime + deltaMs)
                                                    let newEnd = newStart + duration
                                                    GeoOverlays.setOverlayTiming(overlayIdx, newStart, 0, newEnd, 0)
                                                }
                                            }
                                        }

                                        // Keyframe markers
                                        Repeater {
                                            model: GeoOverlays.getAllKeyframes(overlayIdx)
                                            Rectangle {
                                                property var kf: modelData
                                                x: kf.timeMs * pixelsPerSecond / 1000 - 6
                                                y: (trackBar.height - 12) / 2
                                                width: 12
                                                height: 12
                                                rotation: 45
                                                color: kf.extrusion > 0 ? "#ffcc00" : "white"
                                                border.color: "#666"

                                                MouseArea {
                                                    anchors.fill: parent
                                                    anchors.margins: -4
                                                    cursorShape: Qt.PointingHandCursor
                                                    onClicked: {
                                                        kfEditor.overlayIdx = overlayIdx
                                                        kfEditor.kfIdx = index
                                                        kfEditor.open()
                                                    }
                                                }
                                            }
                                        }

                                        // Double-click to add keyframe
                                        MouseArea {
                                            anchors.fill: parent
                                            z: -1
                                            onDoubleClicked: (mouse) => {
                                                let timeMs = mouse.x / pixelsPerSecond * 1000
                                                GeoOverlays.addKeyframe(overlayIdx, timeMs)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Country dialog
    Dialog {
        id: countryDialog
        title: "Select Country"
        modal: true
        anchors.centerIn: parent
        width: 400
        height: 450
        property var countries: []

        onOpened: {
            searchField.text = ""
            countries = GeoJson ? GeoJson.countryList() : []
            countryList.model = countries
        }

        ColumnLayout {
            anchors.fill: parent
            TextField {
                id: searchField
                Layout.fillWidth: true
                placeholderText: "Search..."
                onTextChanged: {
                    if (!text) countryList.model = countryDialog.countries
                    else {
                        let l = text.toLowerCase()
                        countryList.model = countryDialog.countries.filter(c => c.name.toLowerCase().includes(l))
                    }
                }
            }
            ListView {
                id: countryList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                delegate: ItemDelegate {
                    width: countryList.width
                    text: modelData.name
                    onClicked: {
                        GeoOverlays.addCountry(modelData.code, modelData.name, 0)
                        countryDialog.close()
                    }
                }
            }
        }
        standardButtons: Dialog.Cancel
    }

    // Region dialog
    Dialog {
        id: regionDialog
        title: "Select Region"
        modal: true
        anchors.centerIn: parent
        width: 400
        height: 500
        property var regions: []
        property string countryName: ""

        onOpened: {
            regionSearch.text = ""
            countrySel.currentIndex = -1
            regions = []
            regionList.model = []
        }

        ColumnLayout {
            anchors.fill: parent
            ComboBox {
                id: countrySel
                Layout.fillWidth: true
                model: GeoJson ? GeoJson.countryList() : []
                textRole: "name"
                displayText: currentIndex >= 0 ? model[currentIndex].name : "Select country..."
                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && model[currentIndex]) {
                        regionDialog.countryName = model[currentIndex].name
                        regionDialog.regions = GeoJson.regionsForCountry(model[currentIndex].name)
                        regionList.model = regionDialog.regions
                    }
                }
            }
            TextField {
                id: regionSearch
                Layout.fillWidth: true
                placeholderText: "Search..."
                onTextChanged: {
                    if (!text) regionList.model = regionDialog.regions
                    else {
                        let l = text.toLowerCase()
                        regionList.model = regionDialog.regions.filter(r => r.name.toLowerCase().includes(l))
                    }
                }
            }
            ListView {
                id: regionList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                delegate: ItemDelegate {
                    width: regionList.width
                    text: modelData.name
                    onClicked: {
                        GeoOverlays.addRegion(modelData.code, modelData.name, regionDialog.countryName, 0)
                        regionDialog.close()
                    }
                }
            }
        }
        standardButtons: Dialog.Cancel
    }

    // City dialog
    Dialog {
        id: cityDialog
        title: "Select City"
        modal: true
        anchors.centerIn: parent
        width: 400
        height: 450
        property var cities: []

        onOpened: {
            citySearch.text = ""
            cities = GeoJson ? GeoJson.allCities() : []
            cityList.model = cities.slice(0, 100)
        }

        ColumnLayout {
            anchors.fill: parent
            TextField {
                id: citySearch
                Layout.fillWidth: true
                placeholderText: "Search..."
                onTextChanged: {
                    if (!text) cityList.model = cityDialog.cities.slice(0, 100)
                    else {
                        let l = text.toLowerCase()
                        cityList.model = cityDialog.cities.filter(c => c.name.toLowerCase().includes(l)).slice(0, 100)
                    }
                }
            }
            ListView {
                id: cityList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                delegate: ItemDelegate {
                    width: cityList.width
                    text: modelData.name + (modelData.countryName ? " (" + modelData.countryName + ")" : "")
                    onClicked: {
                        GeoOverlays.addCity(modelData.name, modelData.countryName, modelData.lat, modelData.lon, 0)
                        cityDialog.close()
                    }
                }
            }
        }
        standardButtons: Dialog.Cancel
    }

    // Keyframe editor
    Dialog {
        id: kfEditor
        title: "Edit Keyframe"
        modal: true
        anchors.centerIn: parent
        width: 320
        property int overlayIdx: -1
        property int kfIdx: -1
        property var kfData: ({})

        onOpened: {
            if (overlayIdx >= 0 && kfIdx >= 0) {
                kfData = GeoOverlays.getKeyframe(overlayIdx, kfIdx)
                extSlider.value = kfData.extrusion || 0
                opacSlider.value = (kfData.opacity || 1) * 100
                scaleSlider.value = (kfData.scale || 1) * 100
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            RowLayout {
                Text { text: "Extrusion:"; color: Theme.textColor; Layout.preferredWidth: 70 }
                Slider { id: extSlider; Layout.fillWidth: true; from: 0; to: 100
                    onMoved: if (kfEditor.overlayIdx >= 0) GeoOverlays.updateKeyframe(kfEditor.overlayIdx, kfEditor.kfIdx, {"extrusion": value})
                }
                Text { text: extSlider.value.toFixed(0); color: Theme.textColorDim; Layout.preferredWidth: 30 }
            }

            RowLayout {
                Text { text: "Opacity:"; color: Theme.textColor; Layout.preferredWidth: 70 }
                Slider { id: opacSlider; Layout.fillWidth: true; from: 0; to: 100
                    onMoved: if (kfEditor.overlayIdx >= 0) GeoOverlays.updateKeyframe(kfEditor.overlayIdx, kfEditor.kfIdx, {"opacity": value/100})
                }
                Text { text: opacSlider.value.toFixed(0) + "%"; color: Theme.textColorDim; Layout.preferredWidth: 40 }
            }

            RowLayout {
                Text { text: "Scale:"; color: Theme.textColor; Layout.preferredWidth: 70 }
                Slider { id: scaleSlider; Layout.fillWidth: true; from: 50; to: 200
                    onMoved: if (kfEditor.overlayIdx >= 0) GeoOverlays.updateKeyframe(kfEditor.overlayIdx, kfEditor.kfIdx, {"scale": value/100})
                }
                Text { text: scaleSlider.value.toFixed(0) + "%"; color: Theme.textColorDim; Layout.preferredWidth: 40 }
            }

            Button {
                text: "Delete Keyframe"
                Layout.fillWidth: true
                onClicked: {
                    if (kfEditor.overlayIdx >= 0) {
                        GeoOverlays.removeKeyframe(kfEditor.overlayIdx, kfEditor.kfIdx)
                        kfEditor.close()
                    }
                }
            }
        }
        standardButtons: Dialog.Close
    }
}
