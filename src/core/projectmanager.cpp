#include "projectmanager.h"
#include "settings.h"
#include "../animation/keyframemodel.h"
#include "../animation/geooverlaymodel.h"
#include "../animation/animationcontroller.h"
#include "../overlays/overlaymanager.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ProjectManager::ProjectManager(KeyframeModel* keyframes, OverlayManager* overlays, QObject* parent)
    : QObject(parent)
    , m_keyframes(keyframes)
    , m_overlays(overlays)
{
}

QString ProjectManager::projectName() const {
    if (m_projectPath.isEmpty()) {
        return "Untitled";
    }
    return QFileInfo(m_projectPath).baseName();
}

bool ProjectManager::newProject() {
    m_keyframes->clear();
    m_overlays->clear();
    if (m_geoOverlays) m_geoOverlays->clear();

    // Reset animation settings to defaults
    if (m_animation) {
        m_animation->setExplicitDuration(60000.0);  // 60 seconds default
        m_animation->setUseExplicitDuration(true);
        m_animation->stop();
    }

    m_projectPath.clear();
    m_hasUnsavedChanges = false;

    emit projectPathChanged();
    emit projectNameChanged();
    emit hasUnsavedChangesChanged();
    emit projectLoaded();

    return true;
}

bool ProjectManager::loadLastProject() {
    if (!m_settings) return false;

    QString lastPath = m_settings->lastProjectPath();
    // Check if it's a file path (not just a directory)
    if (lastPath.isEmpty() || !lastPath.endsWith(".kart", Qt::CaseInsensitive)) {
        return false;
    }

    QFile file(lastPath);
    if (!file.exists()) {
        return false;
    }

    if (!loadFromFile(lastPath)) {
        return false;
    }

    m_projectPath = lastPath;
    m_hasUnsavedChanges = false;

    emit projectPathChanged();
    emit projectNameChanged();
    emit hasUnsavedChangesChanged();
    emit projectLoaded();

    return true;
}

bool ProjectManager::openProject(const QUrl& path) {
    QString filePath = path.toLocalFile();
    if (filePath.isEmpty()) {
        filePath = path.toString();
    }

    if (!loadFromFile(filePath)) {
        return false;
    }

    m_projectPath = filePath;
    m_hasUnsavedChanges = false;

    // Save as last project for auto-load
    if (m_settings) {
        m_settings->setLastProjectPath(filePath);
    }

    emit projectPathChanged();
    emit projectNameChanged();
    emit hasUnsavedChangesChanged();
    emit projectLoaded();

    return true;
}

bool ProjectManager::saveProject() {
    if (m_projectPath.isEmpty()) {
        return false;  // Need to use saveProjectAs
    }

    return saveToFile(m_projectPath);
}

bool ProjectManager::saveProjectAs(const QUrl& path) {
    QString filePath = path.toLocalFile();
    if (filePath.isEmpty()) {
        filePath = path.toString();
    }

    // Ensure .kart extension
    if (!filePath.endsWith(".kart", Qt::CaseInsensitive)) {
        filePath += ".kart";
    }

    if (!saveToFile(filePath)) {
        return false;
    }

    m_projectPath = filePath;
    emit projectPathChanged();
    emit projectNameChanged();

    return true;
}

void ProjectManager::markModified() {
    if (!m_hasUnsavedChanges) {
        m_hasUnsavedChanges = true;
        emit hasUnsavedChangesChanged();
    }
}

void ProjectManager::clearModified() {
    if (m_hasUnsavedChanges) {
        m_hasUnsavedChanges = false;
        emit hasUnsavedChangesChanged();
    }
}

bool ProjectManager::loadFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error(QString("Cannot open file: %1").arg(path));
        return false;
    }

    QByteArray data = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit error(QString("JSON parse error: %1").arg(parseError.errorString()));
        return false;
    }

    QJsonObject root = doc.object();

    // Check version
    QString version = root["version"].toString();
    if (version.isEmpty()) {
        emit error("Invalid project file: missing version");
        return false;
    }

    // Clear existing data
    m_keyframes->clear();
    m_overlays->clear();
    if (m_geoOverlays) m_geoOverlays->clear();

    // Load keyframes
    QJsonArray keyframesArray = root["keyframes"].toArray();
    m_keyframes->fromJson(keyframesArray);

    // Load overlays (legacy)
    QJsonArray overlaysArray = root["overlays"].toArray();
    m_overlays->fromJson(overlaysArray);

    // Load geo overlays (new system)
    if (m_geoOverlays && root.contains("geoOverlays")) {
        QJsonArray geoOverlaysArray = root["geoOverlays"].toArray();
        m_geoOverlays->fromJson(geoOverlaysArray);
    }

    // Load animation settings
    if (m_animation && root.contains("animation")) {
        QJsonObject animObj = root["animation"].toObject();
        m_animation->setExplicitDuration(animObj["explicitDuration"].toDouble(60000.0));
        m_animation->setUseExplicitDuration(animObj["useExplicitDuration"].toBool(true));

        // Restore playhead position (must be done after keyframes are loaded)
        if (animObj.contains("currentTime")) {
            m_animation->setCurrentTime(animObj["currentTime"].toDouble(0.0));
        }
    }

    return true;
}

bool ProjectManager::saveToFile(const QString& path) {
    QJsonObject root;
    root["version"] = "1.1";  // Bumped version for new format
    root["name"] = projectName();

    // Save keyframes
    root["keyframes"] = m_keyframes->toJson();

    // Save overlays (legacy)
    root["overlays"] = m_overlays->toJson();

    // Save geo overlays (new system)
    if (m_geoOverlays) {
        root["geoOverlays"] = m_geoOverlays->toJson();
    }

    // Save animation settings
    if (m_animation) {
        QJsonObject animObj;
        animObj["explicitDuration"] = m_animation->explicitDuration();
        animObj["useExplicitDuration"] = m_animation->useExplicitDuration();
        animObj["currentTime"] = m_animation->currentTime();  // Save playhead position
        root["animation"] = animObj;
    }

    QJsonDocument doc(root);
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error(QString("Cannot write to file: %1").arg(path));
        return false;
    }

    file.write(data);
    file.close();

    // Save as last project for auto-load
    if (m_settings) {
        m_settings->setLastProjectPath(path);
    }

    m_hasUnsavedChanges = false;
    emit hasUnsavedChangesChanged();
    emit projectSaved();

    return true;
}
