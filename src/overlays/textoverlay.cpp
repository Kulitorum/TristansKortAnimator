#include "textoverlay.h"

TextOverlay::TextOverlay(QObject* parent)
    : Overlay(OverlayType::Text, parent)
{
}

void TextOverlay::setLatitude(double lat) {
    if (!qFuzzyCompare(m_latitude, lat)) {
        m_latitude = lat;
        emit latitudeChanged();
        emit modified();
    }
}

void TextOverlay::setLongitude(double lon) {
    if (!qFuzzyCompare(m_longitude, lon)) {
        m_longitude = lon;
        emit longitudeChanged();
        emit modified();
    }
}

void TextOverlay::setText(const QString& text) {
    if (m_text != text) {
        m_text = text;
        emit textChanged();
        emit modified();
    }
}

void TextOverlay::setColor(const QColor& color) {
    if (m_color != color) {
        m_color = color;
        emit colorChanged();
        emit modified();
    }
}

void TextOverlay::setBackgroundColor(const QColor& color) {
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        emit backgroundColorChanged();
        emit modified();
    }
}

void TextOverlay::setFontSize(int size) {
    size = qBound(8, size, 72);
    if (m_fontSize != size) {
        m_fontSize = size;
        emit fontSizeChanged();
        emit modified();
    }
}

void TextOverlay::setBold(bool bold) {
    if (m_bold != bold) {
        m_bold = bold;
        emit boldChanged();
        emit modified();
    }
}

void TextOverlay::setAlignment(const QString& alignment) {
    if (m_alignment != alignment) {
        m_alignment = alignment;
        emit alignmentChanged();
        emit modified();
    }
}

QJsonObject TextOverlay::toJson() const {
    QJsonObject obj = Overlay::toJson();
    obj["latitude"] = m_latitude;
    obj["longitude"] = m_longitude;
    obj["text"] = m_text;
    obj["color"] = m_color.name(QColor::HexArgb);
    obj["backgroundColor"] = m_backgroundColor.name(QColor::HexArgb);
    obj["fontSize"] = m_fontSize;
    obj["bold"] = m_bold;
    obj["alignment"] = m_alignment;
    return obj;
}

void TextOverlay::fromJson(const QJsonObject& obj) {
    Overlay::fromJson(obj);
    m_latitude = obj["latitude"].toDouble();
    m_longitude = obj["longitude"].toDouble();
    m_text = obj["text"].toString("Label");
    m_color = QColor(obj["color"].toString("#FFFFFFFF"));
    m_backgroundColor = QColor(obj["backgroundColor"].toString("#96000000"));
    m_fontSize = obj["fontSize"].toInt(14);
    m_bold = obj["bold"].toBool(false);
    m_alignment = obj["alignment"].toString("center");
}
