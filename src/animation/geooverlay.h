#pragma once

#include <QString>
#include <QColor>
#include <QJsonObject>
#include <QPointF>
#include <QPolygonF>
#include <QVector>

// Types of geographic overlays
enum class GeoOverlayType {
    Country,    // Polygon - entire country
    Region,     // Polygon - state/province
    City        // Point - city marker
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
    QVector<QPolygonF> polygons;    // For countries/regions
    QPointF point;                   // For cities
    double latitude = 0.0;
    double longitude = 0.0;

    // Timeline properties
    double startTime = 0.0;         // When this overlay starts appearing (ms)
    double fadeInDuration = 500.0;  // Fade in duration (ms)
    double endTime = 0.0;           // When it starts fading out (0 = never/end of animation)
    double fadeOutDuration = 500.0; // Fade out duration (ms)

    // Calculate opacity at a given time
    double opacityAtTime(double timeMs, double totalDuration) const {
        double effectiveEndTime = (endTime > 0) ? endTime : totalDuration;

        // Before start - invisible
        if (timeMs < startTime) {
            return 0.0;
        }

        // During fade in
        if (timeMs < startTime + fadeInDuration) {
            double t = (timeMs - startTime) / fadeInDuration;
            return t;
        }

        // After fade out complete - invisible
        if (timeMs > effectiveEndTime + fadeOutDuration) {
            return 0.0;
        }

        // During fade out
        if (timeMs > effectiveEndTime) {
            double t = (timeMs - effectiveEndTime) / fadeOutDuration;
            return 1.0 - t;
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
        overlay.fadeInDuration = obj["fadeInDuration"].toDouble(500.0);
        overlay.endTime = obj["endTime"].toDouble();
        overlay.fadeOutDuration = obj["fadeOutDuration"].toDouble(500.0);
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
