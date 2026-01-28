#pragma once

#include <QObject>
#include <QVector3D>
#include <QtQml/qqml.h>

class GlobeCamera : public QObject {
    Q_OBJECT

    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)
    Q_PROPERTY(double altitude READ altitude WRITE setAltitude NOTIFY altitudeChanged)
    Q_PROPERTY(double bearing READ bearing WRITE setBearing NOTIFY bearingChanged)
    Q_PROPERTY(double tilt READ tilt WRITE setTilt NOTIFY tiltChanged)
    Q_PROPERTY(double globeRadius READ globeRadius WRITE setGlobeRadius NOTIFY globeRadiusChanged)

    Q_PROPERTY(QVector3D position READ position NOTIFY positionChanged)
    Q_PROPERTY(QVector3D lookAt READ lookAt NOTIFY lookAtChanged)
    Q_PROPERTY(QVector3D upVector READ upVector NOTIFY upVectorChanged)

public:
    explicit GlobeCamera(QObject* parent = nullptr);

    double latitude() const { return m_latitude; }
    void setLatitude(double lat);

    double longitude() const { return m_longitude; }
    void setLongitude(double lon);

    double altitude() const { return m_altitude; }
    void setAltitude(double alt);

    double bearing() const { return m_bearing; }
    void setBearing(double bearing);

    double tilt() const { return m_tilt; }
    void setTilt(double tilt);

    double globeRadius() const { return m_globeRadius; }
    void setGlobeRadius(double radius);

    QVector3D position() const { return m_position; }
    QVector3D lookAt() const { return m_lookAt; }
    QVector3D upVector() const { return m_upVector; }

    // Convert zoom level to altitude
    Q_INVOKABLE double zoomToAltitude(double zoom) const;
    Q_INVOKABLE double altitudeToZoom(double altitude) const;

signals:
    void latitudeChanged();
    void longitudeChanged();
    void altitudeChanged();
    void bearingChanged();
    void tiltChanged();
    void globeRadiusChanged();
    void positionChanged();
    void lookAtChanged();
    void upVectorChanged();
    void cameraChanged();

private:
    void updatePosition();

    // Convert lat/lon to 3D position
    QVector3D latLonToPosition(double lat, double lon, double radius) const;

    double m_latitude = 0.0;
    double m_longitude = 0.0;
    double m_altitude = 300.0;  // Distance from globe surface
    double m_bearing = 0.0;
    double m_tilt = 0.0;  // 0 = looking at center, 90 = looking at horizon
    double m_globeRadius = 100.0;

    QVector3D m_position;
    QVector3D m_lookAt;
    QVector3D m_upVector;
};
