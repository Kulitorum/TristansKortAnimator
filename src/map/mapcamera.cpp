#include "mapcamera.h"
#include <QtMath>
#include <algorithm>

MapCamera::MapCamera(QObject* parent)
    : QObject(parent)
{
    m_speedTimer.start();
}

void MapCamera::setLatitude(double lat) {
    lat = std::clamp(lat, -85.0, 85.0);
    if (!qFuzzyCompare(m_latitude, lat)) {
        m_latitude = lat;
        emit latitudeChanged();
        emit cameraChanged();
    }
}

void MapCamera::setLongitude(double lon) {
    // Wrap longitude to -180 to 180
    while (lon > 180.0) lon -= 360.0;
    while (lon < -180.0) lon += 360.0;

    if (!qFuzzyCompare(m_longitude, lon)) {
        m_longitude = lon;
        emit longitudeChanged();
        emit cameraChanged();
    }
}

void MapCamera::setZoom(double z) {
    z = std::clamp(z, 1.0, 19.0);
    if (!qFuzzyCompare(m_zoom, z)) {
        m_zoom = z;
        emit zoomChanged();
        emit cameraChanged();
    }
}

void MapCamera::setBearing(double b) {
    // Normalize to 0-360
    while (b >= 360.0) b -= 360.0;
    while (b < 0.0) b += 360.0;

    if (!qFuzzyCompare(m_bearing, b)) {
        m_bearing = b;
        emit bearingChanged();
        emit cameraChanged();
    }
}

void MapCamera::setTilt(double t) {
    t = std::clamp(t, 0.0, 60.0);
    if (!qFuzzyCompare(m_tilt, t)) {
        m_tilt = t;
        emit tiltChanged();
        emit cameraChanged();
    }
}

void MapCamera::setPosition(double lat, double lon, double zoom, double bearing, double tilt) {
    bool changed = false;

    lat = std::clamp(lat, -85.0, 85.0);
    while (lon > 180.0) lon -= 360.0;
    while (lon < -180.0) lon += 360.0;
    zoom = std::clamp(zoom, 1.0, 19.0);
    while (bearing >= 360.0) bearing -= 360.0;
    while (bearing < 0.0) bearing += 360.0;
    tilt = std::clamp(tilt, 0.0, 60.0);

    if (!qFuzzyCompare(m_latitude, lat)) {
        m_latitude = lat;
        emit latitudeChanged();
        changed = true;
    }
    if (!qFuzzyCompare(m_longitude, lon)) {
        m_longitude = lon;
        emit longitudeChanged();
        changed = true;
    }
    if (!qFuzzyCompare(m_zoom, zoom)) {
        m_zoom = zoom;
        emit zoomChanged();
        changed = true;
    }
    if (!qFuzzyCompare(m_bearing, bearing)) {
        m_bearing = bearing;
        emit bearingChanged();
        changed = true;
    }
    if (!qFuzzyCompare(m_tilt, tilt)) {
        m_tilt = tilt;
        emit tiltChanged();
        changed = true;
    }

    if (changed) {
        updateMovementSpeed();
        emit cameraChanged();
    }
}

QPointF MapCamera::geoToScreen(double lat, double lon, double viewWidth, double viewHeight) const {
    // Web Mercator projection
    double scale = std::pow(2.0, m_zoom) * TILE_SIZE;

    // Convert geo coordinates to pixel coordinates
    double x = (lon + 180.0) / 360.0 * scale;
    double latRad = lat * M_PI / 180.0;
    double y = (1.0 - std::log(std::tan(latRad) + 1.0 / std::cos(latRad)) / M_PI) / 2.0 * scale;

    // Convert center to pixel coordinates
    double centerX = (m_longitude + 180.0) / 360.0 * scale;
    double centerLatRad = m_latitude * M_PI / 180.0;
    double centerY = (1.0 - std::log(std::tan(centerLatRad) + 1.0 / std::cos(centerLatRad)) / M_PI) / 2.0 * scale;

    // Relative to center
    double screenX = (x - centerX) + viewWidth / 2.0;
    double screenY = (y - centerY) + viewHeight / 2.0;

    return QPointF(screenX, screenY);
}

