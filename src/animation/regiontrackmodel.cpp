#include "regiontrackmodel.h"
#include <QJsonArray>

RegionTrackModel::RegionTrackModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int RegionTrackModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return m_tracks.size();
}

QVariant RegionTrackModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_tracks.size())
        return QVariant();

    const RegionTrack& track = m_tracks.at(index.row());

    switch (role) {
        case RegionCodeRole: return track.regionCode;
        case RegionNameRole: return track.regionName;
        case RegionTypeRole: return track.regionType;
        case FillColorRole: return track.fillColor;
        case BorderColorRole: return track.borderColor;
        case BorderWidthRole: return track.borderWidth;
        case StartTimeRole: return track.startTime;
        case FadeInDurationRole: return track.fadeInDuration;
        case EndTimeRole: return track.endTime;
        case FadeOutDurationRole: return track.fadeOutDuration;
        default: return QVariant();
    }
}

bool RegionTrackModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() >= m_tracks.size())
        return false;

    RegionTrack& track = m_tracks[index.row()];
    bool changed = false;

    switch (role) {
        case RegionCodeRole:
            if (track.regionCode != value.toString()) {
                track.regionCode = value.toString();
                changed = true;
            }
            break;
        case RegionNameRole:
            if (track.regionName != value.toString()) {
                track.regionName = value.toString();
                changed = true;
            }
            break;
        case FillColorRole:
            if (track.fillColor != value.value<QColor>()) {
                track.fillColor = value.value<QColor>();
                changed = true;
            }
            break;
        case BorderColorRole:
            if (track.borderColor != value.value<QColor>()) {
                track.borderColor = value.value<QColor>();
                changed = true;
            }
            break;
        case BorderWidthRole:
            if (track.borderWidth != value.toDouble()) {
                track.borderWidth = value.toDouble();
                changed = true;
            }
            break;
        case StartTimeRole:
            if (track.startTime != value.toDouble()) {
                track.startTime = value.toDouble();
                changed = true;
            }
            break;
        case FadeInDurationRole:
            if (track.fadeInDuration != value.toDouble()) {
                track.fadeInDuration = value.toDouble();
                changed = true;
            }
            break;
        case EndTimeRole:
            if (track.endTime != value.toDouble()) {
                track.endTime = value.toDouble();
                changed = true;
            }
            break;
        case FadeOutDurationRole:
            if (track.fadeOutDuration != value.toDouble()) {
                track.fadeOutDuration = value.toDouble();
                changed = true;
            }
            break;
    }

    if (changed) {
        emit dataChanged(index, index, {role});
        emit trackModified(index.row());
        emit dataModified();
    }

    return changed;
}

QHash<int, QByteArray> RegionTrackModel::roleNames() const {
    return {
        {RegionCodeRole, "regionCode"},
        {RegionNameRole, "regionName"},
        {RegionTypeRole, "regionType"},
        {FillColorRole, "fillColor"},
        {BorderColorRole, "borderColor"},
        {BorderWidthRole, "borderWidth"},
        {StartTimeRole, "startTime"},
        {FadeInDurationRole, "fadeInDuration"},
        {EndTimeRole, "endTime"},
        {FadeOutDurationRole, "fadeOutDuration"}
    };
}

void RegionTrackModel::addTrack(const QString& regionCode, const QString& regionName,
                                 const QString& regionType, double startTime) {
    beginInsertRows(QModelIndex(), m_tracks.size(), m_tracks.size());

    RegionTrack track;
    track.regionCode = regionCode;
    track.regionName = regionName;
    track.regionType = regionType;
    track.startTime = startTime;

    m_tracks.append(track);

    endInsertRows();
    emit countChanged();
    emit dataModified();
}

void RegionTrackModel::removeTrack(int index) {
    if (index < 0 || index >= m_tracks.size()) return;

    beginRemoveRows(QModelIndex(), index, index);
    m_tracks.remove(index);
    endRemoveRows();

    emit countChanged();
    emit dataModified();
}

