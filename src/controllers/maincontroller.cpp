#include "maincontroller.h"
#include "../core/settings.h"
#include "../core/projectmanager.h"
#include "../map/mapcamera.h"
#include "../map/maprenderer.h"
#include "../map/tileprovider.h"
#include "../map/tilecache.h"
#include "../map/geojsonparser.h"
#include "../animation/keyframemodel.h"
#include "../animation/regiontrackmodel.h"
#include "../animation/geooverlaymodel.h"
#include "../animation/animationcontroller.h"
#include "../animation/framebuffer.h"
#include "../overlays/overlaymanager.h"
#include "../export/videoexporter.h"
#include "../map/cityboundaryfetcher.h"
#include <QtMath>

MainController::MainController(QObject* parent)
    : QObject(parent)
{
    // Create all subsystems
    m_settings = new Settings(this);
    m_keyframes = new KeyframeModel(this);
    m_regionTracks = new RegionTrackModel(this);
    m_geoOverlays = new GeoOverlayModel(this);
    m_overlays = new OverlayManager(this);
    m_camera = new MapCamera(this);
    m_tileProvider = new TileProvider(this);
    m_tileCache = new TileCache(m_settings->tileCacheMaxMB(), this);
    m_geojson = new GeoJsonParser(this);
    m_animation = new AnimationController(this);
    m_exporter = new VideoExporter(this);
    m_frameBuffer = new FrameBuffer(this);
    m_cityBoundaryFetcher = new CityBoundaryFetcher(this);

    // ProjectManager needs keyframes and overlays for save/load
    m_projectManager = new ProjectManager(m_keyframes, m_overlays, this);
    m_projectManager->setGeoOverlayModel(m_geoOverlays);
    m_projectManager->setSettings(m_settings);

    // Setup animation controller
    m_animation->setKeyframeModel(m_keyframes);
    m_animation->setCamera(m_camera);

    // Set animation controller on project manager for save/load
    m_projectManager->setAnimationController(m_animation);

    // Setup exporter
    m_exporter->setAnimationController(m_animation);

    // Setup tile cache
    m_tileCache->setMaxDiskCacheMB(m_settings->diskCacheMaxMB());
    m_tileCache->enableDiskCache(m_settings->tileCachePath());

    // Apply initial settings
    m_tileProvider->setCurrentSource(m_settings->tileSource());

    // Setup connections
    setupConnections();

    // Load GeoJSON data
    loadGeoJsonData();
}

MainController::~MainController() {
    // Children are automatically deleted
}

void MainController::setupConnections() {
    // Tile provider <-> cache
    connect(m_tileProvider, &TileProvider::tileReady, this,
            [this](int x, int y, int zoom, const QImage& image) {
        m_tileCache->insert(m_tileProvider->currentSource(), x, y, zoom, image);
    });

    // Settings changes
    connect(m_settings, &Settings::tileSourceChanged, this, [this]() {
        m_tileProvider->setCurrentSource(m_settings->tileSource());
    });

    connect(m_settings, &Settings::tileCacheMaxMBChanged, this, [this]() {
        m_tileCache->setMaxMemorySize(m_settings->tileCacheMaxMB());
    });

    connect(m_settings, &Settings::diskCacheMaxMBChanged, this, [this]() {
        m_tileCache->setMaxDiskCacheMB(m_settings->diskCacheMaxMB());
    });

    // Track data modifications for unsaved changes
    connect(m_keyframes, &KeyframeModel::dataModified, m_projectManager, &ProjectManager::markModified);
    connect(m_overlays, &OverlayManager::dataModified, m_projectManager, &ProjectManager::markModified);

    // Frame buffer connections - invalidate cache when content changes
    connect(m_keyframes, &KeyframeModel::dataModified, m_frameBuffer, &FrameBuffer::invalidate);
    connect(m_keyframes, &KeyframeModel::countChanged, m_frameBuffer, &FrameBuffer::invalidate);
    connect(m_overlays, &OverlayManager::dataModified, m_frameBuffer, &FrameBuffer::invalidate);
    connect(m_keyframes, &KeyframeModel::totalDurationChanged, this, [this]() {
        m_frameBuffer->setTotalDuration(m_keyframes->totalDuration());
    });

    // Keyframe selection - load position to camera when a keyframe is selected
    connect(m_keyframes, &KeyframeModel::keyframeSelected, this, [this](int index) {
        if (index >= 0 && index < m_keyframes->count()) {
            const auto& kf = m_keyframes->at(index);
            m_camera->setPosition(kf.latitude, kf.longitude, kf.zoom(), kf.bearing, kf.tilt);
        }
    });

    // Camera changes - update current keyframe when in edit mode, or auto-create if autoKey is ON
    connect(m_camera, &MapCamera::cameraChanged, this, [this]() {
        // Skip if we're updating camera from animation playback or scrubbing
        if (m_animation->isPlaying() || m_animation->isSeeking()) return;

        if (m_keyframes->editMode()) {
            // Edit mode: update the current keyframe with new position
            m_keyframes->updateCurrentPosition(
                m_camera->latitude(),
                m_camera->longitude(),
                m_camera->zoom(),
                m_camera->bearing(),
                m_camera->tilt()
            );
        } else if (m_settings->autoKey()) {
            // Auto-key mode: automatically create or select keyframe at current time
            ensureKeyframeAtCurrentTime();
        }
    });

    // Update GeoOverlayModel with current animation time for keyframe interpolation
    connect(m_animation, &AnimationController::currentTimeChanged, this, [this]() {
        m_geoOverlays->setCurrentTime(m_animation->currentTime());
    });

    // Track GeoOverlay data modifications for unsaved changes
    connect(m_geoOverlays, &GeoOverlayModel::dataModified, m_projectManager, &ProjectManager::markModified);
}

