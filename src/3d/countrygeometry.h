#pragma once

#include <QQuick3DGeometry>
#include <QVector3D>
#include <QPolygonF>
#include <QColor>

class CountryGeometry : public QQuick3DGeometry {
    Q_OBJECT

    Q_PROPERTY(float extrusionHeight READ extrusionHeight WRITE setExtrusionHeight NOTIFY extrusionHeightChanged)
    Q_PROPERTY(float globeRadius READ globeRadius WRITE setGlobeRadius NOTIFY globeRadiusChanged)
    Q_PROPERTY(QVariantList polygonData READ polygonData WRITE setPolygonData NOTIFY polygonDataChanged)

public:
    explicit CountryGeometry(QQuick3DObject* parent = nullptr);

    float extrusionHeight() const { return m_extrusionHeight; }
    void setExtrusionHeight(float height);

    float globeRadius() const { return m_globeRadius; }
    void setGlobeRadius(float radius);

    QVariantList polygonData() const { return m_polygonData; }
    void setPolygonData(const QVariantList& data);

    // Set polygons from C++ (for use by GeoOverlayModel)
    void setPolygons(const QVector<QPolygonF>& polygons);

signals:
    void extrusionHeightChanged();
    void globeRadiusChanged();
    void polygonDataChanged();

private:
    void updateGeometry();

    // Convert lat/lon to 3D position on sphere at given radius
    QVector3D latLonToPosition(float lat, float lon, float radius) const;

    // Triangulate a polygon (simple ear-clipping for convex-ish polygons)
    QVector<int> triangulatePolygon(const QVector<QVector3D>& vertices) const;

    float m_extrusionHeight = 0.0f;  // Height above sphere surface (0-100 scale)
    float m_globeRadius = 100.0f;
    QVariantList m_polygonData;
    QVector<QPolygonF> m_polygons;
};
