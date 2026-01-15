#include "ffmpegpipeline.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>

FFmpegPipeline::FFmpegPipeline(QObject* parent)
    : QObject(parent)
{
    m_ffmpegPath = findFFmpegPath();
}

FFmpegPipeline::~FFmpegPipeline() {
    abort();
}

bool FFmpegPipeline::start(const QString& outputPath, int width, int height, int framerate) {
    if (m_running) return false;

    if (m_ffmpegPath.isEmpty()) {
        emit error("FFmpeg not found. Please install FFmpeg and add it to PATH.");
        return false;
    }

    m_framesWritten = 0;
    m_errorOutput.clear();

    QStringList args;
    args << "-y"                              // Overwrite output
         << "-f" << "rawvideo"                // Input format
         << "-pix_fmt" << "rgba"              // Input pixel format
         << "-s" << QString("%1x%2").arg(width).arg(height)  // Input size
         << "-r" << QString::number(framerate)               // Input framerate
         << "-i" << "-"                       // Read from stdin
         << "-c:v" << "libx264"               // H.264 codec
         << "-preset" << "medium"             // Encoding speed/quality tradeoff
         << "-crf" << "18"                    // Quality (lower = better, 18 is visually lossless)
         << "-pix_fmt" << "yuv420p"           // Output pixel format for compatibility
         << "-movflags" << "+faststart"       // Enable streaming
         << outputPath;

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::started, this, &FFmpegPipeline::onProcessStarted);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &FFmpegPipeline::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &FFmpegPipeline::onProcessError);
    connect(m_process, &QProcess::readyReadStandardError, this, &FFmpegPipeline::onReadyReadStandardError);

    m_process->start(m_ffmpegPath, args);

    if (!m_process->waitForStarted(5000)) {
        emit error("Failed to start FFmpeg process");
        delete m_process;
        m_process = nullptr;
        return false;
    }

    m_running = true;
    emit runningChanged();
    return true;
}

void FFmpegPipeline::writeFrame(const QImage& frame) {
    if (!m_running || !m_process) return;

    // Convert to RGBA format if needed
    QImage rgbaFrame = frame.format() == QImage::Format_RGBA8888
                       ? frame
                       : frame.convertToFormat(QImage::Format_RGBA8888);

    // Write raw pixel data to stdin
    qint64 written = m_process->write(
        reinterpret_cast<const char*>(rgbaFrame.constBits()),
        rgbaFrame.sizeInBytes()
    );

    if (written > 0) {
        m_framesWritten++;
        emit progressUpdate(m_framesWritten);
    }
}

void FFmpegPipeline::finish() {
    if (!m_running || !m_process) return;

    // Close stdin to signal end of input
    m_process->closeWriteChannel();

    // Wait for process to finish (with timeout)
    if (!m_process->waitForFinished(30000)) {
        emit error("FFmpeg timed out while finishing");
        abort();
    }
}

void FFmpegPipeline::abort() {
    if (!m_process) return;

    m_process->kill();
    m_process->waitForFinished(5000);

    delete m_process;
    m_process = nullptr;
    m_running = false;
    emit runningChanged();
}

bool FFmpegPipeline::isFFmpegAvailable() {
    return !findFFmpegPath().isEmpty();
}

QString FFmpegPipeline::findFFmpegPath() {
    // Try common locations
    QStringList possiblePaths = {
        "ffmpeg",  // In PATH
        "C:/ffmpeg/bin/ffmpeg.exe",
        "C:/Program Files/ffmpeg/bin/ffmpeg.exe",
        "C:/Program Files (x86)/ffmpeg/bin/ffmpeg.exe",
        QStandardPaths::findExecutable("ffmpeg")
    };

    for (const QString& path : possiblePaths) {
        if (path.isEmpty()) continue;

        QFileInfo info(path);
        if (info.exists() && info.isExecutable()) {
            return path;
        }

        // Try with .exe extension on Windows
        if (!path.endsWith(".exe")) {
            QFileInfo exeInfo(path + ".exe");
            if (exeInfo.exists() && exeInfo.isExecutable()) {
                return path + ".exe";
            }
        }
    }

    // Try to find in PATH
    QString inPath = QStandardPaths::findExecutable("ffmpeg");
    if (!inPath.isEmpty()) {
        return inPath;
    }

    return QString();
}

void FFmpegPipeline::setFFmpegPath(const QString& path) {
    m_ffmpegPath = path;
}

void FFmpegPipeline::onProcessStarted() {
    emit started();
}

void FFmpegPipeline::onProcessFinished(int exitCode, QProcess::ExitStatus status) {
    m_running = false;
    emit runningChanged();

    bool success = (exitCode == 0 && status == QProcess::NormalExit);
    if (!success && !m_errorOutput.isEmpty()) {
        emit error(QString("FFmpeg error: %1").arg(m_errorOutput));
    }

    emit finished(success);

    m_process->deleteLater();
    m_process = nullptr;
}

void FFmpegPipeline::onProcessError(QProcess::ProcessError error) {
    QString errorStr;
    switch (error) {
        case QProcess::FailedToStart:
            errorStr = "FFmpeg failed to start";
            break;
        case QProcess::Crashed:
            errorStr = "FFmpeg crashed";
            break;
        case QProcess::Timedout:
            errorStr = "FFmpeg timed out";
            break;
        case QProcess::WriteError:
            errorStr = "Failed to write to FFmpeg";
            break;
        case QProcess::ReadError:
            errorStr = "Failed to read from FFmpeg";
            break;
        default:
            errorStr = "Unknown FFmpeg error";
    }

    emit this->error(errorStr);
}

void FFmpegPipeline::onReadyReadStandardError() {
    if (m_process) {
        m_errorOutput += QString::fromUtf8(m_process->readAllStandardError());
    }
}
