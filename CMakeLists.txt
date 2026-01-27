cmake_minimum_required(VERSION 3.21)
project(TristansKortAnimator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Version configuration
set(VERSION_HEADER "${CMAKE_BINARY_DIR}/version.h")
configure_file(
    "${CMAKE_SOURCE_DIR}/src/version.h.in"
    "${VERSION_HEADER}"
    @ONLY
)

# Qt 6 modules
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    QuickControls2
    Network
    Widgets
    Quick3D
)

# Silence Qt policy warnings
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

# Enable QML debugging in debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(QT_QML_DEBUG)
endif()

# Enable parallel builds
if(NOT DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
    include(ProcessorCount)
    ProcessorCount(N)
    if(N GREATER 0)
        set(CMAKE_BUILD_PARALLEL_LEVEL ${N})
    endif()
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/core/settings.cpp
    src/map/tileprovider.cpp
    src/map/tilecache.cpp
    src/map/mapcamera.cpp
    src/map/maprenderer.cpp
    src/map/geojsonparser.cpp
    src/animation/keyframe.cpp
    src/animation/keyframemodel.cpp
    src/animation/regiontrackmodel.cpp
    src/animation/geooverlaymodel.cpp
    src/animation/interpolator.cpp
    src/animation/animationcontroller.cpp
    src/animation/framebuffer.cpp
    src/animation/overlaykeyframe.cpp
    src/overlays/overlay.cpp
    src/overlays/markeroverlay.cpp
    src/overlays/arrowoverlay.cpp
    src/overlays/textoverlay.cpp
    src/overlays/regionhighlight.cpp
    src/overlays/overlaymanager.cpp
    src/export/ffmpegpipeline.cpp
    src/export/framecapturer.cpp
    src/export/videoexporter.cpp
    src/controllers/maincontroller.cpp
    src/core/projectmanager.cpp
    src/3d/globegeometry.cpp
    src/3d/countrygeometry.cpp
    src/3d/globecamera.cpp
)

set(HEADERS
    src/core/settings.h
    src/map/tileprovider.h
    src/map/tilecache.h
    src/map/mapcamera.h
    src/map/maprenderer.h
    src/map/geojsonparser.h
    src/animation/keyframe.h
    src/animation/keyframemodel.h
    src/animation/regiontrack.h
    src/animation/regiontrackmodel.h
    src/animation/geooverlay.h
    src/animation/geooverlaymodel.h
    src/animation/interpolator.h
    src/animation/easingfunctions.h
    src/animation/animationcontroller.h
    src/animation/framebuffer.h
    src/animation/overlaykeyframe.h
    src/overlays/overlay.h
    src/overlays/markeroverlay.h
    src/overlays/arrowoverlay.h
    src/overlays/textoverlay.h
    src/overlays/regionhighlight.h
    src/overlays/overlaymanager.h
    src/export/ffmpegpipeline.h
    src/export/framecapturer.h
    src/export/videoexporter.h
    src/controllers/maincontroller.h
    src/core/projectmanager.h
    src/3d/globegeometry.h
    src/3d/countrygeometry.h
    src/3d/globecamera.h
)

# Resources
qt_add_resources(RESOURCES resources/resources.qrc)

# Main executable
qt_add_executable(TristansKortAnimator
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# Link libraries
target_link_libraries(TristansKortAnimator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Widgets
    Qt6::Quick3D
)

# Include directories
target_include_directories(TristansKortAnimator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
)

# QML module - mark Theme.qml as singleton
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

# Define QML files list
set(QML_FILES
    qml/main.qml
    qml/main_test3d.qml
    qml/Theme.qml
    qml/components/MapView.qml
    qml/components/GlobeView.qml
    qml/components/Timeline.qml
    qml/components/KeyframeMarker.qml
    qml/components/PreviewControls.qml
    qml/components/PropertyPanel.qml
    qml/components/KeyframePanel.qml
    qml/components/OverlayList.qml
    qml/components/OverlayPanel.qml
    qml/components/MarkerEditor.qml
    qml/components/ArrowEditor.qml
    qml/components/RegionPicker.qml
    qml/components/RegionTrackTimeline.qml
    qml/components/GeoOverlayTimeline.qml
    qml/components/OverlayTimeline.qml
    qml/components/ColorPicker.qml
    qml/components/ExportDialog.qml
    qml/components/StyledButton.qml
    qml/components/StyledSlider.qml
    qml/components/StyledComboBox.qml
    qml/pages/SettingsPage.qml
)

qt_add_qml_module(TristansKortAnimator
    URI TristansKortAnimator
    VERSION 1.0
    QML_FILES ${QML_FILES}
)

# Add QML files as sources so IDE can find them
set_property(TARGET TristansKortAnimator APPEND PROPERTY SOURCES ${QML_FILES})

# Windows specific
if(WIN32)
    set_target_properties(TristansKortAnimator PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
