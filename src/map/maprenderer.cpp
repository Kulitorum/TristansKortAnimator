#include "maprenderer.h"
#include "tileprovider.h"
#include "tilecache.h"
#include "mapcamera.h"
#include "geojsonparser.h"
#include "../overlays/overlaymanager.h"
#include "../overlays/regionhighlight.h"
#include "../animation/framebuffer.h"
#include "../animation/regiontrackmodel.h"
#include "../animation/geooverlaymodel.h"
#include "../animation/geooverlay.h"
#include <QPainter>
#include <QtMath>
#include <QTransform>
#include <cmath>
#include <QMetaObject>
#include <QSet>
#include <QFile>
#include <QTextStream>

MapRenderer::MapRenderer(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
    setRenderTarget(QQuickPaintedItem::FramebufferObject);
}

void MapRenderer::paint(QPainter* painter) {
    if (!m_camera) return;

    // Check if we can use a cached frame
    if (m_useFrameBuffer && m_frameBuffer && m_frameBuffer->hasFrame(m_currentAnimationTime)) {
        QImage cachedFrame = m_frameBuffer->getFrame(m_currentAnimationTime);
        if (!cachedFrame.isNull()) {
            painter->drawImage(QRectF(0, 0, width(), height()), cachedFrame);
            emit renderingComplete();
            return;
        }
    }

    painter->save();

    // Apply camera transforms (bearing and tilt)
    applyTransforms(painter);

    // Render layers in order
    renderTiles(painter);
    renderCountryBorders(painter);
    renderHighlights(painter);
    renderRegionTracks(painter, m_currentAnimationTime, m_totalDuration);
    renderGeoOverlays(painter, m_currentAnimationTime, m_totalDuration);
    renderCityMarkers(painter);
    renderOverlays(painter, m_currentAnimationTime);
    renderLabels(painter);

    resetTransforms(painter);
    painter->restore();

    // Store frame in buffer if not already cached - DISABLED for debugging
    /*
    if (m_useFrameBuffer && m_frameBuffer && !m_frameBuffer->hasFrame(m_currentAnimationTime)) {
        QImage frame = renderToImage(static_cast<int>(width()), static_cast<int>(height()));
        m_frameBuffer->storeFrame(m_currentAnimationTime, frame);
    }
    */

    emit renderingComplete();
}

void MapRenderer::applyTransforms(QPainter* painter) {
    if (!m_camera) return;

    // Apply tilt (fake 3D perspective)
    if (m_camera->tilt() > 0) {
        QTransform tiltTransform;
        // Move origin to bottom center for perspective effect
        tiltTransform.translate(width() / 2, height());
        // Scale vertically to simulate perspective (objects at top appear smaller)
        double tiltFactor = 1.0 - (m_camera->tilt() / 90.0) * 0.5;
        tiltTransform.scale(1.0, tiltFactor);
        tiltTransform.translate(-width() / 2, -height());
        painter->setTransform(tiltTransform, true);
    }

    // Apply bearing (rotation)
    if (m_camera->bearing() != 0) {
        painter->translate(width() / 2, height() / 2);
        painter->rotate(-m_camera->bearing());
        painter->translate(-width() / 2, -height() / 2);
    }
}

void MapRenderer::resetTransforms(QPainter* painter) {
    painter->resetTransform();
}

void MapRenderer::renderTiles(QPainter* painter) {
    if (!m_camera || !m_tileProvider) return;

    // Enable smooth scaling for better quality during zoom transitions
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    double zoom = m_camera->zoom();
    int zoomLevel = m_camera->zoomLevel();
    double scale = std::pow(2.0, zoom - zoomLevel);

    // When scale > 1.5, try to use higher zoom level tiles (scale down instead of up)
    // Scaling down produces better quality than scaling up
    int preferredZoom = zoomLevel;
    if (scale > 1.5 && zoomLevel < 19) {
        preferredZoom = zoomLevel + 1;
        scale = std::pow(2.0, zoom - preferredZoom);  // Will be 0.5 to 0.75
    }

    // Get visible tile range (use preferred zoom for tile coordinates)
    auto range = m_camera->visibleTileRangeAtZoom(width(), height(), preferredZoom);

    // Calculate center tile position at preferred zoom level
    double centerLon = m_camera->longitude();
    double centerLat = m_camera->latitude();
    double n = std::pow(2.0, preferredZoom);

    double centerTileX = (centerLon + 180.0) / 360.0 * n;
    double latRad = centerLat * M_PI / 180.0;
    double centerTileY = (1.0 - std::log(std::tan(latRad) + 1.0 / std::cos(latRad)) / M_PI) / 2.0 * n;

    // Calculate offset for sub-tile positioning
    double offsetX = (centerTileX - std::floor(centerTileX)) * TILE_SIZE * scale;
    double offsetY = (centerTileY - std::floor(centerTileY)) * TILE_SIZE * scale;

    int centerTileXInt = static_cast<int>(std::floor(centerTileX));
    int centerTileYInt = static_cast<int>(std::floor(centerTileY));

    // Get tile source
    int source = m_tileProvider->currentSource();

    // Render tiles
    // Add small overlap to prevent seams between tiles (floating-point precision issue)
    constexpr double TILE_OVERLAP = 0.5;

    for (int ty = range.minY; ty <= range.maxY; ty++) {
        for (int tx = range.minX; tx <= range.maxX; tx++) {
            // Calculate screen position for this tile
            double screenX = width() / 2.0 + (tx - centerTileXInt) * TILE_SIZE * scale - offsetX;
            double screenY = height() / 2.0 + (ty - centerTileYInt) * TILE_SIZE * scale - offsetY;
            double tileSize = TILE_SIZE * scale;

            QImage tile;
            if (m_tileCache && m_tileCache->contains(source, tx, ty, preferredZoom)) {
                // Exact tile available - use it
                tile = m_tileCache->get(source, tx, ty, preferredZoom);
            }

            if (!tile.isNull()) {
                // Slightly expand the destination rect to eliminate seams
                QRectF destRect(screenX - TILE_OVERLAP, screenY - TILE_OVERLAP,
                               tileSize + TILE_OVERLAP * 2, tileSize + TILE_OVERLAP * 2);
                painter->drawImage(destRect, tile);
            } else {
                // Try to render a fallback tile from a lower zoom level
                bool hasFallback = tryRenderFallbackTile(painter, tx, ty, preferredZoom,
                                                          screenX, screenY, tileSize, source);

                // Request the correct tile in background
                QMetaObject::invokeMethod(m_tileProvider, "requestTile",
                                          Qt::QueuedConnection,
                                          Q_ARG(int, tx), Q_ARG(int, ty), Q_ARG(int, preferredZoom));

                // Only show placeholder if no fallback was found
                if (!hasFallback) {
                    painter->fillRect(QRectF(screenX - TILE_OVERLAP, screenY - TILE_OVERLAP,
                                            tileSize + TILE_OVERLAP * 2, tileSize + TILE_OVERLAP * 2),
                                     QColor(30, 30, 50));
                }
            }
        }
    }
}

