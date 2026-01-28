#include "interpolator.h"
#include <QtMath>
#include <cmath>

Interpolator::Interpolator(QObject* parent)
    : QObject(parent)
{
}

CameraState Interpolator::interpolate(const Keyframe& from, const Keyframe& to, double t) {
    // Apply ease-in-out to the time parameter
    double easedT = easeInOut(t);

    CameraState state;

    // Simple linear interpolation of all properties including altitude
    // Because altitude is linear (not logarithmic like zoom), this produces
    // smooth, natural camera paths where position and "zoom" change together
    state.latitude = from.latitude + (to.latitude - from.latitude) * easedT;
    state.longitude = interpolateLongitude(from.longitude, to.longitude, easedT);
    state.altitude = from.altitude + (to.altitude - from.altitude) * easedT;
    state.bearing = interpolateBearing(from.bearing, to.bearing, easedT);
    state.tilt = from.tilt + (to.tilt - from.tilt) * easedT;

    return state;
}

double Interpolator::easeInOut(double t) {
    // Quadratic ease-in-out
    if (t < 0.5) {
        return 2.0 * t * t;
    } else {
        return 1.0 - std::pow(-2.0 * t + 2.0, 2) / 2.0;
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
