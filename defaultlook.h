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

namespace Tab {
enum {Panel, Theme, Compositor, Display, Config, Fluxbox, Plasma, Superkey, Others};
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
    bool message_flag{};
    QHash<QString, QString>  theme_info;
    QString pluginidsystray;
    QString originalhostname;
    QStringList undotheme;
    QString currentdisplaymanager;
    bool setupflag=false;
    bool verbose = false;
    bool panelflag{};
    bool Intel_flag{};
    bool radeon_flag{};
    bool amdgpuflag{};
    bool bluetoothautoenableflag{};
    bool bluetoothbatteryflag{};
    bool enable_recommendsflag{};
    bool vblankflag{};
    bool displayflag = false;
    bool themetabflag = false;
    bool othertabflag = false;
    bool displaysetupflag = false;
    bool brightnessflag = false;
    bool sandboxflag = false;
    bool themeflag = false;
    bool validateflag = false;
    bool tasklistflag = false;
    bool cursor_size_flag = false;
    void tasklistchange();
    bool isXfce = false;
    bool isFluxbox = false;
    bool isKDE = false;
    bool isLightdm = false;
    bool isSuperkey = false;
    bool liqKernelUpdateFlag = false;
    bool debianKernelUpdateFlag = false;
    bool graphicssetupflag=true;
    bool kvmflag=false;
    QString kvmconffile;



    QString g1;
    QString g2;
    QString g3;

    QString vblankinitial;
    void setup();
    void setupuiselections();
    void setuptheme();
    void setupthemechoosers();
    void populatethemelists(const QString &value);
    static void settheme(const QString &type, const QString &theme, const QString &desktop);
    void setupEtc();
    void setupSuperKey();
    void setupDisplay();
    void setupComboTheme();
    void setupBrightness();
    void fliptohorizontal();
    void fliptovertical();
    void whichpanel();
    void message() const;
    bool checkXFCE() const;
    void checkSession();
    static bool checklightdm();
    void CheckComptonRunning();
    void setupCompositor();
    void CheckAptNotifierRunning() const;
    void set_cursor_size();

    void backupPanel();

    void top_or_bottom();
    void left_or_right();
    static void message2();
    void savethemeundo();
    void themeundo();
    QString get_tasklistid();
    void get_cursor_size();

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

    void setupThunar();
    void applyThunar();
    void slotThunarChanged();

    void setmissingxfconfvariables(const QString &activeprofile, const QString &resolution);
    void changecomputername(const QString &hostname);
    bool validatecomputername(const QString &hostname);
    void changedisplaymanager(const QString &dm);
    void kvm_early_switch(const QString &action, const QString &file);

private slots:
    static void on_buttonConfigureXfwm_clicked();
    void on_ButtonApplyEtc_clicked();
    void on_buttonApplyDisplayScaling_clicked();
    void on_buttonCompositorApply_clicked();
    void on_buttonConfigureCompton_clicked();
    static void on_buttonEditComptonConf_clicked();
    void on_buttonGTKscaling_clicked();
    void on_buttonSaveBrightness_clicked();
    void on_buttonThemeApply_clicked();
    void on_buttonThemeUndo_clicked();
    void on_buttonapplyresolution_clicked();
    void on_checkBoxCSD_clicked();
    void on_checkBoxLightdmReset_clicked();
    void on_checkBoxMountInternalDrivesNonRoot_clicked();
    void on_checkBoxSandbox_clicked();
    void on_checkboxAMDtearfree_clicked();
    void on_checkboxIntelDriver_clicked();
    void on_checkboxRadeontearfree_clicked();
    void on_comboBoxDisplay_currentIndexChanged(int index);
    void on_comboBoxvblank_activated(int);
    void on_comboTheme_activated(const int arg1);
    void on_horizontalSliderBrightness_valueChanged(int value);
    void on_horizsliderhardwarebacklight_actionTriggered(int action);
    void on_listWidgetTheme_currentTextChanged(const QString &currentText);
    void on_listWidgetWMtheme_currentTextChanged(const QString &currentText) const;
    void on_listWidgeticons_currentTextChanged(const QString &currentText) const;
    void on_pushButtonPreview_clicked();
    void on_pushButtonRemoveUserThemeSet_clicked();
    void on_pushButtonSettingsToThemeSet_clicked();
    void on_radioSudoRoot_clicked();
    void on_radioSudoUser_clicked();
    void saveBrightness();

    void on_checkBoxbluetoothAutoEnable_clicked();

    void on_checkBoxInstallRecommends_clicked();

    void on_checkBoxDisableFluxboxMenuGeneration_clicked();


    void on_listWidgetCursorThemes_currentTextChanged(const QString &currentText);

    void on_toolButtonSuperFileBrowser_clicked();

    void on_pushButtonSuperKeyApply_clicked();

    void on_lineEditSuperCommand_textChanged(const QString &);

    void on_checkBoxLiqKernelUpdates_clicked();

    void on_checkBoxDebianKernelUpdates_clicked();

    void on_spinBoxPointerSize_valueChanged(int);

    void on_checkBoxComputerName_clicked();

    void on_checkBoxBluetoothBattery_clicked();

    void on_checkBoxDisplayManager_clicked();

    void on_checkBoxKVMVirtLoad_clicked();


    void on_checkBoxFluxboxLegacyStyles_stateChanged(int);

    void saveSettings();

    void loadSettings();
private:
    Ui::defaultlook *ui;
    class TweakPlasma *tweakPlasma = nullptr;
    class TweakXfce *tweakXfce = nullptr;
    class TweakFluxbox *tweakFluxbox = nullptr;

    void pushAbout_clicked();
    void pushHelp_clicked();

    void pushManageTint2_clicked();

    void tabWidget_currentChanged(int index);
    void pushXFCEAppearance_clicked();
    void pushXFCEWMsettings_clicked();
    void pushXFCEPanelSettings_clicked();

    void comboCompositor_currentIndexChanged(const int);
};

#endif // DEFAULTLOOK_H