bool MapRenderer::tryRenderFallbackTile(QPainter* painter, int tx, int ty, int targetZoom,
                                         double screenX, double screenY, double tileSize, int source) {
    if (!m_tileCache) return false;

    // Try parent zoom levels (lower zoom = larger area per tile)
    // Each zoom level down covers 4x the area (2x in each dimension)
    for (int fallbackZoom = targetZoom - 1; fallbackZoom >= qMax(0, targetZoom - 4); fallbackZoom--) {
        // Calculate which tile at fallbackZoom contains our target tile
        int zoomDiff = targetZoom - fallbackZoom;
        int divisor = 1 << zoomDiff;  // 2^zoomDiff

        int parentTx = tx / divisor;
        int parentTy = ty / divisor;

        if (m_tileCache->contains(source, parentTx, parentTy, fallbackZoom)) {
            QImage parentTile = m_tileCache->get(source, parentTx, parentTy, fallbackZoom);
            if (parentTile.isNull()) continue;

            // Calculate which portion of the parent tile to use
            // Each parent tile is divided into divisor x divisor sub-tiles
            int subTileX = tx % divisor;  // Which column within the parent
            int subTileY = ty % divisor;  // Which row within the parent

            int subTileSize = TILE_SIZE / divisor;
            int srcX = subTileX * subTileSize;
            int srcY = subTileY * subTileSize;

            // Extract the relevant portion and scale it up
            // Add small overlap to prevent seams (same as regular tiles)
            constexpr double TILE_OVERLAP = 0.5;
            QRectF srcRect(srcX, srcY, subTileSize, subTileSize);
            QRectF destRect(screenX - TILE_OVERLAP, screenY - TILE_OVERLAP,
                           tileSize + TILE_OVERLAP * 2, tileSize + TILE_OVERLAP * 2);

            // Use smooth scaling for better quality
            painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
            painter->drawImage(destRect, parentTile, srcRect);

            return true;
        }
    }

    return false;
}

