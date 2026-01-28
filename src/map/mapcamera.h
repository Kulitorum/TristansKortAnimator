#pragma once

#include <QObject>
#include <QPointF>
#include <QElapsedTimer>

class MapCamera : public QObject {
    Q_OBJECT

    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)
    Q_PROPERTY(double zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(double bearing READ bearing WRITE setBearing NOTIFY bearingChanged)
    Q_PROPERTY(double tilt READ tilt WRITE setTilt NOTIFY tiltChanged)
    Q_PROPERTY(double movementSpeed READ movementSpeed NOTIFY movementSpeedChanged)

public:
    explicit MapCamera(QObject* parent = nullptr);

    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    double zoom() const { return m_zoom; }
    double bearing() const { return m_bearing; }
    double tilt() const { return m_tilt; }
    double movementSpeed() const { return m_movementSpeed; }

    void setLatitude(double lat);
    void setLongitude(double lon);
    void setZoom(double z);
    void setBearing(double b);
    void setTilt(double t);

    // Set all at once without intermediate signals
    void setPosition(double lat, double lon, double zoom, double bearing = 0.0, double tilt = 0.0);

    // Coordinate conversions (requires viewport size)
    Q_INVOKABLE QPointF geoToScreen(double lat, double lon, double viewWidth, double viewHeight) const;
    Q_INVOKABLE QPointF screenToGeo(double x, double y, double viewWidth, double viewHeight) const;

    // Tile math
    Q_INVOKABLE int tileX() const;
    Q_INVOKABLE int tileY() const;
    Q_INVOKABLE int zoomLevel() const;

    // Get visible tile range for current viewport
    struct TileRange {
        int minX, maxX, minY, maxY, zoom;
    };
    TileRange visibleTileRange(double viewWidth, double viewHeight) const;
    TileRange visibleTileRangeAtZoom(double viewWidth, double viewHeight, int zoomLevel) const;

signals:
    void latitudeChanged();
    void longitudeChanged();
    void zoomChanged();
    void bearingChanged();
    void tiltChanged();
    void cameraChanged();
    void movementSpeedChanged();

private:
    void updateMovementSpeed();

    double m_latitude = 52.5;    // Default: center of Europe
    double m_longitude = 10.0;
    double m_zoom = 5.0;
    double m_bearing = 0.0;
    double m_tilt = 0.0;

    // Speed tracking for label fading
    double m_prevLatitude = 52.5;
    double m_prevLongitude = 10.0;
    double m_prevZoom = 5.0;
    double m_movementSpeed = 0.0;
    QElapsedTimer m_speedTimer;

    static constexpr double TILE_SIZE = 256.0;
};
