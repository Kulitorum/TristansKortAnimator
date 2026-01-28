import QtQuick
import QtQuick3D

Item {
    width: 800
    height: 600

    View3D {
        anchors.fill: parent
        camera: camera

        environment: SceneEnvironment {
            clearColor: "#003366"
            backgroundMode: SceneEnvironment.Color
        }

        PerspectiveCamera {
            id: camera
            z: 300
        }

        DirectionalLight {
            eulerRotation.x: -30
            eulerRotation.y: -70
        }

        Model {
            source: "#Sphere"
            scale: Qt.vector3d(50, 50, 50)
            materials: PrincipledMaterial {
                baseColor: "dodgerblue"
            }
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        color: "#80000000"
        width: label.width + 20
        height: label.height + 10
        radius: 5

        Text {
            id: label
            anchors.centerIn: parent
            text: "You should see a blue sphere above"
            color: "white"
            font.pixelSize: 16
        }
    }
}
