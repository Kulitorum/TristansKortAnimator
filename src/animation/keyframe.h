#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>

enum class InterpolationMode {
    ArcZoom = 0,      // Zoom out, sweep, zoom in (Google Earth style)
    DirectFly,        // Straight line with adaptive zoom
    Orbital,          // Zoom out to space, rotate, dive in
    SnapCut,          // Instant transition
    Glide             // Slow gentle pan
};

enum class EasingType {
    Linear = 0,
    EaseInOut,
    EaseIn,
    EaseOut,
    EaseInOutCubic,
    EaseInOutQuint
};

class Keyframe {
    Q_GADGET

    Q_PROPERTY(double latitude MEMBER latitude)
    Q_PROPERTY(double longitude MEMBER longitude)
    Q_PROPERTY(double zoom MEMBER zoom)
    Q_PROPERTY(double bearing MEMBER bearing)
    Q_PROPERTY(double tilt MEMBER tilt)
    Q_PROPERTY(double timeMs MEMBER timeMs)
    Q_PROPERTY(int interpolationMode MEMBER interpolationModeInt)
    Q_PROPERTY(int easingType MEMBER easingTypeInt)

public:
    double latitude = 0.0;
    double longitude = 0.0;
    double zoom = 5.0;
    double bearing = 0.0;
    double tilt = 0.0;
    double timeMs = 0.0;  // Absolute time position on timeline

    InterpolationMode interpolation = InterpolationMode::DirectFly;
    EasingType easing = EasingType::EaseInOut;

    // For Q_PROPERTY (enums as ints)
    int interpolationModeInt = static_cast<int>(InterpolationMode::DirectFly);
    int easingTypeInt = static_cast<int>(EasingType::EaseInOut);

    // Serialization
    QJsonObject toJson() const;
    static Keyframe fromJson(const QJsonObject& obj);

    // Sync enum ints with actual enums
    void syncEnumInts() {
        interpolationModeInt = static_cast<int>(interpolation);
        easingTypeInt = static_cast<int>(easing);
    }

    void syncEnumsFromInts() {
        interpolation = static_cast<InterpolationMode>(interpolationModeInt);
        easing = static_cast<EasingType>(easingTypeInt);
    }
};

Q_DECLARE_METATYPE(Keyframe)
