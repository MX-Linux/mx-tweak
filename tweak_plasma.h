#ifndef TWEAK_PLASMA_H
#define TWEAK_PLASMA_H

#include <QObject>

namespace Ui {
class Tweak;
}

class TweakPlasma : public QObject
{
    Q_OBJECT
public:
    TweakPlasma() = delete;
    TweakPlasma(Ui::Tweak *ui, bool verbose, QObject *parent = nullptr) noexcept;
    void setup() noexcept;
    bool checkPlasma() const noexcept;

private:
    Ui::Tweak *ui;

    bool verbose;
    QString panelID;
    QString taskManagerID;
    struct {
        bool placement : 1;
        bool workspaces : 1;
        bool singleClick : 1;
        bool reset : 1;
        bool sysTrayIconSize : 1;
        bool autoStartDiscover : 1;
    } flags = {};

    void pushApplyPlasma_clicked() noexcept;
    void comboPlasmaPanelLocation_currentIndexChanged(int) noexcept;
    void checkPlasmaSingleClick_clicked() noexcept;
    void checkPlasmaShowAllWorkspaces_clicked() noexcept;
    void checkPlasmaResetDock_clicked() noexcept;
    void checkPlasmaDiscoverUpdater_clicked() noexcept;

    QString readPlasmaPanelConfig(const QString &Key) const noexcept;
    QString readTaskmanagerConfig(const QString &Key) const noexcept;
    void writePlasmaPanelConfig(const QString &key, const QString &value) const noexcept;
    void writeTaskmanagerConfig(const QString &key, const QString &value) const noexcept;
};

#endif // TWEAK_PLASMA_H