void MapRenderer::renderHighlights(QPainter* painter) {
    if (!m_camera || !m_geojson || !m_geojson->isLoaded()) return;

    double viewW = width();
    double viewH = height();
    if (viewW <= 0 || viewH <= 0) return;

    // Collect highlighted region codes (from both internal highlights and overlay system)
    QSet<QString> highlightedCodes;

    // Add internal highlights
    for (auto it = m_highlights.constBegin(); it != m_highlights.constEnd(); ++it) {
        highlightedCodes.insert(it.key());
    }

    // Add region highlights from overlay manager
    QVector<RegionHighlight*> regionOverlays;
    if (m_overlays) {
        auto visibleOverlays = m_overlays->visibleOverlaysAtTime(0); // TODO: Get actual animation time
        for (auto* overlay : visibleOverlays) {
            if (auto* regionHighlight = qobject_cast<RegionHighlight*>(overlay)) {
                regionOverlays.append(regionHighlight);
                highlightedCodes.insert(regionHighlight->regionCode());
            }
        }
    }

    // If shading non-highlighted is enabled, draw all countries with dim shade first
    if (m_shadeNonHighlighted && !highlightedCodes.isEmpty()) {
        QColor shadeColor(0, 0, 0, static_cast<int>((1.0 - m_nonHighlightedOpacity) * 150));

        for (const auto& feature : m_geojson->features()) {
            if (feature.type == "country" && !highlightedCodes.contains(feature.code)) {
                for (const QPolygonF& geoPoly : feature.polygons) {
                    QPolygonF screenPoly;
                    screenPoly.reserve(geoPoly.size());

                    for (const QPointF& geoPoint : geoPoly) {
                        // Polygons store (lat=x, lon=y) after parsing
                        QPointF screenPoint = m_camera->geoToScreen(geoPoint.x(), geoPoint.y(), viewW, viewH);
                        screenPoly.append(screenPoint);
                    }

                    if (!screenPoly.isEmpty()) {
                        painter->setPen(Qt::NoPen);
                        painter->setBrush(shadeColor);
                        painter->drawPolygon(screenPoly);
                    }
                }
            }
        }
    }

    // Draw internal highlights
    for (auto it = m_highlights.constBegin(); it != m_highlights.constEnd(); ++it) {
        const QString& regionCode = it.key();
        const HighlightStyle& highlight = it.value();

        const GeoFeature* feature = m_geojson->findByCode(regionCode);
        if (!feature) continue;

        for (const QPolygonF& geoPoly : feature->polygons) {
            QPolygonF screenPoly;
            screenPoly.reserve(geoPoly.size());

            for (const QPointF& geoPoint : geoPoly) {
                // GeoJSON stores as (longitude, latitude)
                QPointF screenPoint = m_camera->geoToScreen(geoPoint.y(), geoPoint.x(), viewW, viewH);
                screenPoly.append(screenPoint);
            }

            if (!screenPoly.isEmpty()) {
                // Draw fill
                if (highlight.fillColor.alpha() > 0) {
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(highlight.fillColor);
                    painter->drawPolygon(screenPoly);
                }

                // Draw border
                if (highlight.borderColor.alpha() > 0) {
                    painter->setPen(QPen(highlight.borderColor, 2.0));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawPolygon(screenPoly);
                }
            }
        }
    }

    // Draw region highlights from overlay manager (with their specific colors)
    for (auto* regionHighlight : regionOverlays) {
        const GeoFeature* feature = m_geojson->findByCode(regionHighlight->regionCode());
        if (!feature) continue;

        for (const QPolygonF& geoPoly : feature->polygons) {
            QPolygonF screenPoly;
            screenPoly.reserve(geoPoly.size());

            for (const QPointF& geoPoint : geoPoly) {
                // GeoJSON stores as (longitude, latitude)
                QPointF screenPoint = m_camera->geoToScreen(geoPoint.y(), geoPoint.x(), viewW, viewH);
                screenPoly.append(screenPoint);
            }

            if (!screenPoly.isEmpty()) {
                // Draw fill
                if (regionHighlight->fillColor().alpha() > 0) {
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(regionHighlight->fillColor());
                    painter->drawPolygon(screenPoly);
                }

                // Draw border
                if (regionHighlight->borderColor().alpha() > 0) {
                    painter->setPen(QPen(regionHighlight->borderColor(), regionHighlight->borderWidth()));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawPolygon(screenPoly);
                }
            }
        }
    }
}

void MapRenderer::renderRegionTracks(QPainter* painter, double currentTime, double totalDuration) {
    if (!m_camera || !m_geojson || !m_geojson->isLoaded() || !m_regionTracks) return;

    double viewW = width();
    double viewH = height();
    if (viewW <= 0 || viewH <= 0) return;

    // Get all visible tracks at current time with their calculated opacities
    auto visibleTracks = m_regionTracks->visibleTracksAtTime(currentTime, totalDuration);

    for (const auto& trackPair : visibleTracks) {
        const RegionTrack* track = trackPair.first;
        double opacity = trackPair.second;

        if (opacity <= 0.0) continue;

        // Find the geographic feature for this region
        const GeoFeature* feature = m_geojson->findByCode(track->regionCode);
        if (!feature) {
            // Try finding by name if code didn't match
            feature = m_geojson->findByName(track->regionName);
        }
        if (!feature) continue;

        // Apply opacity to colors
        QColor fillColor = track->fillColor;
        fillColor.setAlphaF(fillColor.alphaF() * opacity);

        QColor borderColor = track->borderColor;
        borderColor.setAlphaF(borderColor.alphaF() * opacity);

        // Draw the region polygons
        for (const QPolygonF& geoPoly : feature->polygons) {
            QPolygonF screenPoly;
            screenPoly.reserve(geoPoly.size());

            for (const QPointF& geoPoint : geoPoly) {
                // Polygons store (lat=x, lon=y) after parsing
                QPointF screenPoint = m_camera->geoToScreen(geoPoint.x(), geoPoint.y(), viewW, viewH);
                screenPoly.append(screenPoint);
            }

            if (!screenPoly.isEmpty()) {
                // Draw fill
                if (fillColor.alpha() > 0) {
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(fillColor);
                    painter->drawPolygon(screenPoly);
                }

                // Draw border
                if (borderColor.alpha() > 0 && track->borderWidth > 0) {
                    painter->setPen(QPen(borderColor, track->borderWidth));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawPolygon(screenPoly);
                }
            }
        }
    }
}

