#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <vector>
#include <memory>
#include "overlay.h"
#include "markeroverlay.h"
#include "arrowoverlay.h"
#include "textoverlay.h"
#include "regionhighlight.h"

class OverlayManager : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)

public:
    enum OverlayRoles {
        IdRole = Qt::UserRole + 1,
        TypeRole,
        NameRole,
        VisibleRole,
        OverlayRole  // Returns the actual overlay object
    };

    explicit OverlayManager(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_overlays.size(); }
    int selectedIndex() const { return m_selectedIndex; }
    void setSelectedIndex(int index);

    // Create overlays
    Q_INVOKABLE MarkerOverlay* createMarker(double lat, double lon);
    Q_INVOKABLE ArrowOverlay* createArrow(double startLat, double startLon, double endLat, double endLon);
    Q_INVOKABLE TextOverlay* createText(double lat, double lon, const QString& text = "Label");
    Q_INVOKABLE RegionHighlight* createRegionHighlight(const QString& regionCode);

    // Management
    Q_INVOKABLE void removeOverlay(int index);
    Q_INVOKABLE void removeOverlayById(const QString& id);
    Q_INVOKABLE void moveOverlay(int from, int to);
    Q_INVOKABLE void duplicateOverlay(int index);
    Q_INVOKABLE Overlay* getOverlay(int index) const;
    Q_INVOKABLE Overlay* getOverlayById(const QString& id) const;
    Q_INVOKABLE void clear();

    // Get specific types
    Q_INVOKABLE MarkerOverlay* getMarker(int index) const;
    Q_INVOKABLE ArrowOverlay* getArrow(int index) const;
    Q_INVOKABLE TextOverlay* getText(int index) const;
    Q_INVOKABLE RegionHighlight* getRegionHighlight(int index) const;

    // Visibility at time
    QVector<Overlay*> visibleOverlaysAtTime(double timeMs) const;

    // Serialization
    QJsonArray toJson() const;
    void fromJson(const QJsonArray& array);

signals:
    void countChanged();
    void selectedIndexChanged();
    void overlayAdded(Overlay* overlay);
    void overlayRemoved(const QString& id);
    void dataModified();

private:
    void addOverlay(Overlay* overlay);

    std::vector<std::unique_ptr<Overlay>> m_overlays;
    int m_selectedIndex = -1;
};
