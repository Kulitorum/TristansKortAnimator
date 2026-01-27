# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

TristansKortAnimator is a Qt 6/QML map animation application for creating animated map flyovers with:
- Keyframe-based camera animation (position, zoom, bearing, tilt)
- Geographic overlays (countries, regions, cities) with fade in/out
- Multiple tile providers (OSM, satellite, terrain)
- Video export capabilities

## Build Commands

```bash
# Configure (first time)
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build
cmake --build build --config Release

# Run
./build/Release/TristansKortAnimator.exe
```

## Project Structure

```
TristansKortAnimator/
├── src/
│   ├── main.cpp                    # Application entry point
│   ├── controllers/
│   │   └── maincontroller.cpp/h    # Central controller, connects all subsystems
│   ├── animation/
│   │   ├── keyframe.cpp/h          # Camera keyframe (lat, lon, zoom, bearing, tilt, timeMs)
│   │   ├── keyframemodel.cpp/h     # Keyframe list model for QML
│   │   ├── animationcontroller.cpp/h # Playback, interpolation timing
│   │   ├── interpolator.cpp/h      # Ease-in-out camera interpolation
│   │   └── geooverlaymodel.cpp/h   # Geographic overlay management
│   ├── map/
│   │   ├── maprenderer.cpp/h       # OpenGL map rendering
│   │   ├── mapcamera.cpp/h         # Camera state (lat/lon/zoom/bearing/tilt)
│   │   ├── tileprovider.cpp/h      # Tile fetching from various sources
│   │   └── geojsonparser.cpp/h     # GeoJSON country/region/city data
│   ├── core/
│   │   ├── settings.cpp/h          # Application settings
│   │   └── projectmanager.cpp/h    # Project save/load (.tkanim JSON)
│   └── export/
│       └── videoexporter.cpp/h     # FFmpeg video export
├── qml/
│   ├── main.qml                    # Main application window
│   ├── Theme.qml                   # Color/style constants
│   └── components/
│       ├── MapView.qml             # Map display with mouse interaction
│       ├── Timeline.qml            # Timeline with keyframe/overlay tracks
│       ├── KeyframePanel.qml       # Keyframe property editor
│       └── OverlayPanel.qml        # Geographic overlay editor
└── resources/
    └── geojson/                    # Natural Earth country/state/city data
```

## Key Concepts

### Keyframes
Camera positions at specific times. Interpolation uses ease-in-out curves.
- `timeMs`: Position on timeline (milliseconds)
- `latitude`, `longitude`: Geographic position
- `zoom`: Map zoom level (1-19)
- `bearing`: Camera rotation (0-360)
- `tilt`: Camera pitch (0-60)

### Geographic Overlays
Country/region/city highlights with timing:
- `startTime`, `endTime`: When overlay is visible
- `fadeInDuration`, `fadeOutDuration`: Fade timing
- `fillColor`, `borderColor`: Appearance

### Timeline
- Camera track: Keyframe markers, drag to reposition in time
- Overlay tracks: Drag bars to move, trim handles for start/end
- Right-click to pan, scroll wheel to zoom
- Shift+drag for rubber-band selection

## Backup System (gits3)

This project uses **gits3** for version control backup to AWS S3.

### Quick Reference
```bash
gits3                           # Interactive mode (arrow key navigation)
gits3 backup -m "message"       # Create backup
gits3 status                    # Show changes since last backup
gits3 history                   # View commit history
gits3 restore <commit-id>       # Restore to specific version
```

### Interactive Mode
Run `gits3` with no arguments for interactive browser:
- Arrow keys to navigate menus
- Select project, view status, create backups, browse history
- Type `//done` to finish multi-line commit messages

### Common Workflows
```bash
# Daily backup
gits3 backup -m "Add overlay fade support"

# Check what changed
gits3 status

# View recent commits
gits3 history --limit 10

# Restore previous version
gits3 restore abc123

# C/C++ project mode (respects .gitignore, excludes build/)
gits3 backup -c -m "message"
```

### Configuration
- Credentials: `%APPDATA%/gits3/config.json`
- Project ID: `.gits3` file in project root
- Run `gits3 config` to set up credentials

### How It Works
- Files stored by SHA-256 hash (content-addressable)
- Deduplication: identical files never uploaded twice
- Text files gzip compressed
- Each backup creates a commit with full file tree snapshot
- Any version can be restored from the backup set

### More Info
Full documentation: `C:\code\UnityBackup\CLAUDE.md`
