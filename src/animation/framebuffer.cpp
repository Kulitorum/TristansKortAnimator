#include "framebuffer.h"
#include <QMutexLocker>
#include <QtMath>

FrameBuffer::FrameBuffer(QObject* parent)
    : QObject(parent)
{
}

void FrameBuffer::setFrameRate(int fps) {
    QMutexLocker locker(&m_mutex);
    if (m_fps != fps) {
        m_fps = qBound(1, fps, 120);
        m_totalFrames = static_cast<int>(std::ceil(m_totalDurationMs / 1000.0 * m_fps));
        clear();
    }
}

void FrameBuffer::setTotalDuration(double durationMs) {
    QMutexLocker locker(&m_mutex);
    if (!qFuzzyCompare(m_totalDurationMs, durationMs)) {
        m_totalDurationMs = durationMs;
        m_totalFrames = static_cast<int>(std::ceil(m_totalDurationMs / 1000.0 * m_fps));
        m_complete = false;
        emit completeChanged();
    }
}

void FrameBuffer::setResolution(int width, int height) {
    bool needsClear = false;
    {
        QMutexLocker locker(&m_mutex);
        if (m_width != width || m_height != height) {
            m_width = width;
            m_height = height;
            needsClear = true;
        }
    }
    if (needsClear) {
        clear();
    }
}

void FrameBuffer::setMaxMemoryMB(int mb) {
    QMutexLocker locker(&m_mutex);
    m_maxMemoryBytes = mb * 1024 * 1024;
    checkMemoryLimit();
}

void FrameBuffer::setEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        emit enabledChanged();
        if (!enabled) {
            clear();
        }
    }
}

double FrameBuffer::progress() const {
    QMutexLocker locker(&m_mutex);
    if (m_totalFrames <= 0) return 0.0;
    return static_cast<double>(m_frames.size()) / m_totalFrames;
}

bool FrameBuffer::hasFrame(double timeMs) const {
    if (!m_enabled) return false;
    QMutexLocker locker(&m_mutex);
    int frameIndex = timeToFrameIndex(timeMs);
    return m_frames.contains(frameIndex);
}

QImage FrameBuffer::getFrame(double timeMs) const {
    QMutexLocker locker(&m_mutex);
    int frameIndex = timeToFrameIndex(timeMs);
    return m_frames.value(frameIndex, QImage());
}

void FrameBuffer::storeFrame(double timeMs, const QImage& frame) {
    if (!m_enabled || frame.isNull()) return;

    QMutexLocker locker(&m_mutex);
    int frameIndex = timeToFrameIndex(timeMs);

    // Don't store if already have this frame
    if (m_frames.contains(frameIndex)) return;

    // Check memory limit before adding
    checkMemoryLimit();

    m_frames.insert(frameIndex, frame);
    emit frameCountChanged();
    emit progressChanged();

    updateComplete();
}

double FrameBuffer::quantizeTime(double timeMs) const {
    int frameIndex = timeToFrameIndex(timeMs);
    return frameIndexToTime(frameIndex);
}

int FrameBuffer::timeToFrameIndex(double timeMs) const {
    if (m_fps <= 0) return 0;
    double frameInterval = 1000.0 / m_fps;
    return static_cast<int>(std::floor(timeMs / frameInterval));
}

double FrameBuffer::frameIndexToTime(int index) const {
    if (m_fps <= 0) return 0.0;
    double frameInterval = 1000.0 / m_fps;
    return index * frameInterval;
}

void FrameBuffer::clear() {
    {
        QMutexLocker locker(&m_mutex);
        m_frames.clear();
        m_complete = false;
    }
    emit frameCountChanged();
    emit completeChanged();
    emit progressChanged();
}

void FrameBuffer::invalidate() {
    clear();
    emit bufferInvalidated();
}

void FrameBuffer::updateComplete() {
    // Check if we have enough frames for a complete loop
    // We consider complete if we have at least 95% of frames
    bool wasComplete = m_complete;
    int threshold = static_cast<int>(m_totalFrames * 0.95);
    m_complete = (m_frames.size() >= threshold) && (m_totalFrames > 0);

    if (m_complete != wasComplete) {
        emit completeChanged();
    }
}

void FrameBuffer::checkMemoryLimit() {
    // Estimate memory usage: width * height * 4 bytes per frame (ARGB32)
    qint64 bytesPerFrame = static_cast<qint64>(m_width) * m_height * 4;
    qint64 currentUsage = bytesPerFrame * m_frames.size();

    // If over limit, remove oldest frames (lowest indices)
    if (currentUsage > m_maxMemoryBytes && !m_frames.isEmpty()) {
        QList<int> keys = m_frames.keys();
        std::sort(keys.begin(), keys.end());

        while (currentUsage > m_maxMemoryBytes && !keys.isEmpty()) {
            m_frames.remove(keys.takeFirst());
            currentUsage = bytesPerFrame * m_frames.size();
        }

        m_complete = false;
        emit completeChanged();
    }
}
