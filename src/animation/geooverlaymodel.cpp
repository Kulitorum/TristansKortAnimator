#include "geooverlaymodel.h"
#include "../map/geojsonparser.h"
#include "../map/cityboundaryfetcher.h"
#include <QJsonArray>
#include <QUuid>
#include <QFile>
#include <QTextStream>
#include <algorithm>

// Helper function to convert Nominatim coordinates to QPolygonF
// GeoJSON format: [lon, lat] - we store as QPointF(lat, lon) for consistency
QVector<QPolygonF> parseNominatimCoordinates(const QJsonArray& coordinates, const QString& geometryType) {
    QVector<QPolygonF> result;

    auto parseRing = [](const QJsonArray& ring) -> QPolygonF {
        QPolygonF polygon;
        for (const auto& pointVal : ring) {
            QJsonArray point = pointVal.toArray();
            if (point.size() >= 2) {
                double lon = point[0].toDouble();
                double lat = point[1].toDouble();
                polygon.append(QPointF(lat, lon));  // Store as (lat, lon)
            }
        }
        return polygon;
    };

    if (geometryType == "Polygon") {
        // Polygon: coordinates is [[ring1], [ring2], ...]
        // We only use the outer ring (first one)
        if (!coordinates.isEmpty()) {
            QJsonArray outerRing = coordinates[0].toArray();
            QPolygonF poly = parseRing(outerRing);
            if (!poly.isEmpty()) {
                result.append(poly);
            }
        }
    } else if (geometryType == "MultiPolygon") {
        // MultiPolygon: coordinates is [[[ring1], ...], [[ring2], ...], ...]
        for (const auto& polygonVal : coordinates) {
            QJsonArray polygonCoords = polygonVal.toArray();
            if (!polygonCoords.isEmpty()) {
                QJsonArray outerRing = polygonCoords[0].toArray();
                QPolygonF poly = parseRing(outerRing);
                if (!poly.isEmpty()) {
                    result.append(poly);
                }
            }
        }
    }

    return result;
}

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
        case KeyframeCountRole: return overlay.keyframes.size();
        case CurrentExtrusionRole: {
            OverlayKeyframe props = overlay.propertiesAtTime(m_currentTime);
            return props.extrusion;
        }
        case CurrentFillColorRole: {
            OverlayKeyframe props = overlay.propertiesAtTime(m_currentTime);
            return props.fillColor;
        }
        case CurrentBorderColorRole: {
            OverlayKeyframe props = overlay.propertiesAtTime(m_currentTime);
            return props.borderColor;
        }
        case CurrentOpacityRole: {
            OverlayKeyframe props = overlay.propertiesAtTime(m_currentTime);
            return props.opacity;
        }
        case CurrentScaleRole: {
            OverlayKeyframe props = overlay.propertiesAtTime(m_currentTime);
            return props.scale;
        }
        case PolygonsRole: {
            QVariantList polygonList;
            for (const auto& polygon : overlay.polygons) {
                QVariantList points;
                for (const auto& pt : polygon) {
                    points.append(QVariantMap{{"x", pt.x()}, {"y", pt.y()}});
                }
                polygonList.append(QVariant(points));
            }
            return polygonList;
        }
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
        {FadeOutDurationRole, "fadeOutDuration"},
        {KeyframeCountRole, "keyframeCount"},
        {CurrentExtrusionRole, "currentExtrusion"},
        {CurrentFillColorRole, "currentFillColor"},
        {CurrentBorderColorRole, "currentBorderColor"},
        {CurrentOpacityRole, "currentOpacity"},
        {CurrentScaleRole, "currentScale"},
        {PolygonsRole, "polygons"}
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
    if (!m_geoJson) {
        qWarning() << "GeoOverlayModel::loadGeometryForOverlay: m_geoJson is null!";
        return;
    }

    if (overlay.type == GeoOverlayType::City) {
        // Cities are points - geometry already set from lat/lon
        overlay.point = QPointF(overlay.longitude, overlay.latitude);
    } else {
        // Countries and regions - load polygons from GeoJSON
        overlay.polygons = m_geoJson->getPolygonsForFeature(overlay.code, overlay.name);
        if (overlay.polygons.isEmpty()) {
            qWarning() << "GeoOverlayModel: No polygons found for" << overlay.name << "code=" << overlay.code;
        }
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
    overlay.endTime = startTime + 10000.0;  // Default 10 second duration from start
    overlay.fadeInDuration = 500.0;   // Default 0.5s fade in
    overlay.fadeOutDuration = 500.0;  // Default 0.5s fade out

    // Default country colors - border only (no fill)
    overlay.fillColor = QColor(0, 0, 0, 0);  // Transparent fill
    overlay.borderColor = QColor(255, 255, 255, 255);  // White border
    overlay.borderWidth = 3.0;

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
    overlay.endTime = startTime + 10000.0;  // Default 10 second duration from start
    overlay.fadeInDuration = 500.0;   // Default 0.5s fade in
    overlay.fadeOutDuration = 500.0;  // Default 0.5s fade out

    // Default region colors - border only (no fill)
    overlay.fillColor = QColor(0, 0, 0, 0);  // Transparent fill
    overlay.borderColor = QColor(255, 255, 255, 255);  // White border
    overlay.borderWidth = 2.5;

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
    overlay.endTime = startTime + 10000.0;  // Default 10 second duration from start
    overlay.fadeInDuration = 300.0;   // Default 0.3s fade in (faster for cities)
    overlay.fadeOutDuration = 300.0;  // Default 0.3s fade out

    // Default city colors - ring style (no fill)
    overlay.fillColor = QColor(0, 0, 0, 0);  // Transparent fill
    overlay.borderColor = QColor(255, 255, 255, 255);  // White border
    overlay.markerRadius = 8.0;
    overlay.showLabel = true;

    m_overlays.append(overlay);

    endInsertRows();
    emit countChanged();
    emit dataModified();

    // Fetch city boundary from Nominatim (async)
    if (m_boundaryFetcher) {
        qDebug() << "Fetching boundary for city:" << name << "," << countryName;
        m_boundaryFetcher->fetchBoundary(name, countryName);
    }
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

        // For cities with cached boundary data, rebuild the polygons
        if (overlay.type == GeoOverlayType::City && overlay.hasCityBoundary) {
            loadCityBoundaryFromCache(overlay);
        }

        m_overlays.append(overlay);
    }

    endResetModel();
    emit countChanged();
}

