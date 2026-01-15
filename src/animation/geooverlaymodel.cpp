#include "geooverlaymodel.h"
#include "../map/geojsonparser.h"
#include <QJsonArray>
#include <QUuid>

GeoOverlayModel::GeoOverlayModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int GeoOverlayModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return m_overlays.size();
}

QVariant GeoOverlayModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_overlays.size())
        return QVariant();

    const GeoOverlay& overlay = m_overlays.at(index.row());

    switch (role) {
        case IdRole: return overlay.id;
        case CodeRole: return overlay.code;
        case NameRole: return overlay.name;
        case ParentNameRole: return overlay.parentName;
        case TypeRole: return static_cast<int>(overlay.type);
        case TypeStringRole: return overlay.typeString();
        case FillColorRole: return overlay.fillColor;
        case BorderColorRole: return overlay.borderColor;
        case BorderWidthRole: return overlay.borderWidth;
        case MarkerRadiusRole: return overlay.markerRadius;
        case ShowLabelRole: return overlay.showLabel;
        case LatitudeRole: return overlay.latitude;
        case LongitudeRole: return overlay.longitude;
        case StartTimeRole: return overlay.startTime;
        case FadeInDurationRole: return overlay.fadeInDuration;
        case EndTimeRole: return overlay.endTime;
        case FadeOutDurationRole: return overlay.fadeOutDuration;
        default: return QVariant();
    }
}

bool GeoOverlayModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() >= m_overlays.size())
        return false;

    GeoOverlay& overlay = m_overlays[index.row()];
    bool changed = false;

    switch (role) {
        case FillColorRole:
            if (overlay.fillColor != value.value<QColor>()) {
                overlay.fillColor = value.value<QColor>();
                changed = true;
            }
            break;
        case BorderColorRole:
            if (overlay.borderColor != value.value<QColor>()) {
                overlay.borderColor = value.value<QColor>();
                changed = true;
            }
            break;
        case BorderWidthRole:
            if (overlay.borderWidth != value.toDouble()) {
                overlay.borderWidth = value.toDouble();
                changed = true;
            }
            break;
        case MarkerRadiusRole:
            if (overlay.markerRadius != value.toDouble()) {
                overlay.markerRadius = value.toDouble();
                changed = true;
            }
            break;
        case ShowLabelRole:
            if (overlay.showLabel != value.toBool()) {
                overlay.showLabel = value.toBool();
                changed = true;
            }
            break;
        case StartTimeRole:
            if (overlay.startTime != value.toDouble()) {
                overlay.startTime = value.toDouble();
                changed = true;
            }
            break;
        case FadeInDurationRole:
            if (overlay.fadeInDuration != value.toDouble()) {
                overlay.fadeInDuration = value.toDouble();
                changed = true;
            }
            break;
        case EndTimeRole:
            if (overlay.endTime != value.toDouble()) {
                overlay.endTime = value.toDouble();
                changed = true;
            }
            break;
        case FadeOutDurationRole:
            if (overlay.fadeOutDuration != value.toDouble()) {
                overlay.fadeOutDuration = value.toDouble();
                changed = true;
            }
            break;
    }

    if (changed) {
        emit dataChanged(index, index, {role});
        emit overlayModified(index.row());
        emit dataModified();
    }

    return changed;
}

QHash<int, QByteArray> GeoOverlayModel::roleNames() const {
    return {
        {IdRole, "overlayId"},
        {CodeRole, "code"},
        {NameRole, "name"},
        {ParentNameRole, "parentName"},
        {TypeRole, "overlayType"},
        {TypeStringRole, "typeString"},
        {FillColorRole, "fillColor"},
        {BorderColorRole, "borderColor"},
        {BorderWidthRole, "borderWidth"},
        {MarkerRadiusRole, "markerRadius"},
        {ShowLabelRole, "showLabel"},
        {LatitudeRole, "latitude"},
        {LongitudeRole, "longitude"},
        {StartTimeRole, "startTime"},
        {FadeInDurationRole, "fadeInDuration"},
        {EndTimeRole, "endTime"},
        {FadeOutDurationRole, "fadeOutDuration"}
    };
}

