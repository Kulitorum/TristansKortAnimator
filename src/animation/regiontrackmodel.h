#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QColor>
#include "regiontrack.h"

class RegionTrackModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum RegionTrackRoles {
        RegionCodeRole = Qt::UserRole + 1,
        RegionNameRole,
        RegionTypeRole,
        FillColorRole,
        BorderColorRole,
        BorderWidthRole,
        StartTimeRole,
        FadeInDurationRole,
        EndTimeRole,
        FadeOutDurationRole
    };

    explicit RegionTrackModel(QObject* parent = nullptr);

    // QAbstractListModel overrides
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    // Track operations
    Q_INVOKABLE void addTrack(const QString& regionCode, const QString& regionName,
                              const QString& regionType, double startTime = 0.0);
    Q_INVOKABLE void removeTrack(int index);
    Q_INVOKABLE void updateTrack(int index, const QVariantMap& data);
    Q_INVOKABLE QVariantMap getTrack(int index) const;
    Q_INVOKABLE void clear();

    // Timing
    Q_INVOKABLE void setTrackTiming(int index, double startTime, double fadeIn,
                                     double endTime, double fadeOut);
    Q_INVOKABLE void setTrackColors(int index, const QColor& fillColor,
                                     const QColor& borderColor, double borderWidth);

    // Get opacity for a track at a given time
    Q_INVOKABLE double trackOpacityAtTime(int index, double timeMs, double totalDuration) const;

    // Get all visible tracks at a given time with their opacities
    QVector<QPair<const RegionTrack*, double>> visibleTracksAtTime(double timeMs, double totalDuration) const;

    int count() const { return m_tracks.size(); }
    const QVector<RegionTrack>& tracks() const { return m_tracks; }

    // Serialization
    QJsonArray toJson() const;
    void fromJson(const QJsonArray& array);

signals:
    void countChanged();
    void trackModified(int index);
    void dataModified();

private:
    QVector<RegionTrack> m_tracks;
};
