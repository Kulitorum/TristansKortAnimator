#pragma once

#include <QObject>
#include <QPointF>
#include "keyframe.h"

struct CameraState {
    double latitude;
    double longitude;
    double zoom;
    double bearing;
    double tilt;
};

class Interpolator : public QObject {
    Q_OBJECT

public:
    explicit Interpolator(QObject* parent = nullptr);

    // Main interpolation function
    CameraState interpolate(const Keyframe& from, const Keyframe& to,
                           double t, InterpolationMode mode, EasingType easing);

    // Individual interpolation modes
    CameraState arcZoomInterpolate(const Keyframe& from, const Keyframe& to, double t);
    CameraState directFlyInterpolate(const Keyframe& from, const Keyframe& to, double t);
    CameraState orbitalInterpolate(const Keyframe& from, const Keyframe& to, double t);
    CameraState glideInterpolate(const Keyframe& from, const Keyframe& to, double t);
    CameraState snapCutInterpolate(const Keyframe& from, const Keyframe& to, double t);

    // Easing functions
    static double applyEasing(double t, EasingType type);

    // Utility functions
    static double greatCircleDistance(double lat1, double lon1, double lat2, double lon2);
    static double calculateArcZoomOut(double distance, double startZoom, double endZoom);

private:
    // Helper for longitude wrapping (handles crossing 180/-180)
    double interpolateLongitude(double from, double to, double t);

    // Bearing interpolation (shortest path around circle)
    double interpolateBearing(double from, double to, double t);

    // Smooth step function for transitions
    double smoothStep(double t);
};
