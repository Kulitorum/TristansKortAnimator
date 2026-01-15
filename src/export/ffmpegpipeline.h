#pragma once

#include <QObject>
#include <QProcess>
#include <QImage>

class FFmpegPipeline : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)

public:
    explicit FFmpegPipeline(QObject* parent = nullptr);
    ~FFmpegPipeline();

    Q_INVOKABLE bool start(const QString& outputPath, int width, int height, int framerate);
    Q_INVOKABLE void writeFrame(const QImage& frame);
    Q_INVOKABLE void finish();
    Q_INVOKABLE void abort();

    Q_INVOKABLE static bool isFFmpegAvailable();
    Q_INVOKABLE static QString findFFmpegPath();
    Q_INVOKABLE void setFFmpegPath(const QString& path);

    bool isRunning() const { return m_running; }

signals:
    void started();
    void finished(bool success);
    void error(const QString& message);
    void progressUpdate(int framesWritten);
    void runningChanged();

private slots:
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);
    void onProcessError(QProcess::ProcessError error);
    void onReadyReadStandardError();

private:
    QProcess* m_process = nullptr;
    QString m_ffmpegPath;
    int m_framesWritten = 0;
    bool m_running = false;
    QString m_errorOutput;
};