void MapRenderer::renderGeoOverlays(QPainter* painter, double currentTime, double totalDuration) {
    if (!m_camera || !m_geoOverlays) return;

    double viewW = width();
    double viewH = height();
    if (viewW <= 0 || viewH <= 0) return;

    // Get all overlays and render visible ones
    const auto& allOverlays = m_geoOverlays->overlays();

    for (const auto& overlay : allOverlays) {
        // Calculate opacity based on timing (fade in/out)
        double opacity = overlay.opacityAtTime(currentTime, totalDuration);

        // Skip invisible overlays
        if (opacity <= 0.0) continue;

        // Apply opacity to colors
        QColor fillColor = overlay.fillColor;
        fillColor.setAlphaF(fillColor.alphaF() * opacity);

        QColor borderColor = overlay.borderColor;
        borderColor.setAlphaF(borderColor.alphaF() * opacity);

        if (overlay.type == GeoOverlayType::City) {
            // Check if city has boundary polygons
            if (!overlay.polygons.isEmpty()) {
                // Render city boundary as polygons (like countries/regions)
                for (const QPolygonF& geoPoly : overlay.polygons) {
                    QPolygonF screenPoly;
                    screenPoly.reserve(geoPoly.size());

                    for (const QPointF& geoPoint : geoPoly) {
                        // Polygons store (lat=x, lon=y) after parsing
                        QPointF screenPoint = m_camera->geoToScreen(geoPoint.x(), geoPoint.y(), viewW, viewH);
                        screenPoly.append(screenPoint);
                    }

                    if (!screenPoly.isEmpty()) {
                        // Draw fill
                        if (fillColor.alpha() > 0) {
                            painter->setPen(Qt::NoPen);
                            painter->setBrush(fillColor);
                            painter->drawPolygon(screenPoly);
                        }

                        // Draw border
                        painter->setPen(QPen(borderColor, overlay.borderWidth > 0 ? overlay.borderWidth : 2.0));
                        painter->setBrush(Qt::NoBrush);
                        painter->drawPolygon(screenPoly);
                    }
                }

                // Draw label if enabled (at centroid position)
                if (overlay.showLabel) {
                    QPointF screenPoint = m_camera->geoToScreen(overlay.latitude, overlay.longitude, viewW, viewH);
                    if (screenPoint.x() >= -50 && screenPoint.x() <= viewW + 50 &&
                        screenPoint.y() >= -50 && screenPoint.y() <= viewH + 50) {

                        QColor textColor = Qt::white;
                        textColor.setAlphaF(opacity);

                        QFont font = painter->font();
                        font.setPixelSize(11);
                        font.setBold(true);
                        painter->setFont(font);

                        // Draw text with shadow for readability
                        QColor shadowColor(0, 0, 0, static_cast<int>(180 * opacity));
                        painter->setPen(shadowColor);
                        painter->drawText(QPointF(screenPoint.x() + 1, screenPoint.y() + 1), overlay.name);

                        painter->setPen(textColor);
                        painter->drawText(screenPoint, overlay.name);
                    }
                }
            } else {
                // Fallback: Render city as a marker circle (no boundary data)
                QPointF screenPoint = m_camera->geoToScreen(overlay.latitude, overlay.longitude, viewW, viewH);

                // Check if on screen
                if (screenPoint.x() >= -50 && screenPoint.x() <= viewW + 50 &&
                    screenPoint.y() >= -50 && screenPoint.y() <= viewH + 50) {

                    double radius = overlay.markerRadius;

                    // Draw circle - border only if fill is transparent
                    if (fillColor.alpha() > 0) {
                        painter->setBrush(fillColor);
                    } else {
                        painter->setBrush(Qt::NoBrush);
                    }

                    if (borderColor.alpha() > 0) {
                        painter->setPen(QPen(borderColor, 3));  // Thicker border for visibility
                    } else {
                        painter->setPen(Qt::NoPen);
                    }
                    painter->drawEllipse(screenPoint, radius, radius);

                    // Draw label if enabled
                    if (overlay.showLabel) {
                        QColor textColor = Qt::white;
                        textColor.setAlphaF(opacity);

                        painter->setPen(textColor);
                        QFont font = painter->font();
                        font.setPixelSize(11);
                        font.setBold(true);
                        painter->setFont(font);

                        // Draw text with shadow for readability
                        QColor shadowColor(0, 0, 0, static_cast<int>(180 * opacity));
                        painter->setPen(shadowColor);
                        painter->drawText(QPointF(screenPoint.x() + radius + 5 + 1, screenPoint.y() + 4 + 1), overlay.name);

                        painter->setPen(textColor);
                        painter->drawText(QPointF(screenPoint.x() + radius + 5, screenPoint.y() + 4), overlay.name);
                    }
                }
            }
        } else {
            // Render country/region polygons
            // Debug: log polygon count
            if (overlay.polygons.isEmpty()) {
                qWarning() << "WARNING: No polygons for" << overlay.name << "code=" << overlay.code;
            }

            for (const QPolygonF& geoPoly : overlay.polygons) {
                QPolygonF screenPoly;
                screenPoly.reserve(geoPoly.size());

                for (const QPointF& geoPoint : geoPoly) {
                    // Polygons store (lat=x, lon=y) after parsing
                    QPointF screenPoint = m_camera->geoToScreen(geoPoint.x(), geoPoint.y(), viewW, viewH);
                    screenPoly.append(screenPoint);
                }

                if (!screenPoly.isEmpty()) {
                    // Draw fill
                    if (fillColor.alpha() > 0) {
                        painter->setPen(Qt::NoPen);
                        painter->setBrush(fillColor);
                        painter->drawPolygon(screenPoly);
                    }

                    // Draw border - ALWAYS draw for debugging
                    painter->setPen(QPen(borderColor, overlay.borderWidth > 0 ? overlay.borderWidth : 3.0));
                    painter->setBrush(Qt::NoBrush);
                    painter->drawPolygon(screenPoly);
                }
            }
        }
    }
}

void MapRenderer::renderOverlays(QPainter* painter, double currentTime) {
    if (!m_camera || !m_overlays) return;

    // Get overlays visible at current time
    auto visibleOverlays = m_overlays->visibleOverlaysAtTime(currentTime);

    for (auto* overlay : visibleOverlays) {
        // Render based on overlay type
        // TODO: Implement overlay rendering (markers, arrows, text)
    }
}

