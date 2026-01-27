# Tristans Kort Animator

A Qt 6 desktop application for creating animated map videos with keyframe-based camera movements, geographic overlays, and timeline-based animation control.

## Features

### Map Rendering
- **5 Tile Sources**: OpenStreetMap, CartoDB Dark Matter, CartoDB Positron, Stamen Terrain, OpenTopoMap
- **Smooth Navigation**: Pan, zoom, rotate (bearing), and tilt controls
- **Tile Caching**: Memory and disk caching for fast tile loading

### Keyframe Animation
- **Camera Keyframes**: Define camera positions (lat, lon, zoom, bearing, tilt) at specific times
- **Multiple Interpolation Modes**:
  - Arc Zoom - Google Earth style zoom out, pan, zoom in
  - Direct Fly - Straight line path with adaptive zoom
  - Orbital - Zoom to space, rotate, dive in
  - Snap Cut - Instant jump
  - Glide - Smooth continuous pan
- **Easing Functions**: Linear, ease-in, ease-out, ease-in-out, cubic, elastic, bounce

### Geographic Overlays
- **Country Highlighting**: Add countries with white border outlines on the map
- **Region/State Highlighting**: Add states and provinces with customizable borders
- **City Markers**: Place city markers with labels
- **Border-Only Style**: Clean white border highlighting without fill for professional appearance
- **Timeline Integration**: All overlays appear on timeline tracks with:
  - Adjustable start and end times
  - Drag-to-reposition timing
  - Visual track bars showing duration

### GeoJSON Data
- **Natural Earth 50m Countries**: High-quality country boundary data
- **Natural Earth 50m States/Provinces**: Regional boundaries for detailed maps
- **Natural Earth 10m Cities**: Major world cities database

### Timeline
- **Visual Timeline**: Horizontal timeline with time ruler and zoom control
- **Camera Track**: Keyframe markers for camera positions
- **Overlay Tracks**: Separate tracks for each geographic overlay
- **Integrated Labels**: Clean track labels matching the camera track style
- **Mouse Interaction**: Drag track bars to reposition, resize from edges, click to select
- **Zoom Control**: Mouse wheel zoom with shift+wheel for scrolling

### Video Export
- **FFmpeg Integration**: Pipe frames directly to FFmpeg for encoding
- **Resolution Options**: 1080p (1920x1080) output
- **Framerate Selection**: 24, 30, or 60 fps
- **H.264 MP4**: Standard video format output

## Requirements

- **Qt 6.5+** with modules: Core, Gui, Quick, QuickControls2, Network, Widgets, Quick3D
- **CMake 3.21+**
- **C++17 compatible compiler** (MSVC 2022, GCC 11+, Clang 13+)
- **FFmpeg** (for video export, must be in PATH)

## Building

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run
./build/Release/TristansKortAnimator
```

### Windows with Qt Creator
1. Open `CMakeLists.txt` in Qt Creator
2. Select your Qt 6 kit (with Quick3D module)
3. Build and run

## Usage

### Creating an Animation

1. **Navigate the Map**: Use mouse to pan, scroll wheel to zoom
2. **Add Keyframes**: Click "+ Keyframe" or press the button when at desired camera position
3. **Adjust Timing**: Drag keyframes on the timeline to adjust timing
4. **Set Interpolation**: Select keyframe and choose interpolation mode in the property panel

### Adding Geographic Overlays

1. **Countries**: Click "Country" button, search and select from the list
2. **Regions**: Click "Region" button, select country first, then choose region/state
3. **Cities**: Click "City" button, search and select a city
4. **Adjust Timing**: Drag overlay track bars on the timeline to set when overlays appear
5. **Customize**: Overlays appear with white border outlines by default

### Exporting Video

1. Click the Export button
2. Choose output path, resolution, and framerate
3. Click Export and wait for FFmpeg to encode

## Project Structure

```
TristansKortAnimator/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── core/           # Settings, ProjectManager
│   ├── map/            # MapRenderer, TileProvider, TileCache, MapCamera, GeoJsonParser
│   ├── animation/      # Keyframe, KeyframeModel, GeoOverlayModel, AnimationController
│   ├── overlays/       # Legacy overlay types and OverlayManager
│   ├── export/         # VideoExporter, FFmpegPipeline
│   ├── controllers/    # MainController
│   └── 3d/             # GlobeGeometry, CountryGeometry, GlobeCamera (3D globe support)
├── qml/
│   ├── main.qml
│   ├── Theme.qml
│   └── components/     # MapView, Timeline, OverlayPanel, PropertyPanel, etc.
└── resources/
    ├── geojson/        # Natural Earth country, state, and city data
    └── icons/          # SVG icons
```

## License

This project is provided as-is for educational and personal use.

## Acknowledgments

- Map tiles from OpenStreetMap contributors, CartoDB, Stamen Design
- Country/state/city boundaries from Natural Earth (public domain)
