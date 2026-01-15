#include "overlaymanager.h"
#include <QJsonArray>

OverlayManager::OverlayManager(QObject* parent)
    : QAbstractListModel(parent)
{
}

int OverlayManager::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return m_overlays.size();
}

QVariant OverlayManager::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_overlays.size())
        return QVariant();

    Overlay* overlay = m_overlays.at(index.row()).get();

    switch (role) {
        case IdRole: return overlay->id();
        case TypeRole: return overlay->typeInt();
        case NameRole: return overlay->name();
        case VisibleRole: return overlay->isVisible();
        case OverlayRole: return QVariant::fromValue(overlay);
        default: return QVariant();
    }
}

bool OverlayManager::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() >= m_overlays.size())
        return false;

    Overlay* overlay = m_overlays.at(index.row()).get();

    switch (role) {
        case NameRole:
            overlay->setName(value.toString());
            emit dataChanged(index, index, {role});
            emit dataModified();
            return true;
        case VisibleRole:
            overlay->setVisible(value.toBool());
            emit dataChanged(index, index, {role});
            emit dataModified();
            return true;
        default:
            return false;
    }
}

QHash<int, QByteArray> OverlayManager::roleNames() const {
    return {
        {IdRole, "overlayId"},
        {TypeRole, "overlayType"},
        {NameRole, "name"},
        {VisibleRole, "visible"},
        {OverlayRole, "overlay"}
    };
}

void OverlayManager::setSelectedIndex(int index) {
    if (index < -1) index = -1;
    if (index >= m_overlays.size()) index = m_overlays.size() - 1;

    if (m_selectedIndex != index) {
        m_selectedIndex = index;
        emit selectedIndexChanged();
    }
}

MarkerOverlay* OverlayManager::createMarker(double lat, double lon) {
    auto marker = new MarkerOverlay(this);
    marker->setLatitude(lat);
    marker->setLongitude(lon);
    addOverlay(marker);
    return marker;
}

ArrowOverlay* OverlayManager::createArrow(double startLat, double startLon, double endLat, double endLon) {
    auto arrow = new ArrowOverlay(this);
    arrow->setStartLat(startLat);
    arrow->setStartLon(startLon);
    arrow->setEndLat(endLat);
    arrow->setEndLon(endLon);
    addOverlay(arrow);
    return arrow;
}

TextOverlay* OverlayManager::createText(double lat, double lon, const QString& text) {
    auto textOverlay = new TextOverlay(this);
    textOverlay->setLatitude(lat);
    textOverlay->setLongitude(lon);
    textOverlay->setText(text);
    addOverlay(textOverlay);
    return textOverlay;
}

RegionHighlight* OverlayManager::createRegionHighlight(const QString& regionCode) {
    auto region = new RegionHighlight(this);
    region->setRegionCode(regionCode);
    region->setName(regionCode);
    addOverlay(region);
    return region;
}

void OverlayManager::addOverlay(Overlay* overlay) {
    beginInsertRows(QModelIndex(), m_overlays.size(), m_overlays.size());
    m_overlays.push_back(std::unique_ptr<Overlay>(overlay));

    connect(overlay, &Overlay::modified, this, [this]() {
        emit dataModified();
    });

    endInsertRows();
    emit countChanged();
    emit overlayAdded(overlay);
    emit dataModified();
}

void OverlayManager::removeOverlay(int index) {
    if (index < 0 || index >= m_overlays.size()) return;

    QString id = m_overlays[index]->id();

    beginRemoveRows(QModelIndex(), index, index);
    m_overlays.erase(m_overlays.begin() + index);
    endRemoveRows();

    if (m_selectedIndex >= m_overlays.size()) {
        setSelectedIndex(m_overlays.size() - 1);
    }

    emit countChanged();
    emit overlayRemoved(id);
    emit dataModified();
}

void OverlayManager::removeOverlayById(const QString& id) {
    for (int i = 0; i < m_overlays.size(); i++) {
        if (m_overlays[i]->id() == id) {
            removeOverlay(i);
            return;
        }
    }
}

