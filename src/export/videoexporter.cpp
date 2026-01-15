#include "videoexporter.h"
#include "ffmpegpipeline.h"
#include "framecapturer.h"
#include "../animation/animationcontroller.h"
#include "../map/maprenderer.h"

VideoExporter::VideoExporter(QObject* parent)
    : QObject(parent)
    , m_ffmpeg(new FFmpegPipeline(this))
    , m_capturer(new FrameCapturer(this))
    , m_frameTimer(new QTimer(this))
{
    connect(m_ffmpeg, &FFmpegPipeline::finished, this, &VideoExporter::onFFmpegFinished);
    connect(m_ffmpeg, &FFmpegPipeline::error, this, &VideoExporter::onFFmpegError);

    m_frameTimer->setSingleShot(true);
    connect(m_frameTimer, &QTimer::timeout, this, &VideoExporter::processNextFrame);
}

VideoExporter::~VideoExporter() {
    cancelExport();
}

void VideoExporter::setAnimationController(AnimationController* controller) {
    m_controller = controller;
    m_capturer->setAnimationController(controller);
}

void VideoExporter::setMapRenderer(MapRenderer* renderer) {
    m_renderer = renderer;
    m_capturer->setRenderer(renderer);
}

void VideoExporter::startExport(const QString& outputPath, int width, int height, int framerate) {
    if (m_exporting) {
        emit exportError("Export already in progress");
        return;
    }

    if (!m_controller) {
        emit exportError("No animation controller set");
        return;
    }

    m_outputPath = outputPath;
    m_width = width;
    m_height = height;
    m_framerate = framerate;
    m_frameDurationMs = 1000.0 / framerate;

    m_totalDuration = m_controller->totalDuration();
    if (m_totalDuration <= 0) {
        emit exportError("No animation to export");
        return;
    }

    m_totalFrames = static_cast<int>(std::ceil(m_totalDuration / m_frameDurationMs));
    m_currentFrame = 0;
    m_progress = 0.0;
    m_cancelled = false;

    m_capturer->setOutputSize(width, height);

    setStatus("Starting FFmpeg...");
    emit totalFramesChanged();

    if (!m_ffmpeg->start(outputPath, width, height, framerate)) {
        emit exportError("Failed to start FFmpeg");
        return;
    }

    m_exporting = true;
    emit exportingChanged();

    setStatus("Rendering frames...");

    // Start rendering frames
    processNextFrame();
}

void VideoExporter::cancelExport() {
    if (!m_exporting) return;

    m_cancelled = true;
    m_frameTimer->stop();
    m_ffmpeg->abort();

    m_exporting = false;
    emit exportingChanged();
    emit exportCancelled();

    setStatus("Export cancelled");
}

void VideoExporter::processNextFrame() {
    if (m_cancelled || !m_exporting) {
        return;
    }

    double timeMs = m_currentFrame * m_frameDurationMs;

    // Check if we've reached the end
    if (timeMs > m_totalDuration) {
        setStatus("Finalizing video...");
        m_ffmpeg->finish();
        return;
    }

    // Capture frame at this time
    QImage frame = m_capturer->captureFrameAtTime(timeMs);

    // Send to FFmpeg
    m_ffmpeg->writeFrame(frame);

    // Update progress
    m_currentFrame++;
    m_progress = static_cast<double>(m_currentFrame) / m_totalFrames;

    emit progressChanged();
    emit currentFrameChanged();

    setStatus(QString("Rendering frame %1 of %2").arg(m_currentFrame).arg(m_totalFrames));

    // Schedule next frame (use timer to avoid blocking UI)
    m_frameTimer->start(0);
}

void VideoExporter::onFFmpegFinished(bool success) {
    m_exporting = false;
    emit exportingChanged();

    if (success && !m_cancelled) {
        setStatus("Export complete!");
        emit exportComplete(m_outputPath);
    } else if (!m_cancelled) {
        setStatus("Export failed");
        emit exportError("FFmpeg encoding failed");
    }
}

void VideoExporter::onFFmpegError(const QString& error) {
    m_frameTimer->stop();
    m_exporting = false;
    emit exportingChanged();

    setStatus("Export failed: " + error);
    emit exportError(error);
}

void VideoExporter::setStatus(const QString& status) {
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}
