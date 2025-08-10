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
    TweakXfce(Ui::defaultlook *ui, bool verbose, QObject *parent = nullptr);
    void setup();
    bool checkXfce() const;
    /* Panel */
    QStringList panelIDs;
    void panelSetup();

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

    void slotSettingChanged();
    void pushXfceApply_clicked();

    /* Panel */
    QString panel;
    void panelWhich();

    void slotPluginScaleChanged(double);
    void pushXfcePanelApply_clicked();
    void comboXfcePanelPlacement_currentIndexChanged(int);
    void comboXfcePanelTasklistPlugin_currentIndexChanged(int);
    void pushXfcePanelTasklistOptions_clicked();
    void pushXfcePanelBackup_clicked();
    void pushXfcePanelRestore_clicked();
    void pushXfcePanelDefault_clicked();

    QString getTasklistID() const;
    void tasklistChange();

    void panelFlipToHorizontal();
    void panelFlipToVertical();
    void panelSetPosition();

    int populatePanelBackups();
    int validateArchive(const QString &path) const;
};

#endif // TWEAK_XFCE_H