void OverlayManager::moveOverlay(int from, int to) {
    if (from < 0 || from >= m_overlays.size()) return;
    if (to < 0 || to >= m_overlays.size()) return;
    if (from == to) return;

    beginMoveRows(QModelIndex(), from, from, QModelIndex(), to > from ? to + 1 : to);

    auto overlay = std::move(m_overlays[from]);
    m_overlays.erase(m_overlays.begin() + from);
    m_overlays.insert(m_overlays.begin() + to, std::move(overlay));

    endMoveRows();
    emit dataModified();
}

void OverlayManager::duplicateOverlay(int index) {
    if (index < 0 || index >= m_overlays.size()) return;

    Overlay* original = m_overlays[index].get();
    QJsonObject json = original->toJson();

    // Create new overlay based on type
    Overlay* copy = nullptr;
    switch (original->type()) {
        case OverlayType::Marker:
            copy = new MarkerOverlay(this);
            break;
        case OverlayType::Arrow:
            copy = new ArrowOverlay(this);
            break;
        case OverlayType::Text:
            copy = new TextOverlay(this);
            break;
        case OverlayType::RegionHighlight:
            copy = new RegionHighlight(this);
            break;
    }

    if (copy) {
        copy->fromJson(json);
        copy->setName(original->name() + " Copy");
        addOverlay(copy);
    }
}

Overlay* OverlayManager::getOverlay(int index) const {
    if (index < 0 || index >= m_overlays.size()) return nullptr;
    return m_overlays[index].get();
}

Overlay* OverlayManager::getOverlayById(const QString& id) const {
    for (const auto& overlay : m_overlays) {
        if (overlay->id() == id) {
            return overlay.get();
        }
    }
    return nullptr;
}

void OverlayManager::clear() {
    beginResetModel();
    m_overlays.clear();
    m_selectedIndex = -1;
    endResetModel();

    emit countChanged();
    emit selectedIndexChanged();
    emit dataModified();
}

MarkerOverlay* OverlayManager::getMarker(int index) const {
    Overlay* o = getOverlay(index);
    if (o && o->type() == OverlayType::Marker) {
        return static_cast<MarkerOverlay*>(o);
    }
    return nullptr;
}

ArrowOverlay* OverlayManager::getArrow(int index) const {
    Overlay* o = getOverlay(index);
    if (o && o->type() == OverlayType::Arrow) {
        return static_cast<ArrowOverlay*>(o);
    }
    return nullptr;
}

TextOverlay* OverlayManager::getText(int index) const {
    Overlay* o = getOverlay(index);
    if (o && o->type() == OverlayType::Text) {
        return static_cast<TextOverlay*>(o);
    }
    return nullptr;
}

RegionHighlight* OverlayManager::getRegionHighlight(int index) const {
    Overlay* o = getOverlay(index);
    if (o && o->type() == OverlayType::RegionHighlight) {
        return static_cast<RegionHighlight*>(o);
    }
    return nullptr;
}

QVector<Overlay*> OverlayManager::visibleOverlaysAtTime(double timeMs) const {
    QVector<Overlay*> result;
    for (const auto& overlay : m_overlays) {
        if (overlay->isVisibleAtTime(timeMs)) {
            result.append(overlay.get());
        }
    }
    return result;
}

QJsonArray OverlayManager::toJson() const {
    QJsonArray array;
    for (const auto& overlay : m_overlays) {
        array.append(overlay->toJson());
    }
    return array;
}

void OverlayManager::fromJson(const QJsonArray& array) {
    beginResetModel();
    m_overlays.clear();

    for (const auto& val : array) {
        QJsonObject obj = val.toObject();
        int type = obj["type"].toInt();

        Overlay* overlay = nullptr;
        switch (static_cast<OverlayType>(type)) {
            case OverlayType::Marker:
                overlay = new MarkerOverlay(this);
                break;
            case OverlayType::Arrow:
                overlay = new ArrowOverlay(this);
                break;
            case OverlayType::Text:
                overlay = new TextOverlay(this);
                break;
            case OverlayType::RegionHighlight:
                overlay = new RegionHighlight(this);
                break;
        }

        if (overlay) {
            overlay->fromJson(obj);
            connect(overlay, &Overlay::modified, this, [this]() {
                emit dataModified();
            });
            m_overlays.push_back(std::unique_ptr<Overlay>(overlay));
        }
    }

    endResetModel();
    emit countChanged();
}
