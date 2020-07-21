/**********************************************************************
 *  defaultlook.h
 **********************************************************************
 * Copyright (C) 2015 MX Authors
 *
 * Authors: Dolphin Oracle
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-tweak.
 *
 * mx-tweak is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-tweak is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-tweak.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef DEFAULTLOOK_H
#define DEFAULTLOOK_H

#include <QDialog>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QHash>

namespace Ui {
class defaultlook;
}

struct Result {
    int exitCode;
    QString output;
};

class defaultlook : public QDialog
{
    Q_OBJECT

protected:
    QProcess *proc;
    QTimer *timer;

public:
    explicit defaultlook(QWidget *parent = 0, QStringList args = QStringList());
    ~defaultlook();
    Result runCmd(QString cmd);
    QString getVersion(QString name);
    QString version;
    QString output;
    QStringList panelIDs;
    QString panel;
    QString plasmaPanelId;
    QString plasmataskmanagerID;
    bool message_flag;
    QHash<QString, QString>  theme_info;
    QString pluginidsystray;
    QString plugintasklist;
    QStringList undotheme;
    bool hibernate_flag;
    bool Intel_flag;
    bool radeon_flag;
    bool amdgpuflag;
    bool vblankflag;
    bool displayflag = false;
    bool brightnessflag = false;
    bool sandboxflag = false;
    bool slitflag = false;
    bool fluxcaptionflag = false;
    bool fluxiconflag = false;
    bool plasmaplacementflag;
    bool plasmaworkspacesflag;
    bool plasmasingleclickflag;
    bool plasmaresetflag;
    bool plasmasystrayiconsizeflag;

    QString g1;
    QString g2;
    QString g3;

    QString vblankinitial;
    void setup();
    void setupuiselections();
    void setuppanel();
    void setuptheme();
    void setupEtc();
    void setupFluxbox();
    void setupPlasma();
    QString readPlasmaPanelConfig(QString Key);
    QString readTaskmanagerConfig(QString Key);
    void writePlasmaPanelConfig(QString key, QString value);
    void writeTaskmanagerConfig(QString key, QString value);
    void setupDisplay();
    void setupConfigoptions();
    void setupComboTheme();
    void setupBrightness();
    void fliptohorizontal();
    void fliptovertical();
    void whichpanel();
    void message();
    bool checkXFCE();
    bool checkFluxbox();
    bool checklightdm();
    bool checkPlasma();
    void CheckComptonRunning();
    void setupCompositor();
    void CheckAptNotifierRunning();

    void backupPanel();
    void restoreDefaultPanel();
    void restoreBackup();

    void top_or_bottom();
    void left_or_right();
    void message2();
    void savethemeundo();
    void themeundo();

    void setBrightness();

    void setupscale();
    void setscale();
    void setupbacklight();
    void setbacklight();
    void setgtkscaling();
    void setupresolutions();
    void setresolution();
    void setrefreshrate(QString arg1, QString arg2, QString arg3);
    void setupGamma();

    void setmissingxfconfvariables(QString arg1, QString arg2);
    void fluxboxchangeinitvariable(QString arg1, QString arg2);
    void fluxboxchangedock();

public slots:




private slots:
    void on_buttonApply_clicked();

    void on_buttonCancel_clicked();

    void on_buttonAbout_clicked();

    void on_buttonHelp_clicked();

    void on_radioDefaultPanel_clicked();

    void on_radioBackupPanel_clicked();

    void on_radioRestoreBackup_clicked();

    void on_checkHorz_clicked();

    void on_checkVert_clicked();

    void on_checkFirefox_clicked();

    void on_checkHexchat_clicked();

    void on_toolButtonXFCEpanelSettings_clicked();

    void on_toolButtonXFCEAppearance_clicked();

    void on_toolButtonXFCEWMsettings_clicked();

    void on_comboboxHorzPostition_currentIndexChanged(const QString &arg1);

    void on_buttonThemeApply_clicked();

    void on_comboTheme_activated(const QString &arg1);

    void on_ButtonApplyEtc_clicked();

    void on_checkBoxSingleClick_clicked();

    void on_checkBoxThunarSingleClick_clicked();

    void on_checkBoxSystrayFrame_clicked();

    void on_comboboxVertpostition_currentIndexChanged(const QString &arg1);

    void on_buttonThemeUndo_clicked();

    void on_buttonConfigureCompton_clicked();

    void on_buttonCompositorApply_clicked();

    void on_buttonEditComptonConf_clicked();

    void on_comboBoxCompositor_currentIndexChanged(const QString &arg1);

    void on_buttonConfigureXfwm_clicked();

    void on_checkBoxShowAllWorkspaces_clicked();

    void on_checkBoxMountInternalDrivesNonRoot_clicked();

    void on_checkboxNoEllipse_clicked();

    void on_pushButtonPreview_clicked();

    void on_checkBoxHibernate_clicked();

    void on_ButtonApplyMiscDefualts_clicked();

    void on_checkBoxLightdmReset_clicked();

    void on_checkBoxThunarCAReset_clicked();

    void on_checkboxIntelDriver_clicked();

    void on_pushButtontasklist_clicked();

    void on_checkboxAMDtearfree_clicked();

    void on_checkboxRadeontearfree_clicked();

    void on_pushButtonSettingsToThemeSet_clicked();

    void on_pushButtonRemoveUserThemeSet_clicked();

    void on_comboBoxvblank_activated(const QString &arg1);

    void on_horizontalSliderBrightness_valueChanged(int value);

    void on_buttonApplyDisplayScaling_clicked();

    void on_comboBoxDisplay_currentIndexChanged(int index);

    void saveBrightness();

    void on_buttonSaveBrightness_clicked();


    void on_buttonGTKscaling_clicked();

    void on_horizsliderhardwarebacklight_actionTriggered(int action);


    void on_buttonapplyresolution_clicked();

    void on_radioSudoUser_clicked();
    void on_radioSudoRoot_clicked();

    void on_checkBoxSandbox_clicked();

    void on_ApplyFluxboxResets_clicked();

    void on_checkboxfluxresetdock_clicked();

    void on_checkboxfluxresetmenu_clicked();

    void on_checkboxfluxreseteverything_clicked();

    void on_combofluxtoolbarlocatoin_currentIndexChanged(int index);

    void on_checkboxfluxtoolbarautohide_clicked();

    void on_spinBoxFluxToolbarWidth_valueChanged(int arg1);

    void on_spinBoxFluxToolbarHeight_valueChanged(int arg1);

    void on_combofluxslitlocation_currentIndexChanged(int index);

    void on_checkboxfluxSlitautohide_clicked();


    void on_comboBoxfluxIcons_currentIndexChanged(int index);

    void on_comboBoxfluxcaptions_currentIndexChanged(int index);

    void on_comboPlasmaPanelLocation_currentIndexChanged(int index);

    void on_checkBoxPlasmaSingleClick_clicked();

    void on_checkBoxPlasmaShowAllWorkspaces_clicked();

    void on_checkboxplasmaresetdock_clicked();

    void on_ButtonApplyPlasma_clicked();

    void on_comboBoxPlasmaSystrayIcons_currentIndexChanged(int index);

private:
    Ui::defaultlook *ui;
};

#endif // DEFAULTLOOK_H
