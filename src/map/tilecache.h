#pragma once

#include <QObject>
#include <QImage>
#include <QCache>
#include <QString>
#include <QDir>

class TileCache : public QObject {
    Q_OBJECT

    Q_PROPERTY(int memoryUsageMB READ memoryUsageMB NOTIFY memoryUsageChanged)
    Q_PROPERTY(int diskUsageMB READ diskUsageMB NOTIFY diskUsageChanged)

public:
    explicit TileCache(int maxMemoryMB = 256, QObject* parent = nullptr);

    bool contains(int source, int x, int y, int zoom) const;
    QImage get(int source, int x, int y, int zoom);
    void insert(int source, int x, int y, int zoom, const QImage& image);

    Q_INVOKABLE void clear();
    Q_INVOKABLE void clearDiskCache();
    Q_INVOKABLE void setMaxMemorySize(int megabytes);
    Q_INVOKABLE void enableDiskCache(const QString& path);

    int memoryUsageMB() const;
    int diskUsageMB() const;

signals:
    void memoryUsageChanged();
    void diskUsageChanged();

private:
    QString tileKey(int source, int x, int y, int zoom) const;
    QString diskPath(int source, int x, int y, int zoom) const;
    bool loadFromDisk(int source, int x, int y, int zoom, QImage& image);
    void saveToDisk(int source, int x, int y, int zoom, const QImage& image);

    QCache<QString, QImage> m_memoryCache;
    QString m_diskCachePath;
    bool m_diskCacheEnabled = false;
    int m_maxMemoryMB = 256;
};
