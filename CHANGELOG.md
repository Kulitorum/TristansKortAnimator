# Changelog

All notable changes to TristansKortAnimator will be documented in this file.

## [1.0.0] - 2025-01-28

### Added
- **Installer**: Inno Setup installer script with CMake integration
  - Auto-generates version.iss from CMake project version
  - Auto-generates setupvars.iss with build paths
  - Post-build windeployqt for Qt dependency deployment
  - Optional `installer` CMake target

- **Speed Curve Editor**: Interactive playback speed control on timeline
  - Continuous speed curve with draggable control points
  - Double-click to add points, right-click to remove
  - Speed range 0 (stopped) to 1 (2x speed)
  - Integrates with C++ AnimationController for persistence

- **3D Globe View**: Experimental Qt Quick3D globe rendering
  - GlobeGeometry for sphere mesh generation
  - CountryGeometry for country boundary extrusion
  - GlobeCamera for orbital camera controls

- **Geographic Overlay System**: Countries, regions, and cities
  - Timeline tracks with drag-to-reposition
  - Fade in/out duration controls
  - White border-only default style
  - GeoJSON data from Natural Earth

- **Tile Rendering Fix**: Eliminated black seams between map tiles
  - Added 0.5px overlap to tile destination rectangles

### Changed
- **UI Overhaul**: Sleek dark theme across all panels
  - Consolidated CRC buttons into single "Territories" dropdown
  - Removed unnecessary toolbar buttons
  - Improved timeline mouse handling (left-click drags, right-click pans)

- **Timeline**: Professional video editor style
  - Camera track with keyframe markers
  - Overlay tracks with draggable/trimmable bars
  - Scroll wheel for vertical/horizontal navigation

### Fixed
- CRC overlay opacity/fade now works correctly during playback
- Timeline allows playback without keyframes (CRC-only animations)
- Explicit duration mode for timeline independent of keyframes

## [0.1.0] - 2025-01-15

### Added
- Initial release
- Map rendering with 5 tile sources
- Keyframe-based camera animation
- Multiple interpolation modes (Arc Zoom, Direct Fly, Orbital, etc.)
- Video export via FFmpeg
- Project save/load (.tkanim format)
