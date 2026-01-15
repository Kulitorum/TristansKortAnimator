#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QImage>
#include <QHash>
#include <QUrl>
#include <QSet>

enum class TileSource {
    EsriSatellite = 0
};

class TileProvider : public QObject {
    Q_OBJECT

    Q_PROPERTY(int currentSource READ currentSource WRITE setCurrentSource NOTIFY currentSourceChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(int pendingCount READ pendingCount NOTIFY pendingCountChanged)

public:
    explicit TileProvider(QObject* parent = nullptr);

    Q_INVOKABLE void requestTile(int x, int y, int zoom);
    Q_INVOKABLE void cancelAllRequests();
    Q_INVOKABLE QString tileSourceName(int index) const;
    Q_INVOKABLE QStringList availableSources() const;

    int currentSource() const { return static_cast<int>(m_currentSource); }
    void setCurrentSource(int source);
    bool isLoading() const { return m_pendingRequests > 0; }
    int pendingCount() const { return m_pendingRequests; }

signals:
    void tileReady(int x, int y, int zoom, const QImage& image);
    void tileFailed(int x, int y, int zoom, const QString& error);
    void currentSourceChanged();
    void loadingChanged();
    void pendingCountChanged();

private slots:
    void onTileDownloaded(QNetworkReply* reply);

private:
    QString buildTileUrl(int x, int y, int zoom) const;
    QString tileKey(int x, int y, int zoom) const;

    QNetworkAccessManager* m_networkManager;
    TileSource m_currentSource = TileSource::EsriSatellite;
    int m_pendingRequests = 0;
    QSet<QString> m_requestedTiles;

    static const QHash<TileSource, QString> s_urlTemplates;
    static const QStringList s_sourceNames;
};