QPointF MapCamera::screenToGeo(double x, double y, double viewWidth, double viewHeight) const {
    double scale = std::pow(2.0, m_zoom) * TILE_SIZE;

    // Convert center to pixel coordinates
    double centerX = (m_longitude + 180.0) / 360.0 * scale;
    double centerLatRad = m_latitude * M_PI / 180.0;
    double centerY = (1.0 - std::log(std::tan(centerLatRad) + 1.0 / std::cos(centerLatRad)) / M_PI) / 2.0 * scale;

    // Convert screen position to world pixel coordinates
    double worldX = centerX + (x - viewWidth / 2.0);
    double worldY = centerY + (y - viewHeight / 2.0);

    // Convert to geo coordinates
    double lon = worldX / scale * 360.0 - 180.0;
    double n = M_PI - 2.0 * M_PI * worldY / scale;
    double lat = 180.0 / M_PI * std::atan(0.5 * (std::exp(n) - std::exp(-n)));

    return QPointF(lat, lon);
}

int MapCamera::tileX() const {
    return static_cast<int>(std::floor((m_longitude + 180.0) / 360.0 * std::pow(2.0, zoomLevel())));
}

int MapCamera::tileY() const {
    double latRad = m_latitude * M_PI / 180.0;
    return static_cast<int>(std::floor((1.0 - std::log(std::tan(latRad) + 1.0 / std::cos(latRad)) / M_PI) / 2.0 * std::pow(2.0, zoomLevel())));
}

int MapCamera::zoomLevel() const {
    return static_cast<int>(std::floor(m_zoom));
}

MapCamera::TileRange MapCamera::visibleTileRange(double viewWidth, double viewHeight) const {
    int z = zoomLevel();
    int maxTile = (1 << z) - 1;

    // Get corners of viewport in geo coordinates
    QPointF topLeft = screenToGeo(0, 0, viewWidth, viewHeight);
    QPointF bottomRight = screenToGeo(viewWidth, viewHeight, viewWidth, viewHeight);

    // Convert to tile coordinates
    auto geoToTile = [z](double lat, double lon) -> std::pair<int, int> {
        int x = static_cast<int>(std::floor((lon + 180.0) / 360.0 * std::pow(2.0, z)));
        double latRad = lat * M_PI / 180.0;
        int y = static_cast<int>(std::floor((1.0 - std::log(std::tan(latRad) + 1.0 / std::cos(latRad)) / M_PI) / 2.0 * std::pow(2.0, z)));
        return {x, y};
    };

    auto [tlX, tlY] = geoToTile(topLeft.x(), topLeft.y());
    auto [brX, brY] = geoToTile(bottomRight.x(), bottomRight.y());

    TileRange range;
    range.zoom = z;
    range.minX = std::clamp(std::min(tlX, brX) - 1, 0, maxTile);
    range.maxX = std::clamp(std::max(tlX, brX) + 1, 0, maxTile);
    range.minY = std::clamp(std::min(tlY, brY) - 1, 0, maxTile);
    range.maxY = std::clamp(std::max(tlY, brY) + 1, 0, maxTile);

    return range;
}

void MapCamera::updateMovementSpeed() {
    qint64 elapsed = m_speedTimer.restart();

    if (elapsed <= 0) {
        return;
    }

    // Calculate distance moved in degrees (approximate)
    double latDiff = m_latitude - m_prevLatitude;
    double lonDiff = m_longitude - m_prevLongitude;
    double zoomDiff = std::abs(m_zoom - m_prevZoom);

    // Handle longitude wrap-around
    if (lonDiff > 180.0) lonDiff -= 360.0;
    if (lonDiff < -180.0) lonDiff += 360.0;

    // Calculate screen-space movement (zoom affects perceived speed)
    // Higher zoom = smaller area visible = position changes feel faster
    double zoomScale = std::pow(2.0, m_zoom);
    double positionSpeed = std::sqrt(latDiff * latDiff + lonDiff * lonDiff) * zoomScale;

    // Zoom changes also contribute to "movement feel"
    double zoomSpeed = zoomDiff * 2.0;

    // Combined speed normalized to per-second
    double speed = (positionSpeed + zoomSpeed) / (elapsed / 1000.0);

    // Smooth the speed value (exponential moving average)
    double smoothingFactor = 0.3;
    double newSpeed = m_movementSpeed * (1.0 - smoothingFactor) + speed * smoothingFactor;

    // Update previous values
    m_prevLatitude = m_latitude;
    m_prevLongitude = m_longitude;
    m_prevZoom = m_zoom;

    if (!qFuzzyCompare(m_movementSpeed, newSpeed)) {
        m_movementSpeed = newSpeed;
        emit movementSpeedChanged();
    }
}
