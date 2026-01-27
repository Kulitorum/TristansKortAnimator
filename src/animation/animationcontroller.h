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
    Q_PROPERTY(double explicitDuration READ explicitDuration WRITE setExplicitDuration NOTIFY explicitDurationChanged)
    Q_PROPERTY(bool useExplicitDuration READ useExplicitDuration WRITE setUseExplicitDuration NOTIFY useExplicitDurationChanged)

public:
    explicit AnimationController(QObject* parent = nullptr);

    void setKeyframeModel(KeyframeModel* model);
    void setCamera(MapCamera* camera);

    bool isPlaying() const { return m_playing; }
    bool isSeeking() const { return m_seeking; }  // True when updating camera from interpolation
    double currentTime() const { return m_currentTimeMs; }
    double totalDuration() const;
    double playbackSpeed() const { return m_playbackSpeed; }
    bool isLooping() const { return m_looping; }
    int currentKeyframeIndex() const { return m_currentKeyframeIndex; }
    double explicitDuration() const { return m_explicitDuration; }
    bool useExplicitDuration() const { return m_useExplicitDuration; }

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
    void setExplicitDuration(double durationMs);
    void setUseExplicitDuration(bool use);

signals:
    void playingChanged();
    void currentTimeChanged();
    void totalDurationChanged();
    void playbackSpeedChanged();
    void loopingChanged();
    void currentKeyframeIndexChanged();
    void animationComplete();
    void frameRendered(double timeMs);
    void explicitDurationChanged();
    void useExplicitDurationChanged();

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
    bool m_seeking = false;  // True during updateCameraFromTime to prevent feedback loops
    double m_currentTimeMs = 0.0;
    double m_playbackSpeed = 1.0;
    bool m_looping = false;
    int m_currentKeyframeIndex = 0;
    double m_explicitDuration = 60000.0;  // Default 60 seconds
    bool m_useExplicitDuration = true;    // Default to explicit duration mode

    static constexpr int TICK_INTERVAL_MS = 16;  // ~60fps preview
};
