# Architecture

Technical architecture documentation for TristansKortAnimator.

## Overview

TristansKortAnimator is a Qt 6 application using QML for the UI and C++ for the core logic. It follows an MVC-like pattern with clear separation between models, views, and controllers.

```
┌─────────────────────────────────────────────────────────────────┐
│                          QML UI Layer                           │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────────────┐│
│  │ MapView  │  │ Timeline │  │ Panels   │  │ PreviewControls  ││
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────────┬─────────┘│
└───────┼─────────────┼─────────────┼─────────────────┼──────────┘
        │             │             │                 │
┌───────┴─────────────┴─────────────┴─────────────────┴──────────┐
│                      MainController                             │
│         (Central hub connecting all subsystems)                 │
└───────┬─────────────┬─────────────┬─────────────────┬──────────┘
        │             │             │                 │
┌───────┴───┐  ┌──────┴────┐  ┌─────┴─────┐  ┌───────┴─────────┐
│ MapCamera │  │ Keyframe  │  │ GeoOverlay│  │ Animation       │
│           │  │ Model     │  │ Model     │  │ Controller      │
└───────────┘  └───────────┘  └───────────┘  └─────────────────┘
```

## Core Components

### MainController (`src/controllers/maincontroller.cpp`)

The central coordinator that:
- Exposes all models and controllers to QML
- Handles auto-key mode (3ds Max style keyframe creation)
- Manages project save/load
- Coordinates between animation playback and camera updates

### MapCamera (`src/map/mapcamera.cpp`)

Camera state management:
- Position: latitude, longitude
- Zoom: continuous value (1.0-19.0)
- Bearing: rotation in degrees (0-360)
- Tilt: pitch angle (0-60)
- Converts between screen coordinates and geographic coordinates

### MapRenderer (`src/map/maprenderer.cpp`)

QQuickPaintedItem that renders:
1. Map tiles (with fallback for missing tiles)
2. Country borders
3. Geographic overlays (countries, regions, cities)
4. Labels

Tile rendering uses a 0.5px overlap to eliminate seams.

### AnimationController (`src/animation/animationcontroller.cpp`)

Playback engine:
- Timer-based tick at 60fps
- Speed curve support (0-2x playback speed)
- Explicit duration mode (independent of keyframes)
- Interpolates camera between keyframes using `Interpolator`

### Interpolator (`src/animation/interpolator.cpp`)

Camera interpolation:
- Adaptive ease-in-out based on altitude
- Linear mode for speed curve (bypasses per-keyframe easing)
- Longitude wrapping (handles antimeridian crossing)
- Bearing interpolation (shortest path around circle)

### KeyframeModel (`src/animation/keyframemodel.cpp`)

QAbstractListModel for camera keyframes:
- Stores keyframes sorted by time
- Provides `progressAtTime()` for interpolation
- Edit mode support for dragging keyframes

### GeoOverlayModel (`src/animation/geooverlaymodel.cpp`)

QAbstractListModel for geographic overlays:
- Countries, regions, cities
- Timing (start/end time, fade in/out)
- Appearance (fill color, border color, border width)
- Polygon data from GeoJSON

## Data Flow

### Animation Playback

```
Timer tick (16ms)
    │
    ▼
AnimationController::tick()
    │
    ├─► Get speed from speed curve
    │
    ├─► Advance currentTime
    │
    ├─► updateCameraFromTime()
    │       │
    │       ├─► KeyframeModel::progressAtTime()
    │       │
    │       ├─► Interpolator::interpolate()
    │       │
    │       └─► MapCamera::setPosition()
    │
    └─► emit frameRendered()
            │
            └─► MapRenderer::update()
```

### Geographic Overlay Rendering

```
MapRenderer::paint()
    │
    ├─► renderTiles()
    │
    ├─► renderGeoOverlays()
    │       │
    │       ├─► For each overlay:
    │       │       │
    │       │       ├─► Calculate opacity at current time
    │       │       │
    │       │       ├─► Transform polygons to screen coordinates
    │       │       │
    │       │       └─► Draw with QPainter
    │       │
    │       └─► (Skip invisible overlays)
    │
    └─► renderLabels()
```

## File Formats

### Project File (.tkanim)

JSON format containing:
```json
{
  "version": 1,
  "duration": 60000,
  "keyframes": [...],
  "geoOverlays": [...],
  "settings": {...}
}
```

### GeoJSON Data

Natural Earth data in `resources/geojson/`:
- `ne_50m_admin_0_countries.json` - Country boundaries
- `ne_50m_admin_1_states_provinces.json` - State/province boundaries
- `ne_10m_populated_places.json` - City locations

## Threading Model

- **Main thread**: UI, QML, rendering
- **Network thread**: Tile fetching (Qt's internal thread pool)
- **Video export**: Separate thread for FFmpeg encoding

All model updates happen on the main thread. TileProvider uses queued connections for thread-safe tile delivery.

## Build System

CMake with Qt6 integration:
- `qt_add_executable` for main target
- `qt_add_qml_module` for QML files
- `configure_file` for version generation
- Post-build `windeployqt` on Windows
- Optional `installer` target for Inno Setup

## Extension Points

### Adding a New Tile Source

1. Add entry to `TileProvider::sources()` in `tileprovider.cpp`
2. Add URL pattern to `TileProvider::tileUrl()`

### Adding a New Overlay Type

1. Create class inheriting from `Overlay` in `src/overlays/`
2. Add to `OverlayManager`
3. Create QML editor component
4. Add rendering in `MapRenderer::renderOverlays()`

### Adding a New Interpolation Mode

1. Add enum value to `Keyframe::InterpolationMode`
2. Implement in `Interpolator::interpolate()`
3. Add UI option in `KeyframePanel.qml`
