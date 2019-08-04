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
    explicit defaultlook(QWidget *parent = 0);
    ~defaultlook();
    Result runCmd(QString cmd);
    QString getVersion(QString name);
    QString version;
    QString output;
    QStringList panelIDs;
    QString panel;
    bool message_flag;
    QHash<QString, QString>  theme_info;
    QString pluginidsystray;
    QString plugintasklist;
    QStringList undotheme;
    bool hibernate_flag;
    bool Intel_flag;
    bool radeon_flag;
    bool amdgpuflag;
    void setup();
    void setupuiselections();
    void setuppanel();
    void setuptheme();
    void setupEtc();
    void setupConfigoptions();
    void setupComboTheme();
    void fliptohorizontal();
    void fliptovertical();
    void whichpanel();
    void message();
    void checkXFCE();
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

    void on_pushButton_clicked();

private:
    Ui::defaultlook *ui;
};

#endif // DEFAULTLOOK_H
