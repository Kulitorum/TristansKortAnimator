#include "regionhighlight.h"

RegionHighlight::RegionHighlight(QObject* parent)
    : Overlay(OverlayType::RegionHighlight, parent)
{
}

void RegionHighlight::setRegionCode(const QString& code) {
    if (m_regionCode != code) {
        m_regionCode = code;
        emit regionCodeChanged();
        emit modified();
    }
}

void RegionHighlight::setRegionName(const QString& name) {
    if (m_regionName != name) {
        m_regionName = name;
        setName(name);  // Also update overlay name
        emit regionNameChanged();
        emit modified();
    }
}

void RegionHighlight::setFillColor(const QColor& color) {
    if (m_fillColor != color) {
        m_fillColor = color;
        emit fillColorChanged();
        emit modified();
    }
}

void RegionHighlight::setBorderColor(const QColor& color) {
    if (m_borderColor != color) {
        m_borderColor = color;
        emit borderColorChanged();
        emit modified();
    }
}

void RegionHighlight::setBorderWidth(double width) {
    width = qBound(0.0, width, 10.0);
    if (!qFuzzyCompare(m_borderWidth, width)) {
        m_borderWidth = width;
        emit borderWidthChanged();
        emit modified();
    }
}

QJsonObject RegionHighlight::toJson() const {
    QJsonObject obj = Overlay::toJson();
    obj["regionCode"] = m_regionCode;
    obj["regionName"] = m_regionName;
    obj["fillColor"] = m_fillColor.name(QColor::HexArgb);
    obj["borderColor"] = m_borderColor.name(QColor::HexArgb);
    obj["borderWidth"] = m_borderWidth;
    return obj;
}

void RegionHighlight::fromJson(const QJsonObject& obj) {
    Overlay::fromJson(obj);
    m_regionCode = obj["regionCode"].toString();
    m_regionName = obj["regionName"].toString();
    m_fillColor = QColor(obj["fillColor"].toString("#64FF0000"));
    m_borderColor = QColor(obj["borderColor"].toString("#FFFF0000"));
    m_borderWidth = obj["borderWidth"].toDouble(2.0);
}
