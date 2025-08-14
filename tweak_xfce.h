#ifndef TWEAK_XFCE_H
#define TWEAK_XFCE_H

#include <QObject>

namespace Ui {
class defaultlook;
}

class TweakXfce : public QObject
{
    Q_OBJECT
public:
    TweakXfce() = delete;
    TweakXfce(Ui::defaultlook *ui, bool verbose, QObject *parent = nullptr) noexcept;
    void setup() noexcept;
    bool checkXfce() const noexcept;
    /* Panel */
    QStringList panelIDs;
    void panelSetup() noexcept;

private:
    Ui::defaultlook *ui;

    bool verbose;

    QString pluginTaskList;
    struct {
        bool hibernate;
        /* Panel */
        bool panel;
        bool tasklist;
        bool scales;
    } flags = {};

    void slotSettingChanged() noexcept;
    void pushXfceApply_clicked() noexcept;

    /* Panel */
    QString panel;
    void panelWhich() noexcept;

    void slotPluginScaleChanged(double) noexcept;
    void pushXfcePanelApply_clicked() noexcept;
    void comboXfcePanelPlacement_currentIndexChanged(int) noexcept;
    void comboXfcePanelTasklistPlugin_currentIndexChanged(int) noexcept;
    void pushXfcePanelTasklistOptions_clicked() noexcept;
    void pushXfcePanelBackup_clicked() noexcept;
    void pushXfcePanelRestore_clicked() noexcept;
    void pushXfcePanelDefault_clicked() noexcept;

    QString getTasklistID() const noexcept;
    void tasklistChange() noexcept;

    void panelFlipToHorizontal() noexcept;
    void panelFlipToVertical() noexcept;
    void panelSetPosition() noexcept;

    int populatePanelBackups() noexcept;
    int validateArchive(const QString &path) const noexcept;
};

#endif // TWEAK_XFCE_H
