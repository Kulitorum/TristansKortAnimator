import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

Rectangle {
    id: overlayPanel
    color: Theme.backgroundColor

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: Theme.headerColor

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: Theme.borderColor
            }

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Overlays")
                color: Theme.textColor
                font.pixelSize: 14
                font.weight: Font.Medium
            }
        }

        // Territories button
        Rectangle {
            Layout.fillWidth: true
            Layout.margins: 8
            height: 36
            radius: 4
            color: territoriesHover.containsMouse ? Theme.primaryColor : Theme.surfaceColorLight
            border.color: Theme.primaryColor
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: qsTr("+ Territory")
                color: territoriesHover.containsMouse ? "#ffffff" : Theme.primaryColor
                font.pixelSize: 12
                font.weight: Font.Medium
            }

            MouseArea {
                id: territoriesHover
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: territoriesMenu.open()
            }

            Menu {
                id: territoriesMenu
                y: parent.height

                MenuItem {
                    text: qsTr("Country")
                    onTriggered: countryPicker.open()
                }
                MenuItem {
                    text: qsTr("Region / State")
                    onTriggered: regionPickerGeo.open()
                }
                MenuItem {
                    text: qsTr("City")
                    onTriggered: cityPicker.open()
                }
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
            model: GeoOverlays

            delegate: Rectangle {
                width: overlayList.width
                height: 40
                color: GeoOverlays.selectedIndex === index ? Theme.surfaceColorLight : "transparent"

                // Selection indicator
                Rectangle {
                    visible: GeoOverlays.selectedIndex === index
                    anchors.left: parent.left
                    width: 3
                    height: parent.height
                    color: Theme.primaryColor
                }

                // Bottom border
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: Theme.borderColor
                    opacity: 0.3
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 8
                    spacing: 8

                    // Color indicator
                    Rectangle {
                        width: 10
                        height: 10
                        radius: 2
                        color: model.borderColor || Theme.primaryColor
                    }

                    // Name
                    Text {
                        text: model.name
                        color: GeoOverlays.selectedIndex === index ? Theme.textColor : Theme.textColorDim
                        font.pixelSize: 12
                        font.weight: GeoOverlays.selectedIndex === index ? Font.Medium : Font.Normal
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    // Type badge
                    Rectangle {
                        width: typeText.width + 8
                        height: 18
                        radius: 9
                        color: Theme.surfaceColor
                        visible: model.overlayType !== undefined

                        Text {
                            id: typeText
                            anchors.centerIn: parent
                            text: {
                                switch(model.overlayType) {
                                    case "country": return "C"
                                    case "region": return "R"
                                    case "city": return "CT"
                                    default: return "?"
                                }
                            }
                            color: Theme.textColorDim
                            font.pixelSize: 9
                            font.weight: Font.Medium
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: GeoOverlays.setSelectedIndex(index)
                }
            }

            // Empty state
            Text {
                visible: overlayList.count === 0
                anchors.centerIn: parent
                text: qsTr("No territories added.\nClick '+ Territory' to add one.")
                color: Theme.textColorDim
                font.pixelSize: 11
                horizontalAlignment: Text.AlignHCenter
                opacity: 0.7
            }
        }

        // Selected overlay actions
        Rectangle {
            Layout.fillWidth: true
            height: actionColumn.height + 16
            color: Theme.headerColor
            visible: GeoOverlays.selectedIndex >= 0

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: Theme.borderColor
            }

            ColumnLayout {
                id: actionColumn
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 8
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        radius: 4
                        color: dupHover.containsMouse ? Theme.surfaceColorLight : Theme.surfaceColor
                        border.color: Theme.borderColor

                        Text {
                            anchors.centerIn: parent
                            text: qsTr("Duplicate")
                            color: Theme.textColorDim
                            font.pixelSize: 11
                        }

                        MouseArea {
                            id: dupHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                // TODO: Duplicate overlay
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        radius: 4
                        color: delHover.containsMouse ? Theme.dangerColor : Theme.surfaceColor
                        border.color: delHover.containsMouse ? Theme.dangerColor : Theme.borderColor

                        Text {
                            anchors.centerIn: parent
                            text: qsTr("Delete")
                            color: delHover.containsMouse ? "#ffffff" : Theme.textColorDim
                            font.pixelSize: 11
                        }

                        MouseArea {
                            id: delHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (GeoOverlays.selectedIndex >= 0) {
                                    GeoOverlays.removeOverlay(GeoOverlays.selectedIndex)
                                }
                            }
                        }
                    }
                }
            }
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
}
