#ifndef TWEAK_FLUXBOX_H
#define TWEAK_FLUXBOX_H

#include <QObject>

namespace Ui {
class Tweak;
}

class TweakFluxbox : public QObject
{
    Q_OBJECT
public:
    TweakFluxbox() = delete;
    TweakFluxbox(Ui::Tweak *ui, bool verbose, QObject *parent = nullptr) noexcept;
    void setup() noexcept;
    bool checkFluxbox() const noexcept;

private:
    Ui::Tweak *ui;

    bool verbose;
    struct {
        bool captions : 1;
        bool icons : 1;
        bool slit : 1;
        bool screenIdle : 1;
    } flags = {};

    void slotSettingChanged() noexcept;
    void pushFluxboxApply_clicked() noexcept;

    void checkFluxboxResetEverything_clicked() noexcept;
    void checkFluxboxResetMenu_clicked() noexcept;
    void checkFluxboxMenuMigrate_clicked() noexcept;
    void spinFluxboxScreenIdleTime_valueChanged(int) noexcept;
    void comboFluxboxSlitLocation_currentIndexChanged(int index) noexcept;
    void pushFluxboxManageTint2_clicked() noexcept;
    void comboFluxboxIcons_currentIndexChanged(int index) noexcept;
    void comboFluxboxCaptions_currentIndexChanged(int index) noexcept;

    void changeInitVariable(const QString &initline, const QString &value) const noexcept;
    void changeDock() const noexcept;
};

#endif // TWEAK_FLUXBOX_H
