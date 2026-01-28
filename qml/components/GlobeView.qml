import QtQuick
import QtQuick3D

Item {
    id: globeView

    property real globeRadius: 100

    View3D {
        id: view3D
        anchors.fill: parent
        camera: camera

        environment: SceneEnvironment {
            clearColor: "#002244"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            z: 300
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
            brightness: 1.5
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(50, 50, 50)
            materials: PrincipledMaterial {
                baseColor: "#4488ff"
                metalness: 0.0
                roughness: 0.5
            }
        }

        Model {
            source: "#Cube"
            x: 100
            scale: Qt.vector3d(30, 30, 30)
            materials: PrincipledMaterial {
                baseColor: "#ff4444"
            }
        }
    }

    // Debug text
    Text {
        anchors.centerIn: parent
        text: "If you see this but no sphere,\nQt Quick 3D may not be working"
        color: "yellow"
        font.pixelSize: 16
        horizontalAlignment: Text.AlignHCenter
    }

    // Info overlay
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 10
        color: "#80000000"
        radius: 4
        padding: 8
        width: infoText.width + 16
        height: infoText.height + 16

        Text {
            id: infoText
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 12
            text: "3D Globe View"
        }
    }

    // View toggle button
    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        width: 80
        height: 30
        color: toggleArea.containsMouse ? "#404040" : "#303030"
        radius: 4

        Text {
            anchors.centerIn: parent
            text: "2D View"
            color: "white"
            font.pixelSize: 12
        }

        MouseArea {
            id: toggleArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: globeView.toggle2DView()
        }
    }

    signal toggle2DView()
}
