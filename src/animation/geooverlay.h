#pragma once

#include <QString>
#include <QColor>
#include <QJsonObject>
#include <QJsonArray>
#include <QPointF>
#include <QPolygonF>
#include <QVector>
#include "overlaykeyframe.h"

// Types of geographic overlays
enum class GeoOverlayType {
    Country,    // Polygon - entire country
    Region,     // Polygon - state/province
    City        // Point - city marker
};

// An animation effect applied to an overlay
struct OverlayEffect {
    QString type;               // "opacity", "extrusion", "scale", "fillColor", "borderColor"
    double startTime = 0.0;     // When effect starts (ms)
    double endTime = 10000.0;   // When effect ends (ms)
    double fadeInDuration = 500.0;
    double fadeOutDuration = 500.0;
    double value = 1.0;         // For non-color effects
    QColor color = Qt::white;   // For color effects

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["type"] = type;
        obj["startTime"] = startTime;
        obj["endTime"] = endTime;
        obj["fadeInDuration"] = fadeInDuration;
        obj["fadeOutDuration"] = fadeOutDuration;
        obj["value"] = value;
        obj["color"] = color.name(QColor::HexArgb);
        return obj;
    }

    static OverlayEffect fromJson(const QJsonObject& obj) {
        OverlayEffect effect;
        effect.type = obj["type"].toString();
        effect.startTime = obj["startTime"].toDouble();
        effect.endTime = obj["endTime"].toDouble(10000.0);
        effect.fadeInDuration = obj["fadeInDuration"].toDouble(500.0);
        effect.fadeOutDuration = obj["fadeOutDuration"].toDouble(500.0);
        effect.value = obj["value"].toDouble(1.0);
        effect.color = QColor(obj["color"].toString());
        return effect;
    }

    // Get effect intensity at a given time (0-1)
    double intensityAtTime(double timeMs) const {
        if (timeMs < startTime) return 0.0;
        if (timeMs > endTime + fadeOutDuration) return 0.0;

        // Fade in
        if (fadeInDuration > 0 && timeMs < startTime + fadeInDuration) {
            return (timeMs - startTime) / fadeInDuration;
        }

        // Fade out
        if (fadeOutDuration > 0 && timeMs > endTime) {
            double t = (timeMs - endTime) / fadeOutDuration;
            return 1.0 - t;
        }

        // Fully active
        if (timeMs <= endTime) return 1.0;

        return 0.0;
    }
};

// A geographic overlay with timeline properties
struct GeoOverlay {
    // Identity
    QString id;             // Unique ID (e.g., "country_USA", "city_Paris")
    QString code;           // ISO code or feature ID
    QString name;           // Display name
    QString parentName;     // Parent country/region name (for context)
    GeoOverlayType type = GeoOverlayType::Country;

    // Appearance
    QColor fillColor = QColor(255, 0, 0, 128);
    QColor borderColor = QColor(255, 0, 0, 255);
    double borderWidth = 2.0;

    // For cities - marker style
    double markerRadius = 8.0;
    bool showLabel = true;

    // Cached geometry (loaded from GeoJSON)
    QVector<QPolygonF> polygons;    // For countries/regions (and cities with boundaries)
    QPointF point;                   // For cities (fallback if no boundary)
    double latitude = 0.0;
    double longitude = 0.0;

    // City boundary data (stored for save/load)
    QJsonArray boundaryCoordinates;     // Raw coordinates from Nominatim
    QString boundaryGeometryType;       // "Polygon" or "MultiPolygon"
    bool hasCityBoundary = false;       // Whether boundary has been fetched

    // Timeline properties
    double startTime = 0.0;         // When this overlay starts appearing (ms)
    double fadeInDuration = 0.0;    // Fade in duration (ms) - 0 = instant
    double endTime = 0.0;           // When it starts fading out (0 = never/end of animation)
    double fadeOutDuration = 0.0;   // Fade out duration (ms) - 0 = instant

    // Legacy: Unified property keyframes (deprecated, kept for compatibility)
    QVector<OverlayKeyframe> keyframes;

