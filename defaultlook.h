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
#include <QFile>
#include <QHash>
#include <QMessageBox>

namespace Ui {
class defaultlook;
}

namespace IconSize {
enum {Default, Small, Medium, Large, Larger, Largest};
}

namespace PanelIndex { // location combo index - 0=bottom, 1=left, 2=top, 3=right
enum {Bottom, Left, Top, Right};
}

namespace PanelLocation { // location plasma settings - 4=bottom, 3 top, 5 left, 6 right
enum {Top = 3, Bottom, Left, Right};
}

namespace Tab {
enum {Panel, Theme, Compositor, Display, Config, Fluxbox, Plasma, Others};
}

namespace ValueSize { // kde value starts from 1 vs. combobox index 1=0,2=1,3=2,4=3,5=4,6=5
enum {Default = 1, Small, Medium, Large, Larger, Largest};
}

class defaultlook : public QDialog
{
    Q_OBJECT

public:
    explicit defaultlook(QWidget *parent = 0, const QStringList &args = QStringList());
    ~defaultlook();
    static QString getVersion(const QString &name);
    QString version;
    QString output;
    QStringList panelIDs;
    QString panel;
    QString plasmaPanelId;
    QString plasmataskmanagerID;
    bool message_flag{};
    QHash<QString, QString>  theme_info;
    QString pluginidsystray;
    QString plugintasklist;
    QStringList undotheme;
    bool verbose = false;
    bool panelflag{};
    bool hibernate_flag{};
    bool Intel_flag{};
    bool radeon_flag{};
    bool amdgpuflag{};
    bool libinput_touchpadflag{};
    bool bluetoothautoenableflag{};
    bool enable_recommendsflag{};
    bool vblankflag{};
    bool displayflag = false;
    bool displaysetupflag = false;
    bool brightnessflag = false;
    bool sandboxflag = false;
    bool slitflag = false;
    bool fluxcaptionflag = false;
    bool fluxiconflag = false;
    bool plasmaplacementflag{};
    bool plasmaworkspacesflag{};
    bool plasmasingleclickflag{};
    bool plasmaresetflag{};
    bool plasmasystrayiconsizeflag{};
    bool themeflag = false;
    bool validateflag = false;
    bool tasklistflag = false;
    void thunarsplitview(bool state);
    void thunarsplitviewhorizontal(bool state);
    void thunarsetupsplitview();
    void resetthunar();
    void thunarsingleclicksetup();
    void thunarsetsingleclick(bool state);
    void tasklistchange();


    QString g1;
    QString g2;
    QString g3;

    QString vblankinitial;
    void setup();
    void setupuiselections();
    void setuppanel();
    void setuptheme();
    void setupthemechoosers();
    void populatethemelists(const QString &value);
    static void settheme(const QString &type, const QString &theme, const QString &desktop);
    void setupEtc();
    void setupFluxbox();
    void setupPlasma();
    QString readPlasmaPanelConfig(const QString &Key) const;
    QString readTaskmanagerConfig(const QString &Key) const;
    void writePlasmaPanelConfig(const QString &key, const QString &value) const;
    void writeTaskmanagerConfig(const QString &key, const QString &value) const;
    void setupDisplay();
    void setupConfigoptions();
    void setupComboTheme();
    void setupBrightness();
    void fliptohorizontal();
    void fliptovertical();
    void whichpanel();
    void message() const;
    bool checkXFCE() const;
    bool checkFluxbox() const;
    static bool checklightdm();
    bool checkPlasma() const;
    void CheckComptonRunning();
    void setupCompositor();
    void CheckAptNotifierRunning() const;

    void backupPanel();
    void migratepanel(const QString &date) const;
    int validatearchive(const QString &path) const;
    static void restoreDefaultPanel();
    void restoreBackup();

    void top_or_bottom();
    void left_or_right();
    static void message2();
    void savethemeundo();
    void themeundo();
    QString get_tasklistid();

    void setBrightness();

    void setupscale();
    void setscale();
    void setupbacklight();
    void setbacklight();
    void setgtkscaling();
    void setupresolutions();
    void setresolution();
    void setrefreshrate(const QString &display, const QString &resolution, const QString &activeprofile) const;
    void setupGamma();

