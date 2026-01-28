#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <cmath>

// 3D Camera keyframe - stores position as lat/lon/altitude for natural interpolation
// Altitude is the camera height above the map surface in meters
// This allows linear interpolation to produce smooth, natural camera paths
class Keyframe {
    Q_GADGET

    Q_PROPERTY(double latitude MEMBER latitude)
    Q_PROPERTY(double longitude MEMBER longitude)
    Q_PROPERTY(double altitude MEMBER altitude)
    Q_PROPERTY(double bearing MEMBER bearing)
    Q_PROPERTY(double tilt MEMBER tilt)
    Q_PROPERTY(double timeMs MEMBER timeMs)

public:
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 1000000.0;  // Default ~1000km (roughly zoom 5)
    double bearing = 0.0;
    double tilt = 0.0;
    double timeMs = 0.0;  // Position on timeline (milliseconds)

    // Convert between altitude and zoom level
    // Formula: altitude = 2^(25 - zoom) meters
    // zoom 0 ≈ 33.5M meters, zoom 20 ≈ 32 meters
    static constexpr double ALTITUDE_BASE = 33554432.0;  // 2^25 meters

    static double zoomToAltitude(double zoom) {
        return ALTITUDE_BASE / std::pow(2.0, zoom);
    }

    static double altitudeToZoom(double altitude) {
        if (altitude <= 0) altitude = 1.0;
        return 25.0 - std::log2(altitude);
    }

    // Get zoom level derived from altitude (for rendering)
    double zoom() const {
        return altitudeToZoom(altitude);
    }

    // Set altitude from zoom level (for backward compatibility)
    void setZoom(double z) {
        altitude = zoomToAltitude(z);
    }

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
