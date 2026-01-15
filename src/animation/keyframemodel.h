#pragma once

#include <QAbstractListModel>
#include <QVector>
#include "keyframe.h"

class KeyframeModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(double totalDuration READ totalDuration NOTIFY totalDurationChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QList<int> selectedIndices READ selectedIndices NOTIFY selectedIndicesChanged)
    Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged)

public:
    enum KeyframeRoles {
        LatitudeRole = Qt::UserRole + 1,
        LongitudeRole,
        ZoomRole,
        BearingRole,
        TiltRole,
        TimeRole,
        InterpolationRole,
        EasingRole
    };

    explicit KeyframeModel(QObject* parent = nullptr);

    // QAbstractListModel overrides
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    // Keyframe operations
    Q_INVOKABLE void addKeyframe(double lat, double lon, double zoom, double bearing = 0, double tilt = 0);
    Q_INVOKABLE void addKeyframeAtTime(double lat, double lon, double zoom, double bearing, double tilt, double timeMs);
    Q_INVOKABLE void insertKeyframe(int index, const QVariantMap& data);
    Q_INVOKABLE void removeKeyframe(int index);
    Q_INVOKABLE void moveKeyframe(int from, int to);
    Q_INVOKABLE void duplicateKeyframe(int index);
    Q_INVOKABLE void updateKeyframe(int index, const QVariantMap& data);
    Q_INVOKABLE QVariantMap getKeyframe(int index) const;
    Q_INVOKABLE void clear();

    // Timeline operations - set absolute time position
    Q_INVOKABLE void setKeyframeTime(int index, double timeMs);
    Q_INVOKABLE void setKeyframeInterpolation(int index, int mode);
    Q_INVOKABLE void setKeyframeEasing(int index, int easing);

    // Direct editing - update current keyframe position from camera
    Q_INVOKABLE void updateCurrentPosition(double lat, double lon, double zoom, double bearing, double tilt);

    // Multi-selection
    Q_INVOKABLE void selectKeyframe(int index, bool addToSelection = false);
    Q_INVOKABLE void deselectKeyframe(int index);
    Q_INVOKABLE void selectRange(int startIndex, int endIndex);
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE bool isSelected(int index) const;
    Q_INVOKABLE void moveSelectedKeyframes(double deltaTimeMs);

    QList<int> selectedIndices() const { return m_selectedIndices; }

    // Edit mode - when true, camera changes update current keyframe
    bool editMode() const { return m_editMode; }
    void setEditMode(bool enabled);

    // Accessors
    int count() const { return m_keyframes.size(); }
    double totalDuration() const;
    int currentIndex() const { return m_currentIndex; }
    void setCurrentIndex(int index);

    const QVector<Keyframe>& keyframes() const { return m_keyframes; }
    const Keyframe& at(int index) const { return m_keyframes.at(index); }

    // Find keyframe at time - returns the index of the keyframe we're transitioning FROM
    int keyframeIndexAtTime(double timeMs) const;
    // Get progress (0-1) between two keyframes at a given time
    double progressAtTime(double timeMs, int& outFromIndex, int& outToIndex) const;

    // Find keyframe within tolerance of time, returns -1 if none found
    Q_INVOKABLE int keyframeNearTime(double timeMs, double toleranceMs) const;
    // Check if there's a keyframe within tolerance
    Q_INVOKABLE bool hasKeyframeNearTime(double timeMs, double toleranceMs) const;

    // Snap time to nearest frame boundary
    Q_INVOKABLE static double snapToFrame(double timeMs, int fps = 30);

    // Navigation
    Q_INVOKABLE void goToNextKeyframe();
    Q_INVOKABLE void goToPreviousKeyframe();

    // Serialization
    QJsonArray toJson() const;
    void fromJson(const QJsonArray& array);

signals:
    void countChanged();
    void totalDurationChanged();
    void currentIndexChanged();
    void keyframeModified(int index);
    void dataModified();
    void selectedIndicesChanged();
    void editModeChanged();
    void keyframeSelected(int index);  // Emitted when a keyframe is selected (for loading to camera)

private:
    void sortByTime();
    void updateSelectionAfterSort();

    QVector<Keyframe> m_keyframes;
    int m_currentIndex = 0;
    QList<int> m_selectedIndices;
    bool m_editMode = false;

    static constexpr double DEFAULT_KEYFRAME_INTERVAL = 4000.0;  // 4 seconds between keyframes
};
