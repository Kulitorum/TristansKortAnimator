pragma Singleton
import QtQuick

QtObject {
    // Colors - Sleek dark theme
    readonly property color backgroundColor: "#0a0e14"
    readonly property color surfaceColor: "#0d151e"
    readonly property color surfaceColorLight: "#111a24"
    readonly property color surfaceColorAlt: "#0f1820"
    readonly property color headerColor: "#080c12"
    readonly property color primaryColor: "#3d9cf0"
    readonly property color primaryColorLight: "#5ab0ff"
    readonly property color primaryColorDark: "#2a7acc"
    readonly property color accentColor: "#3d9cf0"
    readonly property color accentColorLight: "#5ab0ff"
    readonly property color dangerColor: "#ff4757"
    readonly property color successColor: "#4CAF50"
    readonly property color warningColor: "#FF9800"
    readonly property color textColor: "#e0e6ed"
    readonly property color textColorDim: "#6b7d8f"
    readonly property color textColorMuted: "#4a5568"
    readonly property color borderColor: "#1a2a3a"
    readonly property color borderColorLight: "#2a3a4a"

    // Overlay colors for highlighting
    readonly property color highlightRed: "#ff4757"
    readonly property color highlightBlue: "#3d9cf0"
    readonly property color highlightGreen: "#4CAF50"
    readonly property color highlightYellow: "#f7dc6f"
    readonly property color highlightOrange: "#FF9800"
    readonly property color highlightPurple: "#9C27B0"

    // Timeline colors
    readonly property color timelineBackground: "#0a0e14"
    readonly property color timelineRuler: "#080c12"
    readonly property color playheadColor: "#ff4757"
    readonly property color keyframeColor: "#3d9cf0"
    readonly property color keyframeSelectedColor: "#5ab0ff"

    // Fonts
    readonly property string fontFamily: "Segoe UI"
    readonly property int fontSizeSmall: 11
    readonly property int fontSizeNormal: 13
    readonly property int fontSizeLarge: 16
    readonly property int fontSizeTitle: 20
    readonly property int fontSizeHeader: 24

    // Spacing
    readonly property int spacingTiny: 4
    readonly property int spacingSmall: 8
    readonly property int spacingNormal: 12
    readonly property int spacingLarge: 16
    readonly property int spacingXLarge: 24

    // Border radius
    readonly property int radiusSmall: 4
    readonly property int radiusNormal: 8
    readonly property int radiusLarge: 12

    // Animation durations
    readonly property int animationFast: 150
    readonly property int animationNormal: 250
    readonly property int animationSlow: 400

    // Sizing
    readonly property int buttonHeight: 36
    readonly property int inputHeight: 32
    readonly property int toolbarHeight: 48
    readonly property int panelMinWidth: 250
    readonly property int panelMaxWidth: 400
    readonly property int timelineHeight: 280

    // Icon sizes
    readonly property int iconSizeSmall: 16
    readonly property int iconSizeNormal: 24
    readonly property int iconSizeLarge: 32

    // Shadows
    readonly property color shadowColor: "#40000000"
    readonly property int shadowRadius: 8
    readonly property int shadowOffset: 2

    // Map styles (for reference in UI)
    readonly property var tileSourceNames: [
        "OpenStreetMap",
        "CartoDB Dark Matter",
        "CartoDB Positron",
        "Stamen Terrain",
        "OpenTopoMap (Military)",
        "ESRI Satellite"
    ]

    // Interpolation mode names
    readonly property var interpolationNames: [
        "Arc Zoom",
        "Direct Fly",
        "Orbital",
        "Snap Cut",
        "Glide"
    ]

    // Easing names
    readonly property var easingNames: [
        "Linear",
        "Ease In/Out",
        "Ease In",
        "Ease Out",
        "Ease In/Out Cubic",
        "Ease In/Out Quint"
    ]

    // Framerate options
    readonly property var framerateOptions: [24, 30, 60]
}
