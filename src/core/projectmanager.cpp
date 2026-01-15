#include "projectmanager.h"
#include "../animation/keyframemodel.h"
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

    m_projectPath.clear();
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

    // Load keyframes
    QJsonArray keyframesArray = root["keyframes"].toArray();
    m_keyframes->fromJson(keyframesArray);

    // Load overlays
    QJsonArray overlaysArray = root["overlays"].toArray();
    m_overlays->fromJson(overlaysArray);

    return true;
}

bool ProjectManager::saveToFile(const QString& path) {
    QJsonObject root;
    root["version"] = "1.0";
    root["name"] = projectName();

    // Save keyframes
    root["keyframes"] = m_keyframes->toJson();

    // Save overlays
    root["overlays"] = m_overlays->toJson();

    QJsonDocument doc(root);
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error(QString("Cannot write to file: %1").arg(path));
        return false;
    }

    file.write(data);
    file.close();

    m_hasUnsavedChanges = false;
    emit hasUnsavedChangesChanged();
    emit projectSaved();

    return true;
}
