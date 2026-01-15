#pragma once

#include "overlay.h"
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QJsonArray>

struct BezierControlPoint {
    double latitude;
    double longitude;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["latitude"] = latitude;
        obj["longitude"] = longitude;
        return obj;
    }

    static BezierControlPoint fromJson(const QJsonObject& obj) {
        return {obj["latitude"].toDouble(), obj["longitude"].toDouble()};
    }
};

class ArrowOverlay : public Overlay {
    Q_OBJECT

    Q_PROPERTY(double startLat READ startLat WRITE setStartLat NOTIFY pathChanged)
    Q_PROPERTY(double startLon READ startLon WRITE setStartLon NOTIFY pathChanged)
    Q_PROPERTY(double endLat READ endLat WRITE setEndLat NOTIFY pathChanged)
    Q_PROPERTY(double endLon READ endLon WRITE setEndLon NOTIFY pathChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(double strokeWidth READ strokeWidth WRITE setStrokeWidth NOTIFY strokeWidthChanged)
    Q_PROPERTY(bool animated READ isAnimated WRITE setAnimated NOTIFY animatedChanged)
    Q_PROPERTY(double animationSpeed READ animationSpeed WRITE setAnimationSpeed NOTIFY animationSpeedChanged)
    Q_PROPERTY(double animationDuration READ animationDuration WRITE setAnimationDuration NOTIFY animationDurationChanged)
    Q_PROPERTY(QString arrowStyle READ arrowStyle WRITE setArrowStyle NOTIFY arrowStyleChanged)
    Q_PROPERTY(bool showArrowhead READ showArrowhead WRITE setShowArrowhead NOTIFY showArrowheadChanged)

public:
    explicit ArrowOverlay(QObject* parent = nullptr);

    double startLat() const { return m_startLat; }
    double startLon() const { return m_startLon; }
    double endLat() const { return m_endLat; }
    double endLon() const { return m_endLon; }

    void setStartLat(double lat);
    void setStartLon(double lon);
    void setEndLat(double lat);
    void setEndLon(double lon);

    QColor color() const { return m_color; }
    void setColor(const QColor& color);

    double strokeWidth() const { return m_strokeWidth; }
    void setStrokeWidth(double width);

    bool isAnimated() const { return m_animated; }
    void setAnimated(bool animated);

    double animationSpeed() const { return m_animationSpeed; }
    void setAnimationSpeed(double speed);

    double animationDuration() const { return m_animationDuration; }
    void setAnimationDuration(double duration);

    QString arrowStyle() const { return m_arrowStyle; }
    void setArrowStyle(const QString& style);  // "solid", "dashed", "dotted", "troops"

    bool showArrowhead() const { return m_showArrowhead; }
    void setShowArrowhead(bool show);

    // Bezier control points for curved arrows
    Q_INVOKABLE void addControlPoint(double lat, double lon);
    Q_INVOKABLE void removeControlPoint(int index);
    Q_INVOKABLE void updateControlPoint(int index, double lat, double lon);
    Q_INVOKABLE QVariantList controlPoints() const;
    Q_INVOKABLE void clearControlPoints();
    Q_INVOKABLE int controlPointCount() const { return m_controlPoints.size(); }

    // Get animation progress at specific time (0.0 to 1.0)
    double animationProgress(double timeMs) const;

    // Get point along bezier curve at parameter t (0.0 to 1.0)
    QPointF pointAtT(double t) const;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& obj) override;

signals:
    void pathChanged();
    void colorChanged();
    void strokeWidthChanged();
    void animatedChanged();
    void animationSpeedChanged();
    void animationDurationChanged();
    void arrowStyleChanged();
    void showArrowheadChanged();
    void controlPointsChanged();

private:
    double m_startLat = 0.0;
    double m_startLon = 0.0;
    double m_endLat = 0.0;
    double m_endLon = 0.0;
    QColor m_color = QColor("#e94560");
    double m_strokeWidth = 3.0;
    bool m_animated = true;
    double m_animationSpeed = 1.0;
    double m_animationDuration = 2000.0;  // ms for full animation
    QString m_arrowStyle = "solid";
    bool m_showArrowhead = true;
    QVector<BezierControlPoint> m_controlPoints;
};