void MapRenderer::renderLabels(QPainter* painter) {
    if (!m_camera || !m_geojson || !m_geojson->isLoaded()) return;

    double viewW = width();
    double viewH = height();
    if (viewW <= 0 || viewH <= 0) return;

    double zoom = m_camera->zoom();

    // Apply label opacity (fades when camera moves fast)
    if (m_labelOpacity <= 0.01) return;

    painter->setOpacity(m_labelOpacity);

    // Country labels (visible at zoom 2-8)
    if (m_showCountryLabels && zoom >= 2.0 && zoom <= 10.0) {
        // Font size scales with zoom
        int fontSize = static_cast<int>(10 + (zoom - 2) * 1.5);
        QFont countryFont("Arial", fontSize, QFont::Bold);
        painter->setFont(countryFont);

        for (const auto& feature : m_geojson->features()) {
            if (feature.type == "country" && !feature.name.isEmpty() && !feature.centroid.isNull()) {
                // Use centroid for label position
                QPointF screenPos = m_camera->geoToScreen(
                    feature.centroid.x(), feature.centroid.y(), viewW, viewH);

                // Only draw if on screen
                if (screenPos.x() >= -100 && screenPos.x() <= viewW + 100 &&
                    screenPos.y() >= -50 && screenPos.y() <= viewH + 50) {

                    // Draw text with outline for visibility
                    QString name = feature.name;
                    QFontMetrics fm(countryFont);
                    QRect textRect = fm.boundingRect(name);
                    textRect.moveCenter(screenPos.toPoint());

                    // Draw outline
                    painter->setPen(QPen(QColor(0, 0, 0, 180), 3));
                    for (int dx = -1; dx <= 1; dx++) {
                        for (int dy = -1; dy <= 1; dy++) {
                            if (dx != 0 || dy != 0) {
                                painter->drawText(textRect.translated(dx, dy), Qt::AlignCenter, name);
                            }
                        }
                    }

                    // Draw text
                    painter->setPen(Qt::white);
                    painter->drawText(textRect, Qt::AlignCenter, name);
                }
            }
        }
    }

    // Region labels (visible at zoom 5-12)
    if (m_showRegionLabels && zoom >= 5.0 && zoom <= 12.0) {
        int fontSize = static_cast<int>(8 + (zoom - 5) * 1.0);
        QFont regionFont("Arial", fontSize);
        painter->setFont(regionFont);

        for (const auto& feature : m_geojson->features()) {
            if (feature.type == "region" && !feature.name.isEmpty()) {
                QPointF screenPos = m_camera->geoToScreen(
                    feature.centroid.x(), feature.centroid.y(), viewW, viewH);

                if (screenPos.x() >= -50 && screenPos.x() <= viewW + 50 &&
                    screenPos.y() >= -30 && screenPos.y() <= viewH + 30) {

                    QString name = feature.name;
                    QFontMetrics fm(regionFont);
                    QRect textRect = fm.boundingRect(name);
                    textRect.moveCenter(screenPos.toPoint());

                    // Outline
                    painter->setPen(QPen(QColor(0, 0, 0, 150), 2));
                    painter->drawText(textRect.translated(1, 1), Qt::AlignCenter, name);

                    // Text
                    painter->setPen(QColor(220, 220, 220));
                    painter->drawText(textRect, Qt::AlignCenter, name);
                }
            }
        }
    }

    // City labels (visible at zoom 6+)
    if (m_showCityLabels && zoom >= 6.0) {
        // Larger cities at lower zoom, smaller cities at higher zoom
        int minPopulation = 0;
        if (zoom < 8) minPopulation = 1000000;       // Mega cities only
        else if (zoom < 10) minPopulation = 500000;  // Large cities
        else if (zoom < 12) minPopulation = 100000;  // Medium cities
        else minPopulation = 50000;                   // Small cities

        int fontSize = static_cast<int>(8 + (zoom - 6) * 0.8);
        QFont cityFont("Arial", fontSize);
        painter->setFont(cityFont);

        for (const auto& feature : m_geojson->features()) {
            if (feature.type == "city" && !feature.name.isEmpty()) {
                // Check population if available
                int population = feature.properties.value("population", 0).toInt();
                if (population < minPopulation && minPopulation > 0) continue;

                QPointF screenPos = m_camera->geoToScreen(
                    feature.centroid.x(), feature.centroid.y(), viewW, viewH);

                if (screenPos.x() >= -30 && screenPos.x() <= viewW + 30 &&
                    screenPos.y() >= -20 && screenPos.y() <= viewH + 20) {

                    QString name = feature.name;
                    QFontMetrics fm(cityFont);
                    QRect textRect = fm.boundingRect(name);
                    textRect.moveCenter(screenPos.toPoint());
                    textRect.translate(0, -10);  // Offset above the dot

                    // Draw city dot
                    painter->setPen(Qt::NoPen);
                    painter->setBrush(Qt::white);
                    painter->drawEllipse(screenPos, 3, 3);

                    // Outline
                    painter->setPen(QPen(QColor(0, 0, 0, 150), 2));
                    painter->drawText(textRect.translated(1, 1), Qt::AlignCenter, name);

                    // Text
                    painter->setPen(QColor(255, 255, 200));
                    painter->drawText(textRect, Qt::AlignCenter, name);
                }
            }
        }
    }

    painter->setOpacity(1.0);
}

