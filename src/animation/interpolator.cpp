#include "interpolator.h"
#include "easingfunctions.h"
#include <QtMath>
#include <algorithm>

Interpolator::Interpolator(QObject* parent)
    : QObject(parent)
{
}

CameraState Interpolator::interpolate(const Keyframe& from, const Keyframe& to,
                                      double t, InterpolationMode mode, EasingType easing) {
    // Apply easing to the time parameter
    double easedT = applyEasing(t, easing);

    switch (mode) {
        case InterpolationMode::ArcZoom:
            return arcZoomInterpolate(from, to, easedT);
        case InterpolationMode::DirectFly:
            return directFlyInterpolate(from, to, easedT);
        case InterpolationMode::Orbital:
            return orbitalInterpolate(from, to, easedT);
        case InterpolationMode::Glide:
            return glideInterpolate(from, to, easedT);
        case InterpolationMode::SnapCut:
            return snapCutInterpolate(from, to, t); // No easing for snap
        default:
            return directFlyInterpolate(from, to, easedT);
    }
}

CameraState Interpolator::arcZoomInterpolate(const Keyframe& from, const Keyframe& to, double t) {
    // Google Earth style: zoom out, pan across, zoom in
    double distance = greatCircleDistance(from.latitude, from.longitude,
                                          to.latitude, to.longitude);

    // Calculate how far to zoom out based on distance
    double arcHeight = calculateArcZoomOut(distance, from.zoom, to.zoom);

    CameraState state;

    // Three phases: zoom out (0-0.3), pan (0.3-0.7), zoom in (0.7-1.0)
    if (t < 0.3) {
        // Phase 1: Zoom out while starting to move
        double phase = t / 0.3;
        double smoothPhase = smoothStep(phase);

        state.latitude = from.latitude + (to.latitude - from.latitude) * smoothPhase * 0.2;
        state.longitude = interpolateLongitude(from.longitude, to.longitude, smoothPhase * 0.2);
        state.zoom = from.zoom - (from.zoom - arcHeight) * smoothPhase;
    } else if (t < 0.7) {
        // Phase 2: Pan across at zoomed-out level
        double phase = (t - 0.3) / 0.4;

        state.latitude = from.latitude + (to.latitude - from.latitude) * (0.2 + phase * 0.6);
        state.longitude = interpolateLongitude(from.longitude, to.longitude, 0.2 + phase * 0.6);
        state.zoom = arcHeight;
    } else {
        // Phase 3: Zoom in while finishing movement
        double phase = (t - 0.7) / 0.3;
        double smoothPhase = smoothStep(phase);

        state.latitude = from.latitude + (to.latitude - from.latitude) * (0.8 + smoothPhase * 0.2);
        state.longitude = interpolateLongitude(from.longitude, to.longitude, 0.8 + smoothPhase * 0.2);
        state.zoom = arcHeight + (to.zoom - arcHeight) * smoothPhase;
    }

    state.bearing = interpolateBearing(from.bearing, to.bearing, t);
    state.tilt = from.tilt + (to.tilt - from.tilt) * t;

    return state;
}

CameraState Interpolator::directFlyInterpolate(const Keyframe& from, const Keyframe& to, double t) {
    // Simple linear interpolation with smooth zoom adjustment
    CameraState state;

    state.latitude = from.latitude + (to.latitude - from.latitude) * t;
    state.longitude = interpolateLongitude(from.longitude, to.longitude, t);
    state.bearing = interpolateBearing(from.bearing, to.bearing, t);
    state.tilt = from.tilt + (to.tilt - from.tilt) * t;

    // Zoom: slight arc to keep things in view during pan
    double distance = greatCircleDistance(from.latitude, from.longitude,
                                          to.latitude, to.longitude);

    // Only do adaptive zoom for longer distances
    if (distance > 100) {  // > 100 km
        double zoomDip = std::min(2.0, std::log10(distance / 100.0));
        double arcFactor = std::sin(t * M_PI);  // 0 at start/end, 1 at middle
        double baseZoom = from.zoom + (to.zoom - from.zoom) * t;
        state.zoom = baseZoom - zoomDip * arcFactor;
    } else {
        state.zoom = from.zoom + (to.zoom - from.zoom) * t;
    }

    return state;
}

