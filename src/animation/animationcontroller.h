#pragma once

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>

class KeyframeModel;
class Interpolator;
class MapCamera;

class AnimationController : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
    Q_PROPERTY(double currentTime READ currentTime WRITE setCurrentTime NOTIFY currentTimeChanged)
    Q_PROPERTY(double totalDuration READ totalDuration NOTIFY totalDurationChanged)
    Q_PROPERTY(double playbackSpeed READ playbackSpeed WRITE setPlaybackSpeed NOTIFY playbackSpeedChanged)
    Q_PROPERTY(bool looping READ isLooping WRITE setLooping NOTIFY loopingChanged)
    Q_PROPERTY(int currentKeyframeIndex READ currentKeyframeIndex NOTIFY currentKeyframeIndexChanged)

public:
    explicit AnimationController(QObject* parent = nullptr);

    void setKeyframeModel(KeyframeModel* model);
    void setCamera(MapCamera* camera);

    bool isPlaying() const { return m_playing; }
    double currentTime() const { return m_currentTimeMs; }
    double totalDuration() const;
    double playbackSpeed() const { return m_playbackSpeed; }
    bool isLooping() const { return m_looping; }
    int currentKeyframeIndex() const { return m_currentKeyframeIndex; }

public slots:
    void play();
    void pause();
    void stop();
    void togglePlayPause();
    void seekTo(double timeMs);
    void setCurrentTime(double timeMs);
    void setPlaybackSpeed(double speed);
    void setLooping(bool loop);

    void stepForward();   // Go to next keyframe
    void stepBackward();  // Go to previous keyframe

signals:
    void playingChanged();
    void currentTimeChanged();
    void totalDurationChanged();
    void playbackSpeedChanged();
    void loopingChanged();
    void currentKeyframeIndexChanged();
    void animationComplete();
    void frameRendered(double timeMs);

private slots:
    void tick();

private:
    void updateCameraFromTime(double timeMs);

    KeyframeModel* m_keyframes = nullptr;
    MapCamera* m_camera = nullptr;
    Interpolator* m_interpolator = nullptr;

    QTimer* m_timer = nullptr;
    QElapsedTimer m_elapsed;
    qint64 m_lastTickTime = 0;

    bool m_playing = false;
    double m_currentTimeMs = 0.0;
    double m_playbackSpeed = 1.0;
    bool m_looping = false;
    int m_currentKeyframeIndex = 0;

    static constexpr int TICK_INTERVAL_MS = 16;  // ~60fps preview
};
