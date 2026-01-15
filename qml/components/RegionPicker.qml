import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TristansKortAnimator

Dialog {
    id: regionPicker
    title: qsTr("Select Region")
    modal: true
    anchors.centerIn: parent
    width: 400
    height: 500

    signal regionSelected(string code, string name)

    ColumnLayout {
        anchors.fill: parent
        spacing: Theme.spacingNormal

        TextField {
            id: searchField
            placeholderText: qsTr("Search countries...")
            Layout.fillWidth: true
        }

        ListView {
            id: countryList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: GeoJson.countryList()

            delegate: ItemDelegate {
                width: countryList.width
                text: modelData.name + " (" + modelData.code + ")"
                visible: searchField.text === "" ||
                         modelData.name.toLowerCase().includes(searchField.text.toLowerCase())
                height: visible ? implicitHeight : 0

                onClicked: {
                    regionPicker.regionSelected(modelData.code, modelData.name)
                    regionPicker.close()
                }
            }
        }
    }

    standardButtons: Dialog.Cancel
}
