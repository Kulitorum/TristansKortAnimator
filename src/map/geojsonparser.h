#pragma once

#include <QObject>
#include <QPolygonF>
#include <QVariantMap>
#include <QVector>
#include <QJsonObject>

struct GeoFeature {
    QString type;       // "country", "region", "city"
    QString name;
    QString code;       // ISO code
    QVector<QPolygonF> polygons;  // MultiPolygon support
    QPointF centroid;
    QVariantMap properties;
};

class GeoJsonParser : public QObject {
    Q_OBJECT

    Q_PROPERTY(int featureCount READ featureCount NOTIFY loaded)
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY loaded)

public:
    explicit GeoJsonParser(QObject* parent = nullptr);

    Q_INVOKABLE bool loadFromResource(const QString& resourcePath);
    Q_INVOKABLE bool appendFromResource(const QString& resourcePath);
    Q_INVOKABLE bool loadFromFile(const QString& filePath);
    Q_INVOKABLE void loadBuiltInCities();

    const QVector<GeoFeature>& features() const { return m_features; }
    int featureCount() const { return m_features.size(); }
    bool isLoaded() const { return !m_features.isEmpty(); }

    Q_INVOKABLE QVariantList countryList() const;
    Q_INVOKABLE QVariantList regionList(const QString& countryCode) const;
    Q_INVOKABLE QVariantList cityList() const;

    // Find feature by code or name
    const GeoFeature* findByCode(const QString& code) const;
    const GeoFeature* findByName(const QString& name) const;

    // Get polygons for a feature (used by GeoOverlayModel)
    QVector<QPolygonF> getPolygonsForFeature(const QString& code, const QString& name) const;

    // Get all regions for a country
    Q_INVOKABLE QVariantList regionsForCountry(const QString& countryName) const;

    // Get all cities with their coordinates
    Q_INVOKABLE QVariantList allCities() const;

signals:
    void loaded();
    void loadError(const QString& error);

private:
    void parseFeatureCollection(const QJsonObject& root);
    void parseFeature(const QJsonObject& feature);
    QPolygonF parsePolygon(const QJsonArray& coords);
    QPointF calculateCentroid(const QVector<QPolygonF>& polygons);

    QVector<GeoFeature> m_features;
};
