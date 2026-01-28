#include "animationcontroller.h"
#include "keyframemodel.h"
#include "interpolator.h"
#include "../map/mapcamera.h"

AnimationController::AnimationController(QObject* parent)
    : QObject(parent)
    , m_interpolator(new Interpolator(this))
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(TICK_INTERVAL_MS);
    connect(m_timer, &QTimer::timeout, this, &AnimationController::tick);
}

void AnimationController::setKeyframeModel(KeyframeModel* model) {
    if (m_keyframes) {
        disconnect(m_keyframes, nullptr, this, nullptr);
    }
    m_keyframes = model;
    if (m_keyframes) {
        connect(m_keyframes, &KeyframeModel::totalDurationChanged,
                this, &AnimationController::totalDurationChanged);
    }
    emit totalDurationChanged();
}

void AnimationController::setCamera(MapCamera* camera) {
    m_camera = camera;
}

double AnimationController::totalDuration() const {
    if (m_useExplicitDuration) {
        return m_explicitDuration;
    }
    // Fall back to keyframe-based duration, but ensure minimum 60 seconds
    double kfDuration = m_keyframes ? m_keyframes->totalDuration() : 0.0;
    return qMax(kfDuration, 60000.0);
}

void AnimationController::setExplicitDuration(double durationMs) {
    durationMs = qMax(1000.0, durationMs);  // Minimum 1 second
    if (!qFuzzyCompare(m_explicitDuration, durationMs)) {
        m_explicitDuration = durationMs;
        emit explicitDurationChanged();
        emit totalDurationChanged();
    }
}

void AnimationController::setUseExplicitDuration(bool use) {
    if (m_useExplicitDuration != use) {
        m_useExplicitDuration = use;
        emit useExplicitDurationChanged();
        emit totalDurationChanged();
    }
}

void AnimationController::play() {
    if (m_playing) return;
    // Allow playback even without keyframes (for CRC-only animations)
    // Duration comes from explicit duration or keyframes

    m_playing = true;
    m_elapsed.start();
    m_lastTickTime = 0;
    m_timer->start();
    emit playingChanged();
}

void AnimationController::pause() {
    if (!m_playing) return;

    m_playing = false;
    m_timer->stop();
    emit playingChanged();
}

void AnimationController::stop() {
    m_playing = false;
    m_timer->stop();
    m_currentTimeMs = 0.0;
    m_currentKeyframeIndex = 0;

    emit playingChanged();
    emit currentTimeChanged();
    emit currentKeyframeIndexChanged();

    updateCameraFromTime(0.0);
}

void AnimationController::togglePlayPause() {
    if (m_playing) {
        pause();
    } else {
        play();
    }
}

void AnimationController::seekTo(double timeMs) {
    setCurrentTime(timeMs);
}

void AnimationController::setCurrentTime(double timeMs) {
    // Only clamp to >= 0, allow seeking beyond last keyframe to add new ones
    timeMs = qMax(0.0, timeMs);

    if (!qFuzzyCompare(m_currentTimeMs + 1.0, timeMs + 1.0)) {  // Add 1.0 to handle near-zero values
        m_currentTimeMs = timeMs;
        updateCameraFromTime(timeMs);
        emit currentTimeChanged();

        // Update current keyframe index
        if (m_keyframes) {
            int newIndex = m_keyframes->keyframeIndexAtTime(timeMs);
            if (newIndex != m_currentKeyframeIndex) {
                m_currentKeyframeIndex = newIndex;
                emit currentKeyframeIndexChanged();
            }
        }
    }
}

void AnimationController::setPlaybackSpeed(double speed) {
    speed = qBound(0.1, speed, 4.0);
    if (!qFuzzyCompare(m_playbackSpeed, speed)) {
        m_playbackSpeed = speed;
        emit playbackSpeedChanged();
    }
}

void AnimationController::setLooping(bool loop) {
    if (m_looping != loop) {
        m_looping = loop;
        emit loopingChanged();
    }
}

void AnimationController::stepForward() {
    if (!m_keyframes || m_keyframes->count() == 0) return;

    int nextIndex = m_currentKeyframeIndex + 1;
    if (nextIndex >= m_keyframes->count()) {
        nextIndex = m_looping ? 0 : m_keyframes->count() - 1;
    }

    const auto& kf = m_keyframes->at(nextIndex);
    setCurrentTime(kf.timeMs);
}

void AnimationController::stepBackward() {
    if (!m_keyframes || m_keyframes->count() == 0) return;

    int prevIndex = m_currentKeyframeIndex - 1;
    if (prevIndex < 0) {
        prevIndex = m_looping ? m_keyframes->count() - 1 : 0;
    }

    const auto& kf = m_keyframes->at(prevIndex);
    setCurrentTime(kf.timeMs);
}

void AnimationController::tick() {
    if (!m_playing) {
        return;
    }

    qint64 currentTick = m_elapsed.elapsed();
    qint64 deltaMs = currentTick - m_lastTickTime;
    m_lastTickTime = currentTick;

    // Advance time
    double newTime = m_currentTimeMs + deltaMs * m_playbackSpeed;

    // Get both durations
    double keyframeDuration = m_keyframes ? m_keyframes->totalDuration() : 0.0;
    double explicitDuration = totalDuration();

    if (m_looping) {
        // Loop at last keyframe time
        if (keyframeDuration > 0 && newTime >= keyframeDuration) {
            newTime = std::fmod(newTime, keyframeDuration);
        }
    } else {
        // Stop at chosen duration
        if (explicitDuration > 0 && newTime >= explicitDuration) {
            newTime = explicitDuration;
            pause();
            emit animationComplete();
        }
    }

    setCurrentTime(newTime);
    emit frameRendered(m_currentTimeMs);
}

void AnimationController::updateCameraFromTime(double timeMs) {
    if (!m_camera || !m_keyframes || m_keyframes->count() == 0) {
        return;
    }

    // Set seeking flag to prevent feedback loops
    m_seeking = true;

    // Handle single keyframe case
    if (m_keyframes->count() == 1) {
        const auto& kf = m_keyframes->at(0);
        m_camera->setPosition(kf.latitude, kf.longitude, kf.zoom(), kf.bearing, kf.tilt);
        m_seeking = false;
        return;
    }

    // Find which keyframe transition we're in
    int fromIndex, toIndex;
    double progress = m_keyframes->progressAtTime(timeMs, fromIndex, toIndex);

    // If at the last keyframe or invalid, just show it
    if (fromIndex < 0 || fromIndex == toIndex) {
        if (fromIndex >= 0) {
            const auto& kf = m_keyframes->at(fromIndex);
            m_camera->setPosition(kf.latitude, kf.longitude, kf.zoom(), kf.bearing, kf.tilt);
        }
        m_seeking = false;
        return;
    }

    // Interpolate between keyframes with ease-in-out
    const Keyframe& from = m_keyframes->at(fromIndex);
    const Keyframe& to = m_keyframes->at(toIndex);

    CameraState state = m_interpolator->interpolate(from, to, progress);

    // CameraState.zoom() derives zoom from altitude
    m_camera->setPosition(state.latitude, state.longitude, state.zoom(),
                          state.bearing, state.tilt);
    m_seeking = false;
}
