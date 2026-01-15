#pragma once

#include <QObject>
#include <QImage>

class MapRenderer;
class MapCamera;
class AnimationController;

class FrameCapturer : public QObject {
    Q_OBJECT

public:
    explicit FrameCapturer(QObject* parent = nullptr);

    void setRenderer(MapRenderer* renderer);
    void setCamera(MapCamera* camera);
    void setAnimationController(AnimationController* controller);
    void setOutputSize(int width, int height);

    // Capture current state to image
    QImage captureFrame();

    // Capture frame at specific time
    QImage captureFrameAtTime(double timeMs);

    int outputWidth() const { return m_width; }
    int outputHeight() const { return m_height; }

private:
    MapRenderer* m_renderer = nullptr;
    MapCamera* m_camera = nullptr;
    AnimationController* m_controller = nullptr;
    int m_width = 1920;
    int m_height = 1080;
};