void MapRenderer::setTileProvider(TileProvider* provider) {
    if (m_tileProvider) {
        disconnect(m_tileProvider, nullptr, this, nullptr);
    }
    m_tileProvider = provider;
    if (m_tileProvider) {
        connect(m_tileProvider, &TileProvider::tileReady, this, &MapRenderer::onTileReady);
        connect(m_tileProvider, &TileProvider::currentSourceChanged, this, &MapRenderer::requestUpdate);
    }
}

void MapRenderer::setTileCache(TileCache* cache) {
    m_tileCache = cache;
}

void MapRenderer::setGeoJson(GeoJsonParser* geojson) {
    m_geojson = geojson;
}

void MapRenderer::setOverlayManager(OverlayManager* overlays) {
    m_overlays = overlays;
}

void MapRenderer::setRegionTrackModel(RegionTrackModel* regionTracks) {
    m_regionTracks = regionTracks;
    if (m_regionTracks) {
        connect(m_regionTracks, &RegionTrackModel::dataModified, this, &MapRenderer::requestUpdate);
    }
}

void MapRenderer::setGeoOverlayModel(GeoOverlayModel* geoOverlays) {
    m_geoOverlays = geoOverlays;
    if (m_geoOverlays) {
        connect(m_geoOverlays, &GeoOverlayModel::dataModified, this, &MapRenderer::requestUpdate);
    }
}

void MapRenderer::setCamera(MapCamera* camera) {
    if (m_camera) {
        disconnect(m_camera, nullptr, this, nullptr);
    }
    m_camera = camera;
    if (m_camera) {
        connect(m_camera, &MapCamera::cameraChanged, this, &MapRenderer::requestUpdate);
        connect(m_camera, &MapCamera::movementSpeedChanged, this, &MapRenderer::onMovementSpeedChanged);
    }
    emit cameraChanged();
    update();
}

void MapRenderer::onMovementSpeedChanged() {
    if (!m_camera) return;

    double speed = m_camera->movementSpeed();
    double newOpacity;

    if (speed <= SPEED_FADE_START) {
        newOpacity = 1.0;
    } else if (speed >= SPEED_FADE_END) {
        newOpacity = 0.0;
    } else {
        // Linear interpolation between start and end thresholds
        newOpacity = 1.0 - (speed - SPEED_FADE_START) / (SPEED_FADE_END - SPEED_FADE_START);
    }

    if (!qFuzzyCompare(m_labelOpacity, newOpacity)) {
        m_labelOpacity = newOpacity;
        emit labelOpacityChanged();
        update();
    }
}

void MapRenderer::setShowCountryLabels(bool show) {
    if (m_showCountryLabels != show) {
        m_showCountryLabels = show;
        emit showCountryLabelsChanged();
        update();
    }
}

void MapRenderer::setShowRegionLabels(bool show) {
    if (m_showRegionLabels != show) {
        m_showRegionLabels = show;
        emit showRegionLabelsChanged();
        update();
    }
}

void MapRenderer::setShowCityLabels(bool show) {
    if (m_showCityLabels != show) {
        m_showCityLabels = show;
        emit showCityLabelsChanged();
        update();
    }
}

void MapRenderer::setShadeNonHighlighted(bool shade) {
    if (m_shadeNonHighlighted != shade) {
        m_shadeNonHighlighted = shade;
        emit shadeNonHighlightedChanged();
        update();
    }
}

void MapRenderer::setNonHighlightedOpacity(double opacity) {
    if (!qFuzzyCompare(m_nonHighlightedOpacity, opacity)) {
        m_nonHighlightedOpacity = opacity;
        emit nonHighlightedOpacityChanged();
        update();
    }
}

void MapRenderer::highlightRegion(const QString& regionCode, const QColor& fillColor, const QColor& borderColor) {
    m_highlights[regionCode] = {fillColor, borderColor};
    update();
}

void MapRenderer::clearHighlight(const QString& regionCode) {
    m_highlights.remove(regionCode);
    update();
}

void MapRenderer::clearAllHighlights() {
    m_highlights.clear();
    update();
}

void MapRenderer::onTileReady(int x, int y, int zoom, const QImage& image) {
    // Store in cache
    if (m_tileCache && m_tileProvider) {
        m_tileCache->insert(m_tileProvider->currentSource(), x, y, zoom, image);
    }
    update();
}

void MapRenderer::requestUpdate() {
    update();
}

QImage MapRenderer::renderToImage(int targetWidth, int targetHeight) {
    QImage image(targetWidth, targetHeight, QImage::Format_ARGB32);
    image.fill(Qt::black);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // Temporarily disable frame buffer to avoid recursion
    bool wasUsingFrameBuffer = m_useFrameBuffer;
    m_useFrameBuffer = false;

    // Temporarily adjust for target size
    double scaleX = static_cast<double>(targetWidth) / width();
    double scaleY = static_cast<double>(targetHeight) / height();
    painter.scale(scaleX, scaleY);

    // Render all layers directly (not using paint() to avoid signals)
    painter.save();
    applyTransforms(&painter);
    renderTiles(&painter);
    renderCountryBorders(&painter);
    renderHighlights(&painter);
    renderCityMarkers(&painter);
    renderOverlays(&painter, m_currentAnimationTime);
    renderLabels(&painter);
    resetTransforms(&painter);
    painter.restore();

    // Restore frame buffer setting
    m_useFrameBuffer = wasUsingFrameBuffer;

    return image;
}

void MapRenderer::setCurrentAnimationTime(double timeMs) {
    if (!qFuzzyCompare(m_currentAnimationTime, timeMs)) {
        m_currentAnimationTime = timeMs;
        emit currentAnimationTimeChanged();
        update();
    }
}

