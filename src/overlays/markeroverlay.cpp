#include "markeroverlay.h"

MarkerOverlay::MarkerOverlay(QObject* parent)
    : Overlay(OverlayType::Marker, parent)
{
}

void MarkerOverlay::setLatitude(double lat) {
    if (!qFuzzyCompare(m_latitude, lat)) {
        m_latitude = lat;
        emit latitudeChanged();
        emit modified();
    }
}

void MarkerOverlay::setLongitude(double lon) {
    if (!qFuzzyCompare(m_longitude, lon)) {
        m_longitude = lon;
        emit longitudeChanged();
        emit modified();
    }
}

void MarkerOverlay::setIconUrl(const QString& url) {
    if (m_iconUrl != url) {
        m_iconUrl = url;
        emit iconUrlChanged();
        emit modified();
    }
}

void MarkerOverlay::setIconScale(double scale) {
    scale = qBound(0.1, scale, 5.0);
    if (!qFuzzyCompare(m_iconScale, scale)) {
        m_iconScale = scale;
        emit iconScaleChanged();
        emit modified();
    }
}

void MarkerOverlay::setColor(const QColor& color) {
    if (m_color != color) {
        m_color = color;
        emit colorChanged();
        emit modified();
    }
}

void MarkerOverlay::setLabel(const QString& label) {
    if (m_label != label) {
        m_label = label;
        emit labelChanged();
        emit modified();
    }
}

QJsonObject MarkerOverlay::toJson() const {
    QJsonObject obj = Overlay::toJson();
    obj["latitude"] = m_latitude;
    obj["longitude"] = m_longitude;
    obj["iconUrl"] = m_iconUrl;
    obj["iconScale"] = m_iconScale;
    obj["color"] = m_color.name(QColor::HexArgb);
    obj["label"] = m_label;
    return obj;
}

void MarkerOverlay::fromJson(const QJsonObject& obj) {
    Overlay::fromJson(obj);
    m_latitude = obj["latitude"].toDouble();
    m_longitude = obj["longitude"].toDouble();
    m_iconUrl = obj["iconUrl"].toString("qrc:/icons/marker_pin.svg");
    m_iconScale = obj["iconScale"].toDouble(1.0);
    m_color = QColor(obj["color"].toString("#FFFF0000"));
    m_label = obj["label"].toString();
}
