#ifndef TWEAK_COMPOSITOR_H
#define TWEAK_COMPOSITOR_H

#include <QObject>

namespace Ui {
class Tweak;
}

class TweakCompositor : public QObject
{
    Q_OBJECT
public:
    TweakCompositor() = delete;
    TweakCompositor(Ui::Tweak *ui, bool verbose, QObject *parent = nullptr) noexcept;
    static bool check() noexcept;
    void setup() noexcept;

private:
    Ui::Tweak *ui;

    bool verbose;

    QString initVBlank;
    struct {
        bool vblank;
    } flags = {};

    void checkPicomRunning() noexcept;
    void checkAptNotifierRunning() const noexcept;

    void comboCompositor_currentIndexChanged(const int) noexcept;
    void comboCompositorVBlank_activated(int) noexcept;
    void pushCompositorXfwmSettings_clicked() noexcept;
    void pushCompositorPicomSettings_clicked() noexcept;
    void pushCompositorEditPicomConf_clicked() noexcept;
    void pushCompositorApply_clicked() noexcept;
};

#endif // TWEAK_COMPOSITOR_H
