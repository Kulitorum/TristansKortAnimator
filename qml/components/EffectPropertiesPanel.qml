import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

Rectangle {
    id: effectPanel
    color: Theme.backgroundColor

    property int selectedOverlayIndex: -1
    property int selectedEffectIndex: -1
    property bool hasSelection: selectedOverlayIndex >= 0 && selectedEffectIndex >= 0

    function getEffect() {
        if (!hasSelection || !GeoOverlays) return null
        let effects = GeoOverlays.getEffects(selectedOverlayIndex)
        if (selectedEffectIndex >= 0 && selectedEffectIndex < effects.length) {
            return effects[selectedEffectIndex]
        }
        return null
    }

    function selectEffect(overlayIndex, effectIndex) {
        selectedOverlayIndex = overlayIndex
        selectedEffectIndex = effectIndex
        refreshControls()
    }

    function clearSelection() {
        selectedOverlayIndex = -1
        selectedEffectIndex = -1
    }

    property bool updatingFromModel: false

    function refreshControls() {
        if (updatingFromModel) return
        updatingFromModel = true

        let effect = getEffect()
        if (effect) {
            startTimeInput.text = ((effect.startTime || 0) / 1000).toFixed(1)
            endTimeInput.text = ((effect.endTime || 0) / 1000).toFixed(1)
            fadeInInput.text = ((effect.fadeInDuration || 0) / 1000).toFixed(1)
            fadeOutInput.text = ((effect.fadeOutDuration || 0) / 1000).toFixed(1)
            valueSlider.value = effect.value !== undefined ? effect.value : 1.0
        }

        updatingFromModel = false
    }

    Connections {
        target: GeoOverlays
        function onOverlayModified(idx) {
            if (idx === selectedOverlayIndex) refreshControls()
        }
        function onDataModified() {
            refreshControls()
        }
    }

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

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12

                Text {
                    text: qsTr("Effect")
                    color: Theme.textColor
                    font.pixelSize: 14
                    font.weight: Font.Medium
                }

                Item { Layout.fillWidth: true }

                // Delete effect button
                Rectangle {
                    visible: hasSelection
                    width: 24
                    height: 24
                    radius: 4
                    color: delEffectHover.containsMouse ? Theme.dangerColor : "transparent"

                    Text {
                        anchors.centerIn: parent
                        text: "×"
                        color: delEffectHover.containsMouse ? "#ffffff" : Theme.textColorDim
                        font.pixelSize: 16
                    }

                    MouseArea {
                        id: delEffectHover
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (hasSelection) {
                                GeoOverlays.removeEffect(selectedOverlayIndex, selectedEffectIndex)
                                clearSelection()
                            }
                        }
                    }
                }
            }
        }

        // Effect type display
        Rectangle {
            Layout.fillWidth: true
            height: 36
            color: Theme.surfaceColor
            visible: hasSelection

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: Theme.borderColor
                opacity: 0.5
            }

            Row {
                anchors.centerIn: parent
                spacing: 8

                Rectangle {
                    width: 12
                    height: 12
                    radius: 2
                    color: {
                        let effect = getEffect()
                        if (!effect) return Theme.primaryColor
                        switch(effect.type) {
                            case "opacity": return "#4CAF50"
                            case "extrusion": return "#2196F3"
                            case "scale": return "#FF9800"
                            case "fillColor": return "#E91E63"
                            case "borderColor": return "#9C27B0"
                            default: return Theme.primaryColor
                        }
                    }
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: {
                        let effect = getEffect()
                        if (!effect) return ""
                        switch(effect.type) {
                            case "opacity": return "Opacity"
                            case "extrusion": return "Extrusion"
                            case "scale": return "Scale"
                            case "fillColor": return "Fill Color"
                            case "borderColor": return "Border Color"
                            default: return "Effect"
                        }
                    }
                    color: Theme.textColor
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // No selection message
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !hasSelection

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 12

                Text {
                    text: qsTr("No effect selected")
                    color: Theme.textColorDim
                    font.pixelSize: 12
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: qsTr("Click an effect bar in the timeline\nto edit its properties")
                    color: Theme.textColorMuted
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        // Effect properties
        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: hasSelection
            contentHeight: propsColumn.height
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: propsColumn
                width: parent.width
                spacing: 0

                // Timing section
                Rectangle {
                    Layout.fillWidth: true
                    height: timingContent.height + 24
                    color: Theme.surfaceColorAlt

                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: Theme.borderColor
                        opacity: 0.5
                    }

                    ColumnLayout {
                        id: timingContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 10

                        Text {
                            text: qsTr("Timing")
                            color: Theme.textColorDim
                            font.pixelSize: 10
                            font.weight: Font.Medium
                            font.capitalization: Font.AllUppercase
                        }

                        // Start Time
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: qsTr("Start")
                                color: Theme.textColorDim
                                font.pixelSize: 11
                                Layout.preferredWidth: 60
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 28
                                radius: 4
                                color: Theme.surfaceColor
                                border.color: Theme.borderColor

                                TextInput {
                                    id: startTimeInput
                                    anchors.fill: parent
                                    anchors.margins: 6
                                    color: Theme.textColor
                                    font.pixelSize: 11
                                    font.family: "Consolas"
                                    verticalAlignment: TextInput.AlignVCenter
                                    selectByMouse: true

                                    onEditingFinished: {
                                        if (!updatingFromModel && hasSelection) {
                                            GeoOverlays.updateEffect(selectedOverlayIndex, selectedEffectIndex, {"startTime": parseFloat(text) * 1000})
                                        }
                                    }
                                }
                            }

                            Text {
                                text: "s"
                                color: Theme.textColorDim
                                font.pixelSize: 11
                            }
                        }

                        // End Time
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: qsTr("End")
                                color: Theme.textColorDim
                                font.pixelSize: 11
                                Layout.preferredWidth: 60
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 28
                                radius: 4
                                color: Theme.surfaceColor
                                border.color: Theme.borderColor

                                TextInput {
                                    id: endTimeInput
                                    anchors.fill: parent
                                    anchors.margins: 6
                                    color: Theme.textColor
                                    font.pixelSize: 11
                                    font.family: "Consolas"
                                    verticalAlignment: TextInput.AlignVCenter
                                    selectByMouse: true

                                    onEditingFinished: {
                                        if (!updatingFromModel && hasSelection) {
                                            GeoOverlays.updateEffect(selectedOverlayIndex, selectedEffectIndex, {"endTime": parseFloat(text) * 1000})
                                        }
                                    }
                                }
                            }

                            Text {
                                text: "s"
                                color: Theme.textColorDim
                                font.pixelSize: 11
                            }
                        }
                    }
                }

                // Fade section
                Rectangle {
                    Layout.fillWidth: true
                    height: fadeContent.height + 24
                    color: Theme.surfaceColor

                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: Theme.borderColor
                        opacity: 0.5
                    }

                    ColumnLayout {
                        id: fadeContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 10

                        Text {
                            text: qsTr("Fade")
                            color: Theme.textColorDim
                            font.pixelSize: 10
                            font.weight: Font.Medium
                            font.capitalization: Font.AllUppercase
                        }

                        // Fade In
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: qsTr("Fade In")
                                color: Theme.textColorDim
                                font.pixelSize: 11
                                Layout.preferredWidth: 60
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 28
                                radius: 4
                                color: Theme.surfaceColorLight
                                border.color: Theme.borderColor

                                TextInput {
                                    id: fadeInInput
                                    anchors.fill: parent
                                    anchors.margins: 6
                                    color: Theme.textColor
                                    font.pixelSize: 11
                                    font.family: "Consolas"
                                    verticalAlignment: TextInput.AlignVCenter
                                    selectByMouse: true

                                    onEditingFinished: {
                                        if (!updatingFromModel && hasSelection) {
                                            GeoOverlays.updateEffect(selectedOverlayIndex, selectedEffectIndex, {"fadeInDuration": parseFloat(text) * 1000})
                                        }
                                    }
                                }
                            }

                            Text {
                                text: "s"
                                color: Theme.textColorDim
                                font.pixelSize: 11
                            }
                        }

                        // Fade Out
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: qsTr("Fade Out")
                                color: Theme.textColorDim
                                font.pixelSize: 11
                                Layout.preferredWidth: 60
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 28
                                radius: 4
                                color: Theme.surfaceColorLight
                                border.color: Theme.borderColor

                                TextInput {
                                    id: fadeOutInput
                                    anchors.fill: parent
                                    anchors.margins: 6
                                    color: Theme.textColor
                                    font.pixelSize: 11
                                    font.family: "Consolas"
                                    verticalAlignment: TextInput.AlignVCenter
                                    selectByMouse: true

                                    onEditingFinished: {
                                        if (!updatingFromModel && hasSelection) {
                                            GeoOverlays.updateEffect(selectedOverlayIndex, selectedEffectIndex, {"fadeOutDuration": parseFloat(text) * 1000})
                                        }
                                    }
                                }
                            }

                            Text {
                                text: "s"
                                color: Theme.textColorDim
                                font.pixelSize: 11
                            }
                        }
                    }
                }

                // Value section (for non-color effects)
                Rectangle {
                    Layout.fillWidth: true
                    height: valueContent.height + 24
                    color: Theme.surfaceColorAlt
                    visible: {
                        let effect = getEffect()
                        return effect && effect.type !== "fillColor" && effect.type !== "borderColor"
                    }

                    ColumnLayout {
                        id: valueContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Value")
                                color: Theme.textColorDim
                                font.pixelSize: 10
                                font.weight: Font.Medium
                                font.capitalization: Font.AllUppercase
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: valueSlider.value.toFixed(2)
                                color: Theme.textColor
                                font.pixelSize: 11
                                font.family: "Consolas"
                            }
                        }

                        Slider {
                            id: valueSlider
                            Layout.fillWidth: true
                            from: 0
                            to: {
                                let effect = getEffect()
                                if (!effect) return 1
                                switch(effect.type) {
                                    case "opacity": return 1
                                    case "extrusion": return 100
                                    case "scale": return 3
                                    default: return 1
                                }
                            }
                            value: 1
                            onMoved: {
                                if (!updatingFromModel && hasSelection) {
                                    GeoOverlays.updateEffect(selectedOverlayIndex, selectedEffectIndex, {"value": value})
                                }
                            }
                        }
                    }
                }

                // Color section (for color effects)
                Rectangle {
                    Layout.fillWidth: true
                    height: colorContent.height + 24
                    color: Theme.surfaceColor
                    visible: {
                        let effect = getEffect()
                        return effect && (effect.type === "fillColor" || effect.type === "borderColor")
                    }

                    ColumnLayout {
                        id: colorContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 10

                        Text {
                            text: qsTr("Color")
                            color: Theme.textColorDim
                            font.pixelSize: 10
                            font.weight: Font.Medium
                            font.capitalization: Font.AllUppercase
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Rectangle {
                                width: 40
                                height: 28
                                radius: 4
                                color: {
                                    let effect = getEffect()
                                    return effect ? (effect.color || "#ff0000") : "#ff0000"
                                }
                                border.color: Theme.borderColor

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        // TODO: Open color picker
                                    }
                                }
                            }

                            Text {
                                text: qsTr("Click to change")
                                color: Theme.textColorMuted
                                font.pixelSize: 10
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }
    }
}
