#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

class KeyframeModel;
class OverlayManager;
class GeoOverlayModel;
class AnimationController;
class Settings;

class ProjectManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString projectPath READ projectPath NOTIFY projectPathChanged)
    Q_PROPERTY(QString projectName READ projectName NOTIFY projectNameChanged)
    Q_PROPERTY(bool hasUnsavedChanges READ hasUnsavedChanges NOTIFY hasUnsavedChangesChanged)

public:
    explicit ProjectManager(KeyframeModel* keyframes, OverlayManager* overlays, QObject* parent = nullptr);

    void setGeoOverlayModel(GeoOverlayModel* geoOverlays) { m_geoOverlays = geoOverlays; }
    void setAnimationController(AnimationController* anim) { m_animation = anim; }
    void setSettings(Settings* settings) { m_settings = settings; }

    QString projectPath() const { return m_projectPath; }
    QString projectName() const;
    bool hasUnsavedChanges() const { return m_hasUnsavedChanges; }

    Q_INVOKABLE bool newProject();
    Q_INVOKABLE bool openProject(const QUrl& path);
    Q_INVOKABLE bool saveProject();
    Q_INVOKABLE bool saveProjectAs(const QUrl& path);

    // Auto-load last project on startup
    Q_INVOKABLE bool loadLastProject();

public slots:
    void markModified();
    void clearModified();

signals:
    void projectPathChanged();
    void projectNameChanged();
    void hasUnsavedChangesChanged();
    void projectLoaded();
    void projectSaved();
    void error(const QString& message);

private:
    bool loadFromFile(const QString& path);
    bool saveToFile(const QString& path);

    QString m_projectPath;
    bool m_hasUnsavedChanges = false;
    KeyframeModel* m_keyframes;
    OverlayManager* m_overlays;
    GeoOverlayModel* m_geoOverlays = nullptr;
    AnimationController* m_animation = nullptr;
    Settings* m_settings = nullptr;
};
