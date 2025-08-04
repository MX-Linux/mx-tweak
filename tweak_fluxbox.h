#ifndef TWEAK_FLUXBOX_H
#define TWEAK_FLUXBOX_H

#include <QObject>

namespace Ui {
class defaultlook;
}

class TweakFluxbox : public QObject
{
    Q_OBJECT
public:
    TweakFluxbox(Ui::defaultlook *ui, bool verbose, QObject *parent = nullptr);
    void setup();
    bool checkFluxbox() const;

private:
    Ui::defaultlook *ui;

    bool verbose;
    struct {
        bool captions : 1;
        bool icons : 1;
        bool slit : 1;
        bool screenBlank : 1;
    } flags = {};

    void slotSettingChanged();
    void pushFluxboxApply_clicked();

    void checkFluxboxResetEverything_clicked();
    void checkFluxboxResetMenu_clicked();
    void checkFluxboxMenuMigrate_clicked();
    void spinFluxboxScreenBlankingTimeout_valueChanged(int);
    void comboFluxboxSlitLocation_currentIndexChanged(int index);
    void comboFluxboxIcons_currentIndexChanged(int index);
    void comboFluxboxCaptions_currentIndexChanged(int index);

    void changeInitVariable(const QString &initline, const QString &value) const;
    void changeDock() const;
};

#endif // TWEAK_FLUXBOX_H
