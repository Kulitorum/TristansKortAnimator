#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

class Settings : public QObject {
    Q_OBJECT

    // Map settings
    Q_PROPERTY(int tileSource READ tileSource WRITE setTileSource NOTIFY tileSourceChanged)
    Q_PROPERTY(bool showCountryLabels READ showCountryLabels WRITE setShowCountryLabels NOTIFY showCountryLabelsChanged)
    Q_PROPERTY(bool showRegionLabels READ showRegionLabels WRITE setShowRegionLabels NOTIFY showRegionLabelsChanged)
    Q_PROPERTY(bool showCityLabels READ showCityLabels WRITE setShowCityLabels NOTIFY showCityLabelsChanged)
    Q_PROPERTY(bool shadeNonHighlighted READ shadeNonHighlighted WRITE setShadeNonHighlighted NOTIFY shadeNonHighlightedChanged)
    Q_PROPERTY(double nonHighlightedOpacity READ nonHighlightedOpacity WRITE setNonHighlightedOpacity NOTIFY nonHighlightedOpacityChanged)
    Q_PROPERTY(QString tileCachePath READ tileCachePath WRITE setTileCachePath NOTIFY tileCachePathChanged)
    Q_PROPERTY(int tileCacheMaxMB READ tileCacheMaxMB WRITE setTileCacheMaxMB NOTIFY tileCacheMaxMBChanged)
    Q_PROPERTY(int diskCacheMaxMB READ diskCacheMaxMB WRITE setDiskCacheMaxMB NOTIFY diskCacheMaxMBChanged)

    // Export settings
    Q_PROPERTY(int exportWidth READ exportWidth WRITE setExportWidth NOTIFY exportWidthChanged)
    Q_PROPERTY(int exportHeight READ exportHeight WRITE setExportHeight NOTIFY exportHeightChanged)
    Q_PROPERTY(int exportFramerate READ exportFramerate WRITE setExportFramerate NOTIFY exportFramerateChanged)
    Q_PROPERTY(QString ffmpegPath READ ffmpegPath WRITE setFfmpegPath NOTIFY ffmpegPathChanged)
    Q_PROPERTY(QString lastExportPath READ lastExportPath WRITE setLastExportPath NOTIFY lastExportPathChanged)
    Q_PROPERTY(QString lastProjectPath READ lastProjectPath WRITE setLastProjectPath NOTIFY lastProjectPathChanged)

    // UI settings
    Q_PROPERTY(double timelineZoom READ timelineZoom WRITE setTimelineZoom NOTIFY timelineZoomChanged)
    Q_PROPERTY(bool previewAutoPlay READ previewAutoPlay WRITE setPreviewAutoPlay NOTIFY previewAutoPlayChanged)
    Q_PROPERTY(bool autoKey READ autoKey WRITE setAutoKey NOTIFY autoKeyChanged)

    // Default keyframe settings
    Q_PROPERTY(double defaultDuration READ defaultDuration WRITE setDefaultDuration NOTIFY defaultDurationChanged)
    Q_PROPERTY(int defaultInterpolation READ defaultInterpolation WRITE setDefaultInterpolation NOTIFY defaultInterpolationChanged)
    Q_PROPERTY(int defaultEasing READ defaultEasing WRITE setDefaultEasing NOTIFY defaultEasingChanged)

public:
    explicit Settings(QObject* parent = nullptr);

    // Map settings
    int tileSource() const;
    void setTileSource(int source);

    bool showCountryLabels() const;
    void setShowCountryLabels(bool show);

    bool showRegionLabels() const;
    void setShowRegionLabels(bool show);

    bool showCityLabels() const;
    void setShowCityLabels(bool show);

    bool shadeNonHighlighted() const;
    void setShadeNonHighlighted(bool shade);

    double nonHighlightedOpacity() const;
    void setNonHighlightedOpacity(double opacity);

    QString tileCachePath() const;
    void setTileCachePath(const QString& path);

    int tileCacheMaxMB() const;
    void setTileCacheMaxMB(int mb);

    int diskCacheMaxMB() const;
    void setDiskCacheMaxMB(int mb);

    // Export settings
    int exportWidth() const;
    void setExportWidth(int width);

    int exportHeight() const;
    void setExportHeight(int height);

    int exportFramerate() const;
    void setExportFramerate(int fps);

    QString ffmpegPath() const;
    void setFfmpegPath(const QString& path);

    QString lastExportPath() const;
    void setLastExportPath(const QString& path);

    QString lastProjectPath() const;
    void setLastProjectPath(const QString& path);

    // UI settings
    double timelineZoom() const;
    void setTimelineZoom(double zoom);

    bool previewAutoPlay() const;
    void setPreviewAutoPlay(bool autoPlay);

    bool autoKey() const;
    void setAutoKey(bool enabled);

    // Default keyframe settings
    double defaultDuration() const;
    void setDefaultDuration(double ms);

    int defaultInterpolation() const;
    void setDefaultInterpolation(int mode);

    int defaultEasing() const;
    void setDefaultEasing(int easing);

signals:
    void tileSourceChanged();
    void showCountryLabelsChanged();
    void showRegionLabelsChanged();
    void showCityLabelsChanged();
    void shadeNonHighlightedChanged();
    void nonHighlightedOpacityChanged();
    void tileCachePathChanged();
    void tileCacheMaxMBChanged();
    void diskCacheMaxMBChanged();
    void exportWidthChanged();
    void exportHeightChanged();
    void exportFramerateChanged();
    void ffmpegPathChanged();
    void lastExportPathChanged();
    void lastProjectPathChanged();
    void timelineZoomChanged();
    void previewAutoPlayChanged();
    void autoKeyChanged();
    void defaultDurationChanged();
    void defaultInterpolationChanged();
    void defaultEasingChanged();

private:
    QSettings m_settings;
};
