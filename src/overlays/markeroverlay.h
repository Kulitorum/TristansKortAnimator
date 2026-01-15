#pragma once

#include "overlay.h"
#include <QUrl>
#include <QColor>

class MarkerOverlay : public Overlay {
    Q_OBJECT

    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)
    Q_PROPERTY(QString iconUrl READ iconUrl WRITE setIconUrl NOTIFY iconUrlChanged)
    Q_PROPERTY(double iconScale READ iconScale WRITE setIconScale NOTIFY iconScaleChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)

public:
    explicit MarkerOverlay(QObject* parent = nullptr);

    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    QString iconUrl() const { return m_iconUrl; }
    double iconScale() const { return m_iconScale; }
    QColor color() const { return m_color; }
    QString label() const { return m_label; }

    void setLatitude(double lat);
    void setLongitude(double lon);
    void setIconUrl(const QString& url);
    void setIconScale(double scale);
    void setColor(const QColor& color);
    void setLabel(const QString& label);

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& obj) override;

signals:
    void latitudeChanged();
    void longitudeChanged();
    void iconUrlChanged();
    void iconScaleChanged();
    void colorChanged();
    void labelChanged();

private:
    double m_latitude = 0.0;
    double m_longitude = 0.0;
    QString m_iconUrl = "qrc:/icons/marker_pin.svg";
    double m_iconScale = 1.0;
    QColor m_color = Qt::red;
    QString m_label;
};
