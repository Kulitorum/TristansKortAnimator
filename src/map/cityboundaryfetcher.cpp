#include "cityboundaryfetcher.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

CityBoundaryFetcher::CityBoundaryFetcher(QObject* parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
    connect(m_network, &QNetworkAccessManager::finished, this, &CityBoundaryFetcher::onRequestFinished);
}

void CityBoundaryFetcher::fetchBoundary(const QString& cityName, const QString& countryName) {
    m_pendingCity = cityName;
    m_pendingCountry = countryName;
    m_retryCount = 0;

    sendRequest();
}

void CityBoundaryFetcher::sendRequest() {
    QUrl url("https://nominatim.openstreetmap.org/search");
    QUrlQuery query;

    // Try different query strategies based on retry count
    if (m_retryCount == 0) {
        // First try: freeform query which often works better
        query.addQueryItem("q", m_pendingCity + ", " + m_pendingCountry);
    } else if (m_retryCount == 1) {
        // Second try: structured query
        query.addQueryItem("city", m_pendingCity);
        query.addQueryItem("country", m_pendingCountry);
    } else {
        // Third try: search for municipality/kommune
        query.addQueryItem("q", m_pendingCity + " municipality, " + m_pendingCountry);
    }

    query.addQueryItem("format", "json");
    query.addQueryItem("polygon_geojson", "1");
    query.addQueryItem("limit", "10");  // Get more results
    query.addQueryItem("addressdetails", "1");  // Get address details to filter
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "TristansKortAnimator/1.0");

    qDebug() << "Fetching boundary for" << m_pendingCity << "(attempt" << (m_retryCount + 1) << ")";
    m_network->get(request);
}

void CityBoundaryFetcher::onRequestFinished(QNetworkReply* reply) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Request failed for" << m_pendingCity << ":" << reply->errorString();
        emit fetchFailed(m_pendingCity, reply->errorString());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray results = doc.array();

    if (results.isEmpty()) {
        // Retry with different query strategy
        if (m_retryCount < MAX_RETRIES - 1) {
            m_retryCount++;
            qDebug() << "No results, retrying with different query...";
            sendRequest();
            return;
        }
        qWarning() << "No results for" << m_pendingCity;
        emit fetchFailed(m_pendingCity, "No results found");
        return;
    }

    // First pass: prioritize relation types (administrative boundaries)
    for (const auto& resultVal : results) {
        QJsonObject result = resultVal.toObject();
        QString osmType = result["osm_type"].toString();

        // Relations are most likely to have proper boundaries
        if (osmType == "relation") {
            QJsonObject geojson = result["geojson"].toObject();
            QString type = geojson["type"].toString();

            if (type == "Polygon" || type == "MultiPolygon") {
                QJsonArray coords = geojson["coordinates"].toArray();
                QString displayName = result["display_name"].toString();
                qDebug() << "Got boundary for" << m_pendingCity << "(" << type << ", relation) from:" << displayName;
                emit boundaryReady(m_pendingCity, coords, type);
                return;
            }
        }
    }

    // Second pass: accept any result with a polygon
    for (const auto& resultVal : results) {
        QJsonObject result = resultVal.toObject();
        QJsonObject geojson = result["geojson"].toObject();
        QString type = geojson["type"].toString();

        if (type == "Polygon" || type == "MultiPolygon") {
            QJsonArray coords = geojson["coordinates"].toArray();
            QString displayName = result["display_name"].toString();
            qDebug() << "Got boundary for" << m_pendingCity << "(" << type << ") from:" << displayName;
            emit boundaryReady(m_pendingCity, coords, type);
            return;
        }
    }

    // No polygon found - retry with different query strategy
    if (m_retryCount < MAX_RETRIES - 1) {
        m_retryCount++;
        qDebug() << "No polygon in" << results.size() << "results, retrying with different query...";
        sendRequest();
        return;
    }

    // All retries exhausted
    qWarning() << "No polygon boundary found for" << m_pendingCity << "after" << MAX_RETRIES << "attempts";
    emit fetchFailed(m_pendingCity, "No polygon boundary available");
}
