#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>

#include "version.h"
#include "core/settings.h"
#include "core/projectmanager.h"
#include "map/maprenderer.h"
#include "map/tileprovider.h"
#include "map/tilecache.h"
#include "map/mapcamera.h"
#include "map/geojsonparser.h"
#include "animation/keyframe.h"
#include "animation/keyframemodel.h"
#include "animation/regiontrackmodel.h"
#include "animation/geooverlaymodel.h"
#include "animation/animationcontroller.h"
#include "overlays/overlaymanager.h"
#include "export/videoexporter.h"
#include "controllers/maincontroller.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("TristansKortAnimator");
    app.setOrganizationDomain("tristans-kort-animator.local");
    app.setApplicationName("Kort Animator");
    app.setApplicationVersion(VERSION_STRING);
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));

    // Use Material style for modern look
    QQuickStyle::setStyle("Material");

    // Create main controller (owns all subsystems)
    MainController mainController;

    // Setup QML engine
    QQmlApplicationEngine engine;
    QQmlContext* context = engine.rootContext();

    // Expose C++ objects to QML
    context->setContextProperty("Settings", mainController.settings());
    context->setContextProperty("MainController", &mainController);
    context->setContextProperty("ProjectManager", mainController.projectManager());
    context->setContextProperty("Keyframes", mainController.keyframes());
    context->setContextProperty("RegionTracks", mainController.regionTracks());
    context->setContextProperty("GeoOverlays", mainController.geoOverlays());
    context->setContextProperty("Overlays", mainController.overlays());
    context->setContextProperty("Camera", mainController.camera());
    context->setContextProperty("AnimController", mainController.animation());
    context->setContextProperty("Exporter", mainController.exporter());
    context->setContextProperty("TileProvider", mainController.tileProvider());
    context->setContextProperty("GeoJson", mainController.geojson());
    context->setContextProperty("AppVersion", VERSION_STRING);

    // Register QML types
    qmlRegisterType<MapRenderer>("TristansKortAnimator", 1, 0, "MapRenderer");
    qmlRegisterUncreatableType<MapCamera>("TristansKortAnimator", 1, 0, "MapCameraType",
        "MapCamera is created in C++");
    qmlRegisterUncreatableType<KeyframeModel>("TristansKortAnimator", 1, 0, "KeyframeModelType",
        "KeyframeModel is created in C++");

    // Load main QML
    const QUrl url(u"qrc:/qt/qml/TristansKortAnimator/qml/main.qml"_qs);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
