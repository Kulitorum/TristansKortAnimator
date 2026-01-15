#include "keyframemodel.h"
#include <QJsonArray>
#include <algorithm>

KeyframeModel::KeyframeModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int KeyframeModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return m_keyframes.size();
}

QVariant KeyframeModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_keyframes.size())
        return QVariant();

    const Keyframe& kf = m_keyframes.at(index.row());

    switch (role) {
        case LatitudeRole: return kf.latitude;
        case LongitudeRole: return kf.longitude;
        case ZoomRole: return kf.zoom;
        case BearingRole: return kf.bearing;
        case TiltRole: return kf.tilt;
        case TimeRole: return kf.timeMs;
        case InterpolationRole: return kf.interpolationModeInt;
        case EasingRole: return kf.easingTypeInt;
        default: return QVariant();
    }
}

bool KeyframeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() >= m_keyframes.size())
        return false;

    Keyframe& kf = m_keyframes[index.row()];
    bool changed = false;

    switch (role) {
        case LatitudeRole:
            if (kf.latitude != value.toDouble()) {
                kf.latitude = value.toDouble();
                changed = true;
            }
            break;
        case LongitudeRole:
            if (kf.longitude != value.toDouble()) {
                kf.longitude = value.toDouble();
                changed = true;
            }
            break;
        case ZoomRole:
            if (kf.zoom != value.toDouble()) {
                kf.zoom = value.toDouble();
                changed = true;
            }
            break;
        case BearingRole:
            if (kf.bearing != value.toDouble()) {
                kf.bearing = value.toDouble();
                changed = true;
            }
            break;
        case TiltRole:
            if (kf.tilt != value.toDouble()) {
                kf.tilt = value.toDouble();
                changed = true;
            }
            break;
        case TimeRole:
            if (kf.timeMs != value.toDouble()) {
                kf.timeMs = value.toDouble();
                sortByTime();
                emit totalDurationChanged();
                changed = true;
            }
            break;
        case InterpolationRole:
            if (kf.interpolationModeInt != value.toInt()) {
                kf.interpolationModeInt = value.toInt();
                kf.syncEnumsFromInts();
                changed = true;
            }
            break;
        case EasingRole:
            if (kf.easingTypeInt != value.toInt()) {
                kf.easingTypeInt = value.toInt();
                kf.syncEnumsFromInts();
                changed = true;
            }
            break;
    }

    if (changed) {
        emit dataChanged(index, index, {role});
        emit keyframeModified(index.row());
        emit dataModified();
    }

    return changed;
}

QHash<int, QByteArray> KeyframeModel::roleNames() const {
    return {
        {LatitudeRole, "latitude"},
        {LongitudeRole, "longitude"},
        {ZoomRole, "zoom"},
        {BearingRole, "bearing"},
        {TiltRole, "tilt"},
        {TimeRole, "time"},
        {InterpolationRole, "interpolation"},
        {EasingRole, "easing"}
    };
}

void KeyframeModel::addKeyframe(double lat, double lon, double zoom, double bearing, double tilt) {
    // Calculate time for new keyframe: last keyframe time + 4 seconds, or 0 for first
    double newTime = 0.0;
    if (!m_keyframes.isEmpty()) {
        newTime = m_keyframes.last().timeMs + DEFAULT_KEYFRAME_INTERVAL;
    }

    addKeyframeAtTime(lat, lon, zoom, bearing, tilt, newTime);
}

void KeyframeModel::addKeyframeAtTime(double lat, double lon, double zoom, double bearing, double tilt, double timeMs) {
    beginInsertRows(QModelIndex(), m_keyframes.size(), m_keyframes.size());

    Keyframe kf;
    kf.latitude = lat;
    kf.longitude = lon;
    kf.zoom = zoom;
    kf.bearing = bearing;
    kf.tilt = tilt;
    // Snap to frame boundary and ensure non-negative
    kf.timeMs = snapToFrame(qMax(0.0, timeMs));
    kf.syncEnumInts();

    m_keyframes.append(kf);

    endInsertRows();
    sortByTime();  // Sort after adding to maintain time order
    emit countChanged();
    emit totalDurationChanged();
    emit dataModified();
}

