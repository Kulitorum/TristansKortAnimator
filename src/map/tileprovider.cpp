#include "tileprovider.h"
#include <QNetworkRequest>

const QHash<TileSource, QString> TileProvider::s_urlTemplates = {
    {TileSource::EsriSatellite, "https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/%3/%2/%1"}
};

const QStringList TileProvider::s_sourceNames = {
    "Satellite"
};

TileProvider::TileProvider(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &TileProvider::onTileDownloaded);
}

void TileProvider::requestTile(int x, int y, int zoom) {
    QString key = tileKey(x, y, zoom);

    // Don't request same tile twice
    if (m_requestedTiles.contains(key)) {
        return;
    }

    m_requestedTiles.insert(key);

    QString urlStr = buildTileUrl(x, y, zoom);
    QUrl url(urlStr);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      "TristansKortAnimator/1.0 (Map Animation Software)");
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                        QNetworkRequest::PreferCache);

    // Store tile coordinates in request
    request.setAttribute(QNetworkRequest::User, QVariant::fromValue(QPoint(x, y)));
    request.setAttribute(QNetworkRequest::UserMax, zoom);

    m_networkManager->get(request);

    m_pendingRequests++;
    emit loadingChanged();
    emit pendingCountChanged();
}

void TileProvider::cancelAllRequests() {
    // Clear pending tiles set
    m_requestedTiles.clear();
    m_pendingRequests = 0;
    emit loadingChanged();
    emit pendingCountChanged();
}

void TileProvider::onTileDownloaded(QNetworkReply* reply) {
    reply->deleteLater();

    m_pendingRequests = qMax(0, m_pendingRequests - 1);
    emit pendingCountChanged();
    if (m_pendingRequests == 0) {
        emit loadingChanged();
    }

    // Extract tile coordinates from request
    QPoint coords = reply->request().attribute(QNetworkRequest::User).toPoint();
    int zoom = reply->request().attribute(QNetworkRequest::UserMax).toInt();
    int x = coords.x();
    int y = coords.y();

    QString key = tileKey(x, y, zoom);
    m_requestedTiles.remove(key);

    if (reply->error() != QNetworkReply::NoError) {
        emit tileFailed(x, y, zoom, reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QImage image;
    if (!image.loadFromData(data)) {
        emit tileFailed(x, y, zoom, "Failed to decode image");
        return;
    }

    emit tileReady(x, y, zoom, image);
}

QString TileProvider::buildTileUrl(int x, int y, int zoom) const {
    // Always use ESRI Satellite
    QString urlTemplate = s_urlTemplates.value(TileSource::EsriSatellite);
    if (urlTemplate.isEmpty()) {
        urlTemplate = "https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/%3/%2/%1";
    }
    return urlTemplate.arg(x).arg(y).arg(zoom);
}

QString TileProvider::tileKey(int x, int y, int zoom) const {
    return QString("%1_%2_%3_%4").arg(static_cast<int>(m_currentSource)).arg(zoom).arg(x).arg(y);
}

void TileProvider::setCurrentSource(int source) {
    Q_UNUSED(source);
    // Only satellite is supported now
    TileSource newSource = TileSource::EsriSatellite;
    if (m_currentSource != newSource) {
        m_currentSource = newSource;
        cancelAllRequests();
        emit currentSourceChanged();
    }
}

QString TileProvider::tileSourceName(int index) const {
    if (index >= 0 && index < s_sourceNames.size()) {
        return s_sourceNames[index];
    }
    return QString();
}

QStringList TileProvider::availableSources() const {
    return s_sourceNames;
}
