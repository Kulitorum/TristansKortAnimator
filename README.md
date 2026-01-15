# Tristans Kort Animator

A Qt 6 desktop application for creating animated map videos with keyframe-based camera movements, region highlighting, and overlay support.

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

### Region Highlighting
- **Country/State Data**: Natural Earth 50m resolution GeoJSON for countries and states/provinces
- **Region Tracks**: Timeline-based region highlighting with:
  - Customizable fill and border colors
  - Fade in/out timing
  - Drag-to-reposition on timeline
- **City Database**: Built-in major world cities

### Overlays
- **Markers**: Custom pins with labels
- **Arrows**: Bezier curve arrows for showing movement/routes
- **Text Labels**: Positioned text overlays
- **Region Highlights**: Country/state fill with custom colors

### Timeline
- **Visual Timeline**: Horizontal timeline with time ruler
- **Keyframe Markers**: Draggable keyframe indicators
- **Region Tracks**: Additional timeline rows for region highlight timing
- **Zoom Control**: Mouse wheel zoom with shift+wheel for scrolling
- **Multi-Selection**: Shift+drag for rubber band selection, Ctrl+click to toggle

### Video Export
- **FFmpeg Integration**: Pipe frames directly to FFmpeg for encoding
- **Resolution Options**: 1080p (1920x1080) output
- **Framerate Selection**: 24, 30, or 60 fps
- **H.264 MP4**: Standard video format output

## Requirements

- **Qt 6.5+** with modules: Core, Gui, Quick, QuickControls2, Network, Widgets
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
2. Select your Qt 6 kit
3. Build and run

## Usage

### Creating an Animation

1. **Navigate the Map**: Use mouse to pan, scroll wheel to zoom
2. **Add Keyframes**: Click "+ Keyframe" or press the button when at desired camera position
3. **Adjust Timing**: Drag keyframes on the timeline to adjust timing
4. **Set Interpolation**: Select keyframe and choose interpolation mode in the property panel

### Adding Region Highlights

1. Click "+ Region" button
2. Select a country/state from the picker
3. Adjust start time, duration, and fade timing on the region track
4. Right-click track label to edit colors

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
│   ├── animation/      # Keyframe, KeyframeModel, Interpolator, AnimationController
│   ├── overlays/       # Overlay types and OverlayManager
│   ├── export/         # VideoExporter, FFmpegPipeline, FrameCapturer
│   └── controllers/    # MainController
├── qml/
│   ├── main.qml
│   ├── Theme.qml
│   ├── components/     # MapView, Timeline, PropertyPanel, etc.
│   └── pages/          # SettingsPage
└── resources/
    ├── geojson/        # Country and state boundary data
    └── icons/          # SVG icons
```

## License

This project is provided as-is for educational and personal use.

## Acknowledgments

- Map tiles from OpenStreetMap contributors, CartoDB, Stamen Design
- Country/state boundaries from Natural Earth (public domain)
