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
