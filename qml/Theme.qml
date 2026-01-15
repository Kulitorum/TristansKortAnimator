pragma Singleton
import QtQuick

QtObject {
    // Colors - Dark theme suitable for geopolitics/war content
    readonly property color backgroundColor: "#1a1a2e"
    readonly property color surfaceColor: "#16213e"
    readonly property color surfaceColorLight: "#1f3460"
    readonly property color primaryColor: "#e94560"
    readonly property color primaryColorLight: "#ff6b6b"
    readonly property color accentColor: "#0f3460"
    readonly property color accentColorLight: "#1a5490"
    readonly property color textColor: "#eaeaea"
    readonly property color textColorDim: "#a0a0a0"
    readonly property color textColorMuted: "#666666"
    readonly property color borderColor: "#333355"
    readonly property color borderColorLight: "#444477"

    // Overlay colors for highlighting
    readonly property color highlightRed: "#e94560"
    readonly property color highlightBlue: "#4a90d9"
    readonly property color highlightGreen: "#4ecdc4"
    readonly property color highlightYellow: "#f7dc6f"
    readonly property color highlightOrange: "#f39c12"
    readonly property color highlightPurple: "#9b59b6"

    // Timeline colors
    readonly property color timelineBackground: "#0d1b2a"
    readonly property color timelineRuler: "#1b263b"
    readonly property color playheadColor: "#e94560"
    readonly property color keyframeColor: "#4ecdc4"
    readonly property color keyframeSelectedColor: "#e94560"

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
    readonly property int timelineHeight: 150

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