QString GeoOverlayModel::generateId(GeoOverlayType type, const QString& name) {
    QString prefix;
    switch (type) {
        case GeoOverlayType::Country: prefix = "country"; break;
        case GeoOverlayType::Region: prefix = "region"; break;
        case GeoOverlayType::City: prefix = "city"; break;
    }
    // Add short UUID to ensure uniqueness
    QString uuid = QUuid::createUuid().toString(QUuid::Id128).left(8);
    return QString("%1_%2_%3").arg(prefix, name.simplified().replace(' ', '_'), uuid);
}

void GeoOverlayModel::loadGeometryForOverlay(GeoOverlay& overlay) {
    if (!m_geoJson) return;

    if (overlay.type == GeoOverlayType::City) {
        // Cities are points - geometry already set from lat/lon
        overlay.point = QPointF(overlay.longitude, overlay.latitude);
    } else {
        // Countries and regions - load polygons from GeoJSON
        overlay.polygons = m_geoJson->getPolygonsForFeature(overlay.code, overlay.name);
    }
}

void GeoOverlayModel::addCountry(const QString& code, const QString& name, double startTime) {
    beginInsertRows(QModelIndex(), m_overlays.size(), m_overlays.size());

    GeoOverlay overlay;
    overlay.id = generateId(GeoOverlayType::Country, name);
    overlay.code = code;
    overlay.name = name;
    overlay.type = GeoOverlayType::Country;
    overlay.startTime = startTime;

    // Default country colors
    overlay.fillColor = QColor(255, 100, 100, 128);
    overlay.borderColor = QColor(255, 50, 50, 255);
    overlay.borderWidth = 2.0;

    loadGeometryForOverlay(overlay);
    m_overlays.append(overlay);

    endInsertRows();
    emit countChanged();
    emit dataModified();
}

void GeoOverlayModel::addRegion(const QString& code, const QString& name,
                                 const QString& countryName, double startTime) {
    beginInsertRows(QModelIndex(), m_overlays.size(), m_overlays.size());

    GeoOverlay overlay;
    overlay.id = generateId(GeoOverlayType::Region, name);
    overlay.code = code;
    overlay.name = name;
    overlay.parentName = countryName;
    overlay.type = GeoOverlayType::Region;
    overlay.startTime = startTime;

    // Default region colors
    overlay.fillColor = QColor(100, 100, 255, 128);
    overlay.borderColor = QColor(50, 50, 255, 255);
    overlay.borderWidth = 1.5;

    loadGeometryForOverlay(overlay);
    m_overlays.append(overlay);

    endInsertRows();
    emit countChanged();
    emit dataModified();
}

void GeoOverlayModel::addCity(const QString& name, const QString& countryName,
                               double lat, double lon, double startTime) {
    beginInsertRows(QModelIndex(), m_overlays.size(), m_overlays.size());

    GeoOverlay overlay;
    overlay.id = generateId(GeoOverlayType::City, name);
    overlay.name = name;
    overlay.parentName = countryName;
    overlay.type = GeoOverlayType::City;
    overlay.latitude = lat;
    overlay.longitude = lon;
    overlay.point = QPointF(lon, lat);
    overlay.startTime = startTime;

    // Default city colors
    overlay.fillColor = QColor(255, 200, 0, 200);
    overlay.borderColor = QColor(200, 150, 0, 255);
    overlay.markerRadius = 8.0;
    overlay.showLabel = true;

    m_overlays.append(overlay);

    endInsertRows();
    emit countChanged();
    emit dataModified();
}

void GeoOverlayModel::removeOverlay(int index) {
    if (index < 0 || index >= m_overlays.size()) return;

    beginRemoveRows(QModelIndex(), index, index);
    m_overlays.remove(index);
    endRemoveRows();

    emit countChanged();
    emit dataModified();
}

void GeoOverlayModel::updateOverlay(int index, const QVariantMap& data) {
    if (index < 0 || index >= m_overlays.size()) return;

    GeoOverlay& overlay = m_overlays[index];

    if (data.contains("fillColor")) overlay.fillColor = data["fillColor"].value<QColor>();
    if (data.contains("borderColor")) overlay.borderColor = data["borderColor"].value<QColor>();
    if (data.contains("borderWidth")) overlay.borderWidth = data["borderWidth"].toDouble();
    if (data.contains("markerRadius")) overlay.markerRadius = data["markerRadius"].toDouble();
    if (data.contains("showLabel")) overlay.showLabel = data["showLabel"].toBool();
    if (data.contains("startTime")) overlay.startTime = data["startTime"].toDouble();
    if (data.contains("fadeInDuration")) overlay.fadeInDuration = data["fadeInDuration"].toDouble();
    if (data.contains("endTime")) overlay.endTime = data["endTime"].toDouble();
    if (data.contains("fadeOutDuration")) overlay.fadeOutDuration = data["fadeOutDuration"].toDouble();

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit overlayModified(index);
    emit dataModified();
}

