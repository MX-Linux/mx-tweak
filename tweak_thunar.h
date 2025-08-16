#ifndef TWEAK_THUNAR_H
#define TWEAK_THUNAR_H

#include <QObject>

namespace Ui {
class Tweak;
}

class TweakThunar : public QObject
{
    Q_OBJECT
public:
    TweakThunar() = delete;
    TweakThunar(Ui::Tweak *ui, bool fluxbox, QObject *parent = nullptr) noexcept;
    static bool check() noexcept;
    void setup() noexcept;

private:
    Ui::Tweak *ui;

    bool fluxbox;

    void slotSettingChanged() noexcept;
    void slotApplyClicked() noexcept;
};

#endif // TWEAK_THUNAR_H
