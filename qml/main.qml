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
            MenuSeparator {}
            Action {
                text: qsTr("&Loop")
                checkable: true
                checked: AnimController ? AnimController.looping : false
                onTriggered: if (AnimController) AnimController.looping = checked
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

                // Autokey toggle
                Button {
                    text: qsTr("Autokey")
                    checkable: true
                    checked: Settings.autoKey
                    onClicked: Settings.autoKey = checked

                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Auto-add keyframes when moving camera")

                    background: Rectangle {
                        color: parent.checked ? Theme.primaryColor : Theme.surfaceColorLight
                        radius: Theme.radiusSmall
                        border.color: parent.checked ? Theme.primaryColor : Theme.borderColor
                        border.width: 1
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

            // Center - Map and Timeline
            ColumnLayout {
                SplitView.fillWidth: true
                spacing: 0

                // Map view
                MapView {
                    id: mapView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                // Timeline
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Theme.timelineHeight
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
        if (ProjectManager.hasUnsavedChanges) {
            close.accepted = false
            unsavedChangesDialog.pendingAction = () => Qt.quit()
            unsavedChangesDialog.open()
        }
    }
}
