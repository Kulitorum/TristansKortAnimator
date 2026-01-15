#pragma once

#include <QObject>
#include <QMetaType>

class Settings;
class ProjectManager;
class KeyframeModel;
class RegionTrackModel;
class GeoOverlayModel;
class OverlayManager;
class MapCamera;
class MapRenderer;
class TileProvider;
class TileCache;
class GeoJsonParser;
class AnimationController;
class VideoExporter;
class FrameBuffer;

Q_DECLARE_OPAQUE_POINTER(Settings*)
Q_DECLARE_OPAQUE_POINTER(FrameBuffer*)
Q_DECLARE_OPAQUE_POINTER(ProjectManager*)
Q_DECLARE_OPAQUE_POINTER(KeyframeModel*)
Q_DECLARE_OPAQUE_POINTER(RegionTrackModel*)
Q_DECLARE_OPAQUE_POINTER(GeoOverlayModel*)
Q_DECLARE_OPAQUE_POINTER(OverlayManager*)
Q_DECLARE_OPAQUE_POINTER(MapCamera*)
Q_DECLARE_OPAQUE_POINTER(MapRenderer*)
Q_DECLARE_OPAQUE_POINTER(TileProvider*)
Q_DECLARE_OPAQUE_POINTER(GeoJsonParser*)
Q_DECLARE_OPAQUE_POINTER(AnimationController*)
Q_DECLARE_OPAQUE_POINTER(VideoExporter*)

class MainController : public QObject {
    Q_OBJECT

    Q_PROPERTY(Settings* settings READ settings CONSTANT)
    Q_PROPERTY(ProjectManager* projectManager READ projectManager CONSTANT)
    Q_PROPERTY(KeyframeModel* keyframes READ keyframes CONSTANT)
    Q_PROPERTY(RegionTrackModel* regionTracks READ regionTracks CONSTANT)
    Q_PROPERTY(GeoOverlayModel* geoOverlays READ geoOverlays CONSTANT)
    Q_PROPERTY(OverlayManager* overlays READ overlays CONSTANT)
    Q_PROPERTY(MapCamera* camera READ camera CONSTANT)
    Q_PROPERTY(AnimationController* animation READ animation CONSTANT)
    Q_PROPERTY(VideoExporter* exporter READ exporter CONSTANT)
    Q_PROPERTY(TileProvider* tileProvider READ tileProvider CONSTANT)
    Q_PROPERTY(GeoJsonParser* geojson READ geojson CONSTANT)
    Q_PROPERTY(FrameBuffer* frameBuffer READ frameBuffer CONSTANT)

public:
    explicit MainController(QObject* parent = nullptr);
    ~MainController();

    Settings* settings() const { return m_settings; }
    ProjectManager* projectManager() const { return m_projectManager; }
    KeyframeModel* keyframes() const { return m_keyframes; }
    RegionTrackModel* regionTracks() const { return m_regionTracks; }
    GeoOverlayModel* geoOverlays() const { return m_geoOverlays; }
    OverlayManager* overlays() const { return m_overlays; }
    MapCamera* camera() const { return m_camera; }
    AnimationController* animation() const { return m_animation; }
    VideoExporter* exporter() const { return m_exporter; }
    TileProvider* tileProvider() const { return m_tileProvider; }
    GeoJsonParser* geojson() const { return m_geojson; }
    FrameBuffer* frameBuffer() const { return m_frameBuffer; }

    // Quick actions for QML
    Q_INVOKABLE void addKeyframeAtCurrentPosition();
    Q_INVOKABLE void goToKeyframe(int index);
    Q_INVOKABLE void setTileSource(int sourceIndex);

    // Auto-add keyframe when editing camera - call this when user starts manipulating camera
    // If not within 3 frames of an existing keyframe, adds a new one
    Q_INVOKABLE void ensureKeyframeAtCurrentTime();

    // Tile precaching for smooth playback
    Q_INVOKABLE void precacheTilesForKeyframe(int index);
    Q_INVOKABLE void precacheAllKeyframes();

    // Map renderer connection (set from QML after MapView is created)
    Q_INVOKABLE void setMapRenderer(MapRenderer* renderer);

signals:
    void error(const QString& message);

private:
    void setupConnections();
    void loadGeoJsonData();
    void precacheTilesForPosition(double lat, double lon, double zoom);

    // Viewport size for precaching (1080p)
    static constexpr int PRECACHE_WIDTH = 1920;
    static constexpr int PRECACHE_HEIGHT = 1080;

    Settings* m_settings = nullptr;
    ProjectManager* m_projectManager = nullptr;
    KeyframeModel* m_keyframes = nullptr;
    RegionTrackModel* m_regionTracks = nullptr;
    GeoOverlayModel* m_geoOverlays = nullptr;
    OverlayManager* m_overlays = nullptr;
    MapCamera* m_camera = nullptr;
    MapRenderer* m_renderer = nullptr;
    TileProvider* m_tileProvider = nullptr;
    TileCache* m_tileCache = nullptr;
    GeoJsonParser* m_geojson = nullptr;
    AnimationController* m_animation = nullptr;
    VideoExporter* m_exporter = nullptr;
    FrameBuffer* m_frameBuffer = nullptr;
};
