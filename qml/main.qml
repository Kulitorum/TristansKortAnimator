import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Dialogs
import TristansKortAnimator

ApplicationWindow {
    id: root
    visible: true
    width: 1400
    height: 900
    minimumWidth: 1024
    minimumHeight: 768
    title: "Tristans Kort Animator" + (ProjectManager.hasUnsavedChanges ? " *" : "")
    color: Theme.backgroundColor

    // View mode: "2D" or "3D"
    property string viewMode: "2D"

    // Material theme configuration
    Material.theme: Material.Dark
    Material.primary: Theme.primaryColor
    Material.accent: Theme.primaryColor
    Material.background: Theme.surfaceColor
    Material.foreground: Theme.textColor

    // Keyboard shortcuts
    Shortcut {
        sequences: [StandardKey.New]
        onActivated: ProjectManager.newProject()
    }
    Shortcut {
        sequences: [StandardKey.Open]
        onActivated: openDialog.open()
    }
    Shortcut {
        sequences: [StandardKey.Save]
        onActivated: {
            if (!ProjectManager.saveProject()) {
                saveAsDialog.open()
            }
        }
    }
    Shortcut {
        sequence: "Ctrl+Shift+S"
        onActivated: saveAsDialog.open()
    }
    Shortcut {
        sequence: "Space"
        onActivated: AnimController.playing ? AnimController.pause() : AnimController.play()
    }
    Shortcut {
        sequence: "Home"
        onActivated: AnimController.stop()
    }
    Shortcut {
        sequence: "K"
        onActivated: MainController.addKeyframeAtCurrentPosition()
    }
    Shortcut {
        sequence: "V"
        onActivated: root.viewMode = (root.viewMode === "2D" ? "3D" : "2D")
    }

    // Menu bar
    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            Action {
                text: qsTr("&New Project")
                shortcut: StandardKey.New
                onTriggered: ProjectManager.newProject()
            }
            Action {
                text: qsTr("&Open Project...")
                shortcut: StandardKey.Open
                onTriggered: openDialog.open()
            }
            MenuSeparator {}
            Action {
                text: qsTr("&Save")
                shortcut: StandardKey.Save
                onTriggered: {
                    if (!ProjectManager.saveProject()) {
                        saveAsDialog.open()
                    }
                }
            }
            Action {
                text: qsTr("Save &As...")
                shortcut: "Ctrl+Shift+S"
                onTriggered: saveAsDialog.open()
            }
            MenuSeparator {}
            Action {
                text: qsTr("&Export Video...")
                shortcut: "Ctrl+E"
                onTriggered: exportDialog.open()
            }
            MenuSeparator {}
            Action {
                text: qsTr("E&xit")
                shortcut: StandardKey.Quit
                onTriggered: root.close()
            }
        }
        Menu {
            title: qsTr("&Edit")
            Action {
                text: qsTr("Add &Keyframe")
                shortcut: "K"
                onTriggered: MainController.addKeyframeAtCurrentPosition()
            }
            MenuSeparator {}
            Action {
                text: qsTr("Add &Marker")
                onTriggered: Overlays.createMarker(Camera.latitude, Camera.longitude)
            }
            Action {
                text: qsTr("Add &Arrow")
                onTriggered: {
                    // Create arrow from center of view
                    Overlays.createArrow(Camera.latitude, Camera.longitude,
                                        Camera.latitude + 1, Camera.longitude + 1)
                }
            }
            Action {
                text: qsTr("Add &Region Highlight")
                onTriggered: regionPicker.open()
            }
        }
        Menu {
            title: qsTr("&View")
            Action {
                text: qsTr("&Country Labels")
                checkable: true
                checked: Settings.showCountryLabels
                onTriggered: Settings.showCountryLabels = checked
            }
            Action {
                text: qsTr("&Region Labels")
                checkable: true
                checked: Settings.showRegionLabels
                onTriggered: Settings.showRegionLabels = checked
            }
            Action {
                text: qsTr("C&ity Labels")
                checkable: true
                checked: Settings.showCityLabels
                onTriggered: Settings.showCityLabels = checked
            }
            MenuSeparator {}
            Action {
                text: qsTr("&Shade Non-Highlighted Regions")
                checkable: true
                checked: Settings.shadeNonHighlighted
                onTriggered: Settings.shadeNonHighlighted = checked
            }
            MenuSeparator {}
            Action {
                text: root.viewMode === "2D" ? qsTr("Switch to &3D Globe") : qsTr("Switch to &2D Map")
                shortcut: "V"
                onTriggered: root.viewMode = (root.viewMode === "2D" ? "3D" : "2D")
            }
            MenuSeparator {}
            Menu {
                title: qsTr("&Map Style")
                Repeater {
                    model: Theme.tileSourceNames
                    MenuItem {
                        text: modelData
                        checkable: true
                        checked: Settings.tileSource === index
                        onTriggered: Settings.tileSource = index
                    }
                }
            }
        }
        Menu {
            title: qsTr("&Playback")
            Action {
                text: AnimController.playing ? qsTr("&Pause") : qsTr("&Play")
                shortcut: "Space"
                onTriggered: AnimController.playing ? AnimController.pause() : AnimController.play()
            }
            Action {
                text: qsTr("&Stop")
                shortcut: "Home"
                onTriggered: AnimController.stop()
            }
            MenuSeparator {}
            Action {
                text: qsTr("Previous &Keyframe")
                shortcut: "Page Up"
                onTriggered: AnimController.stepBackward()
            }
            Action {
                text: qsTr("Next K&eyframe")
                shortcut: "Page Down"
                onTriggered: AnimController.stepForward()
            }
        }
        Menu {
            title: qsTr("&Help")
            Action {
                text: qsTr("&About")
                onTriggered: aboutDialog.open()
            }
        }
    }

    // Main layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Toolbar
        ToolBar {
            Layout.fillWidth: true
            height: Theme.toolbarHeight
            background: Rectangle {
                color: Theme.surfaceColor
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: Theme.borderColor
                }
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.spacingNormal
                anchors.rightMargin: Theme.spacingNormal
                spacing: Theme.spacingNormal

                // Map style selector
                Label {
                    text: qsTr("Map:")
                    color: Theme.textColorDim
                }
                ComboBox {
                    id: tileSourceCombo
                    model: Theme.tileSourceNames
                    currentIndex: Settings.tileSource
                    onCurrentIndexChanged: Settings.tileSource = currentIndex
                    implicitWidth: 180
                }

                Item { Layout.preferredWidth: Theme.spacingLarge }

                // Add keyframe button
                Button {
                    text: qsTr("+ Keyframe")
                    onClicked: MainController.addKeyframeAtCurrentPosition()
                    highlighted: true
                }

                // Autokey toggle with recording indicator
                Button {
                    id: autoKeyButton
                    text: qsTr("Autokey")
                    checkable: true
                    checked: Settings.autoKey
                    onClicked: Settings.autoKey = checked

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Auto-add keyframes when moving camera (3ds Max style)")

                    background: Rectangle {
                        id: autoKeyBg
                        color: parent.checked ? "#cc3333" : Theme.surfaceColorLight
                        radius: Theme.radiusSmall
                        border.color: parent.checked ? "#ff4444" : Theme.borderColor
                        border.width: parent.checked ? 2 : 1

                        // Pulsing animation when active
                        SequentialAnimation on color {
                            running: autoKeyButton.checked
                            loops: Animation.Infinite
                            ColorAnimation { to: "#dd4444"; duration: 600; easing.type: Easing.InOutQuad }
                            ColorAnimation { to: "#aa2222"; duration: 600; easing.type: Easing.InOutQuad }
                        }
                    }

                    contentItem: Row {
                        spacing: 6
                        anchors.centerIn: parent

                        // Recording dot
                        Rectangle {
                            width: 8
                            height: 8
                            radius: 4
                            color: autoKeyButton.checked ? "#ff6666" : "transparent"
                            anchors.verticalCenter: parent.verticalCenter
                            visible: autoKeyButton.checked

                            SequentialAnimation on opacity {
                                running: autoKeyButton.checked
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.4; duration: 500 }
                                NumberAnimation { to: 1.0; duration: 500 }
                            }
                        }

                        Text {
                            text: autoKeyButton.text
                            color: autoKeyButton.checked ? "white" : Theme.textColor
                            font.bold: autoKeyButton.checked
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                }

                Item { Layout.preferredWidth: Theme.spacingLarge }

                // 2D/3D View toggle
                Rectangle {
                    implicitWidth: viewToggleRow.width + 8
                    implicitHeight: 32
                    color: Theme.surfaceColorLight
                    radius: Theme.radiusSmall
                    border.color: Theme.borderColor
                    border.width: 1

                    RowLayout {
                        id: viewToggleRow
                        anchors.centerIn: parent
                        spacing: 0

                        Rectangle {
                            width: 40
                            height: 28
                            radius: Theme.radiusSmall
                            color: root.viewMode === "2D" ? Theme.primaryColor : "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: "2D"
                                color: root.viewMode === "2D" ? "white" : Theme.textColorDim
                                font.bold: root.viewMode === "2D"
                                font.pixelSize: 12
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.viewMode = "2D"
                            }
                        }

                        Rectangle {
                            width: 40
                            height: 28
                            radius: Theme.radiusSmall
                            color: root.viewMode === "3D" ? Theme.primaryColor : "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: "3D"
                                color: root.viewMode === "3D" ? "white" : Theme.textColorDim
                                font.bold: root.viewMode === "3D"
                                font.pixelSize: 12
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.viewMode = "3D"
                            }
                        }
                    }

                    ToolTip.visible: viewToggleHover.containsMouse
                    ToolTip.text: qsTr("Switch between 2D map and 3D globe view")

                    MouseArea {
                        id: viewToggleHover
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.NoButton
                    }
                }

                Item { Layout.fillWidth: true }

                // Current position display
                Label {
                    text: qsTr("Lat: %1  Lon: %2  Zoom: %3")
                        .arg(Camera.latitude.toFixed(4))
                        .arg(Camera.longitude.toFixed(4))
                        .arg(Camera.zoom.toFixed(1))
                    color: Theme.textColorDim
                    font.family: "Consolas"
                }

                Item { Layout.preferredWidth: Theme.spacingLarge }

                // Export button
                Button {
                    text: qsTr("Export")
                    icon.name: "media-playback-start"
                    onClicked: exportDialog.open()
                }
            }
        }

        // Main content area
        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            // Left panel - Overlays
            Rectangle {
                SplitView.preferredWidth: 280
                SplitView.minimumWidth: Theme.panelMinWidth
                SplitView.maximumWidth: Theme.panelMaxWidth
                color: Theme.surfaceColor

                OverlayPanel {
                    anchors.fill: parent
                }
            }

            // Center - Map/Globe and Timeline (vertical split for resizable timeline)
            SplitView {
                SplitView.fillWidth: true
                orientation: Qt.Vertical

                // Custom handle for vertical resizing
                handle: Rectangle {
                    implicitWidth: parent.width
                    implicitHeight: 6
                    color: SplitHandle.pressed ? Theme.primaryColor :
                           SplitHandle.hovered ? Theme.borderColorLight : Theme.borderColor

                    // Grip indicator dots
                    Row {
                        anchors.centerIn: parent
                        spacing: 4
                        Repeater {
                            model: 3
                            Rectangle {
                                width: 4
                                height: 4
                                radius: 2
                                color: SplitHandle.pressed ? "white" :
                                       SplitHandle.hovered ? Theme.textColor : Theme.textColorDim
                            }
                        }
                    }
                }

                // View container (2D Map or 3D Globe)
                Item {
                    SplitView.fillWidth: true
                    SplitView.fillHeight: true
                    SplitView.minimumHeight: 200

                    // 2D Map view
                    MapView {
                        id: mapView
                        anchors.fill: parent
                        visible: root.viewMode === "2D"
                    }

                    // 3D Globe view
                    Loader {
                        id: globeLoader
                        anchors.fill: parent
                        active: root.viewMode === "3D"
                        source: "components/GlobeView.qml"

                        onLoaded: {
                            if (item) {
                                item.toggle2DView.connect(function() {
                                    root.viewMode = "2D"
                                })
                            }
                        }
                    }
                }

                // Unified Timeline (Camera keyframes + Overlay tracks) - now resizable
                Rectangle {
                    SplitView.fillWidth: true
                    SplitView.preferredHeight: 220
                    SplitView.minimumHeight: 100
                    SplitView.maximumHeight: 500
                    color: Theme.timelineBackground

                    Timeline {
                        anchors.fill: parent
                    }
                }
            }

            // Right panel - Keyframe properties
            Rectangle {
                SplitView.preferredWidth: 300
                SplitView.minimumWidth: Theme.panelMinWidth
                SplitView.maximumWidth: Theme.panelMaxWidth
                color: Theme.surfaceColor

                KeyframePanel {
                    anchors.fill: parent
                }
            }
        }

        // Bottom bar - Preview controls
        PreviewControls {
            Layout.fillWidth: true
        }
    }

    // Dialogs
    FileDialog {
        id: openDialog
        title: qsTr("Open Project")
        nameFilters: ["Kort Animator Projects (*.kart)", "All files (*)"]
        onAccepted: {
            ProjectManager.openProject(selectedFile)
        }
    }

    FileDialog {
        id: saveAsDialog
        title: qsTr("Save Project As")
        nameFilters: ["Kort Animator Projects (*.kart)"]
        fileMode: FileDialog.SaveFile
        defaultSuffix: "kart"
        onAccepted: {
            ProjectManager.saveProjectAs(selectedFile)
        }
    }

    ExportDialog {
        id: exportDialog
    }

    RegionPicker {
        id: regionPicker
        onRegionSelected: (code, name) => {
            Overlays.createRegionHighlight(code)
        }
    }

    Dialog {
        id: aboutDialog
        title: qsTr("About Tristans Kort Animator")
        modal: true
        anchors.centerIn: parent
        width: 400

        contentItem: ColumnLayout {
            spacing: Theme.spacingNormal

            Label {
                text: qsTr("Tristans Kort Animator")
                font.pixelSize: Theme.fontSizeHeader
                font.bold: true
                color: Theme.textColor
            }
            Label {
                text: qsTr("Version %1").arg(AppVersion)
                color: Theme.textColorDim
            }
            Label {
                text: qsTr("A map animation tool for creating geopolitical and war-themed YouTube content.")
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: Theme.textColor
            }
        }

        standardButtons: Dialog.Ok
    }

    // Unsaved changes dialog
    Dialog {
        id: unsavedChangesDialog
        title: qsTr("Unsaved Changes")
        modal: true
        anchors.centerIn: parent
        width: 400

        property var pendingAction: null

        contentItem: Label {
            text: qsTr("You have unsaved changes. Do you want to save before continuing?")
            wrapMode: Text.WordWrap
            color: Theme.textColor
            width: parent.width
        }

        standardButtons: Dialog.Save | Dialog.Discard | Dialog.Cancel

        onAccepted: {
            ProjectManager.saveProject()
            if (pendingAction) pendingAction()
        }
        onDiscarded: {
            if (pendingAction) pendingAction()
        }
    }

    onClosing: (close) => {
        // Auto-save on exit if there's a project path
        if (ProjectManager.hasUnsavedChanges && ProjectManager.projectPath !== "") {
            ProjectManager.saveProject()
        }
        // If unsaved changes and no project path, ask user
        else if (ProjectManager.hasUnsavedChanges) {
            close.accepted = false
            unsavedChangesDialog.pendingAction = () => Qt.quit()
            unsavedChangesDialog.open()
        }
    }

    // Auto-load last project on startup
    Component.onCompleted: {
        ProjectManager.loadLastProject()
    }
}
