#include "overlay.h"

Overlay::Overlay(OverlayType type, QObject* parent)
    : QObject(parent)
    , m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_type(type)
{
    // Set default name based on type
    switch (type) {
        case OverlayType::Marker:
            m_name = "Marker";
            break;
        case OverlayType::Arrow:
            m_name = "Arrow";
            break;
        case OverlayType::Text:
            m_name = "Text";
            break;
        case OverlayType::RegionHighlight:
            m_name = "Region";
            break;
    }
}

void Overlay::setName(const QString& name) {
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
        emit modified();
    }
}

void Overlay::setVisible(bool visible) {
    if (m_visible != visible) {
        m_visible = visible;
        emit visibleChanged();
        emit modified();
    }
}

void Overlay::setOpacity(double opacity) {
    opacity = qBound(0.0, opacity, 1.0);
    if (!qFuzzyCompare(m_opacity, opacity)) {
        m_opacity = opacity;
        emit opacityChanged();
        emit modified();
    }
}

void Overlay::setStartTime(double time) {
    if (!qFuzzyCompare(m_startTime, time)) {
        m_startTime = time;
        emit startTimeChanged();
        emit modified();
    }
}

void Overlay::setEndTime(double time) {
    if (!qFuzzyCompare(m_endTime, time)) {
        m_endTime = time;
        emit endTimeChanged();
        emit modified();
    }
}

bool Overlay::isVisibleAtTime(double timeMs) const {
    if (!m_visible) return false;
    if (timeMs < m_startTime) return false;
    if (m_endTime >= 0 && timeMs > m_endTime) return false;
    return true;
}

QJsonObject Overlay::toJson() const {
    QJsonObject obj;
    obj["id"] = m_id;
    obj["type"] = static_cast<int>(m_type);
    obj["name"] = m_name;
    obj["visible"] = m_visible;
    obj["opacity"] = m_opacity;
    obj["startTime"] = m_startTime;
    obj["endTime"] = m_endTime;
    return obj;
}

void Overlay::fromJson(const QJsonObject& obj) {
    m_id = obj["id"].toString();
    if (m_id.isEmpty()) {
        m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    m_name = obj["name"].toString();
    m_visible = obj["visible"].toBool(true);
    m_opacity = obj["opacity"].toDouble(1.0);
    m_startTime = obj["startTime"].toDouble(0.0);
    m_endTime = obj["endTime"].toDouble(-1.0);
}
