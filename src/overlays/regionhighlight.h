#pragma once

#include "overlay.h"
#include <QColor>

class RegionHighlight : public Overlay {
    Q_OBJECT

    Q_PROPERTY(QString regionCode READ regionCode WRITE setRegionCode NOTIFY regionCodeChanged)
    Q_PROPERTY(QString regionName READ regionName WRITE setRegionName NOTIFY regionNameChanged)
    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(double borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)

public:
    explicit RegionHighlight(QObject* parent = nullptr);

    QString regionCode() const { return m_regionCode; }
    void setRegionCode(const QString& code);

    QString regionName() const { return m_regionName; }
    void setRegionName(const QString& name);

    QColor fillColor() const { return m_fillColor; }
    void setFillColor(const QColor& color);

    QColor borderColor() const { return m_borderColor; }
    void setBorderColor(const QColor& color);

    double borderWidth() const { return m_borderWidth; }
    void setBorderWidth(double width);

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& obj) override;

signals:
    void regionCodeChanged();
    void regionNameChanged();
    void fillColorChanged();
    void borderColorChanged();
    void borderWidthChanged();

private:
    QString m_regionCode;
    QString m_regionName;
    QColor m_fillColor = QColor(255, 0, 0, 100);  // Semi-transparent red
    QColor m_borderColor = Qt::red;
    double m_borderWidth = 2.0;
};
