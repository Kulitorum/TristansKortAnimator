import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

Rectangle {
    id: overlayPanel
    color: Theme.surfaceColor

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingNormal
        spacing: Theme.spacingNormal

        // Header
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: qsTr("Overlays")
                color: Theme.textColor
                font.pixelSize: Theme.fontSizeLarge
                font.bold: true
                Layout.fillWidth: true
            }

            Button {
                text: "+"
                implicitWidth: 30
                implicitHeight: 30
                onClicked: addMenu.open()

                Menu {
                    id: addMenu

                    MenuItem {
                        text: qsTr("Add Marker")
                        onTriggered: Overlays.createMarker(Camera.latitude, Camera.longitude)
                    }
                    MenuItem {
                        text: qsTr("Add Arrow")
                        onTriggered: Overlays.createArrow(Camera.latitude, Camera.longitude,
                                                         Camera.latitude + 1, Camera.longitude + 1)
                    }
                    MenuItem {
                        text: qsTr("Add Text")
                        onTriggered: Overlays.createText(Camera.latitude, Camera.longitude, "Label")
                    }
                    MenuItem {
                        text: qsTr("Add Region")
                        onTriggered: regionPicker.open()
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.borderColor
        }

        // Geographic overlay buttons
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.spacingSmall

            Button {
                text: qsTr("Country")
                Layout.fillWidth: true
                onClicked: countryPicker.open()
            }

            Button {
                text: qsTr("Region")
                Layout.fillWidth: true
                onClicked: regionPickerGeo.open()
            }

            Button {
                text: qsTr("City")
                Layout.fillWidth: true
                onClicked: cityPicker.open()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.borderColor
        }

        // Overlay list
        ListView {
            id: overlayList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: Overlays
            currentIndex: Overlays.selectedIndex

            delegate: Rectangle {
                width: overlayList.width
                height: 40
                color: ListView.isCurrentItem ? Theme.accentColor : "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.spacingSmall
                    anchors.rightMargin: Theme.spacingSmall
                    spacing: Theme.spacingSmall

                    // Type icon
                    Text {
                        text: getTypeIcon(model.overlayType)
                        font.pixelSize: 16
                        color: Theme.textColor
                    }

                    // Name
                    Text {
                        text: model.name
                        color: Theme.textColor
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    // Visibility toggle
                    CheckBox {
                        checked: model.visible
                        onCheckedChanged: Overlays.setData(Overlays.index(index, 0), checked, 259)  // VisibleRole
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: Overlays.selectedIndex = index
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.borderColor
            visible: Overlays.selectedIndex >= 0
        }

        // Selected overlay properties
        ColumnLayout {
            Layout.fillWidth: true
            visible: Overlays.selectedIndex >= 0
            spacing: Theme.spacingSmall

            Text {
                text: qsTr("Properties")
                color: Theme.textColorDim
                font.pixelSize: Theme.fontSizeSmall
            }

            // Common properties for all overlay types
            GridLayout {
                columns: 2
                Layout.fillWidth: true
                columnSpacing: Theme.spacingSmall
                rowSpacing: Theme.spacingSmall

                Text { text: qsTr("Name:"); color: Theme.textColorDim }
                TextField {
                    id: nameField
                    text: Overlays.selectedIndex >= 0 ? Overlays.getOverlay(Overlays.selectedIndex)?.name ?? "" : ""
                    onEditingFinished: {
                        if (Overlays.selectedIndex >= 0) {
                            let overlay = Overlays.getOverlay(Overlays.selectedIndex)
                            if (overlay) overlay.name = text
                        }
                    }
                    Layout.fillWidth: true
                }

                Text { text: qsTr("Opacity:"); color: Theme.textColorDim }
                Slider {
                    from: 0
                    to: 1
                    value: {
                        if (Overlays.selectedIndex >= 0) {
                            let overlay = Overlays.getOverlay(Overlays.selectedIndex)
                            return overlay ? overlay.opacity : 1
                        }
                        return 1
                    }
                    onMoved: {
                        if (Overlays.selectedIndex >= 0) {
                            let overlay = Overlays.getOverlay(Overlays.selectedIndex)
                            if (overlay) overlay.opacity = value
                        }
                    }
                    Layout.fillWidth: true
                }

                // Timeline visibility
                Text { text: qsTr("Start (ms):"); color: Theme.textColorDim }
                SpinBox {
                    from: 0
                    to: 600000
                    stepSize: 100
                    value: {
                        if (Overlays.selectedIndex >= 0) {
                            let overlay = Overlays.getOverlay(Overlays.selectedIndex)
                            return overlay ? overlay.startTime : 0
                        }
                        return 0
                    }
                    onValueModified: {
                        if (Overlays.selectedIndex >= 0) {
                            let overlay = Overlays.getOverlay(Overlays.selectedIndex)
                            if (overlay) overlay.startTime = value
                        }
                    }
                    editable: true
                    Layout.fillWidth: true
                }

                Text { text: qsTr("End (ms):"); color: Theme.textColorDim }
                SpinBox {
                    from: -1
                    to: 600000
                    stepSize: 100
                    value: {
                        if (Overlays.selectedIndex >= 0) {
                            let overlay = Overlays.getOverlay(Overlays.selectedIndex)
                            return overlay ? overlay.endTime : -1
                        }
                        return -1
                    }
                    onValueModified: {
                        if (Overlays.selectedIndex >= 0) {
                            let overlay = Overlays.getOverlay(Overlays.selectedIndex)
                            if (overlay) overlay.endTime = value
                        }
                    }
                    editable: true
                    Layout.fillWidth: true

                    textFromValue: function(value) {
                        return value < 0 ? "∞" : value.toString()
                    }
                }
            }

            // Region-specific properties
            GridLayout {
                columns: 2
                Layout.fillWidth: true
                columnSpacing: Theme.spacingSmall
                rowSpacing: Theme.spacingSmall
                visible: Overlays.selectedIndex >= 0 && Overlays.getOverlay(Overlays.selectedIndex)?.type === 3

                Text { text: qsTr("Fill Color:"); color: Theme.textColorDim }
                RowLayout {
                    Layout.fillWidth: true
                    Rectangle {
                        width: 24
                        height: 24
                        radius: 4
                        color: {
                            if (Overlays.selectedIndex >= 0) {
                                let region = Overlays.getRegionHighlight(Overlays.selectedIndex)
                                return region ? region.fillColor : "transparent"
                            }
                            return "transparent"
                        }
                        border.color: Theme.borderColor
                        border.width: 1

                        MouseArea {
                            anchors.fill: parent
                            onClicked: fillColorPicker.open()
                        }
                    }
                    ComboBox {
                        id: fillColorCombo
                        model: [
                            { name: "Red", color: "#e94560" },
                            { name: "Blue", color: "#4a90d9" },
                            { name: "Green", color: "#4ecdc4" },
                            { name: "Yellow", color: "#f7dc6f" },
                            { name: "Orange", color: "#f39c12" },
                            { name: "Purple", color: "#9b59b6" },
                            { name: "Gray", color: "#7f8c8d" }
                        ]
                        textRole: "name"
                        Layout.fillWidth: true
                        onCurrentIndexChanged: {
                            if (Overlays.selectedIndex >= 0) {
                                let region = Overlays.getRegionHighlight(Overlays.selectedIndex)
                                if (region) {
                                    let c = model[currentIndex].color
                                    region.fillColor = Qt.rgba(
                                        parseInt(c.substr(1,2), 16) / 255,
                                        parseInt(c.substr(3,2), 16) / 255,
                                        parseInt(c.substr(5,2), 16) / 255,
                                        0.4
                                    )
                                }
                            }
                        }
                    }
                }

                Text { text: qsTr("Border Color:"); color: Theme.textColorDim }
                RowLayout {
                    Layout.fillWidth: true
                    Rectangle {
                        width: 24
                        height: 24
                        radius: 4
                        color: {
                            if (Overlays.selectedIndex >= 0) {
                                let region = Overlays.getRegionHighlight(Overlays.selectedIndex)
                                return region ? region.borderColor : "transparent"
                            }
                            return "transparent"
                        }
                        border.color: Theme.borderColor
                        border.width: 1
                    }
                    ComboBox {
                        id: borderColorCombo
                        model: [
                            { name: "Red", color: "#e94560" },
                            { name: "Blue", color: "#4a90d9" },
                            { name: "Green", color: "#4ecdc4" },
                            { name: "Yellow", color: "#f7dc6f" },
                            { name: "Orange", color: "#f39c12" },
                            { name: "Purple", color: "#9b59b6" },
                            { name: "White", color: "#ffffff" },
                            { name: "None", color: "transparent" }
                        ]
                        textRole: "name"
                        Layout.fillWidth: true
                        onCurrentIndexChanged: {
                            if (Overlays.selectedIndex >= 0) {
                                let region = Overlays.getRegionHighlight(Overlays.selectedIndex)
                                if (region) {
                                    region.borderColor = model[currentIndex].color
                                }
                            }
                        }
                    }
                }

                Text { text: qsTr("Border Width:"); color: Theme.textColorDim }
                SpinBox {
                    from: 0
                    to: 10
                    value: {
                        if (Overlays.selectedIndex >= 0) {
                            let region = Overlays.getRegionHighlight(Overlays.selectedIndex)
                            return region ? region.borderWidth : 2
                        }
                        return 2
                    }
                    onValueModified: {
                        if (Overlays.selectedIndex >= 0) {
                            let region = Overlays.getRegionHighlight(Overlays.selectedIndex)
                            if (region) region.borderWidth = value
                        }
                    }
                    Layout.fillWidth: true
                }
            }

            // Action buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.spacingSmall

                Button {
                    text: qsTr("Duplicate")
                    onClicked: Overlays.duplicateOverlay(Overlays.selectedIndex)
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Delete")
                    onClicked: Overlays.removeOverlay(Overlays.selectedIndex)
                    Layout.fillWidth: true
                }
            }
        }
    }

    RegionPicker {
        id: regionPicker
        onRegionSelected: (code, name) => {
            let highlight = Overlays.createRegionHighlight(code)
            if (highlight) highlight.regionName = name
        }
    }

    // Country picker dialog
    Dialog {
        id: countryPicker
        title: qsTr("Select Country")
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        height: 500

        property var allCountries: []

        onOpened: {
            countrySearch.text = ""
            allCountries = GeoJson ? GeoJson.countryList() : []
            countryListView.model = allCountries
            countrySearch.forceActiveFocus()
        }

        contentItem: ColumnLayout {
            spacing: Theme.spacingSmall

            TextField {
                id: countrySearch
                Layout.fillWidth: true
                placeholderText: qsTr("Search countries...")
                onTextChanged: {
                    if (!text) {
                        countryListView.model = countryPicker.allCountries
                    } else {
                        let lower = text.toLowerCase()
                        countryListView.model = countryPicker.allCountries.filter(c =>
                            c.name.toLowerCase().includes(lower)
                        )
                    }
                }
            }

            ListView {
                id: countryListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

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

        standardButtons: Dialog.Cancel
    }

    // Region picker dialog
    Dialog {
        id: regionPickerGeo
        title: qsTr("Select Region/State")
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        height: 500

        property var allRegions: []
        property string selectedCountryName: ""

        onOpened: {
            regionSearchGeo.text = ""
            countrySelector.currentIndex = -1
            allRegions = []
            regionListView.model = []
        }

        contentItem: ColumnLayout {
            spacing: Theme.spacingSmall

            ComboBox {
                id: countrySelector
                Layout.fillWidth: true
                model: GeoJson ? GeoJson.countryList() : []
                textRole: "name"
                displayText: currentIndex >= 0 ? model[currentIndex].name : qsTr("Select country first...")
                onCurrentIndexChanged: {
                    if (currentIndex >= 0 && model[currentIndex]) {
                        regionPickerGeo.selectedCountryName = model[currentIndex].name
                        regionPickerGeo.allRegions = GeoJson.regionsForCountry(model[currentIndex].name)
                        regionListView.model = regionPickerGeo.allRegions
                        regionSearchGeo.text = ""
                    }
                }
            }

            TextField {
                id: regionSearchGeo
                Layout.fillWidth: true
                placeholderText: qsTr("Search regions...")
                onTextChanged: {
                    if (!text) {
                        regionListView.model = regionPickerGeo.allRegions
                    } else {
                        let lower = text.toLowerCase()
                        regionListView.model = regionPickerGeo.allRegions.filter(r =>
                            r.name.toLowerCase().includes(lower)
                        )
                    }
                }
            }

            ListView {
                id: regionListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                delegate: ItemDelegate {
                    width: regionListView.width
                    text: modelData.name
                    onClicked: {
                        let startTime = AnimController ? AnimController.currentTime : 0
                        GeoOverlays.addRegion(modelData.code, modelData.name, regionPickerGeo.selectedCountryName, startTime)
                        regionPickerGeo.close()
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
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 400
        height: 500

        property var allCities: []

        onOpened: {
            citySearch.text = ""
            allCities = GeoJson ? GeoJson.allCities() : []
            cityListView.model = allCities.slice(0, 200)
            citySearch.forceActiveFocus()
        }

        contentItem: ColumnLayout {
            spacing: Theme.spacingSmall

            TextField {
                id: citySearch
                Layout.fillWidth: true
                placeholderText: qsTr("Search cities...")
                onTextChanged: {
                    if (!text) {
                        cityListView.model = cityPicker.allCities.slice(0, 200)
                    } else {
                        let lower = text.toLowerCase()
                        cityListView.model = cityPicker.allCities.filter(c =>
                            c.name.toLowerCase().includes(lower)
                        ).slice(0, 200)
                    }
                }
            }

            ListView {
                id: cityListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

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

        standardButtons: Dialog.Cancel
    }

    function getTypeIcon(type) {
        switch (type) {
            case 0: return "📍"  // Marker
            case 1: return "➡️"  // Arrow
            case 2: return "📝"  // Text
            case 3: return "🗺️"  // Region
            default: return "?"
        }
    }
}