void MapRenderer::setTotalDuration(double durationMs) {
    if (!qFuzzyCompare(m_totalDuration, durationMs)) {
        m_totalDuration = durationMs;
        emit totalDurationChanged();
    }
}

void MapRenderer::setFrameBuffer(FrameBuffer* buffer) {
    if (m_frameBuffer != buffer) {
        m_frameBuffer = buffer;
        update();
    }
}

void MapRenderer::setUseFrameBuffer(bool use) {
    if (m_useFrameBuffer != use) {
        m_useFrameBuffer = use;
        emit useFrameBufferChanged();
        update();
    }
}

void MapRenderer::setShowCountryBorders(bool show) {
    if (m_showCountryBorders != show) {
        m_showCountryBorders = show;
        emit showCountryBordersChanged();
        update();
    }
}

void MapRenderer::setShowCityMarkers(bool show) {
    if (m_showCityMarkers != show) {
        m_showCityMarkers = show;
        emit showCityMarkersChanged();
        update();
    }
}

void MapRenderer::renderCountryBorders(QPainter* painter) {
    if (!m_showCountryBorders || !m_camera || !m_geojson || !m_geojson->isLoaded()) return;

    double viewW = width();
    double viewH = height();
    if (viewW <= 0 || viewH <= 0) return;

    // Border colors
    QColor borderColor(255, 255, 255, 120);  // White semi-transparent
    QColor selectedBorderColor(255, 220, 0, 255);  // Yellow for selected

    painter->setBrush(Qt::NoBrush);

    for (const auto& feature : m_geojson->features()) {
        if (feature.type != "country") continue;

        bool isSelected = (m_selectedFeatureType == "country" && feature.code == m_selectedFeatureCode);

        if (isSelected) {
            painter->setPen(QPen(selectedBorderColor, 3.0));
        } else {
            painter->setPen(QPen(borderColor, 1.0));
        }

        for (const QPolygonF& geoPoly : feature.polygons) {
            QPolygonF screenPoly;
            screenPoly.reserve(geoPoly.size());

            for (const QPointF& geoPoint : geoPoly) {
                QPointF screenPoint = m_camera->geoToScreen(geoPoint.x(), geoPoint.y(), viewW, viewH);
                screenPoly.append(screenPoint);
            }

            if (!screenPoly.isEmpty()) {
                painter->drawPolygon(screenPoly);
            }
        }
    }
}

void MapRenderer::renderCityMarkers(QPainter* painter) {
    if (!m_showCityMarkers || !m_camera || !m_geojson || !m_geojson->isLoaded()) return;

    double viewW = width();
    double viewH = height();
    if (viewW <= 0 || viewH <= 0) return;

    double zoom = m_camera->zoom();

    // Filter cities by zoom level
    int minPopulation = 0;
    if (zoom < 5) minPopulation = 5000000;
    else if (zoom < 7) minPopulation = 1000000;
    else if (zoom < 9) minPopulation = 500000;
    else if (zoom < 11) minPopulation = 100000;
    else minPopulation = 50000;

    QColor markerColor(255, 100, 100, 200);
    QColor selectedMarkerColor(255, 220, 0, 255);
    QColor textColor(255, 255, 255, 220);

    QFont cityFont("Arial", 10);
    painter->setFont(cityFont);

    for (const auto& feature : m_geojson->features()) {
        if (feature.type != "city") continue;

        int population = feature.properties.value("population", 0).toInt();
        if (population < minPopulation) continue;

        QPointF screenPos = m_camera->geoToScreen(feature.centroid.x(), feature.centroid.y(), viewW, viewH);

        // Skip if off screen
        if (screenPos.x() < -20 || screenPos.x() > viewW + 20 ||
            screenPos.y() < -20 || screenPos.y() > viewH + 20) continue;

        bool isSelected = (m_selectedFeatureType == "city" && feature.name == m_selectedFeatureName);

        // Draw marker circle
        double markerSize = isSelected ? 8 : 5;
        painter->setPen(Qt::NoPen);
        painter->setBrush(isSelected ? selectedMarkerColor : markerColor);
        painter->drawEllipse(screenPos, markerSize, markerSize);

        // Draw city name
        if (zoom >= 6) {
            QString name = feature.name;
            QFontMetrics fm(cityFont);
            QRect textRect = fm.boundingRect(name);
            textRect.moveCenter(QPoint(static_cast<int>(screenPos.x()), static_cast<int>(screenPos.y()) - 15));

            // Text outline
            painter->setPen(QPen(QColor(0, 0, 0, 180), 2));
            painter->drawText(textRect.translated(1, 1), Qt::AlignCenter, name);

            // Text
            painter->setPen(isSelected ? selectedMarkerColor : textColor);
            painter->drawText(textRect, Qt::AlignCenter, name);
        }
    }
}

QString MapRenderer::hitTestCountry(double screenX, double screenY) {
    if (!m_camera || !m_geojson || !m_geojson->isLoaded()) return QString();

    QPointF geo = m_camera->screenToGeo(screenX, screenY, width(), height());
    double lat = geo.x();
    double lon = geo.y();

    for (const auto& feature : m_geojson->features()) {
        if (feature.type != "country") continue;

        for (const QPolygonF& poly : feature.polygons) {
            if (pointInPolygon(poly, lat, lon)) {
                return feature.code;
            }
        }
    }

    return QString();
}

