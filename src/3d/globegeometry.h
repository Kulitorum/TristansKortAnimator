#pragma once

#include <QQuick3DGeometry>
#include <QVector3D>
#include <QVector2D>

class GlobeGeometry : public QQuick3DGeometry {
    Q_OBJECT

    Q_PROPERTY(int segments READ segments WRITE setSegments NOTIFY segmentsChanged)
    Q_PROPERTY(float radius READ radius WRITE setRadius NOTIFY radiusChanged)

public:
    explicit GlobeGeometry(QQuick3DObject* parent = nullptr);

    int segments() const { return m_segments; }
    void setSegments(int segments);

    float radius() const { return m_radius; }
    void setRadius(float radius);

signals:
    void segmentsChanged();
    void radiusChanged();

private:
    void updateGeometry();

    // Convert lat/lon to 3D position on sphere
    QVector3D latLonToPosition(float lat, float lon) const;

    // Convert lat/lon to UV coordinates for Web Mercator texture
    QVector2D latLonToUV(float lat, float lon) const;

    int m_segments = 64;
    float m_radius = 100.0f;
};