void MainController::loadGeoJsonData() {
    // Load Natural Earth 50m countries
    if (!m_geojson->loadFromResource(":/geojson/ne_50m_countries.geojson")) {
        qWarning() << "Failed to load countries GeoJSON from resources";
        // Fallback to old sample file
        m_geojson->loadFromResource(":/geojson/countries.geojson");
    }

    // Load Natural Earth 50m states/provinces (appends to existing features)
    if (!m_geojson->appendFromResource(":/geojson/ne_50m_states.geojson")) {
        qWarning() << "Failed to load states GeoJSON from resources";
    }

    // Load Natural Earth 10m cities (appends to existing features)
    if (!m_geojson->appendFromResource(":/geojson/ne_10m_cities.geojson")) {
        qWarning() << "Failed to load cities GeoJSON from resources";
        // Fallback to built-in cities
        m_geojson->loadBuiltInCities();
    }

    qDebug() << "Loaded" << m_geojson->featureCount() << "geographic features";

    // Set the parser on GeoOverlayModel so it can load geometry
    m_geoOverlays->setGeoJsonParser(m_geojson);

    // Set the city boundary fetcher for on-demand city outline download
    m_geoOverlays->setCityBoundaryFetcher(m_cityBoundaryFetcher);
}

void MainController::addKeyframeAtCurrentPosition() {
    // Add keyframe at the current playhead time
    double currentTime = m_animation->currentTime();

    m_keyframes->addKeyframeAtTime(
        m_camera->latitude(),
        m_camera->longitude(),
        m_camera->zoom(),
        m_camera->bearing(),
        m_camera->tilt(),
        currentTime
    );

    // Find the index of the newly added keyframe (may not be at the end due to sorting)
    int newIndex = m_keyframes->keyframeIndexAtTime(currentTime);
    if (newIndex < 0) newIndex = m_keyframes->count() - 1;

    // Select the newly added keyframe and enable edit mode
    m_keyframes->setCurrentIndex(newIndex);
    m_keyframes->setEditMode(true);

    // Precache tiles for the new keyframe
    precacheTilesForPosition(
        m_camera->latitude(),
        m_camera->longitude(),
        m_camera->zoom()
    );
}

void MainController::goToKeyframe(int index) {
    if (index < 0 || index >= m_keyframes->count()) return;

    const auto& kf = m_keyframes->at(index);
    m_camera->setPosition(kf.latitude, kf.longitude, kf.zoom(), kf.bearing, kf.tilt);
    m_keyframes->setCurrentIndex(index);
    m_keyframes->setEditMode(true);  // Enable edit mode when navigating to a keyframe
}

void MainController::setTileSource(int sourceIndex) {
    m_settings->setTileSource(sourceIndex);
}

