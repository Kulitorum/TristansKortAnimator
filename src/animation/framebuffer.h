#pragma once

#include <QObject>
#include <QImage>
#include <QHash>
#include <QMutex>

class FrameBuffer : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool complete READ isComplete NOTIFY completeChanged)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY frameCountChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)

public:
    explicit FrameBuffer(QObject* parent = nullptr);

    // Configuration
    void setFrameRate(int fps);
    void setTotalDuration(double durationMs);
    void setResolution(int width, int height);
    void setMaxMemoryMB(int mb);

    int frameRate() const { return m_fps; }
    double totalDuration() const { return m_totalDurationMs; }
    int width() const { return m_width; }
    int height() const { return m_height; }

    // Enable/disable buffering
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    // Check if we have a complete buffer for looping
    bool isComplete() const { return m_complete; }
    int frameCount() const { return m_frames.size(); }
    int totalFrames() const { return m_totalFrames; }
    double progress() const;

    // Frame access
    bool hasFrame(double timeMs) const;
    QImage getFrame(double timeMs) const;
    void storeFrame(double timeMs, const QImage& frame);

    // Get the frame time quantized to frame rate
    double quantizeTime(double timeMs) const;
    int timeToFrameIndex(double timeMs) const;
    double frameIndexToTime(int index) const;

    // Clear buffer
    Q_INVOKABLE void clear();
    Q_INVOKABLE void invalidate();

signals:
    void completeChanged();
    void frameCountChanged();
    void enabledChanged();
    void progressChanged();
    void bufferInvalidated();

private:
    void updateComplete();
    void checkMemoryLimit();

    mutable QMutex m_mutex;
    QHash<int, QImage> m_frames;  // frame index -> image

    int m_fps = 30;
    double m_totalDurationMs = 0.0;
    int m_width = 1920;
    int m_height = 1080;
    int m_totalFrames = 0;
    int m_maxMemoryBytes = 512 * 1024 * 1024;  // 512 MB default

    bool m_enabled = true;
    bool m_complete = false;
};
