#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QColor>
#include "geooverlay.h"

class GeoJsonParser;
class CityBoundaryFetcher;

// Helper to convert JSON coordinates to QPolygonF
QVector<QPolygonF> parseNominatimCoordinates(const QJsonArray& coordinates, const QString& geometryType);

class GeoOverlayModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum GeoOverlayRoles {
        IdRole = Qt::UserRole + 1,
        CodeRole,
        NameRole,
        ParentNameRole,
        TypeRole,
        TypeStringRole,
        FillColorRole,
        BorderColorRole,
        BorderWidthRole,
        MarkerRadiusRole,
        ShowLabelRole,
        LatitudeRole,
        LongitudeRole,
        StartTimeRole,
        FadeInDurationRole,
        EndTimeRole,
        FadeOutDurationRole,
        // Keyframe-related roles
        KeyframeCountRole,
        CurrentExtrusionRole,
        CurrentFillColorRole,
        CurrentBorderColorRole,
        CurrentOpacityRole,
        CurrentScaleRole,
        PolygonsRole
    };

    explicit GeoOverlayModel(QObject* parent = nullptr);

    void setGeoJsonParser(GeoJsonParser* parser) { m_geoJson = parser; }
    void setCityBoundaryFetcher(CityBoundaryFetcher* fetcher);

    // QAbstractListModel overrides
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    // Add overlays
    Q_INVOKABLE void addCountry(const QString& code, const QString& name, double startTime = 0.0);
    Q_INVOKABLE void addRegion(const QString& code, const QString& name,
                                const QString& countryName, double startTime = 0.0);
    Q_INVOKABLE void addCity(const QString& name, const QString& countryName,
                              double lat, double lon, double startTime = 0.0);

    // Generic operations
    Q_INVOKABLE void removeOverlay(int index);
    Q_INVOKABLE void updateOverlay(int index, const QVariantMap& data);
    Q_INVOKABLE QVariantMap getOverlay(int index) const;
    Q_INVOKABLE void clear();

    // Move overlay in list (for reordering)
    Q_INVOKABLE void moveOverlay(int from, int to);

    // Timing
    Q_INVOKABLE void setOverlayTiming(int index, double startTime, double fadeIn,
                                       double endTime, double fadeOut);
    Q_INVOKABLE void setOverlayColors(int index, const QColor& fillColor,
                                       const QColor& borderColor, double borderWidth);

    // Get opacity for an overlay at a given time
    Q_INVOKABLE double overlayOpacityAtTime(int index, double timeMs, double totalDuration) const;

    // Get all visible overlays at a given time with their opacities
    QVector<QPair<const GeoOverlay*, double>> visibleOverlaysAtTime(double timeMs, double totalDuration) const;

    // Keyframe management
    Q_INVOKABLE int addKeyframe(int overlayIndex, double timeMs);
    Q_INVOKABLE void updateKeyframe(int overlayIndex, int keyframeIndex, const QVariantMap& data);
    Q_INVOKABLE void removeKeyframe(int overlayIndex, int keyframeIndex);
    Q_INVOKABLE void moveKeyframe(int overlayIndex, int keyframeIndex, double newTimeMs);
    Q_INVOKABLE QVariantMap getKeyframe(int overlayIndex, int keyframeIndex) const;
    Q_INVOKABLE int keyframeCount(int overlayIndex) const;
    Q_INVOKABLE QVariantList getAllKeyframes(int overlayIndex) const;
    Q_INVOKABLE QVariantMap propertiesAtTime(int overlayIndex, double timeMs) const;

    // Set current time for animated properties
    void setCurrentTime(double timeMs);
    double currentTime() const { return m_currentTime; }

    int count() const { return m_overlays.size(); }
    const QVector<GeoOverlay>& overlays() const { return m_overlays; }

    // Serialization
    QJsonArray toJson() const;
    void fromJson(const QJsonArray& array);

signals:
    void countChanged();
    void overlayModified(int index);
    void dataModified();
    void keyframeAdded(int overlayIndex, int keyframeIndex);
    void keyframeRemoved(int overlayIndex, int keyframeIndex);
    void keyframeModified(int overlayIndex, int keyframeIndex);
    void currentTimeChanged();

private slots:
    void onBoundaryReady(const QString& cityName, const QJsonArray& coordinates, const QString& geometryType);
    void onBoundaryFetchFailed(const QString& cityName, const QString& error);

private:
    void loadGeometryForOverlay(GeoOverlay& overlay);
    QString generateId(GeoOverlayType type, const QString& name);
    void sortKeyframes(int overlayIndex);
    void loadCityBoundaryFromCache(GeoOverlay& overlay);  // Load boundary from stored JSON

    QVector<GeoOverlay> m_overlays;
    GeoJsonParser* m_geoJson = nullptr;
    CityBoundaryFetcher* m_boundaryFetcher = nullptr;
    double m_currentTime = 0.0;
};