QString MapRenderer::hitTestCity(double screenX, double screenY) {
    if (!m_camera || !m_geojson || !m_geojson->isLoaded()) return QString();

    double viewW = width();
    double viewH = height();
    double hitRadius = 15.0;  // pixels

    for (const auto& feature : m_geojson->features()) {
        if (feature.type != "city") continue;

        QPointF screenPos = m_camera->geoToScreen(feature.centroid.x(), feature.centroid.y(), viewW, viewH);

        double dx = screenPos.x() - screenX;
        double dy = screenPos.y() - screenY;
        double dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= hitRadius) {
            return feature.name;
        }
    }

    return QString();
}

bool MapRenderer::pointInPolygon(const QPolygonF& polygon, double lat, double lon) const {
    QPointF testPoint(lat, lon);
    return polygon.containsPoint(testPoint, Qt::OddEvenFill);
}

void MapRenderer::selectFeatureAt(double screenX, double screenY) {
    // First check cities (smaller hit targets, higher priority)
    QString cityName = hitTestCity(screenX, screenY);
    if (!cityName.isEmpty()) {
        // Find the city feature
        for (const auto& feature : m_geojson->features()) {
            if (feature.type == "city" && feature.name == cityName) {
                m_selectedFeatureCode = feature.code;
                m_selectedFeatureName = feature.name;
                m_selectedFeatureType = "city";
                emit selectedFeatureChanged();
                emit featureClicked(feature.code, feature.name, "city");
                update();
                return;
            }
        }
    }

    // Then check countries
    QString countryCode = hitTestCountry(screenX, screenY);
    if (!countryCode.isEmpty()) {
        const GeoFeature* feature = m_geojson->findByCode(countryCode);
        if (feature) {
            m_selectedFeatureCode = countryCode;
            m_selectedFeatureName = feature->name;
            m_selectedFeatureType = "country";
            emit selectedFeatureChanged();
            emit featureClicked(countryCode, feature->name, "country");
            update();
            return;
        }
    }

    // Nothing hit - clear selection
    clearSelection();
}

void MapRenderer::clearSelection() {
    if (!m_selectedFeatureCode.isEmpty() || !m_selectedFeatureName.isEmpty()) {
        m_selectedFeatureCode.clear();
        m_selectedFeatureName.clear();
        m_selectedFeatureType.clear();
        emit selectedFeatureChanged();
        update();
    }
}

void MapRenderer::toggleFeatureHighlight(const QString& code, const QColor& fillColor, const QColor& borderColor) {
    if (m_highlights.contains(code)) {
        clearHighlight(code);
    } else {
        highlightRegion(code, fillColor, borderColor);
    }
}

void MapRenderer::frameSelectedFeature() {
    if (!m_camera || !m_geojson || m_selectedFeatureName.isEmpty()) {
        return;
    }

    // Find the feature
    const GeoFeature* feature = nullptr;
    if (!m_selectedFeatureCode.isEmpty()) {
        feature = m_geojson->findByCode(m_selectedFeatureCode);
    }
    if (!feature) {
        feature = m_geojson->findByName(m_selectedFeatureName);
    }
    if (!feature) {
        return;
    }

    // For cities, just center on the point with a reasonable zoom
    if (m_selectedFeatureType == "city") {
        m_camera->setPosition(feature->centroid.y(), feature->centroid.x(), 10.0,
                              m_camera->bearing(), m_camera->tilt());
        return;
    }

    // For countries/regions, calculate bounding box from polygons
    if (feature->polygons.isEmpty()) {
        // Fall back to centroid
        m_camera->setPosition(feature->centroid.y(), feature->centroid.x(), 6.0,
                              m_camera->bearing(), m_camera->tilt());
        return;
    }

    // Calculate bounding box (polygons store lat in x, lon in y)
    double minLat = 90.0, maxLat = -90.0;
    double minLon = 180.0, maxLon = -180.0;

    for (const auto& polygon : feature->polygons) {
        for (const auto& point : polygon) {
            // point.x() = lat, point.y() = lon (based on coordinate system used)
            double lat = point.x();
            double lon = point.y();
            minLat = qMin(minLat, lat);
            maxLat = qMax(maxLat, lat);
            minLon = qMin(minLon, lon);
            maxLon = qMax(maxLon, lon);
        }
    }

    // Calculate center
    double centerLat = (minLat + maxLat) / 2.0;
    double centerLon = (minLon + maxLon) / 2.0;

    // Calculate zoom level to fit the bounding box
    double latSpan = maxLat - minLat;
    double lonSpan = maxLon - minLon;

    // Use viewport dimensions to calculate required zoom
    double viewWidth = width();
    double viewHeight = height();
    if (viewWidth <= 0) viewWidth = 800;
    if (viewHeight <= 0) viewHeight = 600;

    // Calculate zoom based on the larger span (with some padding)
    double latZoom = std::log2(180.0 / (latSpan * 1.2)) + 1;
    double lonZoom = std::log2(360.0 / (lonSpan * 1.2)) + 1;

    // Adjust for viewport aspect ratio
    double aspectRatio = viewWidth / viewHeight;
    if (aspectRatio > 1.0) {
        latZoom -= std::log2(aspectRatio) * 0.5;
    } else {
        lonZoom += std::log2(aspectRatio) * 0.5;
    }

    double zoom = qMin(latZoom, lonZoom);
    zoom = qBound(1.0, zoom, 18.0);

    m_camera->setPosition(centerLat, centerLon, zoom,
                          m_camera->bearing(), m_camera->tilt());
}
