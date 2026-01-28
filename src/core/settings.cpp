#include "settings.h"
#include <QStandardPaths>
#include <QDir>

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_settings("TristansKortAnimator", "KortAnimator")
{
}

// Map settings
int Settings::tileSource() const {
    return m_settings.value("map/tileSource", 0).toInt();
}

void Settings::setTileSource(int source) {
    if (tileSource() != source) {
        m_settings.setValue("map/tileSource", source);
        emit tileSourceChanged();
    }
}

bool Settings::showCountryLabels() const {
    return m_settings.value("map/showCountryLabels", true).toBool();
}

void Settings::setShowCountryLabels(bool show) {
    if (showCountryLabels() != show) {
        m_settings.setValue("map/showCountryLabels", show);
        emit showCountryLabelsChanged();
    }
}

bool Settings::showRegionLabels() const {
    return m_settings.value("map/showRegionLabels", true).toBool();
}

void Settings::setShowRegionLabels(bool show) {
    if (showRegionLabels() != show) {
        m_settings.setValue("map/showRegionLabels", show);
        emit showRegionLabelsChanged();
    }
}

bool Settings::showCityLabels() const {
    return m_settings.value("map/showCityLabels", true).toBool();
}

void Settings::setShowCityLabels(bool show) {
    if (showCityLabels() != show) {
        m_settings.setValue("map/showCityLabels", show);
        emit showCityLabelsChanged();
    }
}

bool Settings::shadeNonHighlighted() const {
    return m_settings.value("map/shadeNonHighlighted", false).toBool();
}

void Settings::setShadeNonHighlighted(bool shade) {
    if (shadeNonHighlighted() != shade) {
        m_settings.setValue("map/shadeNonHighlighted", shade);
        emit shadeNonHighlightedChanged();
    }
}

double Settings::nonHighlightedOpacity() const {
    return m_settings.value("map/nonHighlightedOpacity", 0.3).toDouble();
}

void Settings::setNonHighlightedOpacity(double opacity) {
    if (!qFuzzyCompare(nonHighlightedOpacity(), opacity)) {
        m_settings.setValue("map/nonHighlightedOpacity", opacity);
        emit nonHighlightedOpacityChanged();
    }
}

QString Settings::tileCachePath() const {
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/tiles";
    return m_settings.value("map/tileCachePath", defaultPath).toString();
}

void Settings::setTileCachePath(const QString& path) {
    if (tileCachePath() != path) {
        m_settings.setValue("map/tileCachePath", path);
        emit tileCachePathChanged();
    }
}

int Settings::tileCacheMaxMB() const {
    return m_settings.value("map/tileCacheMaxMB", 512).toInt();
}

void Settings::setTileCacheMaxMB(int mb) {
    if (tileCacheMaxMB() != mb) {
        m_settings.setValue("map/tileCacheMaxMB", mb);
        emit tileCacheMaxMBChanged();
    }
}

int Settings::diskCacheMaxMB() const {
    return m_settings.value("map/diskCacheMaxMB", 2048).toInt();  // Default 2GB
}

void Settings::setDiskCacheMaxMB(int mb) {
    if (diskCacheMaxMB() != mb) {
        m_settings.setValue("map/diskCacheMaxMB", mb);
        emit diskCacheMaxMBChanged();
    }
}

// Export settings
int Settings::exportWidth() const {
    return m_settings.value("export/width", 1920).toInt();
}

void Settings::setExportWidth(int width) {
    if (exportWidth() != width) {
        m_settings.setValue("export/width", width);
        emit exportWidthChanged();
    }
}

int Settings::exportHeight() const {
    return m_settings.value("export/height", 1080).toInt();
}

void Settings::setExportHeight(int height) {
    if (exportHeight() != height) {
        m_settings.setValue("export/height", height);
        emit exportHeightChanged();
    }
}

int Settings::exportFramerate() const {
    return m_settings.value("export/framerate", 30).toInt();
}

void Settings::setExportFramerate(int fps) {
    if (exportFramerate() != fps) {
        m_settings.setValue("export/framerate", fps);
        emit exportFramerateChanged();
    }
}

QString Settings::ffmpegPath() const {
    return m_settings.value("export/ffmpegPath", "ffmpeg").toString();
}

void Settings::setFfmpegPath(const QString& path) {
    if (ffmpegPath() != path) {
        m_settings.setValue("export/ffmpegPath", path);
        emit ffmpegPathChanged();
    }
}

QString Settings::lastExportPath() const {
    return m_settings.value("export/lastExportPath",
        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)).toString();
}

void Settings::setLastExportPath(const QString& path) {
    if (lastExportPath() != path) {
        m_settings.setValue("export/lastExportPath", path);
        emit lastExportPathChanged();
    }
}

QString Settings::lastProjectPath() const {
    return m_settings.value("project/lastProjectPath",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
}

void Settings::setLastProjectPath(const QString& path) {
    if (lastProjectPath() != path) {
        m_settings.setValue("project/lastProjectPath", path);
        emit lastProjectPathChanged();
    }
}

// UI settings
double Settings::timelineZoom() const {
    return m_settings.value("ui/timelineZoom", 1.0).toDouble();
}

void Settings::setTimelineZoom(double zoom) {
    if (qFuzzyCompare(timelineZoom(), zoom) == false) {
        m_settings.setValue("ui/timelineZoom", zoom);
        emit timelineZoomChanged();
    }
}

bool Settings::previewAutoPlay() const {
    return m_settings.value("ui/previewAutoPlay", false).toBool();
}

void Settings::setPreviewAutoPlay(bool autoPlay) {
    if (previewAutoPlay() != autoPlay) {
        m_settings.setValue("ui/previewAutoPlay", autoPlay);
        emit previewAutoPlayChanged();
    }
}

bool Settings::autoKey() const {
    return m_settings.value("ui/autoKey", false).toBool();
}

void Settings::setAutoKey(bool enabled) {
    if (autoKey() != enabled) {
        m_settings.setValue("ui/autoKey", enabled);
        emit autoKeyChanged();
    }
}

// Default keyframe settings
double Settings::defaultDuration() const {
    return m_settings.value("keyframe/defaultDuration", 2000.0).toDouble();
}

void Settings::setDefaultDuration(double ms) {
    if (qFuzzyCompare(defaultDuration(), ms) == false) {
        m_settings.setValue("keyframe/defaultDuration", ms);
        emit defaultDurationChanged();
    }
}

int Settings::defaultInterpolation() const {
    return m_settings.value("keyframe/defaultInterpolation", 0).toInt();
}

void Settings::setDefaultInterpolation(int mode) {
    if (defaultInterpolation() != mode) {
        m_settings.setValue("keyframe/defaultInterpolation", mode);
        emit defaultInterpolationChanged();
    }
}

int Settings::defaultEasing() const {
    return m_settings.value("keyframe/defaultEasing", 1).toInt();
}

void Settings::setDefaultEasing(int easing) {
    if (defaultEasing() != easing) {
        m_settings.setValue("keyframe/defaultEasing", easing);
        emit defaultEasingChanged();
    }
}
