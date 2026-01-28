#pragma once

#include <QObject>
#include <QPointF>
#include "keyframe.h"

struct CameraState {
    double latitude;
    double longitude;
    double altitude;  // Height above surface in meters
    double bearing;
    double tilt;

    // Derive zoom from altitude for rendering
    double zoom() const {
        return Keyframe::altitudeToZoom(altitude);
    }
};

class Interpolator : public QObject {
    Q_OBJECT

public:
    explicit Interpolator(QObject* parent = nullptr);

    // Main interpolation function - simple ease-in-out between keyframes
    CameraState interpolate(const Keyframe& from, const Keyframe& to, double t);

    // Linear mode: skip per-keyframe easing (use with speed curve)
    bool linearMode() const { return m_linearMode; }
    void setLinearMode(bool linear) { m_linearMode = linear; }

    // Easing functions
    static double easeInOut(double t);
    static double adaptiveEaseInOut(double t, double smoothness, double fromAlt, double toAlt);

    // Utility functions
    static double greatCircleDistance(double lat1, double lon1, double lat2, double lon2);

private:
    // Helper for longitude wrapping (handles crossing 180/-180)
    double interpolateLongitude(double from, double to, double t);

    // Bearing interpolation (shortest path around circle)
    double interpolateBearing(double from, double to, double t);

    bool m_linearMode = false;
};
