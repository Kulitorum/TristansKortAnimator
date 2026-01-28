#include "overlaykeyframe.h"
#include "easingfunctions.h"
#include <QJsonArray>

// Helper to interpolate colors
static QColor lerpColor(const QColor& from, const QColor& to, double t) {
    return QColor::fromRgbF(
        from.redF() + (to.redF() - from.redF()) * t,
        from.greenF() + (to.greenF() - from.greenF()) * t,
        from.blueF() + (to.blueF() - from.blueF()) * t,
        from.alphaF() + (to.alphaF() - from.alphaF()) * t
    );
}

// Helper to interpolate doubles
static double lerp(double from, double to, double t) {
    return from + (to - from) * t;
}

// Apply easing function based on type
static double applyEasing(double t, EasingType easing) {
    switch (easing) {
        case EasingType::Linear: return Easing::linear(t);
        case EasingType::EaseInOut: return Easing::easeInOutQuad(t);
        case EasingType::EaseIn: return Easing::easeInQuad(t);
        case EasingType::EaseOut: return Easing::easeOutQuad(t);
        case EasingType::EaseInOutCubic: return Easing::easeInOutCubic(t);
        case EasingType::EaseInOutQuint: return Easing::easeInOutQuint(t);
        default: return t;
    }
}

OverlayKeyframe OverlayKeyframe::interpolate(const OverlayKeyframe& from,
                                              const OverlayKeyframe& to,
                                              double progress) {
    // Apply easing to progress
    double t = applyEasing(progress, to.easing);

    OverlayKeyframe result;
    result.timeMs = lerp(from.timeMs, to.timeMs, t);
    result.extrusion = lerp(from.extrusion, to.extrusion, t);
    result.fillColor = lerpColor(from.fillColor, to.fillColor, t);
    result.borderColor = lerpColor(from.borderColor, to.borderColor, t);
    result.opacity = lerp(from.opacity, to.opacity, t);
    result.scale = lerp(from.scale, to.scale, t);
    result.easing = to.easing;  // Use destination easing
    result.syncEnumInt();

    return result;
}

QJsonObject OverlayKeyframe::toJson() const {
    QJsonObject obj;
    obj["timeMs"] = timeMs;
    obj["extrusion"] = extrusion;
    obj["fillColor"] = fillColor.name(QColor::HexArgb);
    obj["borderColor"] = borderColor.name(QColor::HexArgb);
    obj["opacity"] = opacity;
    obj["scale"] = scale;
    obj["easingType"] = easingTypeInt;
    return obj;
}

OverlayKeyframe OverlayKeyframe::fromJson(const QJsonObject& obj) {
    OverlayKeyframe kf;
    kf.timeMs = obj["timeMs"].toDouble(0.0);
    kf.extrusion = obj["extrusion"].toDouble(0.0);
    kf.fillColor = QColor(obj["fillColor"].toString("#80ff0000"));
    kf.borderColor = QColor(obj["borderColor"].toString("#ffff0000"));
    kf.opacity = obj["opacity"].toDouble(1.0);
    kf.scale = obj["scale"].toDouble(1.0);
    kf.easingTypeInt = obj["easingType"].toInt(static_cast<int>(EasingType::EaseInOut));
    kf.syncEnumFromInt();
    return kf;
}

// ============ OverlayPropertyTracks Implementation ============

double OverlayPropertyTracks::interpolateValue(const QVector<PropertyKeyframe>& track,
                                                double timeMs, double defaultVal) {
    if (track.isEmpty()) return defaultVal;
    if (track.size() == 1) return track.first().value;

    // Find surrounding keyframes
    int beforeIdx = -1;
    int afterIdx = -1;

    for (int i = 0; i < track.size(); ++i) {
        if (track[i].timeMs <= timeMs) {
            beforeIdx = i;
        }
        if (track[i].timeMs > timeMs && afterIdx < 0) {
            afterIdx = i;
        }
    }

    // Before first keyframe
    if (beforeIdx < 0) return track.first().value;
    // After last keyframe
    if (afterIdx < 0) return track.last().value;

    // Interpolate
    const auto& from = track[beforeIdx];
    const auto& to = track[afterIdx];
    double duration = to.timeMs - from.timeMs;
    double progress = (duration > 0) ? (timeMs - from.timeMs) / duration : 0.0;

    // Apply ease in/out
    progress = Easing::easeInOutQuad(progress);

    return lerp(from.value, to.value, progress);
}

