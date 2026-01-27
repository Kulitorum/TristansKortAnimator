#include "keyframe.h"

QJsonObject Keyframe::toJson() const {
    QJsonObject obj;
    obj["latitude"] = latitude;
    obj["longitude"] = longitude;
    obj["zoom"] = zoom;
    obj["bearing"] = bearing;
    obj["tilt"] = tilt;
    obj["timeMs"] = timeMs;
    return obj;
}

Keyframe Keyframe::fromJson(const QJsonObject& obj) {
    Keyframe kf;
    kf.latitude = obj["latitude"].toDouble();
    kf.longitude = obj["longitude"].toDouble();
    kf.zoom = obj["zoom"].toDouble(5.0);
    kf.bearing = obj["bearing"].toDouble();
    kf.tilt = obj["tilt"].toDouble();
    kf.timeMs = obj["timeMs"].toDouble(0.0);
    return kf;
}