    // New: Per-property keyframe tracks for independent animation
    OverlayPropertyTracks propertyTracks;

    // Animation effects
    QVector<OverlayEffect> effects;

    // Track expansion state (UI only, not saved)
    bool expanded = false;

    // Get interpolated properties at a given time
    OverlayKeyframe propertiesAtTime(double timeMs) const {
        OverlayKeyframe kf;
        kf.timeMs = timeMs;

        // Use per-property tracks if any have keyframes
        if (propertyTracks.hasAnyKeyframes()) {
            kf.opacity = OverlayPropertyTracks::interpolateValue(propertyTracks.opacity, timeMs, 1.0);
            kf.extrusion = OverlayPropertyTracks::interpolateValue(propertyTracks.extrusion, timeMs, 0.0);
            kf.scale = OverlayPropertyTracks::interpolateValue(propertyTracks.scale, timeMs, 1.0);
            kf.fillColor = OverlayPropertyTracks::interpolateColor(propertyTracks.fillColor, timeMs, fillColor);
            kf.borderColor = OverlayPropertyTracks::interpolateColor(propertyTracks.borderColor, timeMs, borderColor);
            return kf;
        }

        // Fall back to legacy unified keyframes
        if (keyframes.isEmpty()) {
            // Return default keyframe with current appearance settings
            kf.extrusion = 0.0;
            kf.fillColor = fillColor;
            kf.borderColor = borderColor;
            kf.opacity = 1.0;
            kf.scale = 1.0;
            return kf;
        }

        // Single keyframe - return its values
        if (keyframes.size() == 1) {
            kf = keyframes.first();
            kf.timeMs = timeMs;
            return kf;
        }

        // Find surrounding keyframes
        int beforeIdx = -1;
        int afterIdx = -1;

        for (int i = 0; i < keyframes.size(); ++i) {
            if (keyframes[i].timeMs <= timeMs) {
                beforeIdx = i;
            }
            if (keyframes[i].timeMs > timeMs && afterIdx < 0) {
                afterIdx = i;
            }
        }

        // Before first keyframe - use first keyframe values
        if (beforeIdx < 0) {
            kf = keyframes.first();
            kf.timeMs = timeMs;
            return kf;
        }

        // After last keyframe - use last keyframe values
        if (afterIdx < 0) {
            kf = keyframes.last();
            kf.timeMs = timeMs;
            return kf;
        }

        // Interpolate between keyframes
        const OverlayKeyframe& from = keyframes[beforeIdx];
        const OverlayKeyframe& to = keyframes[afterIdx];

        double duration = to.timeMs - from.timeMs;
        double progress = (duration > 0) ? (timeMs - from.timeMs) / duration : 0.0;

        return OverlayKeyframe::interpolate(from, to, progress);
    }

    // Calculate opacity at a given time
    double opacityAtTime(double timeMs, double totalDuration) const {
        // If endTime is 0, use totalDuration, but ensure at least visibility if totalDuration is also 0
        double effectiveEndTime = (endTime > 0) ? endTime : (totalDuration > 0 ? totalDuration : 1e12);

        // Before start - invisible
        if (timeMs < startTime) {
            return 0.0;
        }

        // During fade in (only if fadeInDuration > 0)
        if (fadeInDuration > 0 && timeMs < startTime + fadeInDuration) {
            double t = (timeMs - startTime) / fadeInDuration;
            return t;
        }

        // After fade out complete - invisible (only if fadeOutDuration > 0)
        if (fadeOutDuration > 0 && timeMs > effectiveEndTime + fadeOutDuration) {
            return 0.0;
        }

        // During fade out (only if fadeOutDuration > 0)
        if (fadeOutDuration > 0 && timeMs > effectiveEndTime) {
            double t = (timeMs - effectiveEndTime) / fadeOutDuration;
            return 1.0 - t;
        }

        // After end time with no fade - invisible
        if (endTime > 0 && timeMs > endTime) {
            return 0.0;
        }

        // Fully visible
        return 1.0;
    }

