#pragma once

#include <QQuickPaintedItem>
#include <QImage>
#include <QHash>
#include <QColor>

class TileProvider;
class TileCache;
class MapCamera;
class GeoJsonParser;
class OverlayManager;
class RegionTrackModel;
class GeoOverlayModel;
class FrameBuffer;
struct GeoFeature;

class MapRenderer : public QQuickPaintedItem {
    Q_OBJECT

    Q_PROPERTY(MapCamera* camera READ camera WRITE setCamera NOTIFY cameraChanged)
    Q_PROPERTY(bool showCountryLabels READ showCountryLabels WRITE setShowCountryLabels NOTIFY showCountryLabelsChanged)
    Q_PROPERTY(bool showRegionLabels READ showRegionLabels WRITE setShowRegionLabels NOTIFY showRegionLabelsChanged)
    Q_PROPERTY(bool showCityLabels READ showCityLabels WRITE setShowCityLabels NOTIFY showCityLabelsChanged)
    Q_PROPERTY(double labelOpacity READ labelOpacity NOTIFY labelOpacityChanged)
    Q_PROPERTY(bool shadeNonHighlighted READ shadeNonHighlighted WRITE setShadeNonHighlighted NOTIFY shadeNonHighlightedChanged)
    Q_PROPERTY(double nonHighlightedOpacity READ nonHighlightedOpacity WRITE setNonHighlightedOpacity NOTIFY nonHighlightedOpacityChanged)
    Q_PROPERTY(double currentAnimationTime READ currentAnimationTime WRITE setCurrentAnimationTime NOTIFY currentAnimationTimeChanged)
    Q_PROPERTY(double totalDuration READ totalDuration WRITE setTotalDuration NOTIFY totalDurationChanged)
    Q_PROPERTY(bool useFrameBuffer READ useFrameBuffer WRITE setUseFrameBuffer NOTIFY useFrameBufferChanged)
    Q_PROPERTY(bool showCountryBorders READ showCountryBorders WRITE setShowCountryBorders NOTIFY showCountryBordersChanged)
    Q_PROPERTY(bool showCityMarkers READ showCityMarkers WRITE setShowCityMarkers NOTIFY showCityMarkersChanged)
    Q_PROPERTY(QString selectedFeatureCode READ selectedFeatureCode NOTIFY selectedFeatureChanged)
    Q_PROPERTY(QString selectedFeatureName READ selectedFeatureName NOTIFY selectedFeatureChanged)
    Q_PROPERTY(QString selectedFeatureType READ selectedFeatureType NOTIFY selectedFeatureChanged)

public:
    explicit MapRenderer(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;

    void setTileProvider(TileProvider* provider);
    void setTileCache(TileCache* cache);
    void setGeoJson(GeoJsonParser* geojson);
    void setOverlayManager(OverlayManager* overlays);
    void setRegionTrackModel(RegionTrackModel* regionTracks);
    void setGeoOverlayModel(GeoOverlayModel* geoOverlays);

    MapCamera* camera() const { return m_camera; }
    void setCamera(MapCamera* camera);

    bool showCountryLabels() const { return m_showCountryLabels; }
    void setShowCountryLabels(bool show);

    bool showRegionLabels() const { return m_showRegionLabels; }
    void setShowRegionLabels(bool show);

    bool showCityLabels() const { return m_showCityLabels; }
    void setShowCityLabels(bool show);

    double labelOpacity() const { return m_labelOpacity; }

    bool shadeNonHighlighted() const { return m_shadeNonHighlighted; }
    void setShadeNonHighlighted(bool shade);

    double nonHighlightedOpacity() const { return m_nonHighlightedOpacity; }
    void setNonHighlightedOpacity(double opacity);

    // Animation time for frame buffering
    double currentAnimationTime() const { return m_currentAnimationTime; }
    void setCurrentAnimationTime(double timeMs);

    double totalDuration() const { return m_totalDuration; }
    void setTotalDuration(double durationMs);

    // Frame buffer for caching rendered frames
    void setFrameBuffer(FrameBuffer* buffer);
    FrameBuffer* frameBuffer() const { return m_frameBuffer; }
    bool useFrameBuffer() const { return m_useFrameBuffer; }
    void setUseFrameBuffer(bool use);

    // Country/region highlighting
    Q_INVOKABLE void highlightRegion(const QString& regionCode, const QColor& fillColor, const QColor& borderColor = Qt::transparent);
    Q_INVOKABLE void clearHighlight(const QString& regionCode);
    Q_INVOKABLE void clearAllHighlights();

    // Interactive overlays
    bool showCountryBorders() const { return m_showCountryBorders; }
    void setShowCountryBorders(bool show);

    bool showCityMarkers() const { return m_showCityMarkers; }
    void setShowCityMarkers(bool show);

    // Hit testing and selection
    Q_INVOKABLE QString hitTestCountry(double screenX, double screenY);
    Q_INVOKABLE QString hitTestCity(double screenX, double screenY);
    Q_INVOKABLE void selectFeatureAt(double screenX, double screenY);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void toggleFeatureHighlight(const QString& code, const QColor& fillColor, const QColor& borderColor);
    Q_INVOKABLE void frameSelectedFeature();

    QString selectedFeatureCode() const { return m_selectedFeatureCode; }
    QString selectedFeatureName() const { return m_selectedFeatureName; }
    QString selectedFeatureType() const { return m_selectedFeatureType; }

    // Render to image for export
    QImage renderToImage(int width, int height);

signals:
    void cameraChanged();
    void showCountryLabelsChanged();
    void showRegionLabelsChanged();
    void showCityLabelsChanged();
    void labelOpacityChanged();
    void shadeNonHighlightedChanged();
    void nonHighlightedOpacityChanged();
    void currentAnimationTimeChanged();
    void totalDurationChanged();
    void useFrameBufferChanged();
    void renderingComplete();
    void showCountryBordersChanged();
    void showCityMarkersChanged();
    void selectedFeatureChanged();
    void featureClicked(const QString& code, const QString& name, const QString& type);

public slots:
    void onTileReady(int x, int y, int zoom, const QImage& image);
    void requestUpdate();

private slots:
    void onMovementSpeedChanged();

private:
    void renderTiles(QPainter* painter);
    bool tryRenderFallbackTile(QPainter* painter, int tx, int ty, int targetZoom,
                               double screenX, double screenY, double tileSize, int source);
    void renderHighlights(QPainter* painter);
    void renderRegionTracks(QPainter* painter, double currentTime, double totalDuration);
    void renderGeoOverlays(QPainter* painter, double currentTime, double totalDuration);
    void renderCountryBorders(QPainter* painter);
    void renderCityMarkers(QPainter* painter);
    void renderOverlays(QPainter* painter, double currentTime);
    void renderLabels(QPainter* painter);
    void applyTransforms(QPainter* painter);
    void resetTransforms(QPainter* painter);
    bool pointInPolygon(const QPolygonF& polygon, double lat, double lon) const;

    TileProvider* m_tileProvider = nullptr;
    TileCache* m_tileCache = nullptr;
    MapCamera* m_camera = nullptr;
    GeoJsonParser* m_geojson = nullptr;
    OverlayManager* m_overlays = nullptr;
    RegionTrackModel* m_regionTracks = nullptr;
    GeoOverlayModel* m_geoOverlays = nullptr;
    FrameBuffer* m_frameBuffer = nullptr;

    bool m_showCountryLabels = false;
    bool m_showRegionLabels = false;
    bool m_showCityLabels = false;
    double m_labelOpacity = 1.0;
    bool m_shadeNonHighlighted = false;
    double m_nonHighlightedOpacity = 0.3;
    double m_currentAnimationTime = 0.0;
    double m_totalDuration = 0.0;
    bool m_useFrameBuffer = true;
    bool m_showCountryBorders = false;
    bool m_showCityMarkers = false;
    QString m_selectedFeatureCode;
    QString m_selectedFeatureName;
    QString m_selectedFeatureType;

    // Speed thresholds for label fading
    static constexpr double SPEED_FADE_START = 5.0;   // Start fading at this speed
    static constexpr double SPEED_FADE_END = 50.0;    // Fully faded at this speed

    struct HighlightStyle {
        QColor fillColor;
        QColor borderColor;
    };
    QHash<QString, HighlightStyle> m_highlights;

    static constexpr int TILE_SIZE = 256;
};
