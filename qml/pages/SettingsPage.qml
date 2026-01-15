import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

Page {
    id: settingsPage
    title: qsTr("Settings")

    ScrollView {
        anchors.fill: parent
        anchors.margins: Theme.spacingLarge

        ColumnLayout {
            width: parent.width
            spacing: Theme.spacingLarge

            GroupBox {
                title: qsTr("Map Settings")
                Layout.fillWidth: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: Theme.spacingNormal

                    CheckBox {
                        text: qsTr("Show Country Labels")
                        checked: Settings.showCountryLabels
                        onCheckedChanged: Settings.showCountryLabels = checked
                    }

                    CheckBox {
                        text: qsTr("Show Region Labels")
                        checked: Settings.showRegionLabels
                        onCheckedChanged: Settings.showRegionLabels = checked
                    }

                    CheckBox {
                        text: qsTr("Show City Labels")
                        checked: Settings.showCityLabels
                        onCheckedChanged: Settings.showCityLabels = checked
                    }
                }
            }

            GroupBox {
                title: qsTr("Export Settings")
                Layout.fillWidth: true

                GridLayout {
                    columns: 2
                    anchors.fill: parent

                    Label { text: qsTr("Default Framerate:") }
                    ComboBox {
                        model: Theme.framerateOptions
                        currentIndex: Theme.framerateOptions.indexOf(Settings.exportFramerate)
                        onCurrentIndexChanged: Settings.exportFramerate = Theme.framerateOptions[currentIndex]
                    }

                    Label { text: qsTr("FFmpeg Path:") }
                    TextField {
                        text: Settings.ffmpegPath
                        onEditingFinished: Settings.ffmpegPath = text
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
