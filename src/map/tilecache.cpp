#include "tilecache.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QDirIterator>

TileCache::TileCache(int maxMemoryMB, QObject* parent)
    : QObject(parent)
    , m_maxMemoryMB(maxMemoryMB)
{
    // Set cache cost based on typical tile size (256x256 RGBA = ~256KB)
    m_memoryCache.setMaxCost(maxMemoryMB * 4); // ~4 tiles per MB
}

bool TileCache::contains(int source, int x, int y, int zoom) const {
    QString key = tileKey(source, x, y, zoom);
    return m_memoryCache.contains(key);
}

QImage TileCache::get(int source, int x, int y, int zoom) {
    QString key = tileKey(source, x, y, zoom);

    // Try memory cache first
    if (m_memoryCache.contains(key)) {
        return *m_memoryCache.object(key);
    }

    // Try disk cache
    QImage image;
    if (m_diskCacheEnabled && loadFromDisk(source, x, y, zoom, image)) {
        // Add to memory cache
        m_memoryCache.insert(key, new QImage(image), 1);
        return image;
    }

    return QImage();
}

void TileCache::insert(int source, int x, int y, int zoom, const QImage& image) {
    QString key = tileKey(source, x, y, zoom);

    // Add to memory cache
    m_memoryCache.insert(key, new QImage(image), 1);
    emit memoryUsageChanged();

    // Save to disk cache
    if (m_diskCacheEnabled) {
        saveToDisk(source, x, y, zoom, image);
    }
}

void TileCache::clear() {
    m_memoryCache.clear();
    emit memoryUsageChanged();
}

void TileCache::clearDiskCache() {
    if (m_diskCacheEnabled && !m_diskCachePath.isEmpty()) {
        QDir cacheDir(m_diskCachePath);
        cacheDir.removeRecursively();
        cacheDir.mkpath(".");
        emit diskUsageChanged();
    }
}

void TileCache::setMaxMemorySize(int megabytes) {
    m_maxMemoryMB = megabytes;
    m_memoryCache.setMaxCost(megabytes * 4);
    emit memoryUsageChanged();
}

void TileCache::enableDiskCache(const QString& path) {
    m_diskCachePath = path;
    m_diskCacheEnabled = true;

    QDir cacheDir(path);
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
}

int TileCache::memoryUsageMB() const {
    return m_memoryCache.totalCost() / 4;
}

int TileCache::diskUsageMB() const {
    if (!m_diskCacheEnabled || m_diskCachePath.isEmpty()) {
        return 0;
    }

    qint64 totalSize = 0;
    QDirIterator it(m_diskCachePath, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        totalSize += it.fileInfo().size();
    }
    return static_cast<int>(totalSize / (1024 * 1024));
}

QString TileCache::tileKey(int source, int x, int y, int zoom) const {
    return QString("%1_%2_%3_%4").arg(source).arg(zoom).arg(x).arg(y);
}

QString TileCache::diskPath(int source, int x, int y, int zoom) const {
    return QString("%1/%2/%3/%4/%5.png")
        .arg(m_diskCachePath)
        .arg(source)
        .arg(zoom)
        .arg(x)
        .arg(y);
}

bool TileCache::loadFromDisk(int source, int x, int y, int zoom, QImage& image) {
    QString path = diskPath(source, x, y, zoom);
    return image.load(path);
}

void TileCache::saveToDisk(int source, int x, int y, int zoom, const QImage& image) {
    QString path = diskPath(source, x, y, zoom);
    QFileInfo fileInfo(path);
    fileInfo.dir().mkpath(".");
    image.save(path, "PNG");
    emit diskUsageChanged();
}