    // Serialization
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["code"] = code;
        obj["name"] = name;
        obj["parentName"] = parentName;
        obj["type"] = static_cast<int>(type);
        obj["fillColor"] = fillColor.name(QColor::HexArgb);
        obj["borderColor"] = borderColor.name(QColor::HexArgb);
        obj["borderWidth"] = borderWidth;
        obj["markerRadius"] = markerRadius;
        obj["showLabel"] = showLabel;
        obj["latitude"] = latitude;
        obj["longitude"] = longitude;
        obj["startTime"] = startTime;
        obj["fadeInDuration"] = fadeInDuration;
        obj["endTime"] = endTime;
        obj["fadeOutDuration"] = fadeOutDuration;

        // Serialize legacy keyframes (if any)
        if (!keyframes.isEmpty()) {
            QJsonArray kfArray;
            for (const auto& kf : keyframes) {
                kfArray.append(kf.toJson());
            }
            obj["keyframes"] = kfArray;
        }

        // Serialize per-property tracks
        if (propertyTracks.hasAnyKeyframes()) {
            obj["propertyTracks"] = propertyTracks.toJson();
        }

        // Serialize effects
        if (!effects.isEmpty()) {
            QJsonArray effectsArray;
            for (const auto& effect : effects) {
                effectsArray.append(effect.toJson());
            }
            obj["effects"] = effectsArray;
        }

        // Serialize city boundary if present
        if (hasCityBoundary && !boundaryCoordinates.isEmpty()) {
            obj["boundaryCoordinates"] = boundaryCoordinates;
            obj["boundaryGeometryType"] = boundaryGeometryType;
        }

        return obj;
    }

    static GeoOverlay fromJson(const QJsonObject& obj) {
        GeoOverlay overlay;
        overlay.id = obj["id"].toString();
        overlay.code = obj["code"].toString();
        overlay.name = obj["name"].toString();
        overlay.parentName = obj["parentName"].toString();
        overlay.type = static_cast<GeoOverlayType>(obj["type"].toInt());
        overlay.fillColor = QColor(obj["fillColor"].toString());
        overlay.borderColor = QColor(obj["borderColor"].toString());
        overlay.borderWidth = obj["borderWidth"].toDouble(2.0);
        overlay.markerRadius = obj["markerRadius"].toDouble(8.0);
        overlay.showLabel = obj["showLabel"].toBool(true);
        overlay.latitude = obj["latitude"].toDouble();
        overlay.longitude = obj["longitude"].toDouble();
        overlay.startTime = obj["startTime"].toDouble();
        overlay.fadeInDuration = obj["fadeInDuration"].toDouble(0.0);
        overlay.endTime = obj["endTime"].toDouble();
        overlay.fadeOutDuration = obj["fadeOutDuration"].toDouble(0.0);

        // Deserialize legacy keyframes
        if (obj.contains("keyframes")) {
            QJsonArray kfArray = obj["keyframes"].toArray();
            for (const auto& kfVal : kfArray) {
                overlay.keyframes.append(OverlayKeyframe::fromJson(kfVal.toObject()));
            }
        }

        // Deserialize per-property tracks
        if (obj.contains("propertyTracks")) {
            overlay.propertyTracks = OverlayPropertyTracks::fromJson(obj["propertyTracks"].toObject());
        }

        // Deserialize effects
        if (obj.contains("effects")) {
            QJsonArray effectsArray = obj["effects"].toArray();
            for (const auto& effectVal : effectsArray) {
                overlay.effects.append(OverlayEffect::fromJson(effectVal.toObject()));
            }
        }

        // Deserialize city boundary if present
        if (obj.contains("boundaryCoordinates")) {
            overlay.boundaryCoordinates = obj["boundaryCoordinates"].toArray();
            overlay.boundaryGeometryType = obj["boundaryGeometryType"].toString();
            overlay.hasCityBoundary = !overlay.boundaryCoordinates.isEmpty();
        }

        return overlay;
    }

    QString typeString() const {
        switch (type) {
            case GeoOverlayType::Country: return "Country";
            case GeoOverlayType::Region: return "Region";
            case GeoOverlayType::City: return "City";
        }
        return "Unknown";
    }
};
