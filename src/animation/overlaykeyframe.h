#pragma once

#include <QObject>
#include <QColor>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include "../animation/keyframe.h"  // For EasingType

// Simple keyframe for a single numeric property
struct PropertyKeyframe {
    Q_GADGET
    Q_PROPERTY(double time MEMBER timeMs)
    Q_PROPERTY(double value MEMBER value)

public:
    double timeMs = 0.0;
    double value = 0.0;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["time"] = timeMs;
        obj["value"] = value;
        return obj;
    }

    static PropertyKeyframe fromJson(const QJsonObject& obj) {
        PropertyKeyframe kf;
        kf.timeMs = obj["time"].toDouble();
        kf.value = obj["value"].toDouble();
        return kf;
    }
};

// Keyframe for color property
struct ColorKeyframe {
    Q_GADGET
    Q_PROPERTY(double time MEMBER timeMs)

public:
    double timeMs = 0.0;
    QColor color;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["time"] = timeMs;
        obj["color"] = color.name(QColor::HexArgb);
        return obj;
    }

    static ColorKeyframe fromJson(const QJsonObject& obj) {
        ColorKeyframe kf;
        kf.timeMs = obj["time"].toDouble();
        kf.color = QColor(obj["color"].toString());
        return kf;
    }
};

// Per-property keyframe tracks for an overlay
struct OverlayPropertyTracks {
    QVector<PropertyKeyframe> opacity;      // 0.0 - 1.0
    QVector<PropertyKeyframe> extrusion;    // 0 - 100
    QVector<PropertyKeyframe> scale;        // 0.5 - 2.0
    QVector<ColorKeyframe> fillColor;
    QVector<ColorKeyframe> borderColor;

    // Get interpolated value at time
    static double interpolateValue(const QVector<PropertyKeyframe>& track, double timeMs, double defaultVal);
    static QColor interpolateColor(const QVector<ColorKeyframe>& track, double timeMs, const QColor& defaultVal);

    // Sort all tracks by time
    void sortAll();

    // Serialization
    QJsonObject toJson() const;
    static OverlayPropertyTracks fromJson(const QJsonObject& obj);

    // Check if any tracks have keyframes
    bool hasAnyKeyframes() const {
        return !opacity.isEmpty() || !extrusion.isEmpty() || !scale.isEmpty() ||
               !fillColor.isEmpty() || !borderColor.isEmpty();
    }
};

Q_DECLARE_METATYPE(PropertyKeyframe)
Q_DECLARE_METATYPE(ColorKeyframe)

// Legacy: Keyframe for overlay property animation (unified)
struct OverlayKeyframe {
    Q_GADGET

    Q_PROPERTY(double timeMs MEMBER timeMs)
    Q_PROPERTY(double extrusion MEMBER extrusion)
    Q_PROPERTY(QColor fillColor MEMBER fillColor)
    Q_PROPERTY(QColor borderColor MEMBER borderColor)
    Q_PROPERTY(double opacity MEMBER opacity)
    Q_PROPERTY(double scale MEMBER scale)
    Q_PROPERTY(int easingType MEMBER easingTypeInt)

public:
    double timeMs = 0.0;  // Position on timeline

    // The 5 animatable properties
    double extrusion = 0.0;        // 0-100 height above surface
    QColor fillColor = QColor(255, 0, 0, 128);
    QColor borderColor = QColor(255, 0, 0, 255);
    double opacity = 1.0;          // 0.0-1.0
    double scale = 1.0;            // 0.8-1.5

    // Easing for transition TO this keyframe
    EasingType easing = EasingType::EaseInOut;
    int easingTypeInt = static_cast<int>(EasingType::EaseInOut);

    // Interpolate between two keyframes
    static OverlayKeyframe interpolate(const OverlayKeyframe& from,
                                        const OverlayKeyframe& to,
                                        double progress);

    // Serialization
    QJsonObject toJson() const;
    static OverlayKeyframe fromJson(const QJsonObject& obj);

    // Sync enum int
    void syncEnumInt() {
        easingTypeInt = static_cast<int>(easing);
    }

    void syncEnumFromInt() {
        easing = static_cast<EasingType>(easingTypeInt);
    }
};

Q_DECLARE_METATYPE(OverlayKeyframe)
