#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonArray>

// Simple on-demand city boundary fetcher using Nominatim
class CityBoundaryFetcher : public QObject {
    Q_OBJECT

public:
    explicit CityBoundaryFetcher(QObject* parent = nullptr);

    // Fetch boundary for a city (async - emits boundaryReady when done)
    Q_INVOKABLE void fetchBoundary(const QString& cityName, const QString& countryName);

signals:
    void boundaryReady(const QString& cityName, const QJsonArray& coordinates, const QString& geometryType);
    void fetchFailed(const QString& cityName, const QString& error);

private slots:
    void onRequestFinished(QNetworkReply* reply);

private:
    void sendRequest();

    QNetworkAccessManager* m_network;
    QString m_pendingCity;
    QString m_pendingCountry;
    int m_retryCount = 0;
    static constexpr int MAX_RETRIES = 3;
};
