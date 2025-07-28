#ifndef TWEAK_PLASMA_H
#define TWEAK_PLASMA_H

#include <QObject>

namespace Ui {
class defaultlook;
}

class TweakPlasma : public QObject
{
    Q_OBJECT
public:
    TweakPlasma(Ui::defaultlook *ui, bool verbose, QObject *parent = nullptr);
    void setup();
    bool checkPlasma() const;

private:
    Ui::defaultlook *ui;

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

    void pushApplyPlasma_clicked();
    void comboPlasmaPanelLocation_currentIndexChanged(int);
    void checkPlasmaSingleClick_toggled(bool);
    void checkPlasmaShowAllWorkspaces_toggled(bool);
    void checkPlasmaResetDock_toggled(bool);
    void checkPlasmaDiscoverUpdater_toggled(bool);

    QString readPlasmaPanelConfig(const QString &Key) const;
    QString readTaskmanagerConfig(const QString &Key) const;
    void writePlasmaPanelConfig(const QString &key, const QString &value) const;
    void writeTaskmanagerConfig(const QString &key, const QString &value) const;
};

#endif // TWEAK_PLASMA_H
