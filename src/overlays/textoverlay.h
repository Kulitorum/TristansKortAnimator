#pragma once

#include "overlay.h"
#include <QColor>
#include <QFont>

class TextOverlay : public Overlay {
    Q_OBJECT

    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(bool bold READ isBold WRITE setBold NOTIFY boldChanged)
    Q_PROPERTY(QString alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)

public:
    explicit TextOverlay(QObject* parent = nullptr);

    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    QString text() const { return m_text; }
    QColor color() const { return m_color; }
    QColor backgroundColor() const { return m_backgroundColor; }
    int fontSize() const { return m_fontSize; }
    bool isBold() const { return m_bold; }
    QString alignment() const { return m_alignment; }

    void setLatitude(double lat);
    void setLongitude(double lon);
    void setText(const QString& text);
    void setColor(const QColor& color);
    void setBackgroundColor(const QColor& color);
    void setFontSize(int size);
    void setBold(bool bold);
    void setAlignment(const QString& alignment);

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& obj) override;

signals:
    void latitudeChanged();
    void longitudeChanged();
    void textChanged();
    void colorChanged();
    void backgroundColorChanged();
    void fontSizeChanged();
    void boldChanged();
    void alignmentChanged();

private:
    double m_latitude = 0.0;
    double m_longitude = 0.0;
    QString m_text = "Label";
    QColor m_color = Qt::white;
    QColor m_backgroundColor = QColor(0, 0, 0, 150);
    int m_fontSize = 14;
    bool m_bold = false;
    QString m_alignment = "center";  // "left", "center", "right"
};