void RegionTrackModel::updateTrack(int index, const QVariantMap& data) {
    if (index < 0 || index >= m_tracks.size()) return;

    RegionTrack& track = m_tracks[index];

    if (data.contains("regionCode")) track.regionCode = data["regionCode"].toString();
    if (data.contains("regionName")) track.regionName = data["regionName"].toString();
    if (data.contains("regionType")) track.regionType = data["regionType"].toString();
    if (data.contains("fillColor")) track.fillColor = data["fillColor"].value<QColor>();
    if (data.contains("borderColor")) track.borderColor = data["borderColor"].value<QColor>();
    if (data.contains("borderWidth")) track.borderWidth = data["borderWidth"].toDouble();
    if (data.contains("startTime")) track.startTime = data["startTime"].toDouble();
    if (data.contains("fadeInDuration")) track.fadeInDuration = data["fadeInDuration"].toDouble();
    if (data.contains("endTime")) track.endTime = data["endTime"].toDouble();
    if (data.contains("fadeOutDuration")) track.fadeOutDuration = data["fadeOutDuration"].toDouble();

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit trackModified(index);
    emit dataModified();
}

QVariantMap RegionTrackModel::getTrack(int index) const {
    if (index < 0 || index >= m_tracks.size())
        return QVariantMap();

    const RegionTrack& track = m_tracks.at(index);
    return {
        {"regionCode", track.regionCode},
        {"regionName", track.regionName},
        {"regionType", track.regionType},
        {"fillColor", track.fillColor},
        {"borderColor", track.borderColor},
        {"borderWidth", track.borderWidth},
        {"startTime", track.startTime},
        {"fadeInDuration", track.fadeInDuration},
        {"endTime", track.endTime},
        {"fadeOutDuration", track.fadeOutDuration}
    };
}

void RegionTrackModel::clear() {
    beginResetModel();
    m_tracks.clear();
    endResetModel();

    emit countChanged();
    emit dataModified();
}

void RegionTrackModel::setTrackTiming(int index, double startTime, double fadeIn,
                                       double endTime, double fadeOut) {
    if (index < 0 || index >= m_tracks.size()) return;

    RegionTrack& track = m_tracks[index];
    track.startTime = qMax(0.0, startTime);
    track.fadeInDuration = qMax(0.0, fadeIn);
    track.endTime = qMax(0.0, endTime);
    track.fadeOutDuration = qMax(0.0, fadeOut);

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit trackModified(index);
    emit dataModified();
}

void RegionTrackModel::setTrackColors(int index, const QColor& fillColor,
                                       const QColor& borderColor, double borderWidth) {
    if (index < 0 || index >= m_tracks.size()) return;

    RegionTrack& track = m_tracks[index];
    track.fillColor = fillColor;
    track.borderColor = borderColor;
    track.borderWidth = qBound(0.0, borderWidth, 10.0);

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit trackModified(index);
    emit dataModified();
}

double RegionTrackModel::trackOpacityAtTime(int index, double timeMs, double totalDuration) const {
    if (index < 0 || index >= m_tracks.size()) return 0.0;
    return m_tracks.at(index).opacityAtTime(timeMs, totalDuration);
}

QVector<QPair<const RegionTrack*, double>> RegionTrackModel::visibleTracksAtTime(
        double timeMs, double totalDuration) const {
    QVector<QPair<const RegionTrack*, double>> result;

    for (const auto& track : m_tracks) {
        double opacity = track.opacityAtTime(timeMs, totalDuration);
        if (opacity > 0.0) {
            result.append({&track, opacity});
        }
    }

    return result;
}

QJsonArray RegionTrackModel::toJson() const {
    QJsonArray array;
    for (const auto& track : m_tracks) {
        array.append(track.toJson());
    }
    return array;
}

void RegionTrackModel::fromJson(const QJsonArray& array) {
    beginResetModel();
    m_tracks.clear();

    for (const auto& val : array) {
        m_tracks.append(RegionTrack::fromJson(val.toObject()));
    }

    endResetModel();
    emit countChanged();
}
