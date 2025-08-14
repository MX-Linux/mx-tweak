#ifndef TWEAK_XFCE_PANEL_H
#define TWEAK_XFCE_PANEL_H

#include <QObject>

namespace Ui {
class defaultlook;
}

class TweakXfcePanel : public QObject
{
    Q_OBJECT
public:
    TweakXfcePanel() = delete;
    TweakXfcePanel(Ui::defaultlook *ui, bool verbose, QObject *parent = nullptr) noexcept;
    void setup() noexcept;
    QStringList panelIDs;

private:
    Ui::defaultlook *ui;

    bool verbose;

    QString pluginTaskList;
    struct {
        bool panel;
        bool tasklist;
        bool scales;
    } flags = {};

    /* Panel */
    QString panel;
    void whichPanel() noexcept;

    void slotPluginScaleChanged(double) noexcept;
    void pushXfcePanelApply_clicked() noexcept;
    void comboXfcePanelPlacement_currentIndexChanged(int) noexcept;
    void comboXfcePanelTasklistPlugin_currentIndexChanged(int) noexcept;
    void pushXfcePanelTasklistOptions_clicked() noexcept;
    void pushXfcePanelBackup_clicked() noexcept;
    void pushXfcePanelRestore_clicked() noexcept;
    void pushXfcePanelDefault_clicked() noexcept;
    void pushXfcePanelSettings_clicked() noexcept;

    QString getTasklistID() const noexcept;
    void tasklistChange() noexcept;

    void flipToHorizontal() noexcept;
    void flipToVertical() noexcept;
    void setPosition() noexcept;

    int populateBackups() noexcept;
    int validateArchive(const QString &path) const noexcept;
};

#endif // TWEAK_XFCE_PANEL_H
