#include "countrygeometry.h"
#include <QtMath>
#include <QByteArray>

CountryGeometry::CountryGeometry(QQuick3DObject* parent)
    : QQuick3DGeometry(parent)
{
}

void CountryGeometry::setExtrusionHeight(float height) {
    if (qFuzzyCompare(m_extrusionHeight, height)) return;
    m_extrusionHeight = qBound(0.0f, height, 100.0f);
    emit extrusionHeightChanged();
    updateGeometry();
}

void CountryGeometry::setGlobeRadius(float radius) {
    if (qFuzzyCompare(m_globeRadius, radius)) return;
    m_globeRadius = radius;
    emit globeRadiusChanged();
    updateGeometry();
}

void CountryGeometry::setPolygonData(const QVariantList& data) {
    m_polygonData = data;

    // Convert QVariantList to QVector<QPolygonF>
    m_polygons.clear();
    for (const QVariant& polyVar : data) {
        QVariantList points = polyVar.toList();
        QPolygonF polygon;
        for (const QVariant& pointVar : points) {
            QVariantList coords = pointVar.toList();
            if (coords.size() >= 2) {
                // Format: [lat, lon]
                polygon.append(QPointF(coords[0].toDouble(), coords[1].toDouble()));
            }
        }
        if (polygon.size() >= 3) {
            m_polygons.append(polygon);
        }
    }

    emit polygonDataChanged();
    updateGeometry();
}

void CountryGeometry::setPolygons(const QVector<QPolygonF>& polygons) {
    m_polygons = polygons;
    updateGeometry();
}

QVector3D CountryGeometry::latLonToPosition(float lat, float lon, float radius) const {
    float latRad = qDegreesToRadians(lat);
    float lonRad = qDegreesToRadians(lon);

    float x = radius * qCos(latRad) * qSin(lonRad);
    float y = radius * qSin(latRad);
    float z = radius * qCos(latRad) * qCos(lonRad);

    return QVector3D(x, y, z);
}

QVector<int> CountryGeometry::triangulatePolygon(const QVector<QVector3D>& vertices) const {
    // Simple fan triangulation from centroid - works well for convex polygons
    QVector<int> indices;

    if (vertices.size() < 3) return indices;

    // For simple polygons, use fan triangulation from first vertex
    for (int i = 1; i < vertices.size() - 1; i++) {
        indices.append(0);
        indices.append(i);
        indices.append(i + 1);
    }

    return indices;
}