CameraState Interpolator::orbitalInterpolate(const Keyframe& from, const Keyframe& to, double t) {
    // Dramatic zoom out to space, rotate, dive in
    CameraState state;

    // Very high zoom out
    double spaceZoom = 2.0;  // Almost seeing whole earth

    if (t < 0.35) {
        // Phase 1: Zoom way out
        double phase = t / 0.35;
        double smoothPhase = smoothStep(phase);

        state.latitude = from.latitude;
        state.longitude = from.longitude;
        state.zoom = from.zoom - (from.zoom - spaceZoom) * smoothPhase;
        state.bearing = from.bearing;
    } else if (t < 0.65) {
        // Phase 2: Rotate/pan at space level
        double phase = (t - 0.35) / 0.3;

        state.latitude = from.latitude + (to.latitude - from.latitude) * phase;
        state.longitude = interpolateLongitude(from.longitude, to.longitude, phase);
        state.zoom = spaceZoom;
        state.bearing = interpolateBearing(from.bearing, to.bearing, phase);
    } else {
        // Phase 3: Dive back in
        double phase = (t - 0.65) / 0.35;
        double smoothPhase = smoothStep(phase);

        state.latitude = to.latitude;
        state.longitude = to.longitude;
        state.zoom = spaceZoom + (to.zoom - spaceZoom) * smoothPhase;
        state.bearing = to.bearing;
    }

    state.tilt = from.tilt + (to.tilt - from.tilt) * t;

    return state;
}

CameraState Interpolator::glideInterpolate(const Keyframe& from, const Keyframe& to, double t) {
    // Slow, gentle, continuous movement
    CameraState state;

    // Extra smooth interpolation
    double smoothT = smoothStep(smoothStep(t));  // Double smooth

    state.latitude = from.latitude + (to.latitude - from.latitude) * smoothT;
    state.longitude = interpolateLongitude(from.longitude, to.longitude, smoothT);
    state.zoom = from.zoom + (to.zoom - from.zoom) * smoothT;
    state.bearing = interpolateBearing(from.bearing, to.bearing, smoothT);
    state.tilt = from.tilt + (to.tilt - from.tilt) * smoothT;

    return state;
}

CameraState Interpolator::snapCutInterpolate(const Keyframe& from, const Keyframe& to, double t) {
    // Instant cut - just return destination
    CameraState state;
    state.latitude = to.latitude;
    state.longitude = to.longitude;
    state.zoom = to.zoom;
    state.bearing = to.bearing;
    state.tilt = to.tilt;
    return state;
}

double Interpolator::applyEasing(double t, EasingType type) {
    switch (type) {
        case EasingType::Linear:
            return Easing::linear(t);
        case EasingType::EaseInOut:
            return Easing::easeInOutQuad(t);
        case EasingType::EaseIn:
            return Easing::easeInQuad(t);
        case EasingType::EaseOut:
            return Easing::easeOutQuad(t);
        case EasingType::EaseInOutCubic:
            return Easing::easeInOutCubic(t);
        case EasingType::EaseInOutQuint:
            return Easing::easeInOutQuint(t);
        default:
            return t;
    }
}

double Interpolator::greatCircleDistance(double lat1, double lon1, double lat2, double lon2) {
    // Haversine formula - returns distance in kilometers
    const double R = 6371.0;  // Earth radius in km

    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(lat1 * M_PI / 180.0) * std::cos(lat2 * M_PI / 180.0) *
               std::sin(dLon / 2) * std::sin(dLon / 2);

    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return R * c;
}

double Interpolator::calculateArcZoomOut(double distance, double startZoom, double endZoom) {
    // Calculate appropriate zoom out level based on distance
    // More distance = zoom out more
    double baseZoom = std::min(startZoom, endZoom);

    if (distance < 10) {
        return baseZoom - 0.5;
    } else if (distance < 100) {
        return baseZoom - 1.5;
    } else if (distance < 500) {
        return baseZoom - 2.5;
    } else if (distance < 2000) {
        return std::min(baseZoom - 3.0, 6.0);
    } else {
        return std::min(baseZoom - 4.0, 4.0);
    }
}

double Interpolator::interpolateLongitude(double from, double to, double t) {
    // Handle crossing the antimeridian (180/-180)
    double diff = to - from;

    // If the difference is more than 180, go the short way around
    if (diff > 180.0) {
        diff -= 360.0;
    } else if (diff < -180.0) {
        diff += 360.0;
    }

    double result = from + diff * t;

    // Normalize to -180 to 180
    while (result > 180.0) result -= 360.0;
    while (result < -180.0) result += 360.0;

    return result;
}

double Interpolator::interpolateBearing(double from, double to, double t) {
    // Find shortest path around the circle
    double diff = to - from;

    // Normalize to -180 to 180
    while (diff > 180.0) diff -= 360.0;
    while (diff < -180.0) diff += 360.0;

    double result = from + diff * t;

    // Normalize to 0-360
    while (result >= 360.0) result -= 360.0;
    while (result < 0.0) result += 360.0;

    return result;
}

double Interpolator::smoothStep(double t) {
    // Hermite interpolation: 3t^2 - 2t^3
    return t * t * (3.0 - 2.0 * t);
}
