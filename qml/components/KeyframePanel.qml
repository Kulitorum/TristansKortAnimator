import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

Rectangle {
    id: keyframePanel
    color: Theme.backgroundColor

    property int selectedIndex: Keyframes ? Keyframes.currentIndex : -1
    property bool hasKeyframe: Keyframes && selectedIndex >= 0 && selectedIndex < Keyframes.count

    function getVal(prop, defaultVal) {
        if (!hasKeyframe) return defaultVal
        var kf = Keyframes.getKeyframe(selectedIndex)
        return kf ? kf[prop] : defaultVal
    }

    function refreshSliders() {
        if (hasKeyframe) {
            var kf = Keyframes.getKeyframe(selectedIndex)
            if (kf && kf.latitude !== undefined) {
                latSlider.value = kf.latitude
                lonSlider.value = kf.longitude
                zoomSlider.value = kf.zoom
                bearingSlider.value = kf.bearing
                tiltSlider.value = kf.tilt
                timeSpinBox.value = kf.time / 1000.0
                easingSlider.value = kf.easing !== undefined ? kf.easing : 0.5
            }
        }
    }

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
                    text: qsTr("Keyframe")
                    color: Theme.textColor
                    font.pixelSize: 14
                    font.weight: Font.Medium
                }

                Item { Layout.fillWidth: true }

                Text {
                    visible: hasKeyframe
                    text: qsTr("#%1 of %2").arg(selectedIndex + 1).arg(Keyframes ? Keyframes.count : 0)
                    color: Theme.textColorDim
                    font.pixelSize: 11
                }
            }
        }

        // No keyframe message
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !hasKeyframe

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 12

                Text {
                    text: qsTr("No keyframe selected")
                    color: Theme.textColorDim
                    font.pixelSize: 12
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: qsTr("Click a keyframe marker in the\ntimeline or press K to add one")
                    color: Theme.textColorMuted
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        // Keyframe properties
        Flickable {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: hasKeyframe
            contentHeight: propsColumn.height
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: propsColumn
                width: parent.width
                spacing: 0

                // Time section
                Rectangle {
                    Layout.fillWidth: true
                    height: timeContent.height + 24
                    color: Theme.surfaceColor

                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: Theme.borderColor
                        opacity: 0.5
                    }

                    ColumnLayout {
                        id: timeContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 8

                        Text {
                            text: qsTr("Timing")
                            color: Theme.textColorDim
                            font.pixelSize: 10
                            font.weight: Font.Medium
                            font.capitalization: Font.AllUppercase
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: qsTr("Time:")
                                color: Theme.textColorDim
                                font.pixelSize: 11
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 28
                                color: Theme.surfaceColorLight
                                radius: 4
                                border.color: Theme.borderColor

                                TextInput {
                                    anchors.fill: parent
                                    anchors.margins: 6
                                    text: formatTime(timeSpinBox.value)
                                    color: Theme.textColor
                                    font.pixelSize: 12
                                    font.family: "Consolas"
                                    verticalAlignment: TextInput.AlignVCenter
                                    selectByMouse: true

                                    property real realValue: timeSpinBox.value

                                    onEditingFinished: {
                                        var parts = text.split(":")
                                        var newValue = 0
                                        if (parts.length === 2) {
                                            newValue = parseInt(parts[0]) * 60 + parseFloat(parts[1])
                                        } else {
                                            newValue = parseFloat(text) || 0
                                        }
                                        Keyframes.setKeyframeTime(selectedIndex, newValue * 1000.0)
                                    }
                                }
                            }

                            SpinBox {
                                id: timeSpinBox
                                visible: false
                                from: 0
                                to: 36000
                                value: 0
                            }

                            Text {
                                text: qsTr("sec")
                                color: Theme.textColorDim
                                font.pixelSize: 11
                            }
                        }
                    }
                }

                // Easing section
                Rectangle {
                    Layout.fillWidth: true
                    height: easingContent.height + 24
                    color: Theme.surfaceColorAlt

                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: Theme.borderColor
                        opacity: 0.5
                    }

                    ColumnLayout {
                        id: easingContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Easing")
                                color: Theme.textColorDim
                                font.pixelSize: 10
                                font.weight: Font.Medium
                                font.capitalization: Font.AllUppercase
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: Math.round(easingSlider.value * 100) + "%"
                                color: Theme.textColor
                                font.pixelSize: 11
                            }
                        }

                        Slider {
                            id: easingSlider
                            Layout.fillWidth: true
                            from: 0.0
                            to: 1.0
                            value: 0.5
                            stepSize: 0.05
                            onValueChanged: {
                                if (pressed) {
                                    Keyframes.updateKeyframe(selectedIndex, {"easing": value})
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Snappy")
                                color: Theme.textColorMuted
                                font.pixelSize: 9
                            }
                            Item { Layout.fillWidth: true }
                            Text {
                                text: qsTr("Smooth")
                                color: Theme.textColorMuted
                                font.pixelSize: 9
                            }
                        }
                    }
                }

                // Position section
                Rectangle {
                    Layout.fillWidth: true
                    height: posContent.height + 24
                    color: Theme.surfaceColor

                    Rectangle {
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 1
                        color: Theme.borderColor
                        opacity: 0.5
                    }

                    ColumnLayout {
                        id: posContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 12

                        Text {
                            text: qsTr("Position")
                            color: Theme.textColorDim
                            font.pixelSize: 10
                            font.weight: Font.Medium
                            font.capitalization: Font.AllUppercase
                        }

                        // Latitude
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            RowLayout {
                                Text { text: qsTr("Latitude"); color: Theme.textColorDim; font.pixelSize: 11 }
                                Item { Layout.fillWidth: true }
                                Text { text: latSlider.value.toFixed(2); color: Theme.textColor; font.pixelSize: 11; font.family: "Consolas" }
                            }
                            Slider {
                                id: latSlider
                                Layout.fillWidth: true
                                from: -85
                                to: 85
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
                            spacing: 4
                            RowLayout {
                                Text { text: qsTr("Longitude"); color: Theme.textColorDim; font.pixelSize: 11 }
                                Item { Layout.fillWidth: true }
                                Text { text: lonSlider.value.toFixed(2); color: Theme.textColor; font.pixelSize: 11; font.family: "Consolas" }
                            }
                            Slider {
                                id: lonSlider
                                Layout.fillWidth: true
                                from: -180
                                to: 180
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
                            spacing: 4
                            RowLayout {
                                Text { text: qsTr("Zoom"); color: Theme.textColorDim; font.pixelSize: 11 }
                                Item { Layout.fillWidth: true }
                                Text { text: zoomSlider.value.toFixed(1); color: Theme.textColor; font.pixelSize: 11; font.family: "Consolas" }
                            }
                            Slider {
                                id: zoomSlider
                                Layout.fillWidth: true
                                from: 1
                                to: 19
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
                Rectangle {
                    Layout.fillWidth: true
                    height: camContent.height + 24
                    color: Theme.surfaceColorAlt

                    ColumnLayout {
                        id: camContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 12

                        Text {
                            text: qsTr("Camera")
                            color: Theme.textColorDim
                            font.pixelSize: 10
                            font.weight: Font.Medium
                            font.capitalization: Font.AllUppercase
                        }

                        // Bearing
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            RowLayout {
                                Text { text: qsTr("Bearing"); color: Theme.textColorDim; font.pixelSize: 11 }
                                Item { Layout.fillWidth: true }
                                Text { text: bearingSlider.value.toFixed(0) + "°"; color: Theme.textColor; font.pixelSize: 11; font.family: "Consolas" }
                            }
                            Slider {
                                id: bearingSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 360
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
                            spacing: 4
                            RowLayout {
                                Text { text: qsTr("Tilt"); color: Theme.textColorDim; font.pixelSize: 11 }
                                Item { Layout.fillWidth: true }
                                Text { text: tiltSlider.value.toFixed(0) + "°"; color: Theme.textColor; font.pixelSize: 11; font.family: "Consolas" }
                            }
                            Slider {
                                id: tiltSlider
                                Layout.fillWidth: true
                                from: 0
                                to: 60
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
                Rectangle {
                    Layout.fillWidth: true
                    height: actionsContent.height + 24
                    color: Theme.headerColor

                    Rectangle {
                        anchors.top: parent.top
                        width: parent.width
                        height: 1
                        color: Theme.borderColor
                    }

                    ColumnLayout {
                        id: actionsContent
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.margins: 12
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Rectangle {
                                Layout.fillWidth: true
                                height: 32
                                radius: 4
                                color: gotoHover.containsMouse ? Theme.primaryColor : Theme.surfaceColorLight
                                border.color: Theme.primaryColor

                                Text {
                                    anchors.centerIn: parent
                                    text: qsTr("Go To")
                                    color: gotoHover.containsMouse ? "#ffffff" : Theme.primaryColor
                                    font.pixelSize: 11
                                    font.weight: Font.Medium
                                }

                                MouseArea {
                                    id: gotoHover
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: MainController.goToKeyframe(selectedIndex)
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                height: 32
                                radius: 4
                                color: updateHover.containsMouse ? Theme.surfaceColorLight : Theme.surfaceColor
                                border.color: Theme.borderColor

                                Text {
                                    anchors.centerIn: parent
                                    text: qsTr("Update")
                                    color: Theme.textColorDim
                                    font.pixelSize: 11
                                }

                                MouseArea {
                                    id: updateHover
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        Keyframes.updateKeyframe(selectedIndex, {
                                            "latitude": Camera.latitude,
                                            "longitude": Camera.longitude,
                                            "zoom": Camera.zoom,
                                            "bearing": Camera.bearing,
                                            "tilt": Camera.tilt
                                        })
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 32
                            radius: 4
                            color: deleteHover.containsMouse ? Theme.dangerColor : Theme.surfaceColor
                            border.color: deleteHover.containsMouse ? Theme.dangerColor : Theme.borderColor

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Delete Keyframe")
                                color: deleteHover.containsMouse ? "#ffffff" : Theme.textColorDim
                                font.pixelSize: 11
                            }

                            MouseArea {
                                id: deleteHover
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Keyframes.removeKeyframe(selectedIndex)
                            }
                        }
                    }
                }
            }
        }
    }
}
