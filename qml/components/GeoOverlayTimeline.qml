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

    implicitHeight: Math.max(80, GeoOverlays.count * trackHeight + 50)

    Rectangle {
        anchors.fill: parent
        color: Theme.timelineBackground
    }

    // Header column (overlay names and add buttons)
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

        // Add buttons row
        Row {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 4
            height: 28
            spacing: 4

            Button {
                width: (parent.width - 8) / 3
                height: 24
                text: "Country"
                font.pixelSize: 9
                onClicked: countryPicker.open()
            }

            Button {
                width: (parent.width - 8) / 3
                height: 24
                text: "Region"
                font.pixelSize: 9
                onClicked: regionPicker.open()
            }

            Button {
                width: (parent.width - 8) / 3
                height: 24
                text: "City"
                font.pixelSize: 9
                onClicked: cityPicker.open()
            }
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
                                let newStartTime = (trackBar.x - 20) / pixelsPerSecond * 1000
                                newStartTime = Math.max(0, newStartTime)
                                GeoOverlays.updateOverlay(index, {"startTime": newStartTime})
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
                                        GeoOverlays.updateOverlay(index, {"startTime": newStart})
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
                                        GeoOverlays.updateOverlay(index, {"endTime": newEnd})
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

    // Country picker dialog
    Dialog {
        id: countryPicker
        title: qsTr("Select Country")
        modal: true
        anchors.centerIn: parent
        width: 400
        height: 500

        contentItem: ColumnLayout {
            spacing: Theme.spacingSmall

            TextField {
                id: countrySearch
                Layout.fillWidth: true
                placeholderText: qsTr("Search countries...")
                onTextChanged: countryListView.model = filterCountries(text)
            }

            ListView {
                id: countryListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: GeoJson ? GeoJson.countryList() : []

                delegate: ItemDelegate {
                    width: countryListView.width
                    text: modelData.name
                    onClicked: {
                        let startTime = AnimController ? AnimController.currentTime : 0
                        GeoOverlays.addCountry(modelData.code, modelData.name, startTime)
                        countryPicker.close()
                    }
                }
            }
        }

        function filterCountries(searchText) {
            if (!GeoJson) return []
            let all = GeoJson.countryList()
            if (!searchText) return all
            let lower = searchText.toLowerCase()
            return all.filter(c => c.name.toLowerCase().includes(lower))
        }

        standardButtons: Dialog.Cancel
    }

    // Region picker dialog
    Dialog {
        id: regionPicker
        title: qsTr("Select Region/State")
        modal: true
        anchors.centerIn: parent
        width: 400
        height: 500

        contentItem: ColumnLayout {
            spacing: Theme.spacingSmall

            ComboBox {
                id: countrySelector
                Layout.fillWidth: true
                model: GeoJson ? GeoJson.countryList() : []
                textRole: "name"
                displayText: currentIndex >= 0 ? model[currentIndex].name : qsTr("Select country first...")
                onCurrentIndexChanged: {
                    if (currentIndex >= 0) {
                        regionListView.model = GeoJson.regionsForCountry(model[currentIndex].name)
                    }
                }
            }

            TextField {
                id: regionSearch
                Layout.fillWidth: true
                placeholderText: qsTr("Search regions...")
            }

            ListView {
                id: regionListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: []

                delegate: ItemDelegate {
                    width: regionListView.width
                    text: modelData.name
                    onClicked: {
                        let startTime = AnimController ? AnimController.currentTime : 0
                        GeoOverlays.addRegion(modelData.code, modelData.name, modelData.countryName, startTime)
                        regionPicker.close()
                    }
                }
            }
        }

        standardButtons: Dialog.Cancel
    }

    // City picker dialog
    Dialog {
        id: cityPicker
        title: qsTr("Select City")
        modal: true
        anchors.centerIn: parent
        width: 400
        height: 500

        contentItem: ColumnLayout {
            spacing: Theme.spacingSmall

            TextField {
                id: citySearch
                Layout.fillWidth: true
                placeholderText: qsTr("Search cities...")
                onTextChanged: cityListView.model = filterCities(text)
            }

            ListView {
                id: cityListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: GeoJson ? GeoJson.allCities() : []

                delegate: ItemDelegate {
                    width: cityListView.width
                    text: modelData.name + (modelData.countryName ? " (" + modelData.countryName + ")" : "")
                    onClicked: {
                        let startTime = AnimController ? AnimController.currentTime : 0
                        GeoOverlays.addCity(modelData.name, modelData.countryName, modelData.lat, modelData.lon, startTime)
                        cityPicker.close()
                    }
                }
            }
        }

        function filterCities(searchText) {
            if (!GeoJson) return []
            let all = GeoJson.allCities()
            if (!searchText) return all.slice(0, 200)  // Limit to first 200 by default
            let lower = searchText.toLowerCase()
            return all.filter(c => c.name.toLowerCase().includes(lower)).slice(0, 200)
        }

        standardButtons: Dialog.Cancel
    }

    // Overlay editor dialog
    Dialog {
        id: overlayEditor
        title: qsTr("Edit Overlay Appearance")
        modal: true
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
}
