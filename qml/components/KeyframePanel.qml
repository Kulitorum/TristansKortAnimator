import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

Rectangle {
    id: keyframePanel
    color: Theme.surfaceColor

    property int selectedIndex: Keyframes ? Keyframes.currentIndex : -1
    property bool hasKeyframe: Keyframes && selectedIndex >= 0 && selectedIndex < Keyframes.count

    // Helper functions to get keyframe data
    function getVal(prop, defaultVal) {
        if (!hasKeyframe) return defaultVal
        var kf = Keyframes.getKeyframe(selectedIndex)
        return kf ? kf[prop] : defaultVal
    }

    // Refresh sliders when data changes
    function refreshSliders() {
        if (hasKeyframe) {
            var kf = Keyframes.getKeyframe(selectedIndex)
            if (kf) {
                latSlider.value = kf.latitude
                lonSlider.value = kf.longitude
                zoomSlider.value = kf.zoom
                bearingSlider.value = kf.bearing
                tiltSlider.value = kf.tilt
                timeSpinBox.value = kf.time
                interpCombo.currentIndex = kf.interpolation
                easingCombo.currentIndex = kf.easing
            }
        }
    }

    onSelectedIndexChanged: refreshSliders()

    Connections {
        target: Keyframes
        function onCurrentIndexChanged() {
            selectedIndex = Keyframes.currentIndex
            refreshSliders()
        }
        function onDataModified() { refreshSliders() }
        function onCountChanged() { refreshSliders() }
    }

    Component.onCompleted: refreshSliders()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacingNormal
        spacing: Theme.spacingNormal

        // Header
        Text {
            text: qsTr("Keyframe Properties")
            color: Theme.textColor
            font.pixelSize: Theme.fontSizeLarge
            font.bold: true
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.borderColor
        }

        // No keyframe selected message
        Text {
            visible: !hasKeyframe
            text: qsTr("No keyframe selected.\nAdd a keyframe to edit its properties.")
            color: Theme.textColorDim
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        // Keyframe properties
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: hasKeyframe
            clip: true

            ColumnLayout {
                width: parent.width
                spacing: Theme.spacingNormal

                // Position section
                GroupBox {
                    title: qsTr("Position")
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: Theme.spacingSmall

                        // Latitude
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            RowLayout {
                                Text { text: qsTr("Latitude:"); color: Theme.textColorDim }
                                Item { Layout.fillWidth: true }
                                Text { text: latSlider.value.toFixed(2); color: Theme.textColor }
                            }
                            Slider {
                                id: latSlider
                                Layout.fillWidth: true
                                from: -85
                                to: 85
                                live: true
                                onValueChanged: {
                                    if (pressed) {
                                        Keyframes.updateKeyframe(selectedIndex, {"latitude": value})
                                        Camera.latitude = value
                                    }
                                }
                            }
                        }

                        // Longitude
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            RowLayout {
                                Text { text: qsTr("Longitude:"); color: Theme.textColorDim }
                                Item { Layout.fillWidth: true }
                                Text { text: lonSlider.value.toFixed(2); color: Theme.textColor }
                            }
                            Slider {
                                id: lonSlider
                                Layout.fillWidth: true
                                from: -180
                                to: 180
                                live: true
                                onValueChanged: {
                                    if (pressed) {
                                        Keyframes.updateKeyframe(selectedIndex, {"longitude": value})
                                        Camera.longitude = value
                                    }
                                }
                            }
                        }

                        // Zoom
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            RowLayout {
                                Text { text: qsTr("Zoom:"); color: Theme.textColorDim }
                                Item { Layout.fillWidth: true }
                                Text { text: zoomSlider.value.toFixed(1); color: Theme.textColor }
                            }
                            Slider {
                                id: zoomSlider
                                Layout.fillWidth: true
                                from: 1
                                to: 19
                                live: true
                                onValueChanged: {
                                    if (pressed) {
                                        Keyframes.updateKeyframe(selectedIndex, {"zoom": value})
                                        Camera.zoom = value
                                    }
                                }
                            }
                        }
                    }
                }

                // Camera section
                GroupBox {
                    title: qsTr("Camera")
                    Layout.fillWidth: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: Theme.spacingSmall

                        // Bearing
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            RowLayout {
                                Text { text: qsTr("Bearing:"); color: Theme.textColorDim }
                                Item { Layout.fillWidth: true }
                                Text { text: bearingSlider.value.toFixed(0) + "°"; color: Theme.textColor }
                            }
                            Slider {
                                id: bearingSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 360
                                live: true
                                onValueChanged: {
                                    if (pressed) {
                                        Keyframes.updateKeyframe(selectedIndex, {"bearing": value})
                                        Camera.bearing = value
                                    }
                                }
                            }
                        }

                        // Tilt
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            RowLayout {
                                Text { text: qsTr("Tilt:"); color: Theme.textColorDim }
                                Item { Layout.fillWidth: true }
                                Text { text: tiltSlider.value.toFixed(0) + "°"; color: Theme.textColor }
                            }
                            Slider {
                                id: tiltSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 60
                                live: true
                                onValueChanged: {
                                    if (pressed) {
                                        Keyframes.updateKeyframe(selectedIndex, {"tilt": value})
                                        Camera.tilt = value
                                    }
                                }
                            }
                        }
                    }
                }

                // Animation section
                GroupBox {
                    title: qsTr("Animation")
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        columnSpacing: Theme.spacingSmall
                        rowSpacing: Theme.spacingSmall
                        anchors.fill: parent

                        Text { text: qsTr("Time (ms):"); color: Theme.textColorDim }
                        SpinBox {
                            id: timeSpinBox
                            from: 0
                            to: 600000
                            stepSize: 100
                            value: getVal("time", 0)
                            onValueModified: if (hasKeyframe) Keyframes.setKeyframeTime(selectedIndex, value)
                            editable: true
                            Layout.fillWidth: true
                        }

                        Text { text: qsTr("Interpolation:"); color: Theme.textColorDim }
                        ComboBox {
                            id: interpCombo
                            model: Theme.interpolationNames
                            currentIndex: getVal("interpolation", 0)
                            onActivated: if (hasKeyframe) Keyframes.updateKeyframe(selectedIndex, {"interpolation": currentIndex})
                            Layout.fillWidth: true
                        }

                        Text { text: qsTr("Easing:"); color: Theme.textColorDim }
                        ComboBox {
                            id: easingCombo
                            model: Theme.easingNames
                            currentIndex: getVal("easing", 1)
                            onActivated: if (hasKeyframe) Keyframes.updateKeyframe(selectedIndex, {"easing": currentIndex})
                            Layout.fillWidth: true
                        }
                    }
                }

                // Actions
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.spacingSmall

                    Button {
                        text: qsTr("Go To")
                        onClicked: MainController.goToKeyframe(selectedIndex)
                        Layout.fillWidth: true
                    }

                    Button {
                        text: qsTr("Update")
                        onClicked: {
                            Keyframes.updateKeyframe(selectedIndex, {
                                "latitude": Camera.latitude,
                                "longitude": Camera.longitude,
                                "zoom": Camera.zoom,
                                "bearing": Camera.bearing,
                                "tilt": Camera.tilt
                            })
                        }
                        Layout.fillWidth: true
                    }

                    Button {
                        text: qsTr("Delete")
                        onClicked: Keyframes.removeKeyframe(selectedIndex)
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
