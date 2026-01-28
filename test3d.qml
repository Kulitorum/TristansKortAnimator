import QtQuick
import QtQuick.Window
import QtQuick3D

Window {
    width: 800
    height: 600
    visible: true
    title: "Qt Quick 3D Test"
    color: "black"

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

            NumberAnimation on eulerRotation.y {
                from: 0
                to: 360
                duration: 3000
                loops: Animation.Infinite
            }
        }
    }

    Text {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
        text: "If you see a rotating blue sphere, Qt Quick 3D works!"
        color: "white"
        font.pixelSize: 18
    }
}
