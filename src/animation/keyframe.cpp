#include "keyframe.h"
#include <QDebug>

QJsonObject Keyframe::toJson() const {
    QJsonObject obj;
    obj["version"] = 2;  // New 3D camera format
    obj["latitude"] = latitude;
    obj["longitude"] = longitude;
    obj["altitude"] = altitude;
    obj["bearing"] = bearing;
    obj["tilt"] = tilt;
    obj["timeMs"] = timeMs;
    obj["easing"] = easing;
    return obj;
}

Keyframe Keyframe::fromJson(const QJsonObject& obj) {
    Keyframe kf;

    // Check for new format (has altitude)
    if (obj.contains("altitude")) {
        kf.latitude = obj["latitude"].toDouble();
        kf.longitude = obj["longitude"].toDouble();
        kf.altitude = obj["altitude"].toDouble(1000000.0);
        kf.bearing = obj["bearing"].toDouble();
        kf.tilt = obj["tilt"].toDouble();
        kf.timeMs = obj["timeMs"].toDouble(0.0);
        kf.easing = obj["easing"].toDouble(0.5);  // Default to medium smoothness
    } else {
        // Old format with zoom - refuse to load
        qWarning() << "Old keyframe format detected (uses 'zoom' instead of 'altitude'). Please create a new project.";
        // Return default keyframe
        kf.latitude = 52.5;
        kf.longitude = 10.0;
        kf.altitude = 1000000.0;
    }

    return kf;
}
