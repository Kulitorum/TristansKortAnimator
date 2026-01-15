#include "arrowoverlay.h"
#include <QtMath>

ArrowOverlay::ArrowOverlay(QObject* parent)
    : Overlay(OverlayType::Arrow, parent)
{
}

void ArrowOverlay::setStartLat(double lat) {
    if (!qFuzzyCompare(m_startLat, lat)) {
        m_startLat = lat;
        emit pathChanged();
        emit modified();
    }
}

void ArrowOverlay::setStartLon(double lon) {
    if (!qFuzzyCompare(m_startLon, lon)) {
        m_startLon = lon;
        emit pathChanged();
        emit modified();
    }
}

void ArrowOverlay::setEndLat(double lat) {
    if (!qFuzzyCompare(m_endLat, lat)) {
        m_endLat = lat;
        emit pathChanged();
        emit modified();
    }
}

void ArrowOverlay::setEndLon(double lon) {
    if (!qFuzzyCompare(m_endLon, lon)) {
        m_endLon = lon;
        emit pathChanged();
        emit modified();
    }
}

void ArrowOverlay::setColor(const QColor& color) {
    if (m_color != color) {
        m_color = color;
        emit colorChanged();
        emit modified();
    }
}

void ArrowOverlay::setStrokeWidth(double width) {
    width = qBound(1.0, width, 20.0);
    if (!qFuzzyCompare(m_strokeWidth, width)) {
        m_strokeWidth = width;
        emit strokeWidthChanged();
        emit modified();
    }
}

void ArrowOverlay::setAnimated(bool animated) {
    if (m_animated != animated) {
        m_animated = animated;
        emit animatedChanged();
        emit modified();
    }
}

void ArrowOverlay::setAnimationSpeed(double speed) {
    speed = qBound(0.1, speed, 5.0);
    if (!qFuzzyCompare(m_animationSpeed, speed)) {
        m_animationSpeed = speed;
        emit animationSpeedChanged();
        emit modified();
    }
}

void ArrowOverlay::setAnimationDuration(double duration) {
    duration = qMax(100.0, duration);
    if (!qFuzzyCompare(m_animationDuration, duration)) {
        m_animationDuration = duration;
        emit animationDurationChanged();
        emit modified();
    }
}

void ArrowOverlay::setArrowStyle(const QString& style) {
    if (m_arrowStyle != style) {
        m_arrowStyle = style;
        emit arrowStyleChanged();
        emit modified();
    }
}

void ArrowOverlay::setShowArrowhead(bool show) {
    if (m_showArrowhead != show) {
        m_showArrowhead = show;
        emit showArrowheadChanged();
        emit modified();
    }
}

void ArrowOverlay::addControlPoint(double lat, double lon) {
    m_controlPoints.append({lat, lon});
    emit controlPointsChanged();
    emit pathChanged();
    emit modified();
}

void ArrowOverlay::removeControlPoint(int index) {
    if (index >= 0 && index < m_controlPoints.size()) {
        m_controlPoints.remove(index);
        emit controlPointsChanged();
        emit pathChanged();
        emit modified();
    }
}

void ArrowOverlay::updateControlPoint(int index, double lat, double lon) {
    if (index >= 0 && index < m_controlPoints.size()) {
        m_controlPoints[index] = {lat, lon};
        emit controlPointsChanged();
        emit pathChanged();
        emit modified();
    }
}

QVariantList ArrowOverlay::controlPoints() const {
    QVariantList result;
    for (const auto& cp : m_controlPoints) {
        QVariantMap point;
        point["latitude"] = cp.latitude;
        point["longitude"] = cp.longitude;
        result.append(point);
    }
    return result;
}

void ArrowOverlay::clearControlPoints() {
    if (!m_controlPoints.isEmpty()) {
        m_controlPoints.clear();
        emit controlPointsChanged();
        emit pathChanged();
        emit modified();
    }
}

double ArrowOverlay::animationProgress(double timeMs) const {
    if (!m_animated || m_animationDuration <= 0) {
        return 1.0;
    }

    // Calculate time within this overlay's visibility
    double relativeTime = timeMs - m_startTime;
    if (relativeTime < 0) return 0.0;

    double adjustedDuration = m_animationDuration / m_animationSpeed;
    double progress = relativeTime / adjustedDuration;

    return qBound(0.0, progress, 1.0);
}

QPointF ArrowOverlay::pointAtT(double t) const {
    t = qBound(0.0, t, 1.0);

    if (m_controlPoints.isEmpty()) {
        // Simple linear interpolation
        double lat = m_startLat + (m_endLat - m_startLat) * t;
        double lon = m_startLon + (m_endLon - m_startLon) * t;
        return QPointF(lat, lon);
    }

    // Build list of all points for bezier
    QVector<QPointF> points;
    points.append(QPointF(m_startLat, m_startLon));
    for (const auto& cp : m_controlPoints) {
        points.append(QPointF(cp.latitude, cp.longitude));
    }
    points.append(QPointF(m_endLat, m_endLon));

    // De Casteljau's algorithm for bezier curve
    QVector<QPointF> working = points;
    while (working.size() > 1) {
        QVector<QPointF> next;
        for (int i = 0; i < working.size() - 1; i++) {
            double lat = working[i].x() + (working[i+1].x() - working[i].x()) * t;
            double lon = working[i].y() + (working[i+1].y() - working[i].y()) * t;
            next.append(QPointF(lat, lon));
        }
        working = next;
    }

    return working.first();
}

QJsonObject ArrowOverlay::toJson() const {
    QJsonObject obj = Overlay::toJson();
    obj["startLat"] = m_startLat;
    obj["startLon"] = m_startLon;
    obj["endLat"] = m_endLat;
    obj["endLon"] = m_endLon;
    obj["color"] = m_color.name(QColor::HexArgb);
    obj["strokeWidth"] = m_strokeWidth;
    obj["animated"] = m_animated;
    obj["animationSpeed"] = m_animationSpeed;
    obj["animationDuration"] = m_animationDuration;
    obj["arrowStyle"] = m_arrowStyle;
    obj["showArrowhead"] = m_showArrowhead;

    QJsonArray cpArray;
    for (const auto& cp : m_controlPoints) {
        cpArray.append(cp.toJson());
    }
    obj["controlPoints"] = cpArray;

    return obj;
}

void ArrowOverlay::fromJson(const QJsonObject& obj) {
    Overlay::fromJson(obj);
    m_startLat = obj["startLat"].toDouble();
    m_startLon = obj["startLon"].toDouble();
    m_endLat = obj["endLat"].toDouble();
    m_endLon = obj["endLon"].toDouble();
    m_color = QColor(obj["color"].toString("#FFe94560"));
    m_strokeWidth = obj["strokeWidth"].toDouble(3.0);
    m_animated = obj["animated"].toBool(true);
    m_animationSpeed = obj["animationSpeed"].toDouble(1.0);
    m_animationDuration = obj["animationDuration"].toDouble(2000.0);
    m_arrowStyle = obj["arrowStyle"].toString("solid");
    m_showArrowhead = obj["showArrowhead"].toBool(true);

    m_controlPoints.clear();
    QJsonArray cpArray = obj["controlPoints"].toArray();
    for (const auto& val : cpArray) {
        m_controlPoints.append(BezierControlPoint::fromJson(val.toObject()));
    }
}