QVariantMap GeoOverlayModel::getOverlay(int index) const {
    if (index < 0 || index >= m_overlays.size())
        return QVariantMap();

    const GeoOverlay& overlay = m_overlays.at(index);
    return {
        {"id", overlay.id},
        {"code", overlay.code},
        {"name", overlay.name},
        {"parentName", overlay.parentName},
        {"type", static_cast<int>(overlay.type)},
        {"typeString", overlay.typeString()},
        {"fillColor", overlay.fillColor},
        {"borderColor", overlay.borderColor},
        {"borderWidth", overlay.borderWidth},
        {"markerRadius", overlay.markerRadius},
        {"showLabel", overlay.showLabel},
        {"latitude", overlay.latitude},
        {"longitude", overlay.longitude},
        {"startTime", overlay.startTime},
        {"fadeInDuration", overlay.fadeInDuration},
        {"endTime", overlay.endTime},
        {"fadeOutDuration", overlay.fadeOutDuration}
    };
}

void GeoOverlayModel::clear() {
    beginResetModel();
    m_overlays.clear();
    endResetModel();

    emit countChanged();
    emit dataModified();
}

void GeoOverlayModel::moveOverlay(int from, int to) {
    if (from < 0 || from >= m_overlays.size()) return;
    if (to < 0 || to >= m_overlays.size()) return;
    if (from == to) return;

    // Qt model move semantics
    int destRow = to > from ? to + 1 : to;
    beginMoveRows(QModelIndex(), from, from, QModelIndex(), destRow);
    m_overlays.move(from, to);
    endMoveRows();

    emit dataModified();
}

void GeoOverlayModel::setOverlayTiming(int index, double startTime, double fadeIn,
                                        double endTime, double fadeOut) {
    if (index < 0 || index >= m_overlays.size()) return;

    GeoOverlay& overlay = m_overlays[index];
    overlay.startTime = qMax(0.0, startTime);
    overlay.fadeInDuration = qMax(0.0, fadeIn);
    overlay.endTime = qMax(0.0, endTime);
    overlay.fadeOutDuration = qMax(0.0, fadeOut);

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit overlayModified(index);
    emit dataModified();
}

void GeoOverlayModel::setOverlayColors(int index, const QColor& fillColor,
                                        const QColor& borderColor, double borderWidth) {
    if (index < 0 || index >= m_overlays.size()) return;

    GeoOverlay& overlay = m_overlays[index];
    overlay.fillColor = fillColor;
    overlay.borderColor = borderColor;
    overlay.borderWidth = qBound(0.0, borderWidth, 10.0);

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit overlayModified(index);
    emit dataModified();
}

double GeoOverlayModel::overlayOpacityAtTime(int index, double timeMs, double totalDuration) const {
    if (index < 0 || index >= m_overlays.size()) return 0.0;
    return m_overlays.at(index).opacityAtTime(timeMs, totalDuration);
}

QVector<QPair<const GeoOverlay*, double>> GeoOverlayModel::visibleOverlaysAtTime(
        double timeMs, double totalDuration) const {
    QVector<QPair<const GeoOverlay*, double>> result;

    for (const auto& overlay : m_overlays) {
        double opacity = overlay.opacityAtTime(timeMs, totalDuration);
        if (opacity > 0.0) {
            result.append({&overlay, opacity});
        }
    }

    return result;
}

QJsonArray GeoOverlayModel::toJson() const {
    QJsonArray array;
    for (const auto& overlay : m_overlays) {
        array.append(overlay.toJson());
    }
    return array;
}

void GeoOverlayModel::fromJson(const QJsonArray& array) {
    beginResetModel();
    m_overlays.clear();

    for (const auto& val : array) {
        GeoOverlay overlay = GeoOverlay::fromJson(val.toObject());
        loadGeometryForOverlay(overlay);
        m_overlays.append(overlay);
    }

    endResetModel();
    emit countChanged();
}