void KeyframeModel::insertKeyframe(int index, const QVariantMap& data) {
    if (index < 0 || index > m_keyframes.size()) return;

    beginInsertRows(QModelIndex(), index, index);

    Keyframe kf;
    kf.latitude = data.value("latitude", 0.0).toDouble();
    kf.longitude = data.value("longitude", 0.0).toDouble();
    kf.zoom = data.value("zoom", 5.0).toDouble();
    kf.bearing = data.value("bearing", 0.0).toDouble();
    kf.tilt = data.value("tilt", 0.0).toDouble();
    kf.timeMs = data.value("time", 0.0).toDouble();
    kf.interpolationModeInt = data.value("interpolation", 1).toInt();
    kf.easingTypeInt = data.value("easing", 1).toInt();
    kf.syncEnumsFromInts();

    m_keyframes.insert(index, kf);

    endInsertRows();
    sortByTime();
    emit countChanged();
    emit totalDurationChanged();
    emit dataModified();
}

void KeyframeModel::removeKeyframe(int index) {
    if (index < 0 || index >= m_keyframes.size()) return;

    beginRemoveRows(QModelIndex(), index, index);
    m_keyframes.remove(index);
    endRemoveRows();

    if (m_currentIndex >= m_keyframes.size()) {
        setCurrentIndex(qMax(0, m_keyframes.size() - 1));
    }

    emit countChanged();
    emit totalDurationChanged();
    emit dataModified();
}

void KeyframeModel::moveKeyframe(int from, int to) {
    if (from < 0 || from >= m_keyframes.size()) return;
    if (to < 0 || to >= m_keyframes.size()) return;
    if (from == to) return;

    beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to);
    m_keyframes.move(from, to);
    endMoveRows();

    emit dataModified();
}

void KeyframeModel::duplicateKeyframe(int index) {
    if (index < 0 || index >= m_keyframes.size()) return;

    Keyframe copy = m_keyframes.at(index);
    // Place duplicate 2 seconds after original
    copy.timeMs += 2000.0;

    beginInsertRows(QModelIndex(), m_keyframes.size(), m_keyframes.size());
    m_keyframes.append(copy);
    endInsertRows();

    sortByTime();
    emit countChanged();
    emit totalDurationChanged();
    emit dataModified();
}

void KeyframeModel::updateKeyframe(int index, const QVariantMap& data) {
    if (index < 0 || index >= m_keyframes.size()) return;

    Keyframe& kf = m_keyframes[index];

    if (data.contains("latitude")) kf.latitude = data["latitude"].toDouble();
    if (data.contains("longitude")) kf.longitude = data["longitude"].toDouble();
    if (data.contains("zoom")) kf.zoom = data["zoom"].toDouble();
    if (data.contains("bearing")) kf.bearing = data["bearing"].toDouble();
    if (data.contains("tilt")) kf.tilt = data["tilt"].toDouble();
    if (data.contains("time")) {
        kf.timeMs = data["time"].toDouble();
        sortByTime();
        emit totalDurationChanged();
    }
    if (data.contains("interpolation")) {
        kf.interpolationModeInt = data["interpolation"].toInt();
        kf.syncEnumsFromInts();
    }
    if (data.contains("easing")) {
        kf.easingTypeInt = data["easing"].toInt();
        kf.syncEnumsFromInts();
    }

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit keyframeModified(index);
    emit dataModified();
}

QVariantMap KeyframeModel::getKeyframe(int index) const {
    if (index < 0 || index >= m_keyframes.size())
        return QVariantMap();

    const Keyframe& kf = m_keyframes.at(index);
    return {
        {"latitude", kf.latitude},
        {"longitude", kf.longitude},
        {"zoom", kf.zoom},
        {"bearing", kf.bearing},
        {"tilt", kf.tilt},
        {"time", kf.timeMs},
        {"interpolation", kf.interpolationModeInt},
        {"easing", kf.easingTypeInt}
    };
}

void KeyframeModel::clear() {
    beginResetModel();
    m_keyframes.clear();
    m_currentIndex = 0;
    endResetModel();

    emit countChanged();
    emit totalDurationChanged();
    emit currentIndexChanged();
    emit dataModified();
}

void KeyframeModel::setKeyframeTime(int index, double timeMs) {
    if (index < 0 || index >= m_keyframes.size()) return;

    // Snap to frame boundary and ensure non-negative
    timeMs = snapToFrame(qMax(0.0, timeMs));

    if (m_keyframes[index].timeMs != timeMs) {
        m_keyframes[index].timeMs = timeMs;
        sortByTime();

        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {TimeRole});
        emit totalDurationChanged();
        emit dataModified();
    }
}

void KeyframeModel::setKeyframeInterpolation(int index, int mode) {
    setData(createIndex(index, 0), mode, InterpolationRole);
}

void KeyframeModel::setKeyframeEasing(int index, int easing) {
    setData(createIndex(index, 0), easing, EasingRole);
}

