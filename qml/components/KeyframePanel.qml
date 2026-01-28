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
            if (kf && kf.latitude !== undefined) {
                latSlider.value = kf.latitude
                lonSlider.value = kf.longitude
                zoomSlider.value = kf.zoom
                bearingSlider.value = kf.bearing
                tiltSlider.value = kf.tilt
                timeSpinBox.value = kf.time / 1000.0  // Convert ms to seconds
            }
        }
    }

    // Format time as mm:ss.s
    function formatTime(seconds) {
        var mins = Math.floor(seconds / 60)
        var secs = seconds % 60
        return mins.toString().padStart(2, '0') + ":" + secs.toFixed(1).padStart(4, '0')
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
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: qsTr("Keyframe Properties")
                color: Theme.textColor
                font.pixelSize: Theme.fontSizeLarge
                font.bold: true
            }
            Item { Layout.fillWidth: true }
            Text {
                visible: hasKeyframe
                text: qsTr("#%1 of %2").arg(selectedIndex + 1).arg(Keyframes ? Keyframes.count : 0)
                color: Theme.textColorDim
                font.pixelSize: Theme.fontSizeSmall
            }
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

                // Time section
                GroupBox {
                    title: qsTr("Timing")
                    Layout.fillWidth: true

                    RowLayout {
                        anchors.fill: parent
                        spacing: Theme.spacingSmall

                        Text {
                            text: qsTr("Time:")
                            color: Theme.textColorDim
                        }

                        SpinBox {
                            id: timeSpinBox
                            Layout.fillWidth: true
                            from: 0
                            to: 36000  // 10 hours max
                            stepSize: 1
                            editable: true

                            property real realValue: value

                            textFromValue: function(value, locale) {
                                return formatTime(value)
                            }

                            valueFromText: function(text, locale) {
                                // Parse mm:ss.s format
                                var parts = text.split(":")
                                if (parts.length === 2) {
                                    var mins = parseInt(parts[0]) || 0
                                    var secs = parseFloat(parts[1]) || 0
                                    return mins * 60 + secs
                                }
                                return parseFloat(text) || 0
                            }

                            onValueModified: {
                                // Convert seconds to milliseconds and update keyframe
                                Keyframes.setKeyframeTime(selectedIndex, value * 1000.0)
                            }
                        }

                        Text {
                            text: qsTr("sec")
                            color: Theme.textColorDim
                        }
                    }
                }

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
                }

                // Reorder and delete
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.spacingSmall

                    Button {
                        text: "↑"
                        enabled: selectedIndex > 0
                        onClicked: {
                            Keyframes.moveKeyframe(selectedIndex, selectedIndex - 1)
                            Keyframes.setCurrentIndex(selectedIndex - 1)
                        }
                        Layout.preferredWidth: 40
                    }

                    Button {
                        text: "↓"
                        enabled: Keyframes && selectedIndex < Keyframes.count - 1
                        onClicked: {
                            Keyframes.moveKeyframe(selectedIndex, selectedIndex + 1)
                            Keyframes.setCurrentIndex(selectedIndex + 1)
                        }
                        Layout.preferredWidth: 40
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: qsTr("Delete")
                        onClicked: Keyframes.removeKeyframe(selectedIndex)
                    }
                }
            }
        }
    }
}