void CountryGeometry::updateGeometry() {
    clear();

    if (m_polygons.isEmpty()) {
        update();
        return;
    }

    // Calculate actual extrusion radius
    float extrusionScale = m_extrusionHeight / 100.0f * 10.0f;  // Max 10 units above surface
    float topRadius = m_globeRadius + extrusionScale;
    float bottomRadius = m_globeRadius;

    // Collect all vertices and indices
    QVector<float> vertexData;
    QVector<uint32_t> indexData;

    uint32_t vertexOffset = 0;

    for (const QPolygonF& polygon : m_polygons) {
        if (polygon.size() < 3) continue;

        // Simplify polygon if too many points (for performance)
        QPolygonF simplifiedPoly = polygon;
        int maxPoints = 100;
        if (simplifiedPoly.size() > maxPoints) {
            QPolygonF newPoly;
            int step = simplifiedPoly.size() / maxPoints;
            for (int i = 0; i < simplifiedPoly.size(); i += step) {
                newPoly.append(simplifiedPoly[i]);
            }
            simplifiedPoly = newPoly;
        }

        int n = simplifiedPoly.size();

        // Generate 3D positions for top and bottom
        QVector<QVector3D> topVerts, bottomVerts;
        for (int i = 0; i < n; i++) {
            // Note: QPolygonF stores (lat, lon) in our convention
            float lat = simplifiedPoly[i].x();
            float lon = simplifiedPoly[i].y();

            topVerts.append(latLonToPosition(lat, lon, topRadius));
            bottomVerts.append(latLonToPosition(lat, lon, bottomRadius));
        }

        // === TOP FACE ===
        // Add top face vertices with upward-pointing normals
        uint32_t topStart = vertexOffset;
        for (int i = 0; i < n; i++) {
            QVector3D pos = topVerts[i];
            QVector3D normal = pos.normalized();  // Normal points outward from sphere center

            // Position
            vertexData.append(pos.x());
            vertexData.append(pos.y());
            vertexData.append(pos.z());
            // Normal
            vertexData.append(normal.x());
            vertexData.append(normal.y());
            vertexData.append(normal.z());
            // UV (simplified)
            vertexData.append(0.5f);
            vertexData.append(0.5f);

            vertexOffset++;
        }

        // Triangulate top face
        QVector<int> topIndices = triangulatePolygon(topVerts);
        for (int idx : topIndices) {
            indexData.append(topStart + idx);
        }

        // === BOTTOM FACE === (only if extruded)
        if (m_extrusionHeight > 0.1f) {
            uint32_t bottomStart = vertexOffset;
            for (int i = 0; i < n; i++) {
                QVector3D pos = bottomVerts[i];
                QVector3D normal = -pos.normalized();  // Normal points inward

                vertexData.append(pos.x());
                vertexData.append(pos.y());
                vertexData.append(pos.z());
                vertexData.append(normal.x());
                vertexData.append(normal.y());
                vertexData.append(normal.z());
                vertexData.append(0.5f);
                vertexData.append(0.5f);

                vertexOffset++;
            }

            // Triangulate bottom face (reversed winding)
            QVector<int> bottomIndices = triangulatePolygon(bottomVerts);
            for (int i = bottomIndices.size() - 1; i >= 0; i--) {
                indexData.append(bottomStart + bottomIndices[i]);
            }

            // === SIDE WALLS ===
            for (int i = 0; i < n; i++) {
                int next = (i + 1) % n;

                QVector3D t0 = topVerts[i];
                QVector3D t1 = topVerts[next];
                QVector3D b0 = bottomVerts[i];
                QVector3D b1 = bottomVerts[next];

                // Calculate side normal (perpendicular to edge, pointing outward)
                QVector3D edge = t1 - t0;
                QVector3D outward = (t0 + t1).normalized();
                QVector3D sideNormal = QVector3D::crossProduct(edge, outward).normalized();

                uint32_t sideStart = vertexOffset;

                // Add 4 vertices for this side quad
                // Top-left
                vertexData.append(t0.x()); vertexData.append(t0.y()); vertexData.append(t0.z());
                vertexData.append(sideNormal.x()); vertexData.append(sideNormal.y()); vertexData.append(sideNormal.z());
                vertexData.append(0.0f); vertexData.append(1.0f);
                vertexOffset++;

                // Top-right
                vertexData.append(t1.x()); vertexData.append(t1.y()); vertexData.append(t1.z());
                vertexData.append(sideNormal.x()); vertexData.append(sideNormal.y()); vertexData.append(sideNormal.z());
                vertexData.append(1.0f); vertexData.append(1.0f);
                vertexOffset++;

                // Bottom-left
                vertexData.append(b0.x()); vertexData.append(b0.y()); vertexData.append(b0.z());
                vertexData.append(sideNormal.x()); vertexData.append(sideNormal.y()); vertexData.append(sideNormal.z());
                vertexData.append(0.0f); vertexData.append(0.0f);
                vertexOffset++;

                // Bottom-right
                vertexData.append(b1.x()); vertexData.append(b1.y()); vertexData.append(b1.z());
                vertexData.append(sideNormal.x()); vertexData.append(sideNormal.y()); vertexData.append(sideNormal.z());
                vertexData.append(1.0f); vertexData.append(0.0f);
                vertexOffset++;

                // Two triangles for this quad
                indexData.append(sideStart + 0);
                indexData.append(sideStart + 2);
                indexData.append(sideStart + 1);

                indexData.append(sideStart + 1);
                indexData.append(sideStart + 2);
                indexData.append(sideStart + 3);
            }
        }
    }

    if (vertexData.isEmpty()) {
        update();
        return;
    }

    // Convert to byte arrays
    const int stride = 8 * sizeof(float);  // pos(3) + normal(3) + uv(2)
    QByteArray vData(reinterpret_cast<const char*>(vertexData.data()),
                     vertexData.size() * sizeof(float));
    QByteArray iData(reinterpret_cast<const char*>(indexData.data()),
                     indexData.size() * sizeof(uint32_t));

    setStride(stride);
    setVertexData(vData);
    setIndexData(iData);
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);

    addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,
                 0, QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::NormalSemantic,
                 3 * sizeof(float), QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::TexCoordSemantic,
                 6 * sizeof(float), QQuick3DGeometry::Attribute::F32Type);
    addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,
                 0, QQuick3DGeometry::Attribute::U32Type);

    update();
}
