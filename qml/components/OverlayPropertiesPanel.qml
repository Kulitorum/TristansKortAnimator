import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import TristansKortAnimator

Rectangle {
    id: overlayPanel
    color: Theme.surfaceColor

    property int selectedIndex: GeoOverlays ? GeoOverlays.selectedIndex : -1
    property bool hasOverlay: GeoOverlays && selectedIndex >= 0 && selectedIndex < GeoOverlays.count
    property bool updatingFromModel: false  // Guard to prevent circular updates

    // Helper to get overlay data
    function getVal(prop, defaultVal) {
        if (!hasOverlay) return defaultVal
        var overlay = GeoOverlays.getOverlay(selectedIndex)
        return overlay ? overlay[prop] : defaultVal
    }

    // Refresh all controls when selection changes
    function refreshControls() {
        if (updatingFromModel) return  // Prevent re-entry
        updatingFromModel = true

        if (hasOverlay) {
            var overlay = GeoOverlays.getOverlay(selectedIndex)
            if (overlay) {
                // Colors
                fillColorRect.color = overlay.fillColor || "#80ff0000"
                borderColorRect.color = overlay.borderColor || "#ffff0000"

                // Values
                borderWidthSlider.value = overlay.borderWidth || 2

                // Parse alpha from colors
                var fc = overlay.fillColor
                if (fc) {
                    fillOpacitySlider.value = Math.round(fc.a * 100)
                } else {
                    fillOpacitySlider.value = 50
                }
                var bc = overlay.borderColor
                if (bc) {
                    borderOpacitySlider.value = Math.round(bc.a * 100)
                } else {
                    borderOpacitySlider.value = 100
                }

                // Timing
                startTimeSpinBox.value = (overlay.startTime || 0) / 1000
                fadeInSpinBox.value = Math.round((overlay.fadeInDuration || 0) / 10)
                endTimeSpinBox.value = (overlay.endTime || 0) / 1000
                fadeOutSpinBox.value = Math.round((overlay.fadeOutDuration || 0) / 10)

                // City-specific
                showLabelCheck.checked = overlay.showLabel !== false
                markerRadiusSlider.value = overlay.markerRadius || 8
            }
        }

        updatingFromModel = false
    }

    onSelectedIndexChanged: refreshControls()

    Connections {
        target: GeoOverlays
        function onSelectedIndexChanged() {
            overlayPanel.refreshControls()
        }
        function onOverlayModified(index) {
            if (index === overlayPanel.selectedIndex) overlayPanel.refreshControls()
        }
    }

    Component.onCompleted: refreshControls()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingNormal
        spacing: Theme.spacingNormal

        // Header
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: qsTr("Overlay Properties")
                color: Theme.textColor
                font.pixelSize: Theme.fontSizeLarge
                font.bold: true
            }
            Item { Layout.fillWidth: true }
            Text {
                visible: hasOverlay
                text: getVal("typeString", "")
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                font.bold: true
            }
        }

        // Overlay name
        Text {
            visible: hasOverlay
            text: getVal("name", "")
            color: Theme.textColor
            font.pixelSize: Theme.fontSizeNormal
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.borderColor
        }

        // No overlay selected message
        Text {
            visible: !hasOverlay
            text: qsTr("No overlay selected.\nClick on a CRC in the timeline to edit.")
            color: Theme.textColorDim
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        // Overlay properties
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: hasOverlay
            clip: true

            ColumnLayout {
                width: parent.width
                spacing: Theme.spacingNormal

                // Colors section
                GroupBox {
                    title: qsTr("Appearance")
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: Theme.spacingSmall

                        // Fill Color
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Fill Color:")
                                color: Theme.textColorDim
                                Layout.preferredWidth: 80
                            }
                            Rectangle {
                                id: fillColorRect
                                width: 40
                                height: 24
                                radius: 4
                                border.color: Theme.borderColor
                                border.width: 1

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: fillColorDialog.open()
                                }
                            }
                            Item { Layout.fillWidth: true }
                        }

                        // Fill Opacity
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Fill Opacity:")
                                color: Theme.textColorDim
                                Layout.preferredWidth: 80
                            }
                            Slider {
                                id: fillOpacitySlider
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                value: 50
                                stepSize: 5
                                onValueChanged: {
                                    if (!updatingFromModel && hasOverlay) {
                                        var c = fillColorRect.color
                                        var newColor = Qt.rgba(c.r, c.g, c.b, value / 100)
                                        GeoOverlays.updateOverlay(selectedIndex, {"fillColor": newColor})
                                    }
                                }
                            }
                            Text {
                                text: Math.round(fillOpacitySlider.value) + "%"
                                color: Theme.textColor
                                Layout.preferredWidth: 35
                            }
                        }

                        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.borderColor; opacity: 0.5 }

                        // Border Color
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Outline:")
                                color: Theme.textColorDim
                                Layout.preferredWidth: 80
                            }
                            Rectangle {
                                id: borderColorRect
                                width: 40
                                height: 24
                                radius: 4
                                border.color: Theme.borderColor
                                border.width: 1

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: borderColorDialog.open()
                                }
                            }
                            Item { Layout.fillWidth: true }
                        }

                        // Border Opacity
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Outline Opacity:")
                                color: Theme.textColorDim
                                Layout.preferredWidth: 80
                            }
                            Slider {
                                id: borderOpacitySlider
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                value: 100
                                stepSize: 5
                                onValueChanged: {
                                    if (!updatingFromModel && hasOverlay) {
                                        var c = borderColorRect.color
                                        var newColor = Qt.rgba(c.r, c.g, c.b, value / 100)
                                        GeoOverlays.updateOverlay(selectedIndex, {"borderColor": newColor})
                                    }
                                }
                            }
                            Text {
                                text: Math.round(borderOpacitySlider.value) + "%"
                                color: Theme.textColor
                                Layout.preferredWidth: 35
                            }
                        }

                        // Border Width
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Outline Width:")
                                color: Theme.textColorDim
                                Layout.preferredWidth: 80
                            }
                            Slider {
                                id: borderWidthSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 10
                                value: 2
                                stepSize: 0.5
                                onValueChanged: {
                                    if (!updatingFromModel && hasOverlay) {
                                        GeoOverlays.updateOverlay(selectedIndex, {"borderWidth": value})
                                    }
                                }
                            }
                            Text {
                                text: borderWidthSlider.value.toFixed(1) + "px"
                                color: Theme.textColor
                                Layout.preferredWidth: 35
                            }
                        }
                    }
                }

                // Timing section
                GroupBox {
                    title: qsTr("Timing")
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: Theme.spacingSmall

                        // Start Time
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Start:")
                                color: Theme.textColorDim
                                Layout.preferredWidth: 70
                            }
                            SpinBox {
                                id: startTimeSpinBox
                                Layout.fillWidth: true
                                from: 0
                                to: 3600
                                value: 0
                                editable: true
                                onValueModified: {
                                    if (hasOverlay) {
                                        GeoOverlays.updateOverlay(selectedIndex, {"startTime": value * 1000})
                                    }
                                }
                            }
                            Text { text: "s"; color: Theme.textColorDim }
                        }

                        // Fade In
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Fade In:")
                                color: Theme.textColorDim
                                Layout.preferredWidth: 70
                            }
                            SpinBox {
                                id: fadeInSpinBox
                                Layout.fillWidth: true
                                from: 0
                                to: 6000
                                stepSize: 10
                                value: 0
                                editable: true
                                property int decimals: 2
                                property real realValue: value / 100
                                validator: DoubleValidator {
                                    bottom: Math.min(fadeInSpinBox.from, fadeInSpinBox.to)
                                    top: Math.max(fadeInSpinBox.from, fadeInSpinBox.to)
                                    decimals: fadeInSpinBox.decimals
                                    notation: DoubleValidator.StandardNotation
                                }
                                textFromValue: function(value, locale) {
                                    return Number(value / 100).toLocaleString(locale, 'f', decimals)
                                }
                                valueFromText: function(text, locale) {
                                    return Math.round(Number.fromLocaleString(locale, text) * 100)
                                }
                                onValueModified: {
                                    if (hasOverlay) {
                                        GeoOverlays.updateOverlay(selectedIndex, {"fadeInDuration": value * 10})
                                    }
                                }
                            }
                            Text { text: "s"; color: Theme.textColorDim }
                        }

                        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.borderColor; opacity: 0.5 }

                        // End Time
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("End:")
                                color: Theme.textColorDim
                                Layout.preferredWidth: 70
                            }
                            SpinBox {
                                id: endTimeSpinBox
                                Layout.fillWidth: true
                                from: 0
                                to: 3600
                                value: 0
                                editable: true
                                onValueModified: {
                                    if (hasOverlay) {
                                        GeoOverlays.updateOverlay(selectedIndex, {"endTime": value * 1000})
                                    }
                                }
                            }
                            Text { text: "s"; color: Theme.textColorDim }
                            Text {
                                text: "(0 = end)"
                                color: Theme.textColorMuted
                                font.pixelSize: 10
                            }
                        }

                        // Fade Out
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Fade Out:")
                                color: Theme.textColorDim
                                Layout.preferredWidth: 70
                            }
                            SpinBox {
                                id: fadeOutSpinBox
                                Layout.fillWidth: true
                                from: 0
                                to: 6000
                                stepSize: 10
                                value: 0
                                editable: true
                                property int decimals: 2
                                property real realValue: value / 100
                                validator: DoubleValidator {
                                    bottom: Math.min(fadeOutSpinBox.from, fadeOutSpinBox.to)
                                    top: Math.max(fadeOutSpinBox.from, fadeOutSpinBox.to)
                                    decimals: fadeOutSpinBox.decimals
                                    notation: DoubleValidator.StandardNotation
                                }
                                textFromValue: function(value, locale) {
                                    return Number(value / 100).toLocaleString(locale, 'f', decimals)
                                }
                                valueFromText: function(text, locale) {
                                    return Math.round(Number.fromLocaleString(locale, text) * 100)
                                }
                                onValueModified: {
                                    if (hasOverlay) {
                                        GeoOverlays.updateOverlay(selectedIndex, {"fadeOutDuration": value * 10})
                                    }
                                }
                            }
                            Text { text: "s"; color: Theme.textColorDim }
                        }
                    }
                }

                // City-specific options
                GroupBox {
                    title: qsTr("City Options")
                    Layout.fillWidth: true
                    visible: getVal("typeString", "") === "City"

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: Theme.spacingSmall

                        // Show Label
                        CheckBox {
                            id: showLabelCheck
                            text: qsTr("Show Label")
                            checked: true
                            onCheckedChanged: {
                                if (!updatingFromModel && hasOverlay) {
                                    GeoOverlays.updateOverlay(selectedIndex, {"showLabel": checked})
                                }
                            }
                        }

                        // Marker Radius
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Marker Size:")
                                color: Theme.textColorDim
                                Layout.preferredWidth: 80
                            }
                            Slider {
                                id: markerRadiusSlider
                                Layout.fillWidth: true
                                from: 2
                                to: 30
                                value: 8
                                stepSize: 1
                                onValueChanged: {
                                    if (!updatingFromModel && hasOverlay) {
                                        GeoOverlays.updateOverlay(selectedIndex, {"markerRadius": value})
                                    }
                                }
                            }
                            Text {
                                text: Math.round(markerRadiusSlider.value) + "px"
                                color: Theme.textColor
                                Layout.preferredWidth: 35
                            }
                        }
                    }
                }

                // Quick presets
                GroupBox {
                    title: qsTr("Quick Presets")
                    Layout.fillWidth: true

                    Flow {
                        anchors.fill: parent
                        spacing: Theme.spacingSmall

                        Button {
                            text: "Red"
                            onClicked: {
                                GeoOverlays.updateOverlay(selectedIndex, {
                                    "fillColor": Qt.rgba(1, 0, 0, 0.4),
                                    "borderColor": Qt.rgba(1, 0, 0, 1)
                                })
                                refreshControls()
                            }
                        }
                        Button {
                            text: "Blue"
                            onClicked: {
                                GeoOverlays.updateOverlay(selectedIndex, {
                                    "fillColor": Qt.rgba(0, 0.4, 1, 0.4),
                                    "borderColor": Qt.rgba(0, 0.4, 1, 1)
                                })
                                refreshControls()
                            }
                        }
                        Button {
                            text: "Green"
                            onClicked: {
                                GeoOverlays.updateOverlay(selectedIndex, {
                                    "fillColor": Qt.rgba(0, 0.8, 0.3, 0.4),
                                    "borderColor": Qt.rgba(0, 0.8, 0.3, 1)
                                })
                                refreshControls()
                            }
                        }
                        Button {
                            text: "Yellow"
                            onClicked: {
                                GeoOverlays.updateOverlay(selectedIndex, {
                                    "fillColor": Qt.rgba(1, 0.9, 0, 0.4),
                                    "borderColor": Qt.rgba(1, 0.8, 0, 1)
                                })
                                refreshControls()
                            }
                        }
                        Button {
                            text: "Outline Only"
                            onClicked: {
                                GeoOverlays.updateOverlay(selectedIndex, {
                                    "fillColor": Qt.rgba(0, 0, 0, 0),
                                    "borderColor": Qt.rgba(1, 1, 1, 1),
                                    "borderWidth": 3
                                })
                                refreshControls()
                            }
                        }
                    }
                }

                // Add Effects section
                GroupBox {
                    title: qsTr("Effects")
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: Theme.spacingSmall

                        Text {
                            text: qsTr("Add animation effects to this overlay")
                            color: Theme.textColorDim
                            font.pixelSize: 10
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        // Effect buttons grid
                        GridLayout {
                            Layout.fillWidth: true
                            columns: 2
                            columnSpacing: 4
                            rowSpacing: 4

                            Button {
                                text: "Opacity"
                                Layout.fillWidth: true
                                onClicked: {
                                    GeoOverlays.addEffect(selectedIndex, "opacity")
                                    addEffectRequested("opacity")
                                }

                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 2
                                    color: "#4CAF50"
                                    anchors.left: parent.left
                                    anchors.leftMargin: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            Button {
                                text: "Extrusion"
                                Layout.fillWidth: true
                                onClicked: {
                                    GeoOverlays.addEffect(selectedIndex, "extrusion")
                                    addEffectRequested("extrusion")
                                }

                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 2
                                    color: "#2196F3"
                                    anchors.left: parent.left
                                    anchors.leftMargin: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            Button {
                                text: "Scale"
                                Layout.fillWidth: true
                                onClicked: {
                                    GeoOverlays.addEffect(selectedIndex, "scale")
                                    addEffectRequested("scale")
                                }

                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 2
                                    color: "#FF9800"
                                    anchors.left: parent.left
                                    anchors.leftMargin: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            Button {
                                text: "Fill Color"
                                Layout.fillWidth: true
                                onClicked: {
                                    GeoOverlays.addEffect(selectedIndex, "fillColor")
                                    addEffectRequested("fillColor")
                                }

                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 2
                                    color: "#E91E63"
                                    anchors.left: parent.left
                                    anchors.leftMargin: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            Button {
                                text: "Border Color"
                                Layout.fillWidth: true
                                Layout.columnSpan: 2
                                onClicked: {
                                    GeoOverlays.addEffect(selectedIndex, "borderColor")
                                    addEffectRequested("borderColor")
                                }

                                Rectangle {
                                    width: 8
                                    height: 8
                                    radius: 2
                                    color: "#9C27B0"
                                    anchors.left: parent.left
                                    anchors.leftMargin: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }
                        }

                        // List of current effects
                        Text {
                            visible: effectsList.count > 0
                            text: qsTr("Current effects:")
                            color: Theme.textColorDim
                            font.pixelSize: 10
                            Layout.topMargin: 8
                        }

                        Repeater {
                            id: effectsList
                            model: hasOverlay && GeoOverlays ? GeoOverlays.getEffects(selectedIndex) : []

                            Rectangle {
                                Layout.fillWidth: true
                                height: 24
                                radius: 3
                                color: Theme.surfaceColorLight

                                Row {
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    spacing: 6

                                    Rectangle {
                                        width: 8
                                        height: 8
                                        radius: 2
                                        anchors.verticalCenter: parent.verticalCenter
                                        color: {
                                            switch(modelData.type) {
                                                case "opacity": return "#4CAF50"
                                                case "extrusion": return "#2196F3"
                                                case "scale": return "#FF9800"
                                                case "fillColor": return "#E91E63"
                                                case "borderColor": return "#9C27B0"
                                                default: return Theme.primaryColor
                                            }
                                        }
                                    }

                                    Text {
                                        text: modelData.type || "Effect"
                                        color: Theme.textColor
                                        font.pixelSize: 10
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Item { width: 1; Layout.fillWidth: true }

                                    Text {
                                        text: "×"
                                        color: effectDelMouse.containsMouse ? Theme.primaryColor : Theme.textColorDim
                                        font.pixelSize: 14
                                        anchors.verticalCenter: parent.verticalCenter

                                        MouseArea {
                                            id: effectDelMouse
                                            anchors.fill: parent
                                            anchors.margins: -4
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: GeoOverlays.removeEffect(selectedIndex, index)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // Actions
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.spacingSmall

                    Button {
                        text: qsTr("Duplicate")
                        Layout.fillWidth: true
                        onClicked: {
                            // TODO: Implement duplicate
                        }
                    }

                    Button {
                        text: qsTr("Delete")
                        Layout.fillWidth: true
                        onClicked: {
                            GeoOverlays.removeOverlay(selectedIndex)
                        }
                    }
                }
            }
        }
    }

    // Signal when effect is added
    signal addEffectRequested(string effectType)

    // Color Dialogs
    ColorDialog {
        id: fillColorDialog
        title: qsTr("Select Fill Color")
        onAccepted: {
            var newColor = Qt.rgba(selectedColor.r, selectedColor.g, selectedColor.b, fillOpacitySlider.value / 100)
            fillColorRect.color = newColor
            GeoOverlays.updateOverlay(selectedIndex, {"fillColor": newColor})
        }
    }

    ColorDialog {
        id: borderColorDialog
        title: qsTr("Select Outline Color")
        onAccepted: {
            var newColor = Qt.rgba(selectedColor.r, selectedColor.g, selectedColor.b, borderOpacitySlider.value / 100)
            borderColorRect.color = newColor
            GeoOverlays.updateOverlay(selectedIndex, {"borderColor": newColor})
        }
    }
}
