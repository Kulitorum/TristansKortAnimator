#pragma once

#include <QString>
#include <QColor>
#include <QJsonObject>

struct RegionTrack {
    QString regionCode;      // ISO code (country: "US", state: "US-CA", city name)
    QString regionName;      // Display name
    QString regionType;      // "country", "region", "city"

    QColor fillColor = QColor(255, 0, 0, 80);     // Semi-transparent red default
    QColor borderColor = QColor(255, 0, 0, 255);  // Solid red default
    double borderWidth = 2.0;

    // Timeline timing (all in milliseconds)
    double startTime = 0.0;       // When the region starts appearing
    double fadeInDuration = 500.0;  // How long to fade in
    double endTime = 0.0;         // When the region starts disappearing (0 = stays until end)
    double fadeOutDuration = 500.0; // How long to fade out

    // Calculate opacity at a given time
    double opacityAtTime(double timeMs, double totalDuration) const {
        // Before start time - invisible
        if (timeMs < startTime) {
            return 0.0;
        }

        // During fade in
        if (timeMs < startTime + fadeInDuration) {
            return (timeMs - startTime) / fadeInDuration;
        }

        // Determine effective end time
        double effectiveEnd = (endTime > 0) ? endTime : totalDuration;

        // After end time + fade out - invisible
        if (timeMs >= effectiveEnd + fadeOutDuration) {
            return 0.0;
        }

        // During fade out
        if (timeMs >= effectiveEnd) {
            return 1.0 - (timeMs - effectiveEnd) / fadeOutDuration;
        }

        // Fully visible between fade in complete and fade out start
        return 1.0;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["regionCode"] = regionCode;
        obj["regionName"] = regionName;
        obj["regionType"] = regionType;
        obj["fillColor"] = fillColor.name(QColor::HexArgb);
        obj["borderColor"] = borderColor.name(QColor::HexArgb);
        obj["borderWidth"] = borderWidth;
        obj["startTime"] = startTime;
        obj["fadeInDuration"] = fadeInDuration;
        obj["endTime"] = endTime;
        obj["fadeOutDuration"] = fadeOutDuration;
        return obj;
    }

    static RegionTrack fromJson(const QJsonObject& obj) {
        RegionTrack track;
        track.regionCode = obj["regionCode"].toString();
        track.regionName = obj["regionName"].toString();
        track.regionType = obj["regionType"].toString("country");
        track.fillColor = QColor(obj["fillColor"].toString("#50ff0000"));
        track.borderColor = QColor(obj["borderColor"].toString("#ff0000"));
        track.borderWidth = obj["borderWidth"].toDouble(2.0);
        track.startTime = obj["startTime"].toDouble(0.0);
        track.fadeInDuration = obj["fadeInDuration"].toDouble(500.0);
        track.endTime = obj["endTime"].toDouble(0.0);
        track.fadeOutDuration = obj["fadeOutDuration"].toDouble(500.0);
        return track;
    }
};
