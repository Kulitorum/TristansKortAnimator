#pragma once

#include <QObject>
#include <QColor>
#include <QJsonObject>
#include "../animation/keyframe.h"  // For EasingType

// Keyframe for overlay property animation
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
