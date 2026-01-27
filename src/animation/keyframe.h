#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>

// Simplified keyframe - camera position at a specific time
class Keyframe {
    Q_GADGET

    Q_PROPERTY(double latitude MEMBER latitude)
    Q_PROPERTY(double longitude MEMBER longitude)
    Q_PROPERTY(double zoom MEMBER zoom)
    Q_PROPERTY(double bearing MEMBER bearing)
    Q_PROPERTY(double tilt MEMBER tilt)
    Q_PROPERTY(double timeMs MEMBER timeMs)

public:
    double latitude = 0.0;
    double longitude = 0.0;
    double zoom = 5.0;
    double bearing = 0.0;
    double tilt = 0.0;
    double timeMs = 0.0;  // Position on timeline (milliseconds)

    // Serialization
    QJsonObject toJson() const;
    static Keyframe fromJson(const QJsonObject& obj);
};

Q_DECLARE_METATYPE(Keyframe)

// Keep EasingType for overlay keyframes
enum class EasingType {
    Linear = 0,
    EaseInOut,
    EaseIn,
    EaseOut,
    EaseInOutCubic,
    EaseInOutQuint
};