// Keyframe management
void GeoOverlayModel::sortKeyframes(int overlayIndex) {
    if (overlayIndex < 0 || overlayIndex >= m_overlays.size()) return;

    auto& keyframes = m_overlays[overlayIndex].keyframes;
    std::sort(keyframes.begin(), keyframes.end(),
              [](const OverlayKeyframe& a, const OverlayKeyframe& b) {
                  return a.timeMs < b.timeMs;
              });
}

int GeoOverlayModel::addKeyframe(int overlayIndex, double timeMs) {
    if (overlayIndex < 0 || overlayIndex >= m_overlays.size()) return -1;

    GeoOverlay& overlay = m_overlays[overlayIndex];

    // Create new keyframe with default or interpolated values
    OverlayKeyframe kf;
    if (overlay.keyframes.isEmpty()) {
        // First keyframe - use overlay's current appearance
        kf.timeMs = timeMs;
        kf.extrusion = 0.0;
        kf.fillColor = overlay.fillColor;
        kf.borderColor = overlay.borderColor;
        kf.opacity = 1.0;
        kf.scale = 1.0;
    } else {
        // Get interpolated values at this time
        kf = overlay.propertiesAtTime(timeMs);
        kf.timeMs = timeMs;
    }

    overlay.keyframes.append(kf);
    sortKeyframes(overlayIndex);

    // Find the index of the newly added keyframe
    int kfIndex = 0;
    for (int i = 0; i < overlay.keyframes.size(); ++i) {
        if (qAbs(overlay.keyframes[i].timeMs - timeMs) < 0.01) {
            kfIndex = i;
            break;
        }
    }

    QModelIndex modelIndex = createIndex(overlayIndex, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit keyframeAdded(overlayIndex, kfIndex);
    emit dataModified();

    return kfIndex;
}

void GeoOverlayModel::updateKeyframe(int overlayIndex, int keyframeIndex, const QVariantMap& data) {
    if (overlayIndex < 0 || overlayIndex >= m_overlays.size()) return;

    auto& keyframes = m_overlays[overlayIndex].keyframes;
    if (keyframeIndex < 0 || keyframeIndex >= keyframes.size()) return;

    OverlayKeyframe& kf = keyframes[keyframeIndex];

    if (data.contains("timeMs")) kf.timeMs = data["timeMs"].toDouble();
    if (data.contains("extrusion")) kf.extrusion = data["extrusion"].toDouble();
    if (data.contains("fillColor")) kf.fillColor = data["fillColor"].value<QColor>();
    if (data.contains("borderColor")) kf.borderColor = data["borderColor"].value<QColor>();
    if (data.contains("opacity")) kf.opacity = data["opacity"].toDouble();
    if (data.contains("scale")) kf.scale = data["scale"].toDouble();
    if (data.contains("easingType")) {
        kf.easingTypeInt = data["easingType"].toInt();
        kf.syncEnumFromInt();
    }

    // Re-sort if time changed
    if (data.contains("timeMs")) {
        sortKeyframes(overlayIndex);
    }

    QModelIndex modelIndex = createIndex(overlayIndex, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit keyframeModified(overlayIndex, keyframeIndex);
    emit dataModified();
}

void GeoOverlayModel::removeKeyframe(int overlayIndex, int keyframeIndex) {
    if (overlayIndex < 0 || overlayIndex >= m_overlays.size()) return;

    auto& keyframes = m_overlays[overlayIndex].keyframes;
    if (keyframeIndex < 0 || keyframeIndex >= keyframes.size()) return;

    keyframes.remove(keyframeIndex);

    QModelIndex modelIndex = createIndex(overlayIndex, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit keyframeRemoved(overlayIndex, keyframeIndex);
    emit dataModified();
}

void GeoOverlayModel::moveKeyframe(int overlayIndex, int keyframeIndex, double newTimeMs) {
    if (overlayIndex < 0 || overlayIndex >= m_overlays.size()) return;

    auto& keyframes = m_overlays[overlayIndex].keyframes;
    if (keyframeIndex < 0 || keyframeIndex >= keyframes.size()) return;

    keyframes[keyframeIndex].timeMs = newTimeMs;
    sortKeyframes(overlayIndex);

    QModelIndex modelIndex = createIndex(overlayIndex, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit keyframeModified(overlayIndex, keyframeIndex);
    emit dataModified();
}

QVariantMap GeoOverlayModel::getKeyframe(int overlayIndex, int keyframeIndex) const {
    if (overlayIndex < 0 || overlayIndex >= m_overlays.size()) return {};

    const auto& keyframes = m_overlays[overlayIndex].keyframes;
    if (keyframeIndex < 0 || keyframeIndex >= keyframes.size()) return {};

    const OverlayKeyframe& kf = keyframes[keyframeIndex];
    return {
        {"timeMs", kf.timeMs},
        {"extrusion", kf.extrusion},
        {"fillColor", kf.fillColor},
        {"borderColor", kf.borderColor},
        {"opacity", kf.opacity},
        {"scale", kf.scale},
        {"easingType", kf.easingTypeInt}
    };
}

int GeoOverlayModel::keyframeCount(int overlayIndex) const {
    if (overlayIndex < 0 || overlayIndex >= m_overlays.size()) return 0;
    return m_overlays[overlayIndex].keyframes.size();
}

QVariantList GeoOverlayModel::getAllKeyframes(int overlayIndex) const {
    if (overlayIndex < 0 || overlayIndex >= m_overlays.size()) return {};

    QVariantList result;
    for (const auto& kf : m_overlays[overlayIndex].keyframes) {
        result.append(QVariantMap{
            {"timeMs", kf.timeMs},
            {"extrusion", kf.extrusion},
            {"fillColor", kf.fillColor},
            {"borderColor", kf.borderColor},
            {"opacity", kf.opacity},
            {"scale", kf.scale},
            {"easingType", kf.easingTypeInt}
        });
    }
    return result;
}

QVariantMap GeoOverlayModel::propertiesAtTime(int overlayIndex, double timeMs) const {
    if (overlayIndex < 0 || overlayIndex >= m_overlays.size()) return {};

    OverlayKeyframe kf = m_overlays[overlayIndex].propertiesAtTime(timeMs);
    return {
        {"timeMs", kf.timeMs},
        {"extrusion", kf.extrusion},
        {"fillColor", kf.fillColor},
        {"borderColor", kf.borderColor},
        {"opacity", kf.opacity},
        {"scale", kf.scale}
    };
}

void GeoOverlayModel::setCurrentTime(double timeMs) {
    if (qAbs(m_currentTime - timeMs) < 0.01) return;

    m_currentTime = timeMs;

    // Notify that animated properties may have changed
    if (!m_overlays.isEmpty()) {
        emit dataChanged(createIndex(0, 0), createIndex(m_overlays.size() - 1, 0),
                         {CurrentExtrusionRole, CurrentFillColorRole,
                          CurrentBorderColorRole, CurrentOpacityRole, CurrentScaleRole});
    }
    emit currentTimeChanged();
}

void GeoOverlayModel::setCityBoundaryFetcher(CityBoundaryFetcher* fetcher) {
    if (m_boundaryFetcher) {
        disconnect(m_boundaryFetcher, nullptr, this, nullptr);
    }
    m_boundaryFetcher = fetcher;
    if (m_boundaryFetcher) {
        connect(m_boundaryFetcher, &CityBoundaryFetcher::boundaryReady,
                this, &GeoOverlayModel::onBoundaryReady);
        connect(m_boundaryFetcher, &CityBoundaryFetcher::fetchFailed,
                this, &GeoOverlayModel::onBoundaryFetchFailed);
    }
}

void GeoOverlayModel::onBoundaryReady(const QString& cityName, const QJsonArray& coordinates, const QString& geometryType) {
    // Find the overlay for this city and update its polygon data
    for (int i = 0; i < m_overlays.size(); ++i) {
        GeoOverlay& overlay = m_overlays[i];
        if (overlay.type == GeoOverlayType::City && overlay.name == cityName) {
            // Store the raw JSON data for serialization
            overlay.boundaryCoordinates = coordinates;
            overlay.boundaryGeometryType = geometryType;
            overlay.hasCityBoundary = true;

            // Convert to polygons for rendering
            overlay.polygons = parseNominatimCoordinates(coordinates, geometryType);

            qDebug() << "Loaded boundary for" << cityName << "with" << overlay.polygons.size() << "polygons";

            // Notify of change
            QModelIndex modelIndex = createIndex(i, 0);
            emit dataChanged(modelIndex, modelIndex);
            emit overlayModified(i);
            emit dataModified();
            break;
        }
    }
}

void GeoOverlayModel::onBoundaryFetchFailed(const QString& cityName, const QString& error) {
    qWarning() << "Failed to fetch boundary for" << cityName << ":" << error;
    // City will continue to render as a circle marker
}

void GeoOverlayModel::loadCityBoundaryFromCache(GeoOverlay& overlay) {
    // If we have cached boundary data, convert it to polygons
    if (overlay.hasCityBoundary && !overlay.boundaryCoordinates.isEmpty()) {
        overlay.polygons = parseNominatimCoordinates(overlay.boundaryCoordinates, overlay.boundaryGeometryType);
        qDebug() << "Loaded cached boundary for" << overlay.name << "with" << overlay.polygons.size() << "polygons";
    }
}