void MainController::ensureKeyframeAtCurrentTime() {
    if (!m_keyframes || !m_camera) return;

    double currentTime = m_animation->currentTime();
    double tolerance = 100.0;  // 100ms tolerance for "same" keyframe

    // Check if there's already a keyframe near the current time
    int nearIndex = m_keyframes->keyframeNearTime(currentTime, tolerance);

    if (nearIndex >= 0) {
        // Update existing keyframe at this time
        m_keyframes->updateKeyframe(nearIndex, {
            {"latitude", m_camera->latitude()},
            {"longitude", m_camera->longitude()},
            {"zoom", m_camera->zoom()},
            {"bearing", m_camera->bearing()},
            {"tilt", m_camera->tilt()}
        });
        m_keyframes->setCurrentIndex(nearIndex);
    } else {
        // Create new keyframe at current time
        m_keyframes->addKeyframeAtTime(
            m_camera->latitude(),
            m_camera->longitude(),
            m_camera->zoom(),
            m_camera->bearing(),
            m_camera->tilt(),
            currentTime
        );

        // Select the newly created keyframe
        int newIndex = m_keyframes->keyframeIndexAtTime(currentTime);
        if (newIndex >= 0) {
            m_keyframes->setCurrentIndex(newIndex);
        }
    }
}

void MainController::setMapRenderer(MapRenderer* renderer) {
    m_renderer = renderer;
    if (m_renderer) {
        m_renderer->setTileProvider(m_tileProvider);
        m_renderer->setTileCache(m_tileCache);
        m_renderer->setCamera(m_camera);
        m_renderer->setGeoJson(m_geojson);
        m_renderer->setOverlayManager(m_overlays);
        m_renderer->setRegionTrackModel(m_regionTracks);
        m_renderer->setGeoOverlayModel(m_geoOverlays);
        m_renderer->setFrameBuffer(m_frameBuffer);
        m_exporter->setMapRenderer(m_renderer);

        // Update animation time in renderer
        connect(m_animation, &AnimationController::currentTimeChanged, this, [this]() {
            if (m_renderer) {
                m_renderer->setCurrentAnimationTime(m_animation->currentTime());
            }
        });

        // Update total duration in renderer (use AnimationController's duration which supports explicit mode)
        connect(m_animation, &AnimationController::totalDurationChanged, this, [this]() {
            if (m_renderer) {
                m_renderer->setTotalDuration(m_animation->totalDuration());
            }
        });
        // Set initial duration
        m_renderer->setTotalDuration(m_animation->totalDuration());

        m_frameBuffer->setResolution(static_cast<int>(m_renderer->width()),
                                     static_cast<int>(m_renderer->height()));
    }
}

void MainController::precacheTilesForKeyframe(int index) {
    if (index < 0 || index >= m_keyframes->count()) return;

    const auto& kf = m_keyframes->at(index);
    precacheTilesForPosition(kf.latitude, kf.longitude, kf.zoom());
}

void MainController::precacheAllKeyframes() {
    for (int i = 0; i < m_keyframes->count(); i++) {
        precacheTilesForKeyframe(i);
    }
}

void MainController::precacheTilesForPosition(double lat, double lon, double zoom) {
    if (!m_tileProvider) return;

    // Create a temporary camera to calculate tile range
    int zoomLevel = static_cast<int>(std::floor(zoom));
    int maxTile = (1 << zoomLevel) - 1;

    // Calculate center tile coordinates
    double n = std::pow(2.0, zoomLevel);
    double centerTileX = (lon + 180.0) / 360.0 * n;
    double latRad = lat * M_PI / 180.0;
    double centerTileY = (1.0 - std::log(std::tan(latRad) + 1.0 / std::cos(latRad)) / M_PI) / 2.0 * n;

    // Calculate how many tiles we need to cover the viewport
    double scale = std::pow(2.0, zoom - zoomLevel);
    int tilesX = static_cast<int>(std::ceil(PRECACHE_WIDTH / (256.0 * scale) / 2.0)) + 1;
    int tilesY = static_cast<int>(std::ceil(PRECACHE_HEIGHT / (256.0 * scale) / 2.0)) + 1;

    int centerX = static_cast<int>(std::floor(centerTileX));
    int centerY = static_cast<int>(std::floor(centerTileY));

    // Request tiles in a grid around the center
    int source = m_tileProvider->currentSource();
    for (int dy = -tilesY; dy <= tilesY; dy++) {
        for (int dx = -tilesX; dx <= tilesX; dx++) {
            int tx = centerX + dx;
            int ty = centerY + dy;

            // Clamp to valid tile range
            if (tx < 0 || tx > maxTile || ty < 0 || ty > maxTile) continue;

            // Only request if not already cached
            if (!m_tileCache->contains(source, tx, ty, zoomLevel)) {
                m_tileProvider->requestTile(tx, ty, zoomLevel);
            }
        }
    }
}
