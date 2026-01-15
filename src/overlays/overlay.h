#pragma once

#include <QObject>
#include <QString>
#include <QColor>
#include <QUuid>
#include <QJsonObject>

enum class OverlayType {
    Marker = 0,
    Arrow,
    Text,
    RegionHighlight
};

class Overlay : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(int type READ typeInt CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(double opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(double startTime READ startTime WRITE setStartTime NOTIFY startTimeChanged)
    Q_PROPERTY(double endTime READ endTime WRITE setEndTime NOTIFY endTimeChanged)

public:
    explicit Overlay(OverlayType type, QObject* parent = nullptr);
    virtual ~Overlay() = default;

    QString id() const { return m_id; }
    OverlayType type() const { return m_type; }
    int typeInt() const { return static_cast<int>(m_type); }

    QString name() const { return m_name; }
    void setName(const QString& name);

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible);

    double opacity() const { return m_opacity; }
    void setOpacity(double opacity);

    double startTime() const { return m_startTime; }
    void setStartTime(double time);

    double endTime() const { return m_endTime; }
    void setEndTime(double time);

    // Check if overlay should be visible at given time
    bool isVisibleAtTime(double timeMs) const;

    // Serialization
    virtual QJsonObject toJson() const;
    virtual void fromJson(const QJsonObject& obj);

signals:
    void nameChanged();
    void visibleChanged();
    void opacityChanged();
    void startTimeChanged();
    void endTimeChanged();
    void modified();

protected:
    QString m_id;
    OverlayType m_type;
    QString m_name;
    bool m_visible = true;
    double m_opacity = 1.0;
    double m_startTime = 0.0;
    double m_endTime = -1.0;  // -1 means visible until end
};