    void setmissingxfconfvariables(const QString &activeprofile, const QString &resolution);
    void fluxboxchangeinitvariable(const QString &initline, const QString &value) const;
    void fluxboxchangedock() const;

private slots:
    static void on_buttonCancel_clicked();
    static void on_buttonConfigureXfwm_clicked();
    static void on_buttonHelp_clicked();
    static void on_pushButtonDocklikeSetttings_clicked();
    static void on_pushButtontasklist_clicked();
    void on_ApplyFluxboxResets_clicked();
    void on_ButtonApplyEtc_clicked();
    void on_ButtonApplyMiscDefualts_clicked();
    void on_ButtonApplyPlasma_clicked();
    void on_buttonAbout_clicked();
    void on_buttonApplyDisplayScaling_clicked();
    void on_buttonApply_clicked();
    void on_buttonCompositorApply_clicked();
    void on_buttonConfigureCompton_clicked();
    static void on_buttonEditComptonConf_clicked();
    void on_buttonGTKscaling_clicked();
    void on_buttonSaveBrightness_clicked();
    void on_buttonThemeApply_clicked();
    void on_buttonThemeUndo_clicked();
    void on_buttonapplyresolution_clicked();
    void on_checkBoxCSD_clicked();
    void on_checkBoxDesktopZoom_clicked();
    void on_checkBoxHibernate_clicked();
    void on_checkBoxLightdmReset_clicked();
    void on_checkBoxMenuMigrate_clicked();
    void on_checkBoxMountInternalDrivesNonRoot_clicked();
    void on_checkBoxPlasmaShowAllWorkspaces_clicked();
    void on_checkBoxPlasmaSingleClick_clicked();
    void on_checkBoxSandbox_clicked();
    void on_checkBoxShowAllWorkspaces_clicked();
    void on_checkBoxSingleClick_clicked();
    void on_checkBoxThunarCAReset_clicked();
    void on_checkBoxThunarSingleClick_clicked();
    void on_checkBoxlibinput_clicked();
    void on_checkFirefox_clicked();
    void on_checkHexchat_clicked();
    void on_checkHorz_clicked();
    void on_checkVert_clicked();
    void on_checkboxAMDtearfree_clicked();
    void on_checkboxIntelDriver_clicked();
    void on_checkboxNoEllipse_clicked();
    void on_checkboxRadeontearfree_clicked();
    void on_checkboxfluxSlitautohide_clicked();
    void on_checkboxfluxresetdock_clicked();
    void on_checkboxfluxreseteverything_clicked();
    void on_checkboxfluxresetmenu_clicked();
    void on_checkboxfluxtoolbarautohide_clicked();
    void on_checkboxplasmaresetdock_clicked();
    void on_comboBoxCompositor_currentIndexChanged(const QString &arg1);
    void on_comboBoxDisplay_currentIndexChanged(int index);
    void on_comboBoxPlasmaSystrayIcons_currentIndexChanged(int index);
    void on_comboBoxfluxIcons_currentIndexChanged(int index);
    void on_comboBoxfluxcaptions_currentIndexChanged(int index);
    void on_comboBoxvblank_activated(const QString &arg1);
    void on_comboPlasmaPanelLocation_currentIndexChanged(int index);
    void on_comboTheme_activated(const QString &arg1);
    void on_comboboxHorzPostition_currentIndexChanged(const QString &arg1);
    void on_comboboxVertpostition_currentIndexChanged(const QString &arg1);
    void on_combofluxslitlocation_currentIndexChanged(int index);
    void on_combofluxtoolbarlocatoin_currentIndexChanged(int index);
    void on_horizontalSliderBrightness_valueChanged(int value);
    void on_horizsliderhardwarebacklight_actionTriggered(int action);
    void on_listWidgetTheme_currentTextChanged(const QString &currentText);
    void on_listWidgetWMtheme_currentTextChanged(const QString &currentText) const;
    void on_listWidgeticons_currentTextChanged(const QString &currentText) const;
    void on_pushButtonPreview_clicked();
    void on_pushButtonRemoveUserThemeSet_clicked();
    void on_pushButtonSettingsToThemeSet_clicked();
    void on_radioBackupPanel_clicked();
    void on_radioDefaultPanel_clicked();
    void on_radioRestoreBackup_clicked();
    void on_radioSudoRoot_clicked();
    void on_radioSudoUser_clicked();
    void on_spinBoxFluxToolbarHeight_valueChanged(int arg1);
    void on_spinBoxFluxToolbarWidth_valueChanged(int arg1);
    void on_tabWidget_currentChanged(int index);
    void on_toolButtonXFCEAppearance_clicked();
    void on_toolButtonXFCEWMsettings_clicked();
    void on_toolButtonXFCEpanelSettings_clicked();
    void saveBrightness();

    void on_checkBoxFileDialogActionButtonsPosition_clicked();

    void on_checkBoxbluetoothAutoEnable_clicked();


    void on_checkBoxFluxShowToolbar_clicked();

    void on_buttonManageTint2_clicked();

    void on_lineEditBackupName_returnPressed();

    void on_checkBoxInstallRecommends_clicked();

    void on_checkBoxThunarSplitView_clicked();

    void on_checkBoxsplitviewhorizontal_clicked();

    void on_checkBoxThunarCAReset_2_clicked();

    void on_checkBoxThunarSplitView_2_clicked();

    void on_checkBoxsplitviewhorizontal_2_clicked();

    void on_checkBoxThunarSingleClick_2_clicked();

    void on_radioButtonTasklist_clicked();

    void on_comboBoxTasklistPlugin_currentIndexChanged(int);

private:
    Ui::defaultlook *ui;
};

#endif // DEFAULTLOOK_H