QColor OverlayPropertyTracks::interpolateColor(const QVector<ColorKeyframe>& track,
                                                double timeMs, const QColor& defaultVal) {
    if (track.isEmpty()) return defaultVal;
    if (track.size() == 1) return track.first().color;

    // Find surrounding keyframes
    int beforeIdx = -1;
    int afterIdx = -1;

    for (int i = 0; i < track.size(); ++i) {
        if (track[i].timeMs <= timeMs) {
            beforeIdx = i;
        }
        if (track[i].timeMs > timeMs && afterIdx < 0) {
            afterIdx = i;
        }
    }

    // Before first keyframe
    if (beforeIdx < 0) return track.first().color;
    // After last keyframe
    if (afterIdx < 0) return track.last().color;

    // Interpolate
    const auto& from = track[beforeIdx];
    const auto& to = track[afterIdx];
    double duration = to.timeMs - from.timeMs;
    double progress = (duration > 0) ? (timeMs - from.timeMs) / duration : 0.0;

    // Apply ease in/out
    progress = Easing::easeInOutQuad(progress);

    return lerpColor(from.color, to.color, progress);
}

void OverlayPropertyTracks::sortAll() {
    auto sortByTime = [](const PropertyKeyframe& a, const PropertyKeyframe& b) {
        return a.timeMs < b.timeMs;
    };
    auto sortColorByTime = [](const ColorKeyframe& a, const ColorKeyframe& b) {
        return a.timeMs < b.timeMs;
    };

    std::sort(opacity.begin(), opacity.end(), sortByTime);
    std::sort(extrusion.begin(), extrusion.end(), sortByTime);
    std::sort(scale.begin(), scale.end(), sortByTime);
    std::sort(fillColor.begin(), fillColor.end(), sortColorByTime);
    std::sort(borderColor.begin(), borderColor.end(), sortColorByTime);
}

QJsonObject OverlayPropertyTracks::toJson() const {
    QJsonObject obj;

    // Helper to serialize property keyframes
    auto serializeTrack = [](const QVector<PropertyKeyframe>& track) {
        QJsonArray arr;
        for (const auto& kf : track) {
            arr.append(kf.toJson());
        }
        return arr;
    };

    auto serializeColorTrack = [](const QVector<ColorKeyframe>& track) {
        QJsonArray arr;
        for (const auto& kf : track) {
            arr.append(kf.toJson());
        }
        return arr;
    };

    if (!opacity.isEmpty()) obj["opacity"] = serializeTrack(opacity);
    if (!extrusion.isEmpty()) obj["extrusion"] = serializeTrack(extrusion);
    if (!scale.isEmpty()) obj["scale"] = serializeTrack(scale);
    if (!fillColor.isEmpty()) obj["fillColor"] = serializeColorTrack(fillColor);
    if (!borderColor.isEmpty()) obj["borderColor"] = serializeColorTrack(borderColor);

    return obj;
}

OverlayPropertyTracks OverlayPropertyTracks::fromJson(const QJsonObject& obj) {
    OverlayPropertyTracks tracks;

    // Helper to deserialize property keyframes
    auto deserializeTrack = [](const QJsonArray& arr) {
        QVector<PropertyKeyframe> track;
        for (const auto& val : arr) {
            track.append(PropertyKeyframe::fromJson(val.toObject()));
        }
        return track;
    };

    auto deserializeColorTrack = [](const QJsonArray& arr) {
        QVector<ColorKeyframe> track;
        for (const auto& val : arr) {
            track.append(ColorKeyframe::fromJson(val.toObject()));
        }
        return track;
    };

    if (obj.contains("opacity")) tracks.opacity = deserializeTrack(obj["opacity"].toArray());
    if (obj.contains("extrusion")) tracks.extrusion = deserializeTrack(obj["extrusion"].toArray());
    if (obj.contains("scale")) tracks.scale = deserializeTrack(obj["scale"].toArray());
    if (obj.contains("fillColor")) tracks.fillColor = deserializeColorTrack(obj["fillColor"].toArray());
    if (obj.contains("borderColor")) tracks.borderColor = deserializeColorTrack(obj["borderColor"].toArray());

    return tracks;
}
