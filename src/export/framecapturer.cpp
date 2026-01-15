#include "framecapturer.h"
#include "../map/maprenderer.h"
#include "../map/mapcamera.h"
#include "../animation/animationcontroller.h"

FrameCapturer::FrameCapturer(QObject* parent)
    : QObject(parent)
{
}

void FrameCapturer::setRenderer(MapRenderer* renderer) {
    m_renderer = renderer;
}

void FrameCapturer::setCamera(MapCamera* camera) {
    m_camera = camera;
}

void FrameCapturer::setAnimationController(AnimationController* controller) {
    m_controller = controller;
}

void FrameCapturer::setOutputSize(int width, int height) {
    m_width = width;
    m_height = height;
}

QImage FrameCapturer::captureFrame() {
    if (!m_renderer) {
        return QImage(m_width, m_height, QImage::Format_RGBA8888);
    }

    return m_renderer->renderToImage(m_width, m_height);
}

QImage FrameCapturer::captureFrameAtTime(double timeMs) {
    if (!m_controller) {
        return captureFrame();
    }

    // Set animation to specific time
    m_controller->setCurrentTime(timeMs);

    // Capture the frame
    return captureFrame();
}
