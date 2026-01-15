import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import TristansKortAnimator

Dialog {
    id: exportDialog
    title: qsTr("Export Video")
    modal: true
    anchors.centerIn: parent
    width: 450

    property bool isExporting: Exporter.exporting

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spacingNormal

        // Resolution
        GroupBox {
            title: qsTr("Resolution")
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: Theme.spacingNormal

                RadioButton {
                    text: "1920 x 1080 (1080p)"
                    checked: true
                }
                RadioButton {
                    text: "1280 x 720 (720p)"
                }
            }
        }

        // Framerate
        GroupBox {
            title: qsTr("Framerate")
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: Theme.spacingNormal

                RadioButton {
                    id: fps24
                    text: "24 fps"
                }
                RadioButton {
                    id: fps30
                    text: "30 fps"
                    checked: true
                }
                RadioButton {
                    id: fps60
                    text: "60 fps"
                }
            }
        }

        // Output path
        GroupBox {
            title: qsTr("Output File")
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                spacing: Theme.spacingSmall

                TextField {
                    id: outputPath
                    text: Settings.lastExportPath + "/animation.mp4"
                    Layout.fillWidth: true
                }

                Button {
                    text: qsTr("Browse...")
                    onClicked: saveDialog.open()
                }
            }
        }

        // Duration info
        Text {
            text: qsTr("Animation duration: %1 seconds").arg((Keyframes.totalDuration / 1000).toFixed(1))
            color: Theme.textColorDim
        }

        // Progress
        ColumnLayout {
            Layout.fillWidth: true
            visible: isExporting
            spacing: Theme.spacingSmall

            ProgressBar {
                Layout.fillWidth: true
                value: Exporter.progress
            }

            Text {
                text: Exporter.status
                color: Theme.textColor
            }

            Text {
                text: qsTr("Frame %1 of %2").arg(Exporter.currentFrame).arg(Exporter.totalFrames)
                color: Theme.textColorDim
            }
        }
    }

    footer: DialogButtonBox {
        Button {
            text: isExporting ? qsTr("Cancel") : qsTr("Export")
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            enabled: !isExporting || Exporter.exporting
        }
        Button {
            text: qsTr("Close")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            enabled: !isExporting
        }
    }

    onAccepted: {
        if (isExporting) {
            Exporter.cancelExport()
        } else {
            startExport()
        }
    }

    FileDialog {
        id: saveDialog
        title: qsTr("Save Video As")
        nameFilters: ["MP4 Video (*.mp4)"]
        fileMode: FileDialog.SaveFile
        defaultSuffix: "mp4"
        onAccepted: outputPath.text = selectedFile.toString().replace("file:///", "")
    }

    function startExport() {
        let fps = fps24.checked ? 24 : (fps60.checked ? 60 : 30)
        let width = 1920
        let height = 1080

        Exporter.startExport(outputPath.text, width, height, fps)
    }

    Connections {
        target: Exporter
        function onExportComplete(path) {
            exportDialog.close()
            successDialog.open()
        }
        function onExportError(error) {
            errorDialog.text = error
            errorDialog.open()
        }
    }

    Dialog {
        id: successDialog
        title: qsTr("Export Complete")
        modal: true
        anchors.centerIn: parent

        Label {
            text: qsTr("Video exported successfully!")
        }

        standardButtons: Dialog.Ok
    }

    Dialog {
        id: errorDialog
        title: qsTr("Export Error")
        modal: true
        anchors.centerIn: parent

        property alias text: errorLabel.text

        Label {
            id: errorLabel
            wrapMode: Text.WordWrap
        }

        standardButtons: Dialog.Ok
    }
}
