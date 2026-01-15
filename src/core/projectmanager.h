#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

class KeyframeModel;
class OverlayManager;

class ProjectManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString projectPath READ projectPath NOTIFY projectPathChanged)
    Q_PROPERTY(QString projectName READ projectName NOTIFY projectNameChanged)
    Q_PROPERTY(bool hasUnsavedChanges READ hasUnsavedChanges NOTIFY hasUnsavedChangesChanged)

public:
    explicit ProjectManager(KeyframeModel* keyframes, OverlayManager* overlays, QObject* parent = nullptr);

    QString projectPath() const { return m_projectPath; }
    QString projectName() const;
    bool hasUnsavedChanges() const { return m_hasUnsavedChanges; }

    Q_INVOKABLE bool newProject();
    Q_INVOKABLE bool openProject(const QUrl& path);
    Q_INVOKABLE bool saveProject();
    Q_INVOKABLE bool saveProjectAs(const QUrl& path);

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
};