double KeyframeModel::totalDuration() const {
    if (m_keyframes.isEmpty()) return 0;
    // Total duration is the time of the last keyframe
    return m_keyframes.last().timeMs;
}

void KeyframeModel::setCurrentIndex(int index) {
    if (index < 0) index = 0;
    if (index >= m_keyframes.size()) index = m_keyframes.size() - 1;

    if (m_currentIndex != index) {
        m_currentIndex = index;
        emit currentIndexChanged();

        // When current index changes, also select this keyframe and emit signal to load position
        selectKeyframe(index, false);
        emit keyframeSelected(index);
    }
}

void KeyframeModel::setEditMode(bool enabled) {
    if (m_editMode != enabled) {
        m_editMode = enabled;
        emit editModeChanged();
    }
}

void KeyframeModel::updateCurrentPosition(double lat, double lon, double zoom, double bearing, double tilt) {
    if (!m_editMode) return;

    // Determine which keyframes to update: all selected, or just current
    QList<int> indicesToUpdate;
    if (m_selectedIndices.size() > 1) {
        // Multiple selection: update ALL selected keyframes with same position
        indicesToUpdate = m_selectedIndices;
    } else if (m_currentIndex >= 0 && m_currentIndex < m_keyframes.size()) {
        // Single selection: update current keyframe only
        indicesToUpdate.append(m_currentIndex);
    }

    if (indicesToUpdate.isEmpty()) return;

    bool anyChanged = false;
    for (int idx : indicesToUpdate) {
        if (idx < 0 || idx >= m_keyframes.size()) continue;

        Keyframe& kf = m_keyframes[idx];
        bool changed = false;

        if (kf.latitude != lat) { kf.latitude = lat; changed = true; }
        if (kf.longitude != lon) { kf.longitude = lon; changed = true; }
        if (kf.zoom != zoom) { kf.zoom = zoom; changed = true; }
        if (kf.bearing != bearing) { kf.bearing = bearing; changed = true; }
        if (kf.tilt != tilt) { kf.tilt = tilt; changed = true; }

        if (changed) {
            QModelIndex modelIndex = createIndex(idx, 0);
            emit dataChanged(modelIndex, modelIndex);
            emit keyframeModified(idx);
            anyChanged = true;
        }
    }

    // Note: Don't emit dataModified here to avoid invalidating frame buffer during editing
    Q_UNUSED(anyChanged);
}

void KeyframeModel::selectKeyframe(int index, bool addToSelection) {
    if (index < 0 || index >= m_keyframes.size()) return;

    if (!addToSelection) {
        m_selectedIndices.clear();
    }

    if (!m_selectedIndices.contains(index)) {
        m_selectedIndices.append(index);
        std::sort(m_selectedIndices.begin(), m_selectedIndices.end());
    }

    emit selectedIndicesChanged();
}

void KeyframeModel::deselectKeyframe(int index) {
    if (m_selectedIndices.removeOne(index)) {
        emit selectedIndicesChanged();
    }
}

void KeyframeModel::selectRange(int startIndex, int endIndex) {
    if (startIndex > endIndex) std::swap(startIndex, endIndex);

    startIndex = qMax(0, startIndex);
    endIndex = qMin(m_keyframes.size() - 1, endIndex);

    m_selectedIndices.clear();
    for (int i = startIndex; i <= endIndex; i++) {
        m_selectedIndices.append(i);
    }

    emit selectedIndicesChanged();
}

void KeyframeModel::selectAll() {
    m_selectedIndices.clear();
    for (int i = 0; i < m_keyframes.size(); i++) {
        m_selectedIndices.append(i);
    }
    emit selectedIndicesChanged();
}

void KeyframeModel::clearSelection() {
    if (!m_selectedIndices.isEmpty()) {
        m_selectedIndices.clear();
        emit selectedIndicesChanged();
    }
}

bool KeyframeModel::isSelected(int index) const {
    return m_selectedIndices.contains(index);
}

void KeyframeModel::moveSelectedKeyframes(double deltaTimeMs) {
    if (m_selectedIndices.isEmpty()) return;

    // Calculate minimum time to ensure no keyframe goes negative
    double minTime = 0;
    for (int idx : m_selectedIndices) {
        if (idx >= 0 && idx < m_keyframes.size()) {
            minTime = qMin(minTime, m_keyframes[idx].timeMs + deltaTimeMs);
        }
    }

    // Clamp delta if it would make any keyframe negative
    if (minTime < 0) {
        deltaTimeMs -= minTime;
    }

    // Apply delta to all selected keyframes, snapping to frame boundaries
    for (int idx : m_selectedIndices) {
        if (idx >= 0 && idx < m_keyframes.size()) {
            m_keyframes[idx].timeMs = snapToFrame(m_keyframes[idx].timeMs + deltaTimeMs);
        }
    }

    sortByTime();
    emit totalDurationChanged();
    emit dataModified();
}

