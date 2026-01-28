#include "globegeometry.h"
#include <QtMath>
#include <QByteArray>

GlobeGeometry::GlobeGeometry(QQuick3DObject* parent)
    : QQuick3DGeometry(parent)
{
    updateGeometry();
}

void GlobeGeometry::setSegments(int segments) {
    if (m_segments == segments) return;
    m_segments = qMax(8, segments);
    emit segmentsChanged();
    updateGeometry();
}

void GlobeGeometry::setRadius(float radius) {
    if (qFuzzyCompare(m_radius, radius)) return;
    m_radius = radius;
    emit radiusChanged();
    updateGeometry();
}

QVector3D GlobeGeometry::latLonToPosition(float lat, float lon) const {
    // Convert lat/lon (degrees) to 3D cartesian coordinates
    float latRad = qDegreesToRadians(lat);
    float lonRad = qDegreesToRadians(lon);

    float x = m_radius * qCos(latRad) * qSin(lonRad);
    float y = m_radius * qSin(latRad);
    float z = m_radius * qCos(latRad) * qCos(lonRad);

    return QVector3D(x, y, z);
}

QVector2D GlobeGeometry::latLonToUV(float lat, float lon) const {
    // Web Mercator projection for UV mapping
    // U = (lon + 180) / 360
    // V = 0.5 - ln(tan(lat) + sec(lat)) / (2*PI)

    float u = (lon + 180.0f) / 360.0f;

    // Clamp latitude to avoid infinite values at poles
    float clampedLat = qBound(-85.0f, lat, 85.0f);
    float latRad = qDegreesToRadians(clampedLat);

    float v = 0.5f - qLn(qTan(latRad) + 1.0f / qCos(latRad)) / (2.0f * M_PI);

    return QVector2D(u, v);
}

void GlobeGeometry::updateGeometry() {
    clear();

    // Generate UV sphere with latitude/longitude lines
    int latSegments = m_segments;
    int lonSegments = m_segments * 2;

    // Calculate vertex count
    int vertexCount = (latSegments + 1) * (lonSegments + 1);
    int indexCount = latSegments * lonSegments * 6;

    // Vertex data: position (3 floats), normal (3 floats), UV (2 floats)
    const int stride = (3 + 3 + 2) * sizeof(float);
    QByteArray vertexData;
    vertexData.resize(vertexCount * stride);
    float* vData = reinterpret_cast<float*>(vertexData.data());

    // Index data
    QByteArray indexData;
    indexData.resize(indexCount * sizeof(uint32_t));
    uint32_t* iData = reinterpret_cast<uint32_t*>(indexData.data());

    // Generate vertices
    int vIdx = 0;
    for (int lat = 0; lat <= latSegments; lat++) {
        float latAngle = -90.0f + (180.0f * lat / latSegments);

        for (int lon = 0; lon <= lonSegments; lon++) {
            float lonAngle = -180.0f + (360.0f * lon / lonSegments);

            QVector3D pos = latLonToPosition(latAngle, lonAngle);
            QVector3D normal = pos.normalized();
            QVector2D uv = latLonToUV(latAngle, lonAngle);

            // Position
            vData[vIdx++] = pos.x();
            vData[vIdx++] = pos.y();
            vData[vIdx++] = pos.z();

            // Normal
            vData[vIdx++] = normal.x();
            vData[vIdx++] = normal.y();
            vData[vIdx++] = normal.z();

            // UV
            vData[vIdx++] = uv.x();
            vData[vIdx++] = uv.y();
        }
    }

    // Generate indices (two triangles per quad) - reverse winding for correct face culling
    int iIdx = 0;
    for (int lat = 0; lat < latSegments; lat++) {
        for (int lon = 0; lon < lonSegments; lon++) {
            int current = lat * (lonSegments + 1) + lon;
            int next = current + lonSegments + 1;

            // First triangle (counter-clockwise when viewed from outside)
            iData[iIdx++] = current;
            iData[iIdx++] = current + 1;
            iData[iIdx++] = next;

            // Second triangle
            iData[iIdx++] = current + 1;
            iData[iIdx++] = next + 1;
            iData[iIdx++] = next;
        }
    }

    // Set up geometry
    setStride(stride);
    setVertexData(vertexData);
    setIndexData(indexData);
    setPrimitiveType(QQuick3DGeometry::PrimitiveType::Triangles);
    setBounds(QVector3D(-m_radius, -m_radius, -m_radius),
              QVector3D(m_radius, m_radius, m_radius));

    // Define vertex attributes
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
