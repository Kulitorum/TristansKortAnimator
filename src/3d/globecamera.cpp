#include "globecamera.h"
#include <QtMath>

GlobeCamera::GlobeCamera(QObject* parent)
    : QObject(parent)
{
    updatePosition();
}

void GlobeCamera::setLatitude(double lat) {
    lat = qBound(-89.9, lat, 89.9);
    if (qFuzzyCompare(m_latitude, lat)) return;
    m_latitude = lat;
    emit latitudeChanged();
    updatePosition();
}

void GlobeCamera::setLongitude(double lon) {
    // Normalize to -180 to 180
    while (lon > 180.0) lon -= 360.0;
    while (lon < -180.0) lon += 360.0;
    if (qFuzzyCompare(m_longitude, lon)) return;
    m_longitude = lon;
    emit longitudeChanged();
    updatePosition();
}

void GlobeCamera::setAltitude(double alt) {
    alt = qMax(1.0, alt);  // Minimum altitude above surface
    if (qFuzzyCompare(m_altitude, alt)) return;
    m_altitude = alt;
    emit altitudeChanged();
    updatePosition();
}

void GlobeCamera::setBearing(double bearing) {
    // Normalize to 0-360
    while (bearing >= 360.0) bearing -= 360.0;
    while (bearing < 0.0) bearing += 360.0;
    if (qFuzzyCompare(m_bearing, bearing)) return;
    m_bearing = bearing;
    emit bearingChanged();
    updatePosition();
}

void GlobeCamera::setTilt(double tilt) {
    tilt = qBound(0.0, tilt, 89.0);  // Don't allow full horizontal view
    if (qFuzzyCompare(m_tilt, tilt)) return;
    m_tilt = tilt;
    emit tiltChanged();
    updatePosition();
}

void GlobeCamera::setGlobeRadius(double radius) {
    if (qFuzzyCompare(m_globeRadius, radius)) return;
    m_globeRadius = radius;
    emit globeRadiusChanged();
    updatePosition();
}

QVector3D GlobeCamera::latLonToPosition(double lat, double lon, double radius) const {
    double latRad = qDegreesToRadians(lat);
    double lonRad = qDegreesToRadians(lon);

    float x = radius * qCos(latRad) * qSin(lonRad);
    float y = radius * qSin(latRad);
    float z = radius * qCos(latRad) * qCos(lonRad);

    return QVector3D(x, y, z);
}

double GlobeCamera::zoomToAltitude(double zoom) const {
    // Convert web map zoom level (1-19) to 3D altitude
    // Higher zoom = closer to surface = lower altitude
    // zoom 1 = whole world visible = altitude ~500
    // zoom 19 = street level = altitude ~1
    double maxAltitude = m_globeRadius * 5.0;
    double minAltitude = m_globeRadius * 0.01;

    double t = (zoom - 1.0) / 18.0;  // Normalize 1-19 to 0-1
    t = qBound(0.0, t, 1.0);

    // Exponential mapping
    return maxAltitude * qPow(minAltitude / maxAltitude, t);
}

double GlobeCamera::altitudeToZoom(double altitude) const {
    double maxAltitude = m_globeRadius * 5.0;
    double minAltitude = m_globeRadius * 0.01;

    altitude = qBound(minAltitude, altitude, maxAltitude);

    // Inverse of zoomToAltitude
    double t = qLn(altitude / maxAltitude) / qLn(minAltitude / maxAltitude);
    return 1.0 + t * 18.0;
}

void GlobeCamera::updatePosition() {
    // Calculate camera position based on lat/lon/altitude/tilt/bearing

    // First, get the point on the globe we're looking at
    QVector3D targetOnGlobe = latLonToPosition(m_latitude, m_longitude, m_globeRadius);

    // The "up" direction at this point on the globe (radial outward)
    QVector3D radialUp = targetOnGlobe.normalized();

    // Calculate a reference "north" direction at this point
    // North is the direction of increasing latitude
    QVector3D north;
    if (qAbs(m_latitude) > 89.0) {
        // At poles, north is arbitrary - use world X axis
        north = QVector3D(1, 0, 0);
    } else {
        // North is perpendicular to radialUp and points toward +Y
        QVector3D worldUp(0, 1, 0);
        QVector3D east = QVector3D::crossProduct(worldUp, radialUp).normalized();
        north = QVector3D::crossProduct(radialUp, east).normalized();
    }

    // Apply bearing rotation (rotate around radialUp)
    double bearingRad = qDegreesToRadians(m_bearing);
    QVector3D rotatedNorth = north * qCos(bearingRad) +
                             QVector3D::crossProduct(radialUp, north) * qSin(bearingRad);

    // The camera looks from a position above the target point
    // Tilt affects how much the camera is tilted away from directly above

    // At tilt=0, camera is directly above looking down at target
    // At tilt=90, camera is at horizon level looking sideways

    double tiltRad = qDegreesToRadians(m_tilt);

    // Calculate camera position
    // Start directly above the target point at the given altitude
    double cameraRadius = m_globeRadius + m_altitude;

    if (m_tilt < 1.0) {
        // Looking straight down - camera is directly above target
        m_position = radialUp * cameraRadius;
        m_lookAt = targetOnGlobe;
        m_upVector = rotatedNorth;
    } else {
        // With tilt, camera moves back along the rotatedNorth direction
        // and tilts to look at the target

        // How far back to move (based on tilt and altitude)
        double backDistance = m_altitude * qTan(tiltRad);

        // Camera position: up from center, then back along negative rotatedNorth
        QVector3D upComponent = radialUp * cameraRadius;
        QVector3D backComponent = -rotatedNorth * backDistance;

        m_position = upComponent + backComponent;
        m_lookAt = targetOnGlobe;

        // Up vector should be perpendicular to view direction and roughly aligned with radialUp
        QVector3D viewDir = (m_lookAt - m_position).normalized();
        QVector3D rightDir = QVector3D::crossProduct(viewDir, radialUp).normalized();
        m_upVector = QVector3D::crossProduct(rightDir, viewDir).normalized();
    }

    emit positionChanged();
    emit lookAtChanged();
    emit upVectorChanged();
    emit cameraChanged();
}
