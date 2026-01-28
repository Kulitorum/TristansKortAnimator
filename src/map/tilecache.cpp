#include "tilecache.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QDirIterator>
#include <QDateTime>
#include <QFile>
#include <algorithm>

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
        QMutexLocker locker(&m_diskMutex);
        QDir cacheDir(m_diskCachePath);
        cacheDir.removeRecursively();
        cacheDir.mkpath(".");
        m_cachedDiskUsageBytes = 0;
        m_diskUsageDirty = false;
        emit diskUsageChanged();
    }
}

void TileCache::setMaxMemorySize(int megabytes) {
    m_maxMemoryMB = megabytes;
    m_memoryCache.setMaxCost(megabytes * 4);
    emit memoryUsageChanged();
}

void TileCache::setMaxDiskCacheMB(int megabytes) {
    if (m_maxDiskCacheMB != megabytes) {
        m_maxDiskCacheMB = megabytes;
        emit maxDiskCacheMBChanged();

        // Enforce new limit if we're over
        if (m_diskCacheEnabled) {
            enforceDiskCacheLimit();
        }
    }
}

void TileCache::enableDiskCache(const QString& path) {
    m_diskCachePath = path;
    m_diskCacheEnabled = true;
    m_diskUsageDirty = true;

    QDir cacheDir(path);
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }

    // Initial cache size check
    updateDiskUsageCache();
    enforceDiskCacheLimit();
}

int TileCache::memoryUsageMB() const {
    return m_memoryCache.totalCost() / 4;
}

int TileCache::diskUsageMB() const {
    if (!m_diskCacheEnabled || m_diskCachePath.isEmpty()) {
        return 0;
    }

    // Use cached value if available
    if (!m_diskUsageDirty) {
        return static_cast<int>(m_cachedDiskUsageBytes / (1024 * 1024));
    }

    // Recalculate (this is expensive, so we cache it)
    const_cast<TileCache*>(this)->updateDiskUsageCache();
    return static_cast<int>(m_cachedDiskUsageBytes / (1024 * 1024));
}

void TileCache::updateDiskUsageCache() {
    QMutexLocker locker(&m_diskMutex);

    qint64 totalSize = 0;
    QDirIterator it(m_diskCachePath, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        totalSize += it.fileInfo().size();
    }
    m_cachedDiskUsageBytes = totalSize;
    m_diskUsageDirty = false;
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

    // Update access time by touching the file (for LRU)
    QFile file(path);
    if (file.exists()) {
        // Touch the file to update its modification time (used for LRU)
        file.open(QIODevice::ReadWrite);
        file.close();
    }

    return image.load(path);
}

void TileCache::saveToDisk(int source, int x, int y, int zoom, const QImage& image) {
    QString path = diskPath(source, x, y, zoom);
    QFileInfo fileInfo(path);
    fileInfo.dir().mkpath(".");

    // Save the tile
    qint64 oldSize = fileInfo.exists() ? fileInfo.size() : 0;
    image.save(path, "PNG");

    // Update cached disk usage
    qint64 newSize = QFileInfo(path).size();
    m_cachedDiskUsageBytes += (newSize - oldSize);

    // Check if we need to evict old tiles
    enforceDiskCacheLimit();

    emit diskUsageChanged();
}

void TileCache::enforceDiskCacheLimit() {
    if (!m_diskCacheEnabled || m_diskCachePath.isEmpty()) {
        return;
    }

    qint64 maxBytes = static_cast<qint64>(m_maxDiskCacheMB) * 1024 * 1024;

    // Quick check using cached value
    if (m_cachedDiskUsageBytes <= maxBytes) {
        return;
    }

    QMutexLocker locker(&m_diskMutex);

    // Collect all cached files with their modification times
    struct CacheEntry {
        QString path;
        qint64 size;
        QDateTime lastModified;
    };
    QVector<CacheEntry> entries;
    qint64 totalSize = 0;

    QDirIterator it(m_diskCachePath, {"*.png"}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        QFileInfo info = it.fileInfo();
        entries.append({info.filePath(), info.size(), info.lastModified()});
        totalSize += info.size();
    }

    m_cachedDiskUsageBytes = totalSize;

    if (totalSize <= maxBytes) {
        return;
    }

    // Sort by last modified time (oldest first - LRU)
    std::sort(entries.begin(), entries.end(), [](const CacheEntry& a, const CacheEntry& b) {
        return a.lastModified < b.lastModified;
    });

    // Delete oldest files until we're under the limit
    // Target 90% of max to avoid frequent evictions
    qint64 targetSize = maxBytes * 9 / 10;
    int deletedCount = 0;

    for (const auto& entry : entries) {
        if (totalSize <= targetSize) {
            break;
        }

        if (QFile::remove(entry.path)) {
            totalSize -= entry.size;
            deletedCount++;
        }
    }

    m_cachedDiskUsageBytes = totalSize;

    if (deletedCount > 0) {
        qDebug() << "Disk cache cleanup: removed" << deletedCount << "tiles,"
                 << "new size:" << (totalSize / (1024 * 1024)) << "MB";

        // Clean up empty directories
        QDirIterator dirIt(m_diskCachePath, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        QStringList emptyDirs;
        while (dirIt.hasNext()) {
            dirIt.next();
            QDir dir(dirIt.filePath());
            if (dir.isEmpty()) {
                emptyDirs.prepend(dirIt.filePath());  // Prepend to process deepest first
            }
        }
        for (const QString& dirPath : emptyDirs) {
            QDir().rmdir(dirPath);
        }
    }
}
