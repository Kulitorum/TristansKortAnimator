import QtQuick
import QtQuick.Controls
import TristansKortAnimator

Item {
    id: mapView

    // The C++ map renderer
    MapRenderer {
        id: mapRenderer
        anchors.fill: parent

        showCountryLabels: Settings.showCountryLabels
        showRegionLabels: Settings.showRegionLabels
        showCityLabels: Settings.showCityLabels
        showCountryBorders: true
        showCityMarkers: true
        shadeNonHighlighted: Settings.shadeNonHighlighted
        nonHighlightedOpacity: Settings.nonHighlightedOpacity

        Component.onCompleted: {
            MainController.setMapRenderer(mapRenderer)
        }
    }

    // Mouse interaction area
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
        hoverEnabled: true

        property real lastX: 0
        property real lastY: 0
        property real pressX: 0
        property real pressY: 0
        property bool panning: false
        property bool hasMoved: false
        readonly property real clickThreshold: 5

        onPressed: (mouse) => {
            lastX = mouse.x
            lastY = mouse.y
            pressX = mouse.x
            pressY = mouse.y
            panning = true
            hasMoved = false
            cursorShape = Qt.ClosedHandCursor
        }

        onReleased: (mouse) => {
            panning = false
            cursorShape = Qt.OpenHandCursor

            // Click detection - select feature if no significant movement
            if (!hasMoved && mouse.button === Qt.LeftButton) {
                mapRenderer.selectFeatureAt(mouse.x, mouse.y)
            }
        }

        onPositionChanged: (mouse) => {
            if (panning) {
                let dx = mouse.x - lastX
                let dy = mouse.y - lastY

                // Check if moved beyond click threshold
                if (Math.abs(mouse.x - pressX) > clickThreshold ||
                    Math.abs(mouse.y - pressY) > clickThreshold) {
                    hasMoved = true
                    // Auto-add keyframe if not near existing one, then enable edit mode
                    MainController.ensureKeyframeAtCurrentTime()
                }

                // Pan the map (adjust lat/lon based on mouse movement)
                let scale = Math.pow(2, Camera.zoom) * 256
                let metersPerPixel = 156543.03392 * Math.cos(Camera.latitude * Math.PI / 180) / Math.pow(2, Camera.zoom)

                Camera.longitude -= dx * metersPerPixel / 111320
                Camera.latitude += dy * metersPerPixel / 110540

                lastX = mouse.x
                lastY = mouse.y
            }
        }

        onWheel: (wheel) => {
            // Zoom with scroll wheel centered on mouse position
            // Auto-add keyframe if not near existing one, then enable edit mode
            MainController.ensureKeyframeAtCurrentTime()

            // Get the geo coordinates under the mouse BEFORE zoom
            // Note: screenToGeo returns QPointF(lat, lon) so x=lat, y=lon
            let geoBefore = Camera.screenToGeo(wheel.x, wheel.y, mapView.width, mapView.height)

            // Apply zoom
            let zoomDelta = wheel.angleDelta.y > 0 ? 0.5 : -0.5
            let newZoom = Math.max(1, Math.min(19, Camera.zoom + zoomDelta))
            Camera.zoom = newZoom

            // Get the geo coordinates at the same screen position AFTER zoom
            let geoAfter = Camera.screenToGeo(wheel.x, wheel.y, mapView.width, mapView.height)

            // Adjust camera to keep the original geo point under the mouse
            // x=lat, y=lon in the returned QPointF
            Camera.latitude += (geoBefore.x - geoAfter.x)
            Camera.longitude += (geoBefore.y - geoAfter.y)
        }

        cursorShape: Qt.OpenHandCursor
    }

    // Selected feature info panel
    Rectangle {
        id: selectionPanel
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 10
        visible: mapRenderer.selectedFeatureName !== ""
        color: "#dd222222"
        border.color: "#ffcc00"
        border.width: 2
        radius: 6
        width: selectionContent.width + 30
        height: selectionContent.height + 20

        Column {
            id: selectionContent
            anchors.centerIn: parent
            spacing: 4

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: mapRenderer.selectedFeatureName
                color: "#ffffff"
                font.pixelSize: 16
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: mapRenderer.selectedFeatureType.toUpperCase() +
                      (mapRenderer.selectedFeatureCode ? " (" + mapRenderer.selectedFeatureCode + ")" : "")
                color: "#cccccc"
                font.pixelSize: 12
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 10

                Rectangle {
                    width: 80
                    height: 24
                    color: highlightBtn.containsMouse ? "#444444" : "#333333"
                    radius: 4

                    Text {
                        anchors.centerIn: parent
                        text: "Highlight"
                        color: "#ffcc00"
                        font.pixelSize: 11
                    }

                    MouseArea {
                        id: highlightBtn
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            mapRenderer.toggleFeatureHighlight(
                                mapRenderer.selectedFeatureCode,
                                Qt.rgba(1, 0.8, 0, 0.3),
                                Qt.rgba(1, 0.8, 0, 1)
                            )
                        }
                    }
                }

                Rectangle {
                    width: 60
                    height: 24
                    color: clearBtn.containsMouse ? "#444444" : "#333333"
                    radius: 4

                    Text {
                        anchors.centerIn: parent
                        text: "Clear"
                        color: "#aaaaaa"
                        font.pixelSize: 11
                    }

                    MouseArea {
                        id: clearBtn
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: mapRenderer.clearSelection()
                    }
                }
            }
        }
    }

    // Zoom controls overlay
    Column {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Theme.spacingNormal
        spacing: Theme.spacingSmall

        Button {
            text: "+"
            width: 40
            height: 40
            onClicked: {
                MainController.ensureKeyframeAtCurrentTime()
                Camera.zoom = Math.min(19, Camera.zoom + 1)
            }
        }

        Button {
            text: "-"
            width: 40
            height: 40
            onClicked: {
                MainController.ensureKeyframeAtCurrentTime()
                Camera.zoom = Math.max(1, Camera.zoom - 1)
            }
        }
    }

    // Tilt control
    Slider {
        id: tiltSlider
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: Theme.spacingNormal
        orientation: Qt.Vertical
        height: 150
        from: 0
        to: 60
        value: Camera.tilt
        onMoved: {
            MainController.ensureKeyframeAtCurrentTime()
            Camera.tilt = value
        }

        ToolTip {
            visible: tiltSlider.pressed
            text: qsTr("Tilt: %1°").arg(Math.round(tiltSlider.value))
        }
    }

    // Bearing control (rotation wheel)
    Rectangle {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: Theme.spacingNormal
        width: 80
        height: 80
        radius: 40
        color: Theme.surfaceColor
        border.color: Theme.borderColor
        border.width: 1

        Text {
            anchors.centerIn: parent
            text: "N"
            color: Theme.textColor
            font.bold: true
            rotation: -Camera.bearing
        }

        MouseArea {
            anchors.fill: parent
            property real startAngle: 0

            onPressed: (mouse) => {
                let dx = mouse.x - width/2
                let dy = mouse.y - height/2
                startAngle = Math.atan2(dy, dx) * 180 / Math.PI
            }

            onPositionChanged: (mouse) => {
                if (pressed) {
                    MainController.ensureKeyframeAtCurrentTime()
                    let dx = mouse.x - width/2
                    let dy = mouse.y - height/2
                    let currentAngle = Math.atan2(dy, dx) * 180 / Math.PI
                    let delta = currentAngle - startAngle
                    Camera.bearing = (Camera.bearing + delta + 360) % 360
                    startAngle = currentAngle
                }
            }
        }
    }

    // Loading indicator
    BusyIndicator {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: Theme.spacingNormal
        running: TileProvider.loading
        visible: running
    }
}
