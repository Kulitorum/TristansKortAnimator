#pragma once

#include <QObject>
#include <QTimer>

class FFmpegPipeline;
class FrameCapturer;
class AnimationController;
class MapRenderer;

class VideoExporter : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool exporting READ isExporting NOTIFY exportingChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(int currentFrame READ currentFrame NOTIFY currentFrameChanged)
    Q_PROPERTY(int totalFrames READ totalFrames NOTIFY totalFramesChanged)

public:
    explicit VideoExporter(QObject* parent = nullptr);
    ~VideoExporter();

    void setAnimationController(AnimationController* controller);
    void setMapRenderer(MapRenderer* renderer);

    bool isExporting() const { return m_exporting; }
    double progress() const { return m_progress; }
    QString status() const { return m_status; }
    int currentFrame() const { return m_currentFrame; }
    int totalFrames() const { return m_totalFrames; }

public slots:
    void startExport(const QString& outputPath, int width, int height, int framerate);
    void cancelExport();

signals:
    void exportingChanged();
    void progressChanged();
    void statusChanged();
    void currentFrameChanged();
    void totalFramesChanged();
    void exportComplete(const QString& path);
    void exportError(const QString& error);
    void exportCancelled();

private slots:
    void processNextFrame();
    void onFFmpegFinished(bool success);
    void onFFmpegError(const QString& error);

private:
    void setStatus(const QString& status);

    FFmpegPipeline* m_ffmpeg = nullptr;
    FrameCapturer* m_capturer = nullptr;
    AnimationController* m_controller = nullptr;
    MapRenderer* m_renderer = nullptr;

    QTimer* m_frameTimer = nullptr;

    bool m_exporting = false;
    bool m_cancelled = false;
    double m_progress = 0.0;
    QString m_status;
    int m_currentFrame = 0;
    int m_totalFrames = 0;

    QString m_outputPath;
    int m_width = 1920;
    int m_height = 1080;
    int m_framerate = 30;
    double m_frameDurationMs = 33.33;
    double m_totalDuration = 0.0;
};