void KeyframeModel::updateSelectionAfterSort() {
    // After sorting, indices may have changed
    // For now, clear selection - a more sophisticated approach would track keyframes by ID
    // m_selectedIndices.clear();
    // emit selectedIndicesChanged();
}

int KeyframeModel::keyframeIndexAtTime(double timeMs) const {
    if (m_keyframes.isEmpty()) return -1;

    // Find the keyframe we're transitioning FROM (the one before or at current time)
    for (int i = m_keyframes.size() - 1; i >= 0; i--) {
        if (timeMs >= m_keyframes[i].timeMs) {
            return i;
        }
    }
    return 0;
}

double KeyframeModel::progressAtTime(double timeMs, int& outFromIndex, int& outToIndex) const {
    outFromIndex = -1;
    outToIndex = -1;

    if (m_keyframes.size() < 2) {
        if (m_keyframes.size() == 1) {
            outFromIndex = 0;
            outToIndex = 0;
        }
        return 0.0;
    }

    // Find which two keyframes we're between
    outFromIndex = keyframeIndexAtTime(timeMs);

    // If we're at or past the last keyframe, we're done
    if (outFromIndex >= m_keyframes.size() - 1) {
        outFromIndex = m_keyframes.size() - 1;
        outToIndex = outFromIndex;
        return 1.0;
    }

    outToIndex = outFromIndex + 1;

    const Keyframe& from = m_keyframes[outFromIndex];
    const Keyframe& to = m_keyframes[outToIndex];

    double segmentDuration = to.timeMs - from.timeMs;
    if (segmentDuration <= 0) return 0.0;

    double progress = (timeMs - from.timeMs) / segmentDuration;
    return qBound(0.0, progress, 1.0);
}

void KeyframeModel::sortByTime() {
    // Sort keyframes by their time position
    std::sort(m_keyframes.begin(), m_keyframes.end(), [](const Keyframe& a, const Keyframe& b) {
        return a.timeMs < b.timeMs;
    });

    // Notify that data may have changed
    if (!m_keyframes.isEmpty()) {
        emit dataChanged(createIndex(0, 0), createIndex(m_keyframes.size() - 1, 0));
    }
}

int KeyframeModel::keyframeNearTime(double timeMs, double toleranceMs) const {
    for (int i = 0; i < m_keyframes.size(); i++) {
        if (qAbs(m_keyframes[i].timeMs - timeMs) <= toleranceMs) {
            return i;
        }
    }
    return -1;
}

bool KeyframeModel::hasKeyframeNearTime(double timeMs, double toleranceMs) const {
    return keyframeNearTime(timeMs, toleranceMs) >= 0;
}

double KeyframeModel::snapToFrame(double timeMs, int fps) {
    if (fps <= 0) fps = 30;
    double frameMs = 1000.0 / fps;
    return qRound(timeMs / frameMs) * frameMs;
}

void KeyframeModel::goToNextKeyframe() {
    if (m_keyframes.isEmpty()) return;

    int nextIndex = m_currentIndex + 1;
    if (nextIndex >= m_keyframes.size()) {
        nextIndex = m_keyframes.size() - 1;  // Stay at last
    }

    if (nextIndex != m_currentIndex) {
        setCurrentIndex(nextIndex);
    }
}

void KeyframeModel::goToPreviousKeyframe() {
    if (m_keyframes.isEmpty()) return;

    int prevIndex = m_currentIndex - 1;
    if (prevIndex < 0) {
        prevIndex = 0;  // Stay at first
    }

    if (prevIndex != m_currentIndex) {
        setCurrentIndex(prevIndex);
    }
}

QJsonArray KeyframeModel::toJson() const {
    QJsonArray array;
    for (const auto& kf : m_keyframes) {
        array.append(kf.toJson());
    }
    return array;
}

void KeyframeModel::fromJson(const QJsonArray& array) {
    beginResetModel();
    m_keyframes.clear();

    for (const auto& val : array) {
        m_keyframes.append(Keyframe::fromJson(val.toObject()));
    }

    sortByTime();
    endResetModel();

    emit countChanged();
    emit totalDurationChanged();
}
