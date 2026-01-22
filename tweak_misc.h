#ifndef TWEAK_MISC_H
#define TWEAK_MISC_H

#include <QObject>

namespace Ui {
class Tweak;
}

class TweakMisc : public QObject
{
    Q_OBJECT
public:
    TweakMisc() = delete;
    TweakMisc(Ui::Tweak *ui, bool verbose, QObject *parent = nullptr) noexcept;
    void setup() noexcept;

private:
    Ui::Tweak *ui;
    bool verbose;
    struct {
        bool changeinitsystem : 1;
        bool sandbox : 1;
        bool intel : 1;
        bool amdgpu : 1;
        bool radeon : 1;
        bool bluetoothAutoEnable : 1;
        bool bluetoothBattery : 1;
        bool enableRecommends : 1;
        bool updateKernelDebian : 1;
        bool updateKernelLiquorix : 1;
        bool kvm : 1;
    } flags = {};
    QString kvmConfFile;
    QString currentDisplayManager;

    bool validateHostName(const QString &hostname) const noexcept;

    void slotSettingChanged() noexcept;
    void pushMiscApply_clicked() noexcept;

    void checkMiscSandbox_clicked() noexcept;
    void checkMiscBluetoothAutoEnable_clicked() noexcept;
    void checkMiscBluetoothBattery_clicked() noexcept;
    void checkMiscKVMVirtLoad_clicked() noexcept;
    void checkMiscInstallRecommends_clicked() noexcept;
    void checkMiscLiqKernelUpdates_clicked() noexcept;
    void checkMiscDebianKernelUpdates_clicked() noexcept;
    void checkMiscIntelDriver_clicked() noexcept;
    void checkMiscTearfreeAMD_clicked() noexcept;
    void checkMiscTearfreeRadeon_clicked() noexcept;
};

#endif // TWEAK_MISC_H
