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
        case ZoomRole: return kf.zoom();  // Derived from altitude
        case BearingRole: return kf.bearing;
        case TiltRole: return kf.tilt;
        case TimeRole: return kf.timeMs;
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
            if (kf.zoom() != value.toDouble()) {
                kf.setZoom(value.toDouble());  // Converts to altitude
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
        {TimeRole, "time"}
    };
}

void KeyframeModel::addKeyframe(double lat, double lon, double zoom, double bearing, double tilt) {
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
    kf.setZoom(zoom);  // Converts zoom to altitude internally
    kf.bearing = bearing;
    kf.tilt = tilt;
    kf.timeMs = snapToFrame(qMax(0.0, timeMs));

    m_keyframes.append(kf);

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
    copy.timeMs += 2000.0;  // 2 seconds after

    beginInsertRows(QModelIndex(), m_keyframes.size(), m_keyframes.size());
    m_keyframes.append(copy);
    endInsertRows();

    sortByTime();
    emit countChanged();
    emit totalDurationChanged();
    emit dataModified();
}

void KeyframeModel::duplicateKeyframeAtTime(int index, double timeMs) {
    if (index < 0 || index >= m_keyframes.size()) return;

    Keyframe copy = m_keyframes.at(index);
    copy.timeMs = snapToFrame(qMax(0.0, timeMs));

    beginInsertRows(QModelIndex(), m_keyframes.size(), m_keyframes.size());
    m_keyframes.append(copy);
    endInsertRows();

    sortByTime();

    // Select the newly created keyframe
    int newIndex = keyframeIndexAtTime(copy.timeMs);
    if (newIndex >= 0) {
        setCurrentIndex(newIndex);
    }

    emit countChanged();
    emit totalDurationChanged();
    emit dataModified();
}

void KeyframeModel::updateKeyframe(int index, const QVariantMap& data) {
    if (index < 0 || index >= m_keyframes.size()) return;

    Keyframe& kf = m_keyframes[index];

    if (data.contains("latitude")) kf.latitude = data["latitude"].toDouble();
    if (data.contains("longitude")) kf.longitude = data["longitude"].toDouble();
    if (data.contains("zoom")) kf.setZoom(data["zoom"].toDouble());
    if (data.contains("bearing")) kf.bearing = data["bearing"].toDouble();
    if (data.contains("tilt")) kf.tilt = data["tilt"].toDouble();
    if (data.contains("time")) {
        kf.timeMs = data["time"].toDouble();
        sortByTime();
        emit totalDurationChanged();
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
        {"zoom", kf.zoom()},  // Derived from altitude
        {"bearing", kf.bearing},
        {"tilt", kf.tilt},
        {"time", kf.timeMs}
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

double KeyframeModel::totalDuration() const {
    if (m_keyframes.isEmpty()) return 0;
    return m_keyframes.last().timeMs;
}

void KeyframeModel::setCurrentIndex(int index) {
    if (index < 0) index = 0;
    if (index >= m_keyframes.size()) index = m_keyframes.size() - 1;

    if (m_currentIndex != index) {
        m_currentIndex = index;
        emit currentIndexChanged();
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
    if (m_currentIndex < 0 || m_currentIndex >= m_keyframes.size()) return;

    Keyframe& kf = m_keyframes[m_currentIndex];
    bool changed = false;

    if (kf.latitude != lat) { kf.latitude = lat; changed = true; }
    if (kf.longitude != lon) { kf.longitude = lon; changed = true; }
    if (kf.zoom() != zoom) { kf.setZoom(zoom); changed = true; }
    if (kf.bearing != bearing) { kf.bearing = bearing; changed = true; }
    if (kf.tilt != tilt) { kf.tilt = tilt; changed = true; }

    if (changed) {
        QModelIndex modelIndex = createIndex(m_currentIndex, 0);
        emit dataChanged(modelIndex, modelIndex);
        emit keyframeModified(m_currentIndex);
    }
}

int KeyframeModel::keyframeIndexAtTime(double timeMs) const {
    if (m_keyframes.isEmpty()) return -1;

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

    outFromIndex = keyframeIndexAtTime(timeMs);

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

int KeyframeModel::keyframeNearTime(double timeMs, double toleranceMs) const {
    for (int i = 0; i < m_keyframes.size(); i++) {
        if (qAbs(m_keyframes[i].timeMs - timeMs) <= toleranceMs) {
            return i;
        }
    }
    return -1;
}

double KeyframeModel::snapToFrame(double timeMs, int fps) {
    if (fps <= 0) fps = 30;
    double frameMs = 1000.0 / fps;
    return qRound(timeMs / frameMs) * frameMs;
}

void KeyframeModel::sortByTime() {
    std::sort(m_keyframes.begin(), m_keyframes.end(), [](const Keyframe& a, const Keyframe& b) {
        return a.timeMs < b.timeMs;
    });

    if (!m_keyframes.isEmpty()) {
        emit dataChanged(createIndex(0, 0), createIndex(m_keyframes.size() - 1, 0));
    }
}

void KeyframeModel::goToNextKeyframe() {
    if (m_keyframes.isEmpty()) return;

    int nextIndex = m_currentIndex + 1;
    if (nextIndex >= m_keyframes.size()) {
        nextIndex = m_keyframes.size() - 1;
    }

    if (nextIndex != m_currentIndex) {
        setCurrentIndex(nextIndex);
    }
}

void KeyframeModel::goToPreviousKeyframe() {
    if (m_keyframes.isEmpty()) return;

    int prevIndex = m_currentIndex - 1;
    if (prevIndex < 0) {
        prevIndex = 0;
    }

    if (prevIndex != m_currentIndex) {
        setCurrentIndex(prevIndex);
    }
}

// Multi-selection methods
bool KeyframeModel::isSelected(int index) const {
    return m_selectedIndices.contains(index);
}

void KeyframeModel::selectKeyframe(int index, bool addToSelection) {
    if (index < 0 || index >= m_keyframes.size()) return;

    if (!addToSelection) {
        m_selectedIndices.clear();
    }
    m_selectedIndices.insert(index);
    emit selectionChanged();
}

void KeyframeModel::deselectKeyframe(int index) {
    if (m_selectedIndices.remove(index)) {
        emit selectionChanged();
    }
}

void KeyframeModel::selectRange(int firstIndex, int lastIndex) {
    if (firstIndex > lastIndex) std::swap(firstIndex, lastIndex);
    firstIndex = qMax(0, firstIndex);
    lastIndex = qMin(m_keyframes.size() - 1, lastIndex);

    m_selectedIndices.clear();
    for (int i = firstIndex; i <= lastIndex; i++) {
        m_selectedIndices.insert(i);
    }
    emit selectionChanged();
}

void KeyframeModel::clearSelection() {
    if (!m_selectedIndices.isEmpty()) {
        m_selectedIndices.clear();
        emit selectionChanged();
    }
}

void KeyframeModel::moveSelectedKeyframes(double deltaTimeMs) {
    if (m_selectedIndices.isEmpty()) return;

    // Move all selected keyframes by delta
    for (int index : m_selectedIndices) {
        if (index >= 0 && index < m_keyframes.size()) {
            double newTime = qMax(0.0, m_keyframes[index].timeMs + deltaTimeMs);
            m_keyframes[index].timeMs = snapToFrame(newTime);
        }
    }

    sortByTime();
    emit totalDurationChanged();
    emit dataModified();
}

QVariantList KeyframeModel::selectedIndices() const {
    QVariantList list;
    for (int index : m_selectedIndices) {
        list.append(index);
    }
    return list;
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
