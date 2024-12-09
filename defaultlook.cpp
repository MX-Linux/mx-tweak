/**********************************************************************
 *  defaultlook.cpp
 **********************************************************************
 * Copyright (C) 2015 MX Authors
 *
 * Authors: Dolphin Oracle
 *         MX Linux <http://mxlinux.org>
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

#include "QDebug"
#include "QDir"
#include <QDateTime>
#include <QDirIterator>
#include <QFileDialog>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QStringList>
#include <QTextStream>
#include <QTimer>

#include "about.h"
#include "cmd.h"
#include "defaultlook.h"
#include "remove_user_theme_set.h"
#include "theming_to_tweak.h"
#include "ui_defaultlook.h"
#include "version.h"
#include "window_buttons.h"
#include "xfwm_compositor_settings.h"

defaultlook::defaultlook(QWidget *parent, const QStringList &args) :
    QDialog(parent),
    ui(new Ui::defaultlook)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    // check session type
    checkSession();
    if ( args.contains(QStringLiteral("--display"))) {
        if (isXfce) {
            displayflag = true;
        } else {
            QMessageBox::information(nullptr, tr("MX Tweak"),
                                     tr("--display switch only valid for Xfce"));
        }
    }

    if (args.contains(QStringLiteral("--theme"))){
        themetabflag = true;
    }

    if (args.contains(QStringLiteral("--other"))){
        othertabflag = true;
    }
    if (args.contains(QStringLiteral("--verbose"))) {
        verbose = true;
    }


    setup();


}

defaultlook::~defaultlook()
{
    delete ui;
}

void defaultlook::checkSession() {
    QString test = runCmd(QStringLiteral("ps -aux |grep  -E \'plasmashell|xfce4-session|fluxbox|lightdm|xfce-superkey\' |grep -v grep")).output;
    if (test.contains("xfce4-session")){
        isXfce = true;
    } else if (test.contains("plasmashell")) {
        isKDE = true;
    } else if (test.contains("fluxbox")){
        isFluxbox = true;
    }
    if (test.contains("lightdm")){
        isLightdm = true;
    }
    if (test.contains("xfce-superkey")){
        isSuperkey = true;
    }
    if (verbose) qDebug() << "isXfce is " << isXfce << "isKDE is " << isKDE << "isFluxbox is " << isFluxbox << "isLightdm is " << isLightdm << "isSuperkey is" << isSuperkey;

}
// setup versious items first time program runs
void defaultlook::setup()
{
    this->setWindowTitle(tr("MX Tweak"));
    this->adjustSize();

    QString home_path = QDir::homePath();
    if (!checklightdm()) {
        ui->checkBoxLightdmReset->hide();
    }

    if (themetabflag) ui->tabWidget->setCurrentIndex(Tab::Theme);
    if (othertabflag) ui->tabWidget->setCurrentIndex(Tab::Others);

    if (isXfce) {
        ui->toolButtonXFCEpanelSettings->setIcon(QIcon::fromTheme("org.xfce.panel"));
        ui->toolButtonXFCEAppearance->setIcon(QIcon::fromTheme("org.xfce.settings.appearance"));
        ui->toolButtonXFCEWMsettings->setIcon(QIcon::fromTheme("org.xfce.xfwm4"));
        whichpanel();
        message_flag = false;
        //setup theme tab
        ui->pushButtonPreview->hide();
        ui->buttonThemeUndo->hide();
        ui->buttonThemeUndo->setEnabled(false);
        //set first tab as default
        if (!themetabflag && !othertabflag)ui->tabWidget->setCurrentIndex(Tab::Panel);
        ui->tabWidget->removeTab(Tab::Plasma);
        ui->tabWidget->removeTab(Tab::Fluxbox);
        //setup panel tab
        setuppanel();
        setuptheme();
        //setup compositor tab
        setupCompositor();
        //setup theme combo box
        setupComboTheme();
        //setup Config Options
        setupConfigoptions();
        //setup other tab;
        setupEtc();
        if (!isSuperkey || ! QFile("/usr/bin/xfce-superkey-launcher").exists()){
            ui->tabWidget->removeTab(Tab::Superkey);
        } else {
            setupSuperKey();
        }
    }

    //setup fluxbox
    else if (isFluxbox) {
        ui->comboTheme->hide();
        ui->label->hide();
        ui->buttonThemeApply->hide();
        ui->buttonThemeUndo->hide();
        ui->pushButtonPreview->hide();
        ui->pushButtonRemoveUserThemeSet->hide();
        ui->label_4->hide();
        ui->label_5->hide();
        ui->label_6->hide();
        ui->label_7->hide();
        //ui->listWidgetTheme->hide();
        //ui->listWidgeticons->hide();
        //ui->label_28->hide();
        //ui->label_30->hide();
        ui->toolButtonXFCEAppearance->hide();
        ui->toolButtonXFCEWMsettings->hide();
        ui->toolButtonXFCEpanelSettings->hide();
        if (!themetabflag && !othertabflag) ui->tabWidget->setCurrentIndex(Tab::Fluxbox);
        ui->tabWidget->removeTab(Tab::Superkey);
        ui->tabWidget->removeTab(Tab::Plasma);
        ui->tabWidget->removeTab(Tab::Config);
        ui->tabWidget->removeTab(Tab::Display);
        ui->tabWidget->removeTab(Tab::Compositor);
        ui->tabWidget->removeTab(Tab::Panel);
        setupFluxbox();
        setuptheme();
        //setup other tab;
        setupEtc();

    }
//Panel, Theme, Compositor, Display, Config, Fluxbox, Plasma, Superkey, Others
    //setup plasma
    else if (isKDE) {
        ui->label_4->hide();
        ui->label_5->hide();
        ui->label_6->hide();
        ui->label_7->hide();
        ui->buttonThemeUndo->hide();
        ui->pushButtonPreview->hide();
        ui->pushButtonRemoveUserThemeSet->hide();
        ui->toolButtonXFCEAppearance->hide();
        ui->toolButtonXFCEWMsettings->hide();
        ui->toolButtonXFCEpanelSettings->hide();
        if (!themetabflag && !othertabflag) ui->tabWidget->setCurrentIndex(Tab::Plasma);
        ui->tabWidget->removeTab(Tab::Superkey);
        ui->tabWidget->removeTab(Tab::Fluxbox);
        ui->tabWidget->removeTab(Tab::Config);
        ui->tabWidget->removeTab(Tab::Display);
        ui->tabWidget->removeTab(Tab::Compositor);
        ui->tabWidget->removeTab(Tab::Panel);
        setupPlasma();
        setuptheme();
        setupComboTheme();
        //setup other tab;
        setupEtc();


    //for other non-supported desktops, show only
    } else {
        ui->label_4->hide();
        ui->label_5->hide();
        ui->label_6->hide();
        ui->label_7->hide();
        ui->toolButtonXFCEAppearance->hide();
        ui->toolButtonXFCEWMsettings->hide();
        ui->toolButtonXFCEpanelSettings->hide();
        ui->tabWidget->setCurrentIndex(Tab::Others);
        for (int i = 6; i >= 0; --i)
            ui->tabWidget->removeTab(i);
        //setup other tab;
        setupEtc();
    }

    //copy template file to ~/.local/share/mx-tweak-data if it doesn't exist
    QDir userdir(home_path + "/.local/share/mx-tweak-data");
    QFileInfo template_file(home_path + "/.local/share/mx-tweak-data/mx.tweak.template");
    if (template_file.exists()) {
        if (verbose) qDebug() << "template file found";
    } else {
        if (userdir.exists()) {
            runCmd("cp /usr/share/mx-tweak-data/mx.tweak.template " + userdir.absolutePath());
        } else {
            runCmd("mkdir -p " + userdir.absolutePath());
            runCmd("cp /usr/share/mx-tweak-data/mx.tweak.template " + userdir.absolutePath());
        }
    }
    version = getVersion(QStringLiteral("mx-tweak"));
    this->adjustSize();
}

void defaultlook::whichpanel()
{
    // take the first panel we see as default
    QString panel_content;
    panel_content = runCmd(QStringLiteral("LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels | grep -v Value | grep -v ^$")).output;
    panelIDs = panel_content.split(QStringLiteral("\n"));
    panel = panelIDs.value(0);
    if (verbose) qDebug() << "panels found: " << panelIDs;
    if (verbose) qDebug() << "panel to use: " << panel;
}

void defaultlook::fliptohorizontal()
{
    QString file_content;
    QStringList pluginIDs;
    file_content = runCmd("LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/plugin-ids | grep -v Value | grep -v ^$").output;
    pluginIDs = file_content.split(QStringLiteral("\n"));
    if (verbose) qDebug() << pluginIDs;

    // figure out moving the systray, if it exists

    // figure out systrayID, pusleaudio plugin, and tasklistID

    QString systrayID = runCmd(QStringLiteral(R"(grep \"systray\" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml)")).output;
    systrayID=systrayID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "systray: " << systrayID;

    QString tasklistID = get_tasklistid();

    QString pulseaudioID = runCmd(QStringLiteral("grep pulseaudio ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    pulseaudioID=pulseaudioID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "pulseaudio: " << pulseaudioID;

    QString powerID = runCmd(QStringLiteral("grep power-manager-plugin ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    powerID=powerID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "powerID: " << powerID;

    QString workspacesID = runCmd(QStringLiteral("grep pager ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    workspacesID = workspacesID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "workspacesID: " << workspacesID;

   // if systray exists, do a bunch of stuff to relocate it a list of plugins.  If not present, do nothing to list

    if (systrayID !=QLatin1String("")) {

        //get tasklist index in list
        int tasklistindex = pluginIDs.indexOf(tasklistID);
        if (verbose) qDebug() << "tasklistIDindex 1" << tasklistindex;

        //check next plugin in list to see if its an expanding separator
        int expsepindex = tasklistindex + 1;
        if (verbose) qDebug() << "expsepindex" << expsepindex;
        QString expsepID = pluginIDs.value(expsepindex);
        if (verbose) qDebug() << "expsepID to test" << expsepID;
        QString test = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + expsepID + "/expand").output;
        if (verbose) qDebug() << "test parm" << test;

        //move the notification area (systray) to above window buttons (tasklist) in the list if tasklist exists

        if (tasklistID !=QLatin1String("")) {
            pluginIDs.removeAll(systrayID);
            tasklistindex = pluginIDs.indexOf(tasklistID);
            if (verbose) qDebug() << "tasklistIDindex 2" << tasklistindex;
            pluginIDs.insert(tasklistindex, systrayID);
            if (verbose) qDebug() << "reordered list" << pluginIDs;

            //move the expanding separator

            if (test == QLatin1String("true")) {
                pluginIDs.removeAll(expsepID);
                tasklistindex = pluginIDs.indexOf(tasklistID);
                if (verbose) qDebug() << "tasklistIDindex 2" << tasklistindex;
                pluginIDs.insert(tasklistindex, expsepID);
                if (verbose) qDebug() << "reordered list" << pluginIDs;
            }
        }

        //if the tasklist isn't present, try to make a decision about where to put the systray

        if (tasklistID == QLatin1String("")) {

            //try to move to in front of clock if present

            QString clockID = runCmd(QStringLiteral(R"(grep -m1 "clock\|datetime" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml)")).output;
            QString switchID;
            clockID=clockID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
            if (verbose) qDebug() << "clockID: " << clockID;
            if (clockID != QLatin1String("")) {
                switchID = clockID;

                //if clock found check if next plugin down is a separator and if so put it there

                int clocksepindex = pluginIDs.indexOf(clockID) + 1;
                QString clocksepcheck = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + pluginIDs.value(clocksepindex)).output;
                if (verbose) qDebug() << "clocksepcheck: " << clocksepcheck;
                if (clocksepcheck == QLatin1String("separator")) {
                    switchID = pluginIDs.value(clocksepindex);
                }

                // if there is no clock, put it near the end and hope for the best

            } else {
                switchID = pluginIDs.value(1);
            }

            // move the systray

            int switchIDindex = 0;
            pluginIDs.removeAll(systrayID);
            switchIDindex = pluginIDs.indexOf(switchID) + 1;
            if (verbose) qDebug() << "switchIDindex 2" << switchIDindex;
            pluginIDs.insert(switchIDindex, systrayID);
            if (verbose) qDebug() << "reordered list" << pluginIDs;
        }

        //if pulsaudio plugin is present, move it to in front of systray
        if (pulseaudioID != QLatin1String("")) {
            int switchIDindex = 0;
            pluginIDs.removeAll(pulseaudioID);
            switchIDindex = pluginIDs.indexOf(systrayID) + 1;
            pluginIDs.insert(switchIDindex, pulseaudioID);
            if (verbose) qDebug() << "reorderd PA list" << pluginIDs;
        }
        //if power-manager plugin is present, move it to in behind of systray
        if (powerID != QLatin1String("")) {
            int switchIDindex = 0;
            pluginIDs.removeAll(powerID);
            switchIDindex = pluginIDs.indexOf(systrayID);
            pluginIDs.insert(switchIDindex, powerID);
            if (verbose) qDebug() << "reorderd PA list" << pluginIDs;
        }
    }

    //now reverse the list

    std::reverse(pluginIDs.begin(), pluginIDs.end());
    if (verbose) qDebug() << "reversed list" << pluginIDs;

    //now build xfconf command

    QStringListIterator changeIterator(pluginIDs);
    QString cmdstring;
    while (changeIterator.hasNext()) {
        QString value = changeIterator.next();
        cmdstring = QString(cmdstring + "-s " + value + " ");
        if (verbose) qDebug() << cmdstring;
    }

    //flip the panel plugins and hold on, it could be a bumpy ride

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/plugin-ids " + cmdstring);

    //change orientation to horizontal

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/mode -s 0");

    //change mode of tasklist labels if it exists

    if (tasklistID != QLatin1String("")) {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + tasklistID + "/show-labels -s true");
    }

    //change mode of pager if exists
    //xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspaceID + "/rows --type int --set 1"
    //check current workspaces rows
    QString workspacesrows = runCmd("xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspacesID + "/rows").output;
    if ( workspacesrows == QLatin1String("1") || workspacesrows == QLatin1String("2")) {
        runCmd("xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspacesID + "/rows --type int --set 1");
    }

    //deteremine top or bottom horizontal placement
    top_or_bottom();

    runCmd(QStringLiteral("xfce4-panel --restart"));
}

void defaultlook::fliptovertical()
{
    QString file_content;
    QStringList pluginIDs;
    file_content = runCmd("LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/plugin-ids | grep -v Value | grep -v ^$").output;
    pluginIDs = file_content.split(QStringLiteral("\n"));
    if (verbose) qDebug() << pluginIDs;

    // figure out moving the systray, if it exists

    QString systrayID = runCmd(QStringLiteral(R"(grep \"systray\" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml)")).output;
    systrayID=systrayID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "systray: " << systrayID;

    QString tasklistID = get_tasklistid();

    QString pulseaudioID = runCmd(QStringLiteral("grep pulseaudio ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    pulseaudioID=pulseaudioID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "pulseaudio: " << pulseaudioID;

    QString powerID = runCmd(QStringLiteral("grep power-manager-plugin ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    powerID=powerID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "powerID: " << powerID;

    QString workspacesID = runCmd(QStringLiteral("grep pager ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    workspacesID=workspacesID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "workspacesID: " << workspacesID;

    //if systray exists, do a bunch of stuff to try to move it in a logical way

    if (systrayID != QLatin1String("")) {

        // figure out whiskerID, appmenuID, systrayID, tasklistID, and pagerID

        QString whiskerID = runCmd(QStringLiteral("grep whisker ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
        whiskerID=whiskerID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
        if (verbose) qDebug() << "whisker: " << whiskerID;

        QString pagerID = runCmd(QStringLiteral("grep pager ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
        pagerID=pagerID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
        if (verbose) qDebug() << "pager: " << pagerID;

        QString appmenuID = runCmd(QStringLiteral("grep applicationsmenu ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
        appmenuID=appmenuID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
        if (verbose) qDebug() << "appmenuID: " << appmenuID;

        //get tasklist index in list
        int tasklistindex = pluginIDs.indexOf(tasklistID);
        if (verbose) qDebug() << "tasklistIDindex 1" << tasklistindex;

        //check next plugin in list to see if its an expanding separator
        int expsepindex = tasklistindex + 1;
        if (verbose) qDebug() << "expsepindex" << expsepindex;
        QString expsepID = pluginIDs.value(expsepindex);
        if (verbose) qDebug() << "expsepID to test" << expsepID;
        QString testexpandsep = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + expsepID + "/expand").output;
        if (verbose) qDebug() << "test parm" << testexpandsep;

        //move the notification area (systray) to an appropriate area.

        //1.  determine if menu is present, place in front of menu

        QString switchID;
        if (whiskerID != QLatin1String("")) {
            switchID = whiskerID;
            if (verbose) qDebug() << "switchID whisker: " << switchID;
        } else {
            if (appmenuID != QLatin1String("")) {
                switchID = appmenuID;
                if (verbose) qDebug() << "switchID appmenu: " << switchID;
            }
        }

        //        //2.  if so, check second plugin is separator, if so place in front of separator

        //        if (switchID != "") {
        //            QString test = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + pluginIDs.value(1)).output;
        //            if (test == "separator") {
        //                if (verbose) qDebug() << "test parm" << test;
        //                switchID = pluginIDs.value(1);
        //                if (verbose) qDebug() << "switchID sep: " << switchID;
        //            }
        //        }

        //3.  if so, check third plugin is pager.  if so, place tasklist in front of pager

        if (switchID != QLatin1String("")) {
            QString test = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + pluginIDs.value(1)).output;
            if (test == QLatin1String("pager")) {
                if (verbose) qDebug() << "test parm" << test;
                switchID = pluginIDs.value(1);
                if (verbose) qDebug() << "switchID pager: " << switchID;
            }
        }

        // if the menu doesn't exist, give a default value that is sane but might not be correct

        if (switchID == QLatin1String("")) {
            switchID = pluginIDs.value(1);
            if (verbose) qDebug() << "switchID default: " << switchID;
        }

        //4.  move the systray

        pluginIDs.removeAll(systrayID);
        int switchindex = pluginIDs.indexOf(switchID) + 1;
        if (verbose) qDebug() << "switchindex" << switchindex;
        pluginIDs.insert(switchindex, systrayID);
        if (verbose) qDebug() << "reordered list" << pluginIDs;

        //if pulsaudio plugin is present, move it to in front of systray
        if (pulseaudioID != QLatin1String("")) {
            int switchIDindex = 0;
            pluginIDs.removeAll(pulseaudioID);
            switchIDindex = pluginIDs.indexOf(systrayID) + 1;
            pluginIDs.insert(switchIDindex, pulseaudioID);
            if (verbose) qDebug() << "reorderd PA list" << pluginIDs;
        }

        //if powerID plugin is present, move it to in behind of systray
        if (powerID != QLatin1String("")) {
            int switchIDindex = 0;
            pluginIDs.removeAll(powerID);
            switchIDindex = pluginIDs.indexOf(systrayID);
            pluginIDs.insert(switchIDindex, powerID);
            if (verbose) qDebug() << "reorderd PA list" << pluginIDs;
        }
        //move the expanding separator

        if (testexpandsep == QLatin1String("true")) {
            pluginIDs.removeAll(expsepID);
            tasklistindex = pluginIDs.indexOf(tasklistID);
            if (verbose) qDebug() << "tasklistIDindex 2" << tasklistindex;
            pluginIDs.insert(tasklistindex, expsepID);
            if (verbose) qDebug() << "reordered list" << pluginIDs;
        }
    }

    //now reverse the list

    std::reverse(pluginIDs.begin(), pluginIDs.end());
    if (verbose) qDebug() << "reversed list" << pluginIDs;

    //now build xfconf command

    QStringListIterator changeIterator(pluginIDs);
    QString cmdstring;
    while (changeIterator.hasNext()) {
        QString value = changeIterator.next();
        cmdstring = QString(cmdstring + "-s " + value + " ");
        if (verbose) qDebug() << cmdstring;
    }
    //flip the panel plugins and pray for a miracle


    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/plugin-ids " + cmdstring);

    //change orientation to vertical

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel + "/mode -n -t int -s 2");
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel + "/position -s 'p=5;x=0;y=0'");

    //change mode of tasklist labels if they exist

    if (tasklistID != QLatin1String("")) {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + tasklistID + "/show-labels -s false");
    }

    //change mode of pager if exists
    //xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspaceID + "/rows --type int --set 2"
    //check current workspaces rows
    QString workspacesrows = runCmd("xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspacesID + "/rows").output;
    if ( workspacesrows == QLatin1String("1") || workspacesrows == QLatin1String("2")) {
        runCmd("xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspacesID + "/rows --type int --set 2");
    }

    //determine left_or_right placement
    left_or_right();

    //restart xfce4-panel

    system("xfce4-panel --restart");
}

void defaultlook::on_buttonApply_clicked()
{
    if (! validateflag){
        ui->buttonApply->setEnabled(false);
    }
    //backups and default panel
    if (ui->radioDefaultPanel->isChecked()) {
        restoreDefaultPanel();
        runCmd(QStringLiteral("sleep .5"));
        whichpanel();
    }

    if (ui->radioRestoreBackup->isChecked()) {
        restoreBackup();
        runCmd(QStringLiteral("sleep .5"));
        whichpanel();
    }

    if (ui->radioBackupPanel->isChecked()) {
        backupPanel();
    }

    // tasklist switch
    if (verbose) qDebug() << "tasklist flag is " << tasklistflag;
    if (ui->radioButtonTasklist->isChecked()){
        if ( tasklistflag ) tasklistchange();
    }

    //flip panels
    if (ui->checkHorz->isChecked()) {
        QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/mode").output;

        if (test == QLatin1String("1") || test ==QLatin1String("2")) {
            fliptohorizontal();
        }

        runCmd(QStringLiteral("sleep .5"));
    }

    if (ui->checkVert->isChecked()) {
        QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/mode").output;

        if (test == QLatin1String("") || test ==QLatin1String("0")) {
            fliptovertical();
        }
        runCmd(QStringLiteral("sleep .5"));
    }

    if (ui->radioButtonSetPanelPluginScales->isChecked()){
        runCmd("sed -i '/xfce4-power-manager-plugin/,/\\}/ s/scale(.*)/scale(" + QString::number(ui->doubleSpinBoxpmplugin->value()) + ")/' ~/.config/gtk-3.0/xfce4-panel-tweaks.css");
        runCmd("sed -i '/pulseaudio/,/\\}/ s/scale(.*)/scale(" + QString::number(ui->doubleSpinBoxpaplugin->value()) + ")/' ~/.config/gtk-3.0/xfce4-panel-tweaks.css");
        runCmd("xfce4-panel --restart");
        setuppanel();
    }
    if (! validateflag ){
        setuppanel();
    }
}


void defaultlook::on_buttonCancel_clicked()
{
    qApp->quit();
}

void defaultlook::on_buttonAbout_clicked()
{
    this->hide();
    displayAboutMsgBox(tr("About MX Tweak"),
                       "<p align=\"center\"><b><h2>" + tr("MX Tweak") + "</h2></b></p><p align=\"center\">" +
                       tr("Version: ") + VERSION + "</p><p align=\"center\"><h3>" +
                       tr("App for quick default ui theme changes and tweaks") +
                       "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p>"
                       "<p align=\"center\">" + tr("Copyright (c) MX Linux") + "<br /><br /></p>",
                       QStringLiteral("/usr/share/doc/mx-tweak/license.html"), tr("%1 License").arg(this->windowTitle()));
    this->show();
}

void defaultlook::on_buttonHelp_clicked()
{
    QLocale locale;
    QString lang = locale.bcp47Name();

    QString url = QStringLiteral("file:///usr/share/doc/mx-tweak/mx-tweak.html");

    if (lang.startsWith(QLatin1String("fr"))) {
        url = QStringLiteral("https://mxlinux.org/wiki/help-files/help-tweak-ajustements");
    }
    displayDoc(url, tr("%1 Help").arg(tr("MX Tweak")));
}

void defaultlook::message() const
{
    QString cmd = QStringLiteral("ps -aux |grep -v grep|grep firefox");
    if ( system(cmd.toUtf8()) != 0 ) {
        if (verbose) qDebug() << "Firefox not running" ;
    } else {
        QMessageBox::information(nullptr, tr("MX Tweak"),
                                 tr("Finished! Firefox may require a restart for changes to take effect"));
    }
}

bool defaultlook::checkXFCE() const
{
    QString test = runCmd(QStringLiteral("pgrep xfce4-session")).output;
    if (verbose) qDebug() << "current xfce desktop test is " << test;
    return (!test.isEmpty());
}

bool defaultlook::checkFluxbox() const
{
    QString test = runCmd(QStringLiteral("pgrep fluxbox")).output;
    if (verbose) qDebug() << "current fluxbox test is" << test;
    return (!test.isEmpty());
}

bool defaultlook::checklightdm()
{
    QFileInfo test(QStringLiteral("/etc/lightdm/lightdm-gtk-greeter.conf"));
    return (test.exists());
}

bool defaultlook::checkPlasma() const
{
    QString test = runCmd(QStringLiteral("pgrep plasma")).output;
    if (verbose) qDebug() << test;
    return (!test.isEmpty());
}

// backs up the current panel configuration
void defaultlook::backupPanel()
{
    //ensure .restore folder exists
    QString home_path = QDir::homePath();
    if ( ! QDir(home_path + "/.restore/").exists()){
        runCmd("mkdir -p " + home_path + "/.restore/");
    }
    //validate file name
    qDebug() << "ui file name " << ui->lineEditBackupName->text();
    //QRegExp rx("(@|\\$|%|\\&|\\*|(|)|{|}|[|]|/|\\|\\?");
    QRegExp rx("\\$|@|%|\\&|\\*|\\(|\\)|\\[|\\]|\\{|\\}|\\||\\?");
    int rxtest = rx.indexIn(ui->lineEditBackupName->text());
    qDebug() << "rxtest" << rxtest;
    if ( rxtest > 0 ){
        QMessageBox::information(nullptr, tr("MX Tweak"),
                                 tr("Plese remove special characters") + "@,$,%,&,*,(,),[,],{,},|,\\,?" + tr("from file name"));
        validateflag = true;
        } else {
        QString path = home_path + "/.restore/" + ui->lineEditBackupName->text() + ".tar.xz";
        qDebug() << path;
        //check filename existence
        if (QFileInfo::exists(path)) {
            QMessageBox::information(nullptr, tr("MX Tweak"), tr("File name already exists.  Choose another name"));
        } else {
            path = "$HOME/.restore/\"" + ui->lineEditBackupName->text() + ".tar.xz\"";
            runCmd("tar --create --xz --file=" + path + " --directory=$HOME/.config/xfce4 panel xfconf/xfce-perchannel-xml/xfce4-panel.xml");
        }
    }
}

void defaultlook::restoreDefaultPanel()
{
    // copy template files
    runCmd(QStringLiteral("xfce4-panel --quit;pkill xfconfd; rm -Rf ~/.config/xfce4/panel; cp -Rf /etc/skel/.config/xfce4/panel ~/.config/xfce4; sleep 1; \
           cp -f /etc/skel/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml; \
           sleep 5; xfce4-panel &"));
}

void defaultlook::restoreBackup()
{
    //validate file first


    switch(validatearchive("$HOME/.restore/\"" + ui->comboBoxAvailableBackups->currentText() + ".tar.xz\"")){
    case 0:
        runCmd("xfce4-panel --quit; pkill xfconfd; rm -Rf ~/.config/xfce4/panel ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml; \
           tar -xf $HOME/.restore/\"" + ui->comboBoxAvailableBackups->currentText() + ".tar.xz\" --directory=$HOME/.config/xfce4; \
           sleep 5; xfce4-panel &");
        break;
    case 1: QMessageBox::information(nullptr, tr("MX Tweak"),
                                     tr("File is not a valid tar.xz archive file"));
        break;
    case 2:  QMessageBox::information(nullptr, tr("MX Tweak"),
                                      tr("Archive does not contain a panel config"));
        break;
    }
}

void defaultlook::on_checkHorz_clicked()
{
    ui->buttonApply->setEnabled(true);
    if (ui->checkHorz->isChecked()) {
        ui->checkVert->setChecked(false);
        ui->radioBackupPanel->setChecked(false);
        ui->radioDefaultPanel->setChecked(false);
        ui->radioRestoreBackup->setChecked(false);
        ui->radioButtonTasklist->setChecked(false);
        ui->radioButtonSetPanelPluginScales->setChecked(false);
        ui->doubleSpinBoxpaplugin->setEnabled(false);
        ui->doubleSpinBoxpmplugin->setEnabled(false);
    }
}

void defaultlook::on_checkVert_clicked()
{
    ui->buttonApply->setEnabled(true);
    if (ui->checkVert->isChecked()) {
        ui->checkHorz->setChecked(false);
        ui->radioBackupPanel->setChecked(false);
        ui->radioDefaultPanel->setChecked(false);
        ui->radioRestoreBackup->setChecked(false);
        ui->radioButtonTasklist->setChecked(false);
        ui->radioButtonSetPanelPluginScales->setChecked(false);
        ui->doubleSpinBoxpaplugin->setEnabled(false);
        ui->doubleSpinBoxpmplugin->setEnabled(false);
        ui->Label_Volume_plugin->setEnabled(false);
        ui->Label_power_manager_plugin->setEnabled(false);
    }
}

void defaultlook::on_radioDefaultPanel_clicked()
{
    ui->buttonApply->setEnabled(true);
    if (ui->radioDefaultPanel->isChecked()) {
        ui->checkHorz->setChecked(false);
        ui->radioBackupPanel->setChecked(false);
        ui->checkVert->setChecked(false);
        ui->radioRestoreBackup->setChecked(false);
        ui->lineEditBackupName->hide();
        ui->radioButtonTasklist->setChecked(false);
        ui->radioButtonSetPanelPluginScales->setChecked(false);
        ui->doubleSpinBoxpaplugin->setEnabled(false);
        ui->doubleSpinBoxpmplugin->setEnabled(false);
        ui->Label_Volume_plugin->setEnabled(false);
        ui->Label_power_manager_plugin->setEnabled(false);
    }
}

void defaultlook::on_radioBackupPanel_clicked()
{
    ui->buttonApply->setEnabled(true);
    if (ui->radioBackupPanel->isChecked()) {
        ui->checkHorz->setChecked(false);
        ui->checkVert->setChecked(false);
        ui->radioDefaultPanel->setChecked(false);
        ui->radioRestoreBackup->setChecked(false);
        ui->lineEditBackupName->setText("panel_backup_" + QDateTime::currentDateTime().toString("dd.MM.yyyy.hh.mm.ss"));
        ui->lineEditBackupName->show();
        ui->radioButtonTasklist->setChecked(false);
        ui->radioButtonSetPanelPluginScales->setChecked(false);
        ui->doubleSpinBoxpaplugin->setEnabled(false);
        ui->doubleSpinBoxpmplugin->setEnabled(false);
        ui->Label_Volume_plugin->setEnabled(false);
        ui->Label_power_manager_plugin->setEnabled(false);
    }
}

void defaultlook::on_radioRestoreBackup_clicked()
{
    ui->buttonApply->setEnabled(true);
    if (ui->radioRestoreBackup->isChecked()) {
        ui->checkHorz->setChecked(false);
        ui->radioBackupPanel->setChecked(false);
        ui->radioDefaultPanel->setChecked(false);
        ui->checkVert->setChecked(false);
        ui->lineEditBackupName->hide();
        ui->radioButtonTasklist->setChecked(false);
        ui->radioButtonSetPanelPluginScales->setChecked(false);
        ui->doubleSpinBoxpaplugin->setEnabled(false);
        ui->doubleSpinBoxpmplugin->setEnabled(false);
        ui->Label_Volume_plugin->setEnabled(false);
        ui->Label_power_manager_plugin->setEnabled(false);
    }
}

void defaultlook::on_radioButtonTasklist_clicked()
{
    ui->buttonApply->setEnabled(true);
    if (ui->radioButtonTasklist->isChecked()) {
        ui->checkHorz->setChecked(false);
        ui->radioBackupPanel->setChecked(false);
        ui->radioDefaultPanel->setChecked(false);
        ui->checkVert->setChecked(false);
        ui->lineEditBackupName->hide();
        ui->radioRestoreBackup->setChecked(false);
        ui->radioButtonSetPanelPluginScales->setChecked(false);
        ui->doubleSpinBoxpaplugin->setEnabled(false);
        ui->doubleSpinBoxpmplugin->setEnabled(false);
        ui->Label_Volume_plugin->setEnabled(false);
        ui->Label_power_manager_plugin->setEnabled(false);
    }

 }

void defaultlook::on_radioButtonSetPanelPluginScales_clicked()
{
    ui->buttonApply->setEnabled(true);
    if (ui->radioButtonSetPanelPluginScales->isChecked()) {
        ui->checkHorz->setChecked(false);
        ui->radioBackupPanel->setChecked(false);
        ui->radioDefaultPanel->setChecked(false);
        ui->checkVert->setChecked(false);
        ui->lineEditBackupName->hide();
        ui->radioRestoreBackup->setChecked(false);
        ui->radioButtonTasklist->setChecked(false);
        ui->doubleSpinBoxpaplugin->setEnabled(true);
        ui->doubleSpinBoxpmplugin->setEnabled(true);
        ui->Label_Volume_plugin->setEnabled(true);
        ui->Label_power_manager_plugin->setEnabled(true);
    }
}

void defaultlook::top_or_bottom()
{
    if (!panelflag) {
        return;
    }
    //move to user selected top or bottom border per mx-16 defaults  p=11 is top, p=12 is bottom

    QString top_bottom;
    if (ui->comboboxHorzPostition->currentIndex() == 0) {
        top_bottom = QStringLiteral("12");
    }

    if (ui->comboboxHorzPostition->currentIndex() == 1) {
        top_bottom = QStringLiteral("11");
    }

    if (verbose) qDebug() << "position index is : " << ui->comboboxHorzPostition->currentIndex();
    if (verbose) qDebug() << "position is :" << top_bottom;

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel + "/position -s 'p=" + top_bottom + ";x=0;y=0'");
}

void defaultlook::left_or_right()
{
    if (!panelflag) {
        return;
    }
    //move to user selected top or bottom border per mx-16 defaults  p=5 is left, p=1 is right

    QString left_right;
    if (ui->comboboxVertpostition->currentIndex() == 0) {
        left_right = QStringLiteral("5");
    }

    if (ui->comboboxVertpostition->currentIndex() == 1) {
        left_right = QStringLiteral("1");
    }

    if (verbose) qDebug() << "position index is : " << ui->comboboxVertpostition->currentIndex();
    if (verbose) qDebug() << "position is :" << left_right;

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel + "/position -s 'p=" + left_right + ";x=0;y=0'");
}

void defaultlook::message2()
{
    QMessageBox::information(nullptr, tr("Panel settings"),
                             tr("Your current panel settings have been backed up in a hidden folder called .restore in your home folder (~/.restore/)"));
}

void defaultlook::on_toolButtonXFCEpanelSettings_clicked()
{
    this->hide();
    system("xfce4-panel --preferences");
    system("xprop -spy -name \"Panel Preferences\" >/dev/null");
    this->show();
    QString test;
    bool flag = false;

    //restart panel if background style of any panel is 1 - solid color, affects transparency
    QStringList panelproperties = runCmd(QStringLiteral("xfconf-query -c xfce4-panel --list |grep background-style")).output.split('\n');

    QStringListIterator changeIterator(panelproperties);

    while (changeIterator.hasNext()) {
        QString value = changeIterator.next();
        test = runCmd("xfconf-query -c xfce4-panel -p " + value).output;
        if ( test == QLatin1String("1") ) {
            flag = true;
        }
    }

    if (flag) {
        system("xfce4-panel --restart");
    }

    setuppanel();
}

void defaultlook::on_toolButtonXFCEAppearance_clicked()
{
    this->hide();
    system("xfce4-appearance-settings");
    this->show();
}

void defaultlook::on_toolButtonXFCEWMsettings_clicked()
{
    this->hide();
    system("xfwm4-settings");
    this->show();
}

void defaultlook::on_comboboxHorzPostition_currentIndexChanged(const QString & /*arg1*/)
{
    if (verbose) qDebug() << "top or bottom output " << ui->comboboxHorzPostition->currentText();
    QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/mode").output;
    if (verbose) qDebug() << "test value, blank or 0 runs left_or_right" << test;
    if (test == QLatin1String("")) {
        top_or_bottom();
    }
    if (test == QLatin1String("0")) {
        top_or_bottom();
    }
}

void defaultlook::on_comboboxVertpostition_currentIndexChanged(const QString & /*arg1*/)
{
    if (verbose) qDebug() << "left or right output " << ui->comboboxVertpostition->currentText();
    QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/mode").output;
    if (verbose) qDebug() << "test value, 1 or 2 runs top_or_bottom" << test;
    if (test == QLatin1String("1")) {
        left_or_right();
    }
    if (test == QLatin1String("2")) {
        left_or_right();
    }
}

void defaultlook::setuppanel()
{
    QString home_path = QDir::homePath();
    QFileInfo backuppanel(home_path + "/.restore/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml");
    if (backuppanel.exists()) {
        migratepanel(backuppanel.lastModified().toString("dd.MM.yyyy.hh.mm.ss"));
        //message2();
    }

    //setup pulseaudio plugin scale functoin
    //get files setup if they don't exist
    if (QFileInfo(home_path + "/.config/gtk-3.0/gtk.css").exists()) {
        if (verbose) qDebug() << "existing gtk.css found";
        QString cmd = "cat " + home_path + "/.config/gtk-3.0/gtk.css |grep -q xfce4-panel-tweaks.css";
        if (system(cmd.toUtf8()) == 0 ) {
            if (verbose) qDebug() << "include statement found";
        } else {
            if (verbose) qDebug() << "adding include statement";
            QString cmd = "echo '@import url(\"xfce4-panel-tweaks.css\");' >> " + home_path + "/.config/gtk-3.0/gtk.css";
            system(cmd.toUtf8());
        }
    } else {
        if (verbose) qDebug() << "creating simple gtk.css file";
        QString cmd = "echo '@import url(\"xfce4-panel-tweaks.css\");' >> " + home_path + "/.config/gtk-3.0/gtk.css";
        system(cmd.toUtf8());
    }

    if (!QFileInfo(home_path + "/.config/gtk-3.0/xfce4-panel-tweaks.css").exists()) {
        QString cmd = "cp /usr/share/mx-tweak/xfce4-panel-tweaks.css " + home_path + "/.config/gtk-3.0/";
        system(cmd.toUtf8());
    }
    //check for existence of plugins before running these commands, hide buttons and labels if not present.
    //Get value of scale
    QString plugins = runCmd("grep plugin " + home_path + "/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
    bool volumeplugin;
    bool powerplugin;
    if (plugins.contains("pulseaudio")){
        ui->doubleSpinBoxpaplugin->setValue(runCmd("grep -A 1 pulseaudio " + home_path + "/.config/gtk-3.0/xfce4-panel-tweaks.css |grep scale |cut -d'(' -f2 |cut -d')' -f1").output.toDouble());
        volumeplugin = true;
    } else {
        ui->doubleSpinBoxpaplugin->hide();
        ui->Label_Volume_plugin->hide();
        volumeplugin = false;
    }
    if (plugins.contains("power-manager-plugin")){
        ui->doubleSpinBoxpmplugin->setValue(runCmd("grep -A 1 xfce4-power-manager-plugin " + home_path + "/.config/gtk-3.0/xfce4-panel-tweaks.css |grep scale |cut -d'(' -f2 |cut -d')' -f1").output.toDouble());
        powerplugin = true;
    } else {
        ui->doubleSpinBoxpmplugin->hide();
        ui->Label_power_manager_plugin->hide();
        powerplugin = false;
    }

    if (! volumeplugin && ! powerplugin){
        ui->label_panel_plugin_scales->hide();
        ui->radioButtonSetPanelPluginScales->hide();
    }



    ui->comboBoxAvailableBackups->clear();
    ui->lineEditBackupName->hide();
    ui->lineEditBackupName->setText("panel_backup_" + QDateTime::currentDateTime().toString("dd.MM.yyyy.hh.mm.ss"));
    QStringList availablebackups = QDir(home_path + "/.restore").entryList(QStringList() << "*.tar.xz",QDir::Files);

    // if backup available, make the restore backup option available

    if ( availablebackups.isEmpty()){
        backupPanel();
        availablebackups = QDir(home_path + "/.restore").entryList(QStringList() << "*.tar.xz",QDir::Files);
    }
    availablebackups.replaceInStrings(".tar.xz", "");
    ui->comboBoxAvailableBackups->addItems(availablebackups);
    ui->radioBackupPanel->setToolTip(home_path + "/.restore");
    ui->radioRestoreBackup->setToolTip(home_path + "/.restore");
    ui->radioDefaultPanel->setToolTip(QStringLiteral("/etc/skel/.config/xfce4"));


    panelflag = false;
    ui->buttonApply->setEnabled(false);
    if (ui->buttonApply->icon().isNull()) {
        ui->buttonApply->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }

    //hide tasklist setting if not present
    bool tasklist = true;
    bool docklike = true;
    bool tasklistcombodisplay = true;

    if ( system("xfconf-query -c xfce4-panel -p /plugins -lv |grep tasklist") != 0 ) {
        ui->labelTasklist->hide();
        ui->pushButtontasklist->hide();
        tasklist = false;
    }

    //hide docklike settings if not present

    if ( system("xfconf-query -c xfce4-panel -p /plugins -lv |grep docklike") != 0 ) {
        ui->labelDocklikeSettings->hide();
        ui->pushButtonDocklikeSetttings->hide();
        docklike = false;
    }

    //check status of docklike external package, hide chooser if not installed

    QString check = runCmd("LANG=c dpkg-query -s xfce4-docklike-plugin |grep Status").output;
    if ( ! check.contains("installed")){
        docklike = true;
    }

    //display tasklist plugin selector if only one tasklist in use
    if ( tasklist && docklike ){
        ui->label_tasklist_plugin->hide();
        ui->comboBoxTasklistPlugin->hide();
        tasklistcombodisplay =  false;
    }

    //index 0 is docklike, index 1 is window buttons
    if ( tasklist && tasklistcombodisplay ){
        ui->comboBoxTasklistPlugin->setCurrentIndex(1);
    }

    if ( docklike && tasklistcombodisplay){
        ui->comboBoxTasklistPlugin->setCurrentIndex(0);
    }

    tasklistflag = false;

    //reset all checkboxes to unchecked

    ui->checkVert->setChecked(false);
    ui->checkHorz->setChecked(false);
    ui->radioBackupPanel->setChecked(false);
    ui->radioDefaultPanel->setChecked(false);
    ui->radioRestoreBackup->setChecked(false);
    ui->radioButtonTasklist->setChecked(false);
    ui->radioButtonSetPanelPluginScales->setChecked(false);
    ui->doubleSpinBoxpaplugin->setEnabled(false);
    ui->doubleSpinBoxpmplugin->setEnabled(false);

    //only enable options that make sense

    //if panel is already horizontal, set vertical option available, and vice versa  "" and "0" are horizontal

    QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/mode").output;
    QString test2 = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel + "/position").output.section(QStringLiteral(";"), 0,0);
    if (verbose) qDebug() << "test2" << test2;

    if (test == QLatin1String("") || test == QLatin1String("0")) {
        ui->checkVert->setEnabled(true);
        ui->checkHorz->setChecked(true);
        if (test2 == QLatin1String("p=11") || test2 == QLatin1String("p=6") || test2 == QLatin1String("p=2")) {
            ui->comboboxHorzPostition->setCurrentIndex(1);
        }

    }

    if (test == QLatin1String("1") || test == QLatin1String("2")) {
        ui->checkVert->setChecked(true);
        ui->checkHorz->setEnabled(true);
        if (test2 == QLatin1String("p=1")) {
            ui->comboboxVertpostition->setCurrentIndex(1);
        }
    }

    panelflag = true;
}

void defaultlook::setupPlasma()
{
    QString home_path = QDir::homePath();
    //get panel ID
    plasmaPanelId = runCmd(QStringLiteral("grep --max-count 1 -B 8 panel $HOME/.config/plasma-org.kde.plasma.desktop-appletsrc |grep Containment")).output;
    QString panellocation = readPlasmaPanelConfig(QStringLiteral("location"));

    switch(panellocation.toInt()) {
    case PanelLocation::Top:
        ui->comboPlasmaPanelLocation->setCurrentIndex(PanelIndex::Top);
        break;
    case PanelLocation::Bottom:
        ui->comboPlasmaPanelLocation->setCurrentIndex(PanelIndex::Bottom);
        break;
    case PanelLocation::Left:
        ui->comboPlasmaPanelLocation->setCurrentIndex(PanelIndex::Left);
        break;
    case PanelLocation::Right:
        ui->comboPlasmaPanelLocation->setCurrentIndex(PanelIndex::Right);
        break;
    default: ui->comboPlasmaPanelLocation->setCurrentIndex(PanelIndex::Bottom);
    }

    //setup plasma-discover autostart
    if (QFile("/usr/lib/x86_64-linux-gnu/libexec/DiscoverNotifier").exists()){
        QString plasmadiscoverautostart = home_path + "/.config/autostart/org.kde.discover.notifier.desktop";
        if (runCmd("grep Hidden=true " + plasmadiscoverautostart).exitCode == 0 ){
            ui->checkBoxPlasmaDiscoverUpdater->setChecked(false);
        } else {
            ui->checkBoxPlasmaDiscoverUpdater->setChecked(true);
        }
    } else {
        ui->checkBoxPlasmaDiscoverUpdater->hide();
    }

    //setup singleclick
    QString singleclick = runCmd(QStringLiteral("kreadconfig5 --group KDE --key SingleClick")).output;
    ui->checkBoxPlasmaSingleClick->setChecked(singleclick != QLatin1String("false"));

    //get taskmanager ID and setup showOnlyCurrentDesktop
    plasmataskmanagerID = runCmd(QStringLiteral("grep --max-count 1 -B 2 taskmanager $HOME/.config/plasma-org.kde.plasma.desktop-appletsrc |grep Containment")).output;
    QString showOnlyCurrentDesktop = readTaskmanagerConfig(QStringLiteral("showOnlyCurrentDesktop"));
    if (showOnlyCurrentDesktop == QLatin1String("true")) {
        ui->checkBoxPlasmaShowAllWorkspaces->setChecked(false);
    } else {
        ui->checkBoxPlasmaShowAllWorkspaces->setChecked(true);
    }

    ui->ButtonApplyPlasma->setDisabled(true);

    ui->checkboxplasmaresetdock->setChecked(false);

    plasmaplacementflag = false;
    plasmaworkspacesflag = false;
    plasmasingleclickflag = false;
    plasmaresetflag = false;
    plasmasystrayiconsizeflag = false;
    plasmadisoverautostartflag = false;
}

QString defaultlook::readTaskmanagerConfig(const QString &key) const
{
    QString ID = plasmataskmanagerID.section(QStringLiteral("["),2,2).section(QStringLiteral("]"),0,0);
    QString Applet = plasmataskmanagerID.section(QStringLiteral("["),4,4).section(QStringLiteral("]"),0,0);
    if (verbose) qDebug() << "plasma taskmanager ID is " << ID;
    if (verbose) qDebug() << "plasma taskmanger Applet ID is " << Applet;
    //read key
    QString value = runCmd("kreadconfig5 --file plasma-org.kde.plasma.desktop-appletsrc --group Containments --group " + ID + " --group Applets --group " + Applet + " --key " + key).output;
    if (value.isEmpty()) {
        value = QStringLiteral("false");
    }
    if (verbose) qDebug() << "key is " << value;
    return value;
}

QString defaultlook::readPlasmaPanelConfig(const QString &key) const
{
    QString ID;
    ID = plasmaPanelId.section(QStringLiteral("["),2,2).section(QStringLiteral("]"),0,0);
    if (verbose) qDebug() << "plasma panel ID" << ID;
    //read key
    QString value = runCmd("kreadconfig5 --file plasma-org.kde.plasma.desktop-appletsrc --group Containments --group " + ID + " --key " + key).output;
    if (verbose) qDebug() << "key is " << value;
    return value;
}


void defaultlook::setupFluxbox()
{
    //resets
    QFileInfo resetALL(QStringLiteral("/usr/bin/mxflux_install.sh"));
    QFileInfo resetDefaultDock(QStringLiteral("/etc/skel/.fluxbox/scripts/DefaultDock.mxdk"));
    QFileInfo resetDefaultMenu(QStringLiteral("/etc/skel/.fluxbox/menu-mx"));
    QFileInfo idesktogglepresent(QStringLiteral("/usr/bin/idesktoggle"));
    QFileInfo ideskpresent(QStringLiteral("/usr/bin/idesk"));
    QFileInfo menumigrate(QStringLiteral("/usr/bin/menu-migrate"));

    if (!menumigrate.exists()) {
        ui->checkBoxMenuMigrate->setDisabled(true);
    }

    if (!resetALL.exists()) {
        ui->checkboxfluxreseteverything->setDisabled(true);
    }

    if (!resetDefaultDock.exists()) {
        ui->checkboxfluxresetdock->setDisabled(true);
    }

    if (!resetDefaultMenu.exists()) {
        ui->checkboxfluxresetmenu->setDisabled(true);
    }

    if (!idesktogglepresent.exists() || !ideskpresent.exists()) {
        ui->comboBoxfluxIcons->setDisabled(true);
        ui->comboBoxfluxcaptions->setDisabled(true);
    } else {
        QString test;
        test = runCmd(QStringLiteral("grep \\#Caption $HOME/.idesktop/*.lnk")).output;
        if (test.isEmpty()) {
            ui->comboBoxfluxcaptions->setCurrentIndex(0);
            test = runCmd(QStringLiteral("grep CaptionOnHover $HOME/.ideskrc |grep -v ToolTip")).output.section(QStringLiteral(":"),1,1).trimmed();
            if (verbose) qDebug() << "hover test" << test;
            if (test.contains(QLatin1String("true"))) {
                ui->comboBoxfluxcaptions->setCurrentIndex(2);
            }
        }
        test = runCmd(QStringLiteral("grep \\#Icon $HOME/.idesktop/*.lnk")).output;
        if (test.isEmpty()) {
            ui->comboBoxfluxIcons->setCurrentIndex(0);
        }

    }

    fluxiconflag = false;
    fluxcaptionflag =false;

    //screenblanking value;
    QString screenblanktimeout = runCmd("xset q |grep timeout | awk '{print $2}'").output.trimmed();
    ui->spinBoxScreenBlankingTimeout->setValue(screenblanktimeout.toInt()/60);
    ui->spinBoxScreenBlankingTimeout->setToolTip("set to 0 minutes to disable screensaver");

    //toolbar autohide
    QString toolbarautohide = runCmd(QStringLiteral("grep screen0.toolbar.autoHide $HOME/.fluxbox/init")).output.section(QStringLiteral(":"),1,1).trimmed();
    if (verbose) qDebug() << "Toolbar autohide" << toolbarautohide;
    if (toolbarautohide == QLatin1String("true")) {
        ui->checkboxfluxtoolbarautohide->setChecked(true);
    } else {
        ui->checkboxfluxtoolbarautohide->setChecked(false);
    }
    QString toolbarvisible = runCmd(QStringLiteral("grep session.screen0.toolbar.visible: $HOME/.fluxbox/init")).output.section(QStringLiteral(":"),1,1).trimmed();
    if (verbose) qDebug() << "Toolbar visible" << toolbarautohide;
    if (toolbarvisible == QLatin1String("true")) {
        ui->checkBoxFluxShowToolbar->setChecked(true);
    } else {
        ui->checkBoxFluxShowToolbar->setChecked(false);
    }
    //toolbar location
    QString toolbarlocation = runCmd(QStringLiteral("grep screen0.toolbar.placement $HOME/.fluxbox/init")).output.section(QStringLiteral(":"),1,1).trimmed();
    if (verbose) qDebug() << "Toolbar location" << toolbarlocation;
    ui->combofluxtoolbarlocatoin->setCurrentText(toolbarlocation);
    //slit location
    QString slitlocation = runCmd(QStringLiteral("grep screen0.slit.placement $HOME/.fluxbox/init")).output.section(QStringLiteral(":"),1,1).trimmed();
    if (verbose) qDebug() << "Slit location" << slitlocation;
    ui->combofluxslitlocation->setCurrentText(slitlocation);
    //toolbar width;
    QString toolbarwidth = runCmd(QStringLiteral("grep screen0.toolbar.widthPercent $HOME/.fluxbox/init")).output.section(QStringLiteral(":"),1,1).trimmed();
    if (verbose) qDebug() << "Toolbar width" << toolbarwidth;
    ui->spinBoxFluxToolbarWidth->setValue(toolbarwidth.toInt());
    //toolbar height
    QString toolbarheight = runCmd(QStringLiteral("grep screen0.toolbar.height $HOME/.fluxbox/init")).output.section(QStringLiteral(":"),1,1).trimmed();
    if (verbose) qDebug() << "Toolbar height" << toolbarheight;
    ui->spinBoxFluxToolbarHeight->setValue(toolbarheight.toInt());
    //slit autohide
    QString slitautohide = runCmd(QStringLiteral("grep screen0.slit.autoHide $HOME/.fluxbox/init")).output.section(QStringLiteral(":"),1,1).trimmed();
    if (verbose) qDebug() << "slit autohide" << slitautohide;
    ui->checkboxfluxSlitautohide->setChecked(slitautohide == QLatin1String("true"));
    ui->ApplyFluxboxResets->setDisabled(true);

    //thunar options
    //only if thunar exists, else hide
    if (QFile("/usr/bin/thunar").exists()){
        //thunar single click
        thunarsingleclicksetup();
        //thunarsetupsplitview
        thunarsetupsplitview();
    } else {
        ui->checkBoxThunarCAReset_2->hide();
        ui->checkBoxThunarSplitView_2->hide();
        ui->checkBoxsplitviewhorizontal_2->hide();
        ui->checkBoxThunarSingleClick_2->hide();
    }
}

void defaultlook::setupEtc()
{
    QString home_path = QDir::homePath();
    QString DESKTOP = runCmd(QStringLiteral("echo $XDG_SESSION_DESKTOP")).output;
    if (verbose) qDebug() << "setupetc nocsd desktop is:" << DESKTOP;

    ui->checkBoxLightdmReset->setChecked(false);
    QString test;
    if (!isLightdm) {
        ui->checkBoxLightdmReset->hide();
    }
    if (graphicssetupflag){
        ui->checkboxIntelDriver->hide();
        ui->labelIntel->hide();
        ui->checkboxAMDtearfree->hide();
        ui->labelamdgpu->hide();
        ui->checkboxRadeontearfree->hide();
        ui->labelradeon->hide();
        graphicssetupflag=false;
    }

    ui->ButtonApplyEtc->setEnabled(false);
    if (ui->ButtonApplyEtc->icon().isNull()) {
        ui->ButtonApplyEtc->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }
    //set values for checkboxes

    //fluxbox menu auto generation on package install, removal, and upgrades
    if ( QFile("/usr/bin/mxfb-menu-generator").exists()){
        if (QFile(home_path + "/.fluxbox/mxfb-menu-generator-disabled.chk").exists()){
            ui->checkBoxDisableFluxboxMenuGeneration->setChecked(false);
        } else {
            ui->checkBoxDisableFluxboxMenuGeneration->setChecked(true);
        }
    } else {
        ui->checkBoxDisableFluxboxMenuGeneration->hide();
    }

    //setup udisks option
    QFileInfo fileinfo(QStringLiteral("/etc/tweak-udisks.chk"));
    if (fileinfo.exists()) {
        ui->checkBoxMountInternalDrivesNonRoot->setChecked(true);
    } else {
        ui->checkBoxMountInternalDrivesNonRoot->setChecked(false);
    }

    //setup sudo override function

    int rootest = runCmd(QStringLiteral("pkexec /usr/lib/mx-tweak/mx-tweak-rootcheck.sh")).exitCode;
    if ( rootest == 0 ){
        ui->radioSudoRoot->setChecked(true);
    } else {
        ui->radioSudoUser->setChecked(true);
    }

    //if root accout disabled, disable root authentication changes
    test = runCmd(QStringLiteral("pkexec /usr/lib/mx-tweak/mx-tweak-check.sh")).output;
    if ( test.contains(QLatin1String("NP"))) {
        ui->radioSudoRoot->setEnabled(false);
        ui->radioSudoUser->setEnabled(false);
        ui->label_11->setEnabled(false);
    }

    //setup user namespaces option (99-sandbox-mx.conf)
    sandboxflag = false;
    QString userns_clone = runCmd(QStringLiteral("/usr/sbin/sysctl -n kernel.unprivileged_userns_clone")).output;
    //QString yama_ptrace = runCmd("/usr/sbin/sysctl -n kernel.yama.ptrace_scope").output;
    if (verbose) qDebug() << "userns_clone is: " << userns_clone;
    if (userns_clone == QLatin1String("0") || userns_clone == QLatin1String("1")) {
        //if (userns_clone == "1" && yama_ptrace == "1") {
        ui->checkBoxSandbox->setChecked(userns_clone == QLatin1String("1"));
    } else {
        ui->checkBoxSandbox->hide();
    }

    //setup bluetooth auto enable, hide box if config file doesn't exist
    if ( QFile("/etc/bluetooth/main.conf").exists()){
        bluetoothautoenableflag = false;
        test = runCmd(QStringLiteral("grep ^AutoEnable /etc/bluetooth/main.conf")).output;
        test = test.section("=",1,1);
        if ( test == "true"){
            ui->checkBoxbluetoothAutoEnable->setChecked(true);
        } else {
            ui->checkBoxbluetoothAutoEnable->setChecked(false);
        }
    } else {
        ui->checkBoxbluetoothAutoEnable->hide();
    }

    //setup bluetooth battery info
    if ( QFile("/etc/bluetooth/main.conf").exists()){
        bluetoothbatteryflag = false;
        test = runCmd(QStringLiteral("grep -E '^(#\\s*)?Experimental' /etc/bluetooth/main.conf")).output;
        if (verbose) qDebug() << "bluetooth battery " << test;
        if (test.contains("#")){
            ui->checkBoxBluetoothBattery->setChecked(false);
        } else if ( test.contains("true") ){
            ui->checkBoxBluetoothBattery->setChecked(true);
        } else if ( test.contains("false")) {
            ui->checkBoxBluetoothBattery->setChecked(false);
        }
    } else {
        ui->checkBoxBluetoothBattery->hide();
    }

    //setup early KVM module loading
    test = runCmd("LANG=C grep \"enable_virt_at_load=0\" /etc/modprobe.d/* | grep kvm").output;
    if (!test.isEmpty()){
        if (!test.section(":",1,1).startsWith("#" )){
        ui->checkBoxKVMVirtLoad->setChecked(true);
        kvmconffile=test.section(":",0,0);
        if (verbose) qDebug() << "kvm conf file is " << kvmconffile;
        } else {
            ui->checkBoxKVMVirtLoad->setChecked(false);
            kvmconffile = "/etc/modprobe.d/kvm.conf";
        }
    } else {
        ui->checkBoxKVMVirtLoad->setChecked(false);
        kvmconffile = "/etc/modprobe.d/kvm.conf";
    }
    //set flag false so future changes processed, but not an unchanged checkbox
    kvmflag=false;

    //setup apt install_recommends
    //enable checkbox only if Install-Recommends is set to 1. default is 0 or no if no existanct apt.conf
    if ( QFile("/etc/apt/apt.conf").exists()){
        test = runCmd(QStringLiteral("grep Install-Recommends /etc/apt/apt.conf")).output;
        if ( test.contains("1")){
            ui->checkBoxInstallRecommends->setChecked(true);
        } else {
            ui->checkBoxInstallRecommends->setChecked(false);
        }
    } else {
        ui->checkBoxInstallRecommends->setChecked(false);
    }

    //setup kernel auto updates

    if (runCmd("LC_ALL=C dpkg --status linux-image-amd64 linux-image-686 linux-image-686-pae 2>/dev/null |grep 'ok installed'").output.isEmpty()){
        ui->checkBoxDebianKernelUpdates->setChecked(false);
        ui->checkBoxDebianKernelUpdates->hide();
    } else {
        ui->checkBoxDebianKernelUpdates->setChecked(true);
    }
    if (runCmd("LC_ALL=C dpkg --status linux-image-liquorix-amd64 2>/dev/null |grep 'ok installed'").output.isEmpty()){
        ui->checkBoxLiqKernelUpdates->setChecked(false);
        ui->checkBoxLiqKernelUpdates->hide();
    } else {
        ui->checkBoxLiqKernelUpdates->setChecked(true);
    }
    QString autoupdate = runCmd("apt-mark showhold").output;
    if ( autoupdate.contains("linux-image-686") || autoupdate.contains("linux-image-amd64") ){
        ui->checkBoxDebianKernelUpdates->setChecked(false);
    }
    if ( autoupdate.contains("linux-image-liquorix-amd64") ){
        ui->checkBoxLiqKernelUpdates->setChecked(false);
    }
    debianKernelUpdateFlag = false;
    liqKernelUpdateFlag = false;

    //hostname
    ui->checkBoxComputerName->setChecked(false);
    ui->lineEditHostname->setEnabled(false);
    originalhostname = runCmd("hostname").output;
    ui->lineEditHostname->setText(originalhostname);

    //setup NOCSD GTK3 option
    if (!QFileInfo::exists(QStringLiteral("/usr/bin/gtk3-nocsd"))) {
        if (verbose) qDebug() << "gtk3-nocsd not found";
        ui->checkBoxCSD->hide();
    } else {
        if (verbose) qDebug() << "gtk3-nocsd found";
    }
    if (verbose) qDebug() << "home path nocsd is" << home_path + "/.config/MX-Linux/nocsd/" + DESKTOP;
    if (QFileInfo::exists(home_path + "/.config/MX-Linux/nocsd/" + DESKTOP)) {
        ui->checkBoxCSD->setChecked(false);
    } else {
        ui->checkBoxCSD->setChecked(true);
    }

    Intel_flag = false;
    amdgpuflag = false;
    radeon_flag =false;
    enable_recommendsflag = false;
    //setup Intel checkbox

    QString partcheck = runCmd(QStringLiteral(R"(for i in $(lspci -n | awk '{print $2,$1}' | grep -E '^(0300|0302|0380)' | cut -f2 -d\ ); do lspci -kns "$i"; done)")).output;
    if (verbose) qDebug()<< "partcheck = " << partcheck;

    if ( partcheck.contains(QLatin1String("i915"))) {
        ui->checkboxIntelDriver->show();
        ui->labelIntel->show();
    }

    if ( partcheck.contains(QLatin1String("Kernel driver in use: amdgpu"))) {
        ui->checkboxAMDtearfree->show();
        ui->labelamdgpu->show();
    }

    if ( partcheck.contains(QLatin1String("Kernel driver in use: radeon"))) {
        ui->checkboxRadeontearfree->show();
        ui->labelradeon->show();
    }

    QFileInfo intelfile(QStringLiteral("/etc/X11/xorg.conf.d/20-intel.conf"));
    ui->checkboxIntelDriver->setChecked(intelfile.exists());

    QFileInfo amdfile(QStringLiteral("/etc/X11/xorg.conf.d/20-amd.conf"));
    ui->checkboxAMDtearfree->setChecked(amdfile.exists());

    QFileInfo radeonfile(QStringLiteral("/etc/X11/xorg.conf.d/20-radeon.conf"));
    ui->checkboxRadeontearfree->setChecked(radeonfile.exists());

    //setup display manager combo box
    QString displaymanagers=runCmd("dpkg --list sddm gdm3 lightdm slim slimski xdm wdm lxdm nodm 2>/dev/null |grep ii | awk '{print $2}'").output;
    QStringList displaymanagerlist = displaymanagers.split(QStringLiteral("\n"));
    if ( displaymanagerlist.count() > 1) {
        //only add items once
        if (ui->comboBoxDisplayManager->count() == 0) {
            ui->comboBoxDisplayManager->addItems(displaymanagerlist);
            //set default selection to current display manager
            //read from /etc/X11/default-display-manager if it exits, else use running dm
            QFile defaultdisplay("/etc/X11/default-display-manager");
            if (defaultdisplay.exists()){
                if (defaultdisplay.open(QIODevice::ReadOnly | QIODevice::Text)){
                    QTextStream in(&defaultdisplay);
                    currentdisplaymanager=in.readAll().section("/",3,3).remove("\n");
                    defaultdisplay.close();
                }
            } else {
                currentdisplaymanager = runCmd("ps -aux |grep  -E '/usr/.*bin/sddm|/usr/.*bin*/gdm3|.*bin*/lightdm|.*bin*/slim|.*bin*/slimski|.*bin*/xdm.*bin*/wdm.*bin*/lxdm.*bin*/nodm' |grep -v grep | awk '{print $11}'").output.section("/", 3,3);
            }
            if (verbose) qDebug() << "current display manager is " << currentdisplaymanager;
            ui->comboBoxDisplayManager->setCurrentText(currentdisplaymanager);
        }
    } else {
        ui->checkBoxDisplayManager->hide();
        ui->comboBoxDisplayManager->hide();
    }
}

void defaultlook::setuptheme()
{
    ui->buttonThemeApply->setEnabled(false);
    if (ui->buttonThemeApply->icon().isNull()) {
        ui->buttonThemeApply->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }

    ui->pushButtonPreview->setEnabled(false);
    //reset all checkboxes to unchecked

    if (isXfce || isFluxbox){
    populatethemelists(QStringLiteral("gtk-3.0"));
    populatethemelists(QStringLiteral("icons"));
    populatethemelists(QStringLiteral("cursors"));
    get_cursor_size();
    cursor_size_flag = true;
    }

    if (isXfce){
         populatethemelists(QStringLiteral("xfwm4"));
    } else if (isFluxbox) {
         populatethemelists(QStringLiteral("fluxbox"));
    }

    if (isKDE) {
        ui->label_28->setText("<b>" + tr("Plasma Widget Themes","theme style of the kde plasma widgets") + "</b>");
        ui->label_29->setText("<b>" + tr("Color Schemes", "plasma widget color schemes") + "</b>");
        ui->label->setText("<b>" + tr("Plasma Look & Feel Global Themes", "plasma global themes") + "</b>");
        populatethemelists(QStringLiteral("plasma"));
        populatethemelists(QStringLiteral("colorscheme"));
        populatethemelists(QStringLiteral("kdecursors"));
        populatethemelists(QStringLiteral("icons"));
        ui->pushButtonSettingsToThemeSet->hide();
        ui->spinBoxPointerSize->hide();
        ui->label_35->hide();
    }


//dead code
    //only enable options that make sense

    // check theme overrides

//    QString home_path = QDir::homePath();
//    QFileInfo file(home_path + "/.config/FirefoxDarkThemeOverride.check");
//    if (file.exists()) {
//        ui->checkFirefox->setChecked(true);
//    }

//    //check status of hex chat dark tweak
//    QFileInfo file_hexchat(home_path + "/.config/hexchat/hexchat.conf");
//    if (file_hexchat.exists()) {
//        //check for absolutePath()setting
//        QString code = runCmd("grep 'gui_input_style = 0' " + file_hexchat.absoluteFilePath()).output;
//        if (verbose) qDebug() << "hexchat command :" << code;
//        if (code == QLatin1String("gui_input_style = 0")) {
//            ui->checkHexchat->setChecked(true);
//        }
//    }
}

void defaultlook::get_cursor_size() {
    QString home_path = QDir::homePath();
    QString size = "0";
    QString sizecheck;

    //check .Xresources
    if (isFluxbox){
        QString file = home_path + "/.Xresources";
        if ( QFile(file).exists()){
            sizecheck = runCmd("grep Xcursor.size " + file).output.section(":",1,1).simplified();
            if ( sizecheck.isEmpty()){
                size = "0";
            } else {
                size = sizecheck;
            }
        }
        ui->spinBoxPointerSize->setValue(size.toInt());
    }

    //check Xfce
    if (isXfce){
        sizecheck = runCmd("xfconf-query --channel xsettings --property /Gtk/CursorThemeSize").output.simplified();
        if ( sizecheck.isEmpty()){
            size = "0";
        } else {
            size = sizecheck;
        }
        ui->spinBoxPointerSize->setValue(size.toInt());
    }


}

void defaultlook::set_cursor_size() {

    if (cursor_size_flag) {
        QString home_path = QDir::homePath();
        QString size = QString::number(ui->spinBoxPointerSize->value());
        QString cmd;

        //check .Xresources
        if (isFluxbox){
            QString file = home_path + "/.Xresources";
            if (runCmd("grep Xcursor.size " + file).exitCode == 0) {
                cmd = "sed -i 's/Xcursor.size:.*/Xcursor.size: " + size + "/' " + file;
            } else {
                cmd = "echo Xcursor.size: " + size + " >" + file;
            }
        }

        //check Xfce
        if (isXfce){
            cmd = ("xfconf-query --channel xsettings --property /Gtk/CursorThemeSize -t int -s " + size + " --create");
        }

        system(cmd.toUtf8());
        //restart fluxbox after set
        if (isFluxbox){
            system("xrdb -merge $HOME/.Xresources && fluxbox-remote restart");
        }
    }

}

void defaultlook::setupCompositor()
{
    //set comboboxvblank to current setting

    vblankflag = false;
    vblankinitial = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/vblank_mode")).output;
    if (verbose) qDebug() << "vblank = " << vblankinitial;
    ui->comboBoxvblank->setCurrentText(vblankinitial);

    //deal with compositors

    QString cmd = QStringLiteral("ps -aux |grep -v grep |grep -q compiz");
    if (system(cmd.toUtf8()) == 0) {
        ui->tabWidget->removeTab(Tab::Compositor);
    } else {
        ui->buttonCompositorApply->setEnabled(false);
        if (ui->buttonCompositorApply->icon().isNull()) {
            ui->buttonCompositorApply->setIcon(QIcon(":/icons/dialog-ok.svg"));
        }
        ui->buttonConfigureCompton->setEnabled(false);
        ui->buttonConfigureXfwm->setEnabled(false);
        ui->buttonEditComptonConf->setEnabled(false);

        // check to see if compton is enabled
        QString home_path = QDir::homePath();
        if (verbose) qDebug() << "Home Path =" << home_path;
        QFileInfo file_start(home_path + "/.config/autostart/zcompton.desktop");
        //check to see if compton.desktop startup file exists
        if (file_start.exists()) {
            if (verbose) qDebug() << "compton startup file exists";
        } else {
            //copy in a startup file, startup initially disabled
            runCmd("cp /usr/share/mx-tweak/zcompton.desktop " + file_start.absoluteFilePath());
        }

        //check to see if existing compton.conf file
        QFileInfo file_conf(home_path + "/.config/compton.conf");
        if (file_conf.exists()) {
            if (verbose) qDebug() << "Found existing conf file";
        } else {
            runCmd("cp /usr/share/mx-tweak/compton.conf " + file_conf.absoluteFilePath());
        }
        CheckComptonRunning();
    }
}

void defaultlook::setupConfigoptions()
{
    QString home_path = QDir::homePath();
    ui->ButtonApplyMiscDefualts->setEnabled(false);
    float versioncheck = 4.18;

    QString XfceVersion = runCmd("dpkg-query --show xfce4-session | awk '{print $2}'").output.section(".",0,1);
    if (verbose) qDebug() << "XfceVersion = " << XfceVersion.toFloat();
    if ( XfceVersion.toFloat() < versioncheck ){
        ui->label_Xfce_CSD->hide();
    }

    //set xfce values
    if (isXfce) {
        //check single click status
        QString test;
        test = runCmd(QStringLiteral("xfconf-query  -c xfce4-desktop -p /desktop-icons/single-click")).output;
        ui->checkBoxSingleClick->setChecked(test == QLatin1String("true"));

        //check single click thunar status
        thunarsingleclicksetup();

        //check split window status
        thunarsetupsplitview();
        //check systray frame status
        //frame has been removed from systray

        // show all workspaces - tasklist/show buttons feature
        plugintasklist = runCmd(QStringLiteral(R"(grep \"tasklist\" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml |cut -d '=' -f2 | cut -d '' -f1| cut -d '"' -f2)")).output;
        if (verbose) qDebug() << "tasklist is " << plugintasklist;
        if ( ! plugintasklist.isEmpty()) {
            test = runCmd("xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/include-all-workspaces").output;
            ui->checkBoxShowAllWorkspaces->setChecked(test == QLatin1String("true"));
        } else {
            ui->checkBoxShowAllWorkspaces->setEnabled(false);
        }

        // set percentages in notifications
        test = runCmd(QStringLiteral("xfconf-query -c xfce4-notifyd -p /show-text-with-gauge")).output;
        ui->checkBoxNotificatonPercentages->setChecked(test == QLatin1String("true"));

        //switch zoom_desktop

        test = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/zoom_desktop")).output;
        ui->checkBoxDesktopZoom->setChecked(test == QLatin1String("true"));

        //setup no-ellipse option
        QFileInfo fileinfo2(home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css");
        ui->checkboxNoEllipse->setChecked(fileinfo2.exists());

        //setup classic file dialog buttons
        test = runCmd(QStringLiteral("xfconf-query -c xsettings -p /Gtk/DialogsUseHeader")).output;
        ui->checkBoxFileDialogActionButtonsPosition->setChecked(test == QLatin1String("false"));

        //setup hibernate switch
        //first, hide if running live
        test = runCmd(QStringLiteral("df -T / |tail -n1 |awk '{print $2}'")).output;
        if (verbose) qDebug() << test;
        if ( test == QLatin1String("aufs") || test == QLatin1String("overlay") ) {
            ui->checkBoxHibernate->hide();
            ui->label_hibernate->hide();
        }

        //hide hibernate if there is no swap
        QString swaptest = runCmd(QStringLiteral("/usr/sbin/swapon --show")).output;
        if (verbose) qDebug() << "swaptest swap present is " << swaptest;
        if (swaptest.isEmpty()) {
            ui->checkBoxHibernate->hide();
            ui->label_hibernate->hide();
        }

        // also hide hibernate if /etc/uswsusp.conf is missing
        //only on mx-21 and up
        test = runCmd(QStringLiteral("cat /etc/mx-version")).output;
        if ( test.contains(QLatin1String("MX-19"))) {
            QFileInfo file(QStringLiteral("/etc/uswsusp.conf"));
            if (file.exists()) {
                if (verbose) qDebug() << "uswsusp.conf found";
            } else {
                ui->checkBoxHibernate->hide();
                ui->label_hibernate->hide();
            }
        }

        //and hide hibernate if swap is encrypted
        QString cmd = QStringLiteral("grep swap /etc/crypttab |grep -q luks");
        int swaptest2 = system(cmd.toUtf8());
        if (verbose) qDebug() << "swaptest encrypted is " << swaptest2;
        if (swaptest2 == 0) {
            ui->checkBoxHibernate->hide();
            ui->label_hibernate->hide();
        }

        //set checkbox
        test = runCmd(QStringLiteral("xfconf-query -c xfce4-session -p /shutdown/ShowHibernate")).output;
        if ( test == QLatin1String("true")) {
            ui->checkBoxHibernate->setChecked(true);
            hibernate_flag = true;
        } else {
            ui->checkBoxHibernate->setChecked(false);
            hibernate_flag = false;
        }

        ui->checkBoxThunarCAReset->setChecked(false);
    }
}

void defaultlook::CheckComptonRunning()
{
    //Index for combo box:  0=none, 1=xfce, 2=compton

    if ( system("ps -ax -o comm,pid |grep -w ^compton") == 0 ) {
        if (verbose) qDebug() << "Compton is running";
        ui->comboBoxCompositor->setCurrentIndex(2);
    } else {
        if (verbose) qDebug() << "Compton is NOT running";

        //check if compton is present on system, remove from choices if not
        QFileInfo compton(QStringLiteral("/usr/bin/compton"));

        //adjust for picom
        if (compton.symLinkTarget() == QLatin1String("/usr/bin/picom") ) {
            QFileInfo picom(compton.symLinkTarget());
            ui->comboBoxCompositor->setItemText(2,picom.baseName());
            ui->buttonConfigureCompton->setText(picom.baseName() + " " + tr("settings"));
        } else {
            //hide compton settings
            if ( !compton.exists() ) {
                ui->comboBoxCompositor->removeItem(2);
                ui->buttonConfigureCompton->hide();
                ui->buttonEditComptonConf->hide();
            }
        }

        //check if xfce compositor is enabled
        QString test;
        test = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/use_compositing")).output;
        if (verbose) qDebug() << "etc test is "<< test;
        if (test == QLatin1String("true")) {
            ui->comboBoxCompositor->setCurrentIndex(1);
        } else {
            ui->comboBoxCompositor->setCurrentIndex(0);
        }
    }
}

void defaultlook::CheckAptNotifierRunning() const
{
    if ( system("ps -aux |grep -v grep| grep python |grep --quiet apt-notifier") == 0 ) {
        if (verbose) qDebug() << "apt-notifier is running";
        //check if icon is supposed to be hidden by user
        if ( system("cat /home/$USER/.config/apt-notifierrc |grep --quiet DontShowIcon") == 0 ) {
            if (verbose) qDebug() << "apt-notifier set to hide icon, do not restart";
        } else {
            if (verbose) qDebug() << "unhide apt-notifier icon";
            system("/usr/bin/apt-notifier-unhide-Icon");
        }
    } else {
        if (verbose) qDebug() << "apt-notifier not running, do NOT restart";
    }
}

void defaultlook::setupscale()
{
    //setup scale for currently shown display and in the active profile
    QString xscale = QStringLiteral("1");
    QString yscale = QStringLiteral("1");
    double scale = 1;

    //get active profile
    QString activeprofile = runCmd(QStringLiteral("LANG=C xfconf-query --channel displays -p /ActiveProfile")).output;
    //get scales for display show in combobox
    int exitcode = runCmd("LANG=C xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() +"/Scale/X").exitCode;
    if ( exitcode == 0 ) {
        xscale = runCmd("LANG=C xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() +"/Scale/X").output;
    }
    exitcode = runCmd("LANG=C xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() +"/Scale/Y").exitCode;
    if ( exitcode == 0 ) {
        yscale = runCmd("LANG=C xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() + "/Scale/Y").output;
    }
    // since we want scales equal, set the scale spin box to xscale.  invert so that 2 = .5

    scale = 1 / xscale.toDouble();
    if (verbose) qDebug() << "active profile is: " << activeprofile << " xscale is " << xscale << " yscale is " << yscale << " scale is: " << scale;

    //set value
    ui->doubleSpinBoxscale->setValue(scale);

    //hide scale setup if X and Y don't match
    if ( xscale != yscale) {
        ui->doubleSpinBoxscale->hide();
        ui->label_scale->hide();
        ui->buttonApplyDisplayScaling->hide();
    }
}

void defaultlook::setscale()
{
    //get active profile and desired scale for given resolution
    double scale = 1 / ui->doubleSpinBoxscale->value();
    QString resolution = runCmd("xrandr |grep " + ui->comboBoxDisplay->currentText() + " |cut -d' ' -f3 |cut -d'+' -f1").output;
    if (verbose) qDebug() << "resolution is : " << resolution;
    QString scalestring = QString::number(scale, 'G', 5);
    QString activeprofile = runCmd(QStringLiteral("LANG=C xfconf-query --channel displays -p /ActiveProfile")).output;

    //set missing variables
    setmissingxfconfvariables(activeprofile, resolution);

    //set scale value
    QString cmd = "xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() +"/Scale/Y -t double -s " + scalestring + " --create";
    if (verbose) qDebug() << "cmd is " << cmd;
    runCmd(cmd);
    cmd = "xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() +"/Scale/X -t double -s " + scalestring + " --create";
    runCmd(cmd);
    if (verbose) qDebug() << "cmd is " << cmd;

    //set initial scale with xrandr
    QString cmd2 = "xrandr --output " + ui->comboBoxDisplay->currentText() + " --scale " + scalestring + "x" + scalestring;
    runCmd(cmd2);
}

void defaultlook::on_buttonApplyDisplayScaling_clicked()
{
    setscale();
}

void defaultlook::on_comboBoxDisplay_currentIndexChanged(int  /*index*/)
{
    setupBrightness();
    setupscale();
    setupresolutions();
    setupGamma();
}

void defaultlook::setupDisplay()
{
    //populate combobox
    QString displaydata = runCmd(QStringLiteral("LANG=C xrandr |grep -w connected | cut -d' ' -f1")).output;
    QStringList displaylist = displaydata.split(QStringLiteral("\n"));
    ui->comboBoxDisplay->clear();
    ui->comboBoxDisplay->addItems(displaylist);
    setupBrightness();
    setupGamma();
    setupscale();
    setupbacklight();
    setupresolutions();
    brightnessflag = true;

    //get gtk scaling value
    QString GTKScale = runCmd(QStringLiteral("LANG=C xfconf-query --channel xsettings -p /Gdk/WindowScalingFactor")).output;
    ui->spinBoxgtkscaling->setValue(GTKScale.toInt());
    //disable resolution stuff
}

void defaultlook::setupresolutions()
{
    QString display = ui->comboBoxDisplay->currentText();
    ui->comboBoxresolutions->clear();
    QString cmd = "LANG=C /usr/lib/mx-tweak/mx-tweak-lib-randr.sh " + display + " resolutions";
    if (verbose) qDebug() << "get resolution command is :" << cmd;
    QString resolutions = runCmd(cmd).output;
    if (verbose) qDebug() << "resolutions are :" << resolutions;
    QStringList resolutionslist = resolutions.split(QStringLiteral("\n"));
    ui->comboBoxresolutions->addItems(resolutionslist);
    //set current resolution as default
    QString resolution = runCmd("xrandr |grep " + ui->comboBoxDisplay->currentText() + " |cut -d+ -f1 |grep -oE '[^ ]+$'").output;
    if (verbose) qDebug() << "resolution is : " << resolution;
    ui->comboBoxresolutions->setCurrentText(resolution);
}

void defaultlook::setresolution()
{
    QString activeprofile = runCmd(QStringLiteral("LANG=C xfconf-query --channel displays -p /ActiveProfile")).output;
    QString display = ui->comboBoxDisplay->currentText();
    QString resolution = ui->comboBoxresolutions->currentText();
    QString cmd = "xrandr --output " + display + " --mode " + resolution;
    if (verbose) qDebug() << "resolution change command is " << cmd;
    runCmd(cmd);
    //setmissingvariables
    setmissingxfconfvariables(activeprofile, resolution);
    //set refresh rate
    setrefreshrate(display, resolution, activeprofile);
}

void defaultlook::setmissingxfconfvariables(const QString &activeprofile, const QString &resolution)
{
    //set resolution, set active, set scales, set display name

    //set display name
    runCmd("xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() + " -t string -s " + ui->comboBoxDisplay->currentText() + " --create");

    //set resolution
    runCmd("xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() + "/Resolution -t string -s " + resolution.simplified() + " --create");

    //set active profile
    runCmd("xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() + "/Active -t bool -s true --create");
}

void defaultlook::setrefreshrate(const QString &display, const QString &resolution, const QString &activeprofile) const
{
    //set refreshrate too
    QString refreshrate = runCmd("/usr/lib/mx-tweak/mx-tweak-lib-randr.sh " + display + " refreshrate").output;
    refreshrate=refreshrate.simplified();
    QStringList refreshratelist = refreshrate.split(QRegExp("\\s"));
    refreshratelist.removeAll(resolution);
    if (verbose) qDebug() << "defualt refreshreate list is :" << refreshratelist.at(0).section(QStringLiteral("*"),0,0);
    runCmd("xfconf-query --channel displays -p /" + activeprofile + "/" + display + "/RefreshRate -t double -s " + refreshratelist.at(0).section(QStringLiteral("*"),0,0) + " --create; sleep 1");
}

void defaultlook::setupbacklight()
{
    //check for backlights
    QString test = runCmd(QStringLiteral("ls /sys/class/backlight")).output;
    if ( ! test.isEmpty()) {
        //get backlight value for currently
        QString backlight=runCmd(QStringLiteral("sudo /usr/lib/mx-tweak/backlight-brightness -g")).output;
        int backlight_slider_value = backlight.toInt();
        ui->horizsliderhardwarebacklight->setValue(backlight_slider_value);
        ui->horizsliderhardwarebacklight->setToolTip(backlight);
        ui->backlight_label->setText(backlight);
        if (verbose) qDebug() << "backlight string is " << backlight;
        if (verbose) qDebug() << " backlight_slider_value is " << backlight_slider_value;
    } else {
        ui->horizsliderhardwarebacklight->hide();
        ui->backlight_label->hide();
        ui->label_xbacklight->hide();
    }
}

void defaultlook::setbacklight()
{
    QString backlight = QString::number(ui->horizsliderhardwarebacklight->value());
    QString cmd = "sudo /usr/lib/mx-tweak/backlight-brightness -s " + backlight;
    ui->backlight_label->setText(backlight);
    system(cmd.toUtf8());
}

void defaultlook::setgtkscaling()
{
    runCmd("xfconf-query --channel xsettings -p /Gdk/WindowScalingFactor -t int -s " + QString::number(ui->spinBoxgtkscaling->value()));
    runCmd(QStringLiteral("xfce4-panel --restart"));
}

void defaultlook::setupBrightness()
{
    //get brightness value for currently shown display
    QString brightness=runCmd("LANG=C xrandr --verbose | awk '/" + ui->comboBoxDisplay->currentText() +"/{flag=1;next}/Clones/{flag=0}flag'|grep Brightness|cut -d' ' -f2").output;
    int brightness_slider_value = static_cast<int>(brightness.toFloat() * 100);
    ui->horizontalSliderBrightness->setValue(brightness_slider_value);
    if (verbose) qDebug() << "brightness string is " << brightness;
    if (verbose) qDebug() << " brightness_slider_value is " << brightness_slider_value;
    ui->horizontalSliderBrightness->setToolTip(QString::number(ui->horizontalSliderBrightness->value()));
}

void defaultlook::setupGamma()
{
    QString gamma = runCmd("/usr/lib/mx-tweak/mx-tweak-lib-randr.sh " + ui->comboBoxDisplay->currentText() + " gamma").output;
    gamma=gamma.simplified();
    gamma = gamma.section(QStringLiteral(":"),1,3).simplified();
    double gamma1 = 1.0 / gamma.section(QStringLiteral(":"),0,0).toDouble();
    double gamma2 = 1.0 / gamma.section(QStringLiteral(":"),1,1).toDouble();
    double gamma3 = 1.0 / gamma.section(QStringLiteral(":"),2,2).toDouble();
    g1 = QString::number(gamma1,'G', 3);
    g2 = QString::number(gamma2,'G', 3);
    g3 = QString::number(gamma3,'G', 3);
    if (verbose) qDebug() << "gamma is " << g1 << " " << g2 << " " << g3;
}

void defaultlook::on_horizontalSliderBrightness_valueChanged(int  /*value*/)
{
    QString slider_value = QString::number(ui->horizontalSliderBrightness->value());
    ui->horizontalSliderBrightness->setToolTip(slider_value);
    ui->label_brightness_slider->setText(slider_value);
    if ( brightnessflag ) {
        setBrightness();
    }
}

void defaultlook::setBrightness()
{
    QString cmd;
    double num = ui->horizontalSliderBrightness->value() / 100.0;
    if (verbose) qDebug() << "num is :" << num;
    QString brightness = QString::number(num, 'G', 5);
    if (verbose) qDebug() << "changed brightness is :" << brightness;
    cmd = "xrandr --output " + ui->comboBoxDisplay->currentText() + " --brightness " + brightness + " --gamma " + g1 + ":" + g2 + ":" +g3;
    system(cmd.toUtf8());
}

void defaultlook::saveBrightness()
{
    //save cmd used in user's home file under .config
    //make directory when its not present
    double num = ui->horizontalSliderBrightness->value() / 100.0;
    if (verbose) qDebug() << "num is :" << num;
    QString brightness = QString::number(num, 'G', 5);
    QString home_path = QDir::homePath();
    QString config_file_path = home_path + "/.config/MX-Linux/MX-Tweak/brightness";
    if ( ! QFileInfo::exists(config_file_path)) {
        runCmd("mkdir -p " + config_file_path);
    }
    //save config in file named after the display
    runCmd("echo 'xrandr --output " + ui->comboBoxDisplay->currentText() + " --brightness " + brightness + " --gamma " + g1 + ":" + g2 + ":" + g3 + "'>" + config_file_path + "/" + ui->comboBoxDisplay->currentText());
}

void defaultlook::setupComboTheme()
{
    QStringList theme_list;
    ui->comboTheme->clear();
    if (isXfce) {
        //build theme list
        QString home_path = QDir::homePath();
        if (verbose) qDebug() << "home path is " << home_path;
        bool xsettings_gtk_theme_present = false;
        bool icontheme_present = false;
        bool xfwm4_theme_present = false;
        QStringList filter(QStringLiteral("*.tweak"));
        QDirIterator it(QStringLiteral("/usr/share/mx-tweak-data"), filter, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            xsettings_gtk_theme_present = false;
            icontheme_present = false;
            xfwm4_theme_present = false;
            QFileInfo file_info(it.next());
            QString filename = file_info.absoluteFilePath();
            QString name = runCmd("cat '" + filename + "'|grep ^Name=").output.section(QStringLiteral("="),1,1);
            QString xsettings_gtk_theme = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xsettings_gtk_theme=").output.section(QStringLiteral("="),1,1);
            if (verbose) qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
            QString xsettings_icon_theme = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xsettings_icon_theme=").output.section(QStringLiteral("="),1,1);
            if (verbose) qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
            QString xfwm4_window_decorations = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xfwm4_window_decorations=").output.section(QStringLiteral("="),1,1);
            if (verbose) qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;

            //check theme existence, only list if all 3 elements present
            QFileInfo xsettings_theme("/usr/share/themes/" + xsettings_gtk_theme);
            QFileInfo xfwm4_theme("/usr/share/themes/" + xfwm4_window_decorations);
            QFileInfo icon_theme("/usr/share/icons/" + xsettings_icon_theme);
            QFileInfo xsettings_theme_home(home_path + "/.themes/" + xsettings_gtk_theme);
            QFileInfo xfwm4_theme_home("" + home_path + "/.themes/" + xfwm4_window_decorations);
            QFileInfo icon_theme_home("" + home_path + "/.icons/" + xsettings_icon_theme);
            QFileInfo xsettings_theme_home_alt(home_path + "/.local/share/themes/" + xsettings_gtk_theme);
            QFileInfo xfwm4_theme_home_alt("" + home_path + "/.local/share/themes/" + xfwm4_window_decorations);
            if (verbose) qDebug() << "xsettings_theme_home path" << xsettings_theme_home.absoluteFilePath();

            if ( xsettings_theme.exists() || xsettings_theme_home.exists() || xsettings_theme_home_alt.exists()) {
                xsettings_gtk_theme_present = true;
                if (verbose) qDebug() << "xsettings_gtk_theme_present" << xsettings_gtk_theme_present;
            }

            if ( xfwm4_theme.exists() || xfwm4_theme_home.exists() || xfwm4_theme_home_alt.exists()) {
                xfwm4_theme_present = true;
            }

            if ( icon_theme.exists() || icon_theme_home.exists() || xfwm4_theme_home_alt.exists()) {
                icontheme_present = true;
            }

            if ( xsettings_gtk_theme_present && xfwm4_theme_present && icontheme_present ) {
                if (verbose) qDebug() << "filename is " << filename;
                if (verbose) qDebug()<< "theme combo name" << name;
                theme_list << name;
                theme_info.insert(name,filename);
                if (verbose) qDebug() << "theme info hash value is" << name << " " << theme_info[name];
            }
        }
        theme_list.sort();
        theme_list.insert(0, tr("Choose a theme set"));

        //add user entries in ~/.local/share/mx-tweak-data

        QDirIterator it2(home_path + "/.local/share/mx-tweak-data", filter, QDir::Files, QDirIterator::Subdirectories);
        while (it2.hasNext()) {
            xsettings_gtk_theme_present = false;
            icontheme_present = false;
            xfwm4_theme_present = false;
            QString home_path = QDir::homePath();
            QFileInfo file_info(it2.next());
            QString filename = file_info.absoluteFilePath();
            QString name = runCmd("cat '" + filename + "'|grep ^Name=").output.section(QStringLiteral("="),1,1);

            QString xsettings_gtk_theme = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xsettings_gtk_theme=").output.section(QStringLiteral("="),1,1);
            if (verbose) qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
            QString xsettings_icon_theme = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xsettings_icon_theme=").output.section(QStringLiteral("="),1,1);
            if (verbose) qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
            QString xfwm4_window_decorations = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xfwm4_window_decorations=").output.section(QStringLiteral("="),1,1);
            if (verbose) qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;

            //check theme existence, only list if all 3 elements present
            QFileInfo xsettings_theme("/usr/share/themes/" + xsettings_gtk_theme);
            QFileInfo xfwm4_theme("/usr/share/themes/" + xfwm4_window_decorations);
            QFileInfo icon_theme("/usr/share/icons/" + xsettings_icon_theme);
            QFileInfo xsettings_theme_home(home_path + "/.themes/" + xsettings_gtk_theme);
            QFileInfo xfwm4_theme_home("" + home_path + "/.themes/" + xfwm4_window_decorations);
            QFileInfo icon_theme_home("" + home_path + "/.icons/" + xsettings_icon_theme);
            QFileInfo xsettings_theme_home_alt(home_path + "/.local/share/themes/" + xsettings_gtk_theme);
            QFileInfo xfwm4_theme_home_alt("" + home_path + "/.local/share/themes/" + xfwm4_window_decorations);
            QFileInfo icon_theme_home_alt("" + home_path + "/.local/share/icons/" + xsettings_icon_theme);
            if (verbose) qDebug() << "xsettings_theme_home path" << xsettings_theme_home.absoluteFilePath();

            if (xsettings_theme.exists() || xsettings_theme_home.exists() || xsettings_theme_home_alt.exists()) {
                xsettings_gtk_theme_present = true;
            }

            if (xfwm4_theme.exists() || xfwm4_theme_home.exists() || xfwm4_theme_home_alt.exists()) {
                xfwm4_theme_present = true;
            }

            if (icon_theme.exists() || icon_theme_home.exists() || icon_theme_home_alt.exists()) {
                icontheme_present = true;
            }

            if (xsettings_gtk_theme_present && xfwm4_theme_present && icontheme_present) {
                if (verbose) qDebug() << "filename is " << filename;
                if (verbose) qDebug()<< "theme combo name" << name;
                theme_list << name;
                theme_info.insert(name,filename);
                if (verbose) qDebug() << "theme info hash value is" << name << " " << theme_info[name];
            }
        }
    }
    QString current;
    if (isKDE){
        QString themes = runCmd("plasma-apply-lookandfeel --list").output;
        themes.append("\n");
        theme_list = themes.split(QStringLiteral("\n"));
        current = runCmd("grep LookAndFeelPackage $HOME/.config/kdeglobals").output.section("=",1,1);
        if (verbose) qDebug() << "current is " << current;
    }

    ui->comboTheme->addItems(theme_list);
    if (current.isEmpty()){
            ui->comboTheme->setCurrentIndex(0);
          } else {
            ui->comboTheme->setCurrentText(current);
    }
}

void defaultlook::on_comboTheme_activated(const QString & /*arg1*/)
{
    if (isXfce) {
        if (ui->comboTheme->currentIndex() != 0) {
            ui->buttonThemeApply->setEnabled(true);
            ui->pushButtonPreview->setEnabled(true);
        }
    } else if (isKDE) {
        ui->buttonThemeApply->setEnabled(true);
    }
}

void defaultlook::on_buttonThemeApply_clicked()
{
    themeflag = false;
    if (isXfce){
        savethemeundo();
        ui->buttonThemeApply->setEnabled(false);
        ui->buttonThemeUndo->setEnabled(true);
        QString themename = theme_info.value(ui->comboTheme->currentText());
        QFileInfo fileinfo(themename);
        //initialize variables
        QString backgroundColor = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep background-rgba=").output.section(QStringLiteral("=") , 1,1);
        if (verbose) qDebug() << "backgroundColor = " << backgroundColor;
        QString color1 = backgroundColor.section(QStringLiteral(","),0,0);
        QString color2 = backgroundColor.section(QStringLiteral(","), 1, 1);
        QString color3 = backgroundColor.section(QStringLiteral(","),2,2);
        QString color4 = backgroundColor.section(QStringLiteral(","),3,3);
        if (verbose) qDebug() << "sep colors" << color1 << color2 << color3 << color4;
        QString background_image = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep background-image=").output.section(QStringLiteral("="),1,1);
        if (verbose) qDebug() << "backgroundImage = " << background_image;
        QString background_style = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep background-style=").output.section(QStringLiteral("="),1,1);
        if (verbose) qDebug() << "backgroundstyle = " << background_style;
        QString xsettings_gtk_theme = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep xsettings_gtk_theme=").output.section(QStringLiteral("="),1,1);
        if (verbose) qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
        QString xsettings_icon_theme = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep xsettings_icon_theme=").output.section(QStringLiteral("="),1,1);
        if (verbose) qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
        QString xfwm4_window_decorations = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep xfwm4_window_decorations=").output.section(QStringLiteral("="),1,1);
        if (verbose) qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;
        QString cursorthemename = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep CursorThemeName=").output.section(QStringLiteral("="),1,1);
        if (verbose) qDebug() << "CursorThemeName = " << cursorthemename;
        //  use xfconf system to change values

        message_flag = true;

        //set gtk theme
        runCmd("xfconf-query -c xsettings -p /Net/ThemeName -s " + xsettings_gtk_theme);
        runCmd(QStringLiteral("sleep .5"));
        runCmd("gsettings set org.gnome.desktop.interface gtk-theme \"" + xsettings_gtk_theme + "\"");
        if (xsettings_gtk_theme.toLower().contains("dark")){
            runCmd("gsettings set org.gnome.desktop.interface color-scheme prefer-dark");
        } else {
            runCmd("gsettings set org.gnome.desktop.interface color-scheme default");
        }

        //set window decorations theme
        runCmd("xfconf-query -c xfwm4 -p /general/theme -s " + xfwm4_window_decorations);
        runCmd(QStringLiteral("sleep .5"));

        //set icon theme
        runCmd("xfconf-query -c xsettings -p /Net/IconThemeName -s " + xsettings_icon_theme);
        runCmd(QStringLiteral("sleep .5"));

        //set cursor theme if exists
        if ( ! cursorthemename.isEmpty()){
            runCmd("xfconf-query -c xsettings -p /Gtk/CursorThemeName -s " + cursorthemename);
        }

        //deal with panel customizations for each panel

        QStringListIterator changeIterator(panelIDs);
        while (changeIterator.hasNext()) {
            QString value = changeIterator.next();

            //set panel background mode

            if (background_style == QLatin1String("1") || background_style == QLatin1String("2") || background_style == QLatin1String("0")) {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style -t int -s " + background_style + " --create");
            } else {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style -t int -s 0 --create");
            }

            //set panel background image

            QFileInfo image(background_image);

            if (image.exists()) {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image -t string -s " + background_image + " --create");
            } else {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image --reset");
            }

            //set panel color

            if (backgroundColor != QLatin1String("")) {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-rgba -t double -t double -t double -t double -s " + color1 + " -s " + color2 + " -s " + color3 + " -s " + color4 + " --create");
            }
        }

        //set whisker themeing
        QString home_path = QDir::homePath();
        QFileInfo whisker_check(home_path + "/.config/gtk-3.0/gtk.css");
        if (whisker_check.exists()) {
            if (verbose) qDebug() << "existing gtk.css found";
            QString cmd = "cat " + home_path + "/.config/gtk-3.0/gtk.css |grep -q whisker-tweak.css";
            if (system(cmd.toUtf8()) == 0 ) {
                if (verbose) qDebug() << "include statement found";
            } else {
                if (verbose) qDebug() << "adding include statement";
                QString cmd = "echo '@import url(\"whisker-tweak.css\");' >> " + home_path + "/.config/gtk-3.0/gtk.css";
                system(cmd.toUtf8());
            }
        } else {
            if (verbose) qDebug() << "creating simple gtk.css file";
            QString cmd = "echo '@import url(\"whisker-tweak.css\");' >> " + home_path + "/.config/gtk-3.0/gtk.css";
            system(cmd.toUtf8());
        }

        //add whisker info
        runCmd("awk '/<begin_gtk_whisker_theme_code>/{flag=1;next}/<end_gtk_whisker_theme_code>/{flag=0}flag' \"" +fileinfo.absoluteFilePath() +"\" > " + home_path + "/.config/gtk-3.0/whisker-tweak.css");

        //restart xfce4-panel

        system("xfce4-panel --restart");
    }

    if (isKDE){
        runCmd("plasma-apply-lookandfeel --apply " + ui->comboTheme->currentText());
    }
    setuptheme();
}

void defaultlook::on_ButtonApplyEtc_clicked()
{
    QString intel_option;
    QString amd_option;
    QString radeon_option;
    QString lightdm_option;
    QString bluetooth_option;
    QString recommends_option;
    QString debian_kernel_updates_option;
    QString liq_kernel_updates_option;
    QString DESKTOP = runCmd(QStringLiteral("echo $XDG_SESSION_DESKTOP")).output;
    QString home_path = QDir::homePath();
    ui->ButtonApplyEtc->setEnabled(false);

    intel_option.clear();
    lightdm_option.clear();
    bluetooth_option.clear();
    recommends_option.clear();

    //deal with udisks option
    QFileInfo fileinfo(QStringLiteral("/etc/tweak-udisks.chk"));
    int sudooverride = runCmd(QStringLiteral("pkexec /usr/lib/mx-tweak/mx-tweak-rootcheck.sh")).exitCode;
    QString cmd;
    QString udisks_option;
    QString sudo_override_option;
    QString user_name_space_override_option;
    udisks_option.clear();

    if (verbose) qDebug() << "applyetc DESKTOP is " << DESKTOP;
    if (verbose) qDebug() << "home path applyetc is " << home_path + "/.config/MX-Linux/nocsd/" + DESKTOP;
    if (ui->checkBoxCSD->isChecked()) {
        if ( QFileInfo::exists(home_path + "/.config/MX-Linux/nocsd/" + DESKTOP)) {
            runCmd("rm " + home_path + "/.config/MX-Linux/nocsd/" + DESKTOP);
        }
    } else {
        int test = runCmd("mkdir -p " + home_path + "/.config/MX-Linux/nocsd/").exitCode;
        if ( test != 0 ) {
            if (verbose) qDebug() << "could not make directory";
        }
        test = runCmd("touch " + home_path + "/.config/MX-Linux/nocsd/" + DESKTOP ).exitCode;
        if ( test != 0 ) {
            if (verbose) qDebug() << "could not write nocsd desktop file";
        }
    }
    //fluxbox menu autogeneration
    if ( QFile("/usr/bin/mxfb-menu-generator").exists()){
        if (! ui->checkBoxDisableFluxboxMenuGeneration->isChecked()){
            runCmd("echo '#this file is used to disable automatic updating of the All Apps menu' > " + home_path + "/.fluxbox/mxfb-menu-generator-disabled.chk");
        } else {
            runCmd("rm " + home_path + "/.fluxbox/mxfb-menu-generator-disabled.chk");
        }
    }

    //internal drive mounting for non root users
    if (ui->checkBoxMountInternalDrivesNonRoot->isChecked()) {
        if (fileinfo.exists()) {
            if (verbose) qDebug() << "no change to internal drive mount settings";
        } else {
            udisks_option = QStringLiteral("enable_user_mount");
        }
    } else {
        if (fileinfo.exists()) {
            udisks_option = QStringLiteral("disable_user_mount");
        } else {
            if (verbose) qDebug() << "no change to internal drive mount settings";
        }
    }

    //reset lightdm greeter config
    if (ui->checkBoxLightdmReset->isChecked()) {
        lightdm_option = QStringLiteral("lightdm_reset");
    }

    //graphics driver overrides

    if ( Intel_flag ) {
        QFileInfo check_intel(QStringLiteral("/etc/X11/xorg.conf.d/20-intel.conf"));
        if ( check_intel.exists()) {
            //backup existing 20-intel.conf file to home folder
            cmd = QStringLiteral("cp /etc/X11/xorg.conf.d/20-intel.conf /home/$USER/20-intel.conf.$(date +%Y%m%H%M%S)");
            system(cmd.toUtf8());
        }
        if (ui->checkboxIntelDriver->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            intel_option = QStringLiteral("enable_intel");
        } else {
            //remove 20-intel.conf
            intel_option = QStringLiteral("disable_intel");
        }
    }

    if ( amdgpuflag ) {
        QFileInfo check_amd(QStringLiteral("/etc/X11/xorg.conf.d/20-amd.conf"));
        if ( check_amd.exists()) {
            //backup existing 20-amd.conf file to home folder
            cmd = QStringLiteral("cp /etc/X11/xorg.conf.d/20-amd.conf /home/$USER/20-amd.conf.$(date +%Y%m%H%M%S)");
            system(cmd.toUtf8());
        }
        if (ui->checkboxAMDtearfree->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            amd_option = QStringLiteral("enable_amd");
        } else {
            //remove 20-amd.conf
            amd_option = QStringLiteral("disable_amd");
        }
    }

    if ( radeon_flag ) {
        QFileInfo check_radeon(QStringLiteral("/etc/X11/xorg.conf.d/20-radeon.conf"));
        if ( check_radeon.exists()) {
            //backup existing 20-radeon.conf file to home folder
            cmd = QStringLiteral("cp /etc/X11/xorg.conf.d/20-radeon.conf /home/$USER/20-radeon.conf.$(date +%Y%m%H%M%S)");
            system(cmd.toUtf8());
        }
        if (ui->checkboxRadeontearfree->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            radeon_option = QStringLiteral("enable_radeon");
        } else {
            //remove 20-radeon.conf
            radeon_option = QStringLiteral("disable_radeon");
        }
    }

    //bluetooth auto enable
    if (bluetoothautoenableflag) {
        if (ui->checkBoxbluetoothAutoEnable->isChecked()) {
            bluetooth_option = QStringLiteral("enable_bluetooth");
            //blueman
            if (QFile::exists("/usr/bin/blueman")) {
                runCmd(QStringLiteral("gsettings set org.blueman.plugins.powermanager auto-power-on true"));
            }
            //kde bluedevil
            if (QFile::exists("/usr/bin/kwriteconfig5")) {
                runCmd(QStringLiteral("kwriteconfig5 --file kded5rc --group Module-bluedevil --key autoload true"));
            }
        } else {
            bluetooth_option = QStringLiteral("disable_bluetooth");
            //blueman
            if (QFile::exists("/usr/bin/blueman")) {
                runCmd(QStringLiteral("gsettings set org.blueman.plugins.powermanager auto-power-on false"));
            }
            //kde bluedevil
            if (QFile::exists("/usr/bin/kwriteconfig5")) {
                runCmd(QStringLiteral("kwriteconfig5 --file kded5rc --group Module-bluedevil --key autoload false"));
            }

        }
    }

    //bluetooth battery info
    if (bluetoothbatteryflag){
        if (ui->checkBoxBluetoothBattery->isChecked()){
            runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh bluetooth_battery true");
        } else {
            runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh bluetooth_battery false");
        }
    }

    //install recommends option
    if ( enable_recommendsflag ){
        if ( ui->checkBoxInstallRecommends->isChecked()){
            recommends_option = "install_recommends";
        } else
            recommends_option = "noinstall_recommends";
    }

    //deal with sudo override

    if (ui->radioSudoUser->isChecked()) {
        if (sudooverride == 0) {
            sudo_override_option = QStringLiteral("enable_sudo_override");
        } else {
            if (verbose) qDebug() << "no change to admin password settings";
        }
    } else {
        if (sudooverride == 0) {
            if (verbose) qDebug() << "no change to admin password settings";
        } else {
            sudo_override_option = QStringLiteral("disable_sudo_override");
        }
    }

    //deal with user namespace override
    if (sandboxflag) {
        if (ui->checkBoxSandbox->isChecked()) {
            user_name_space_override_option = QStringLiteral("enable_sandbox");
        } else {
            user_name_space_override_option = QStringLiteral("disable_sandbox");
        }
    }

    //debian kernel updates
    if (debianKernelUpdateFlag){
        if ( ui->checkBoxDebianKernelUpdates->isChecked()){
            debian_kernel_updates_option = "unhold_debian_kernel_updates";
            qDebug() << "debian update option" << debian_kernel_updates_option;
        } else {
            debian_kernel_updates_option = "hold_debian_kernel_updates";
        }
    }
    //liquorix kernel updates
    if (liqKernelUpdateFlag){
        if ( ui->checkBoxDebianKernelUpdates->isChecked()){
            liq_kernel_updates_option = "unhold_liquorix_kernel_updates";
        } else {
            liq_kernel_updates_option = "hold_liquorix_kernel_updates";
        }
    }

    //hostname setting
    //if name doesn't validate, don't make any changes to any options, and don't reset gui.
    if (ui->checkBoxComputerName->isChecked()){
        if (validatecomputername(ui->lineEditHostname->text())){
            changecomputername(ui->lineEditHostname->text());
        } else {
            return;
        }
    }
    // display manager change
    if (ui->checkBoxDisplayManager->isChecked()){
        //don't do anything if selection is still default
        if (ui->comboBoxDisplayManager->currentText() != currentdisplaymanager){
            changedisplaymanager(ui->comboBoxDisplayManager->currentText());
        }
        ui->checkBoxDisplayManager->setChecked(false);
    }

    //kvm_early_switch
    if (kvmflag){
        if (ui->checkBoxKVMVirtLoad->isChecked()){
           kvm_early_switch("on", kvmconffile);
        } else {
            kvm_early_switch("off", kvmconffile);
        }
    }

    //checkbox options
    if ( ! udisks_option.isEmpty() || ! sudo_override_option.isEmpty() || ! user_name_space_override_option.isEmpty() || ! intel_option.isEmpty() || ! lightdm_option.isEmpty() || ! amd_option.isEmpty() || ! radeon_option.isEmpty() || !bluetooth_option.isEmpty() || !recommends_option.isEmpty() || !debian_kernel_updates_option.isEmpty() || !liq_kernel_updates_option.isEmpty()){
        runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh " + udisks_option + " " + sudo_override_option + " " + user_name_space_override_option + " " + intel_option + " " + amd_option + " " + radeon_option + " " + bluetooth_option + " " + recommends_option + " " + lightdm_option + " " + debian_kernel_updates_option + " " + liq_kernel_updates_option);

    }
    //reset gui
    setupEtc();
}

void defaultlook::changecomputername(QString hostname){
    runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh hostname " + hostname);
}

void defaultlook::kvm_early_switch(QString action, QString file){
    if (verbose) qDebug() << "kvm flag is " << kvmflag << "action is " << action << " file is " << file;
    runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh kvm_early_switch " + action + " " + file);
}

void defaultlook::changedisplaymanager(QString dm){
    runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh displaymanager " + dm);
}

bool defaultlook::validatecomputername(QString hostname){
    // see if name is reasonable
    if (hostname.isEmpty()) {
        QMessageBox::critical(this, this->windowTitle(), tr("Please enter a computer name.", "question to enter a name for the computer hostname"));
        return false;
    } else if (hostname.contains(QRegularExpression("[^0-9a-zA-Z-.]|^[.-]|[.-]$|\\.\\."))) {
        QMessageBox::critical(this, this->windowTitle(),
            tr("Sorry, your computer name contains invalid characters.\nYou'll have to select a different\nname before proceeding.", "unacceptable characters are found in hostname, pick a new name"));
        return false;
    }
    return true;
}

void defaultlook::on_checkBoxSingleClick_clicked()
{
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}

void defaultlook::on_checkBoxThunarSingleClick_clicked()
{
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}


void defaultlook::on_checkboxNoEllipse_clicked()
{
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}

void defaultlook::savethemeundo()
{
    QString home_path = QDir::homePath();
    QFile::remove(QStringLiteral("undo.txt"));
    QString undovalue;
    QStringListIterator changeIterator(panelIDs);
    QString undocommand;
    QString whiskeriterator = QString::number(undotheme.count());
    if (verbose) qDebug() << "whisker interator is " << whiskeriterator;

    while (changeIterator.hasNext()) {
        QString value = changeIterator.next();

        //backup panel background mode
        if (runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style").exitCode == 0) {
            undovalue = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style").output;
            //runCmd("echo xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style -s " + undovalue + " >> undo.txt");
            undocommand = undocommand + "xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style -s "+ undovalue + " ; ";
        } else {
            //runCmd("echo xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style -r >> undo.txt");
            undocommand = undocommand + "xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style -r ; ";
        }

        //backup panel background image

        if (runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image").exitCode == 0) {
            undovalue = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image").output;
            //runCmd("echo xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image -s " + undovalue + " >> undo.txt");
            undocommand = undocommand + "xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image -s " + undovalue +" ; ";
        } else {
            //runCmd("echo xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image -r >> undo.txt");
            undocommand = undocommand + " xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image -r ; ";
        }

        //backup panel color

        if (runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color").exitCode == 0) {
            undovalue = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color |cut -d ':' -f2").output.trimmed();
            undovalue.replace(QLatin1String("\n"), QLatin1String(","));
            if (verbose) qDebug() << "backup color test" << undovalue;
            QString color1 = undovalue.section(QStringLiteral(","),0,0);
            QString color2 = undovalue.section(QStringLiteral(","), 1, 1);
            QString color3 = undovalue.section(QStringLiteral(","),2,2);
            QString color4 = undovalue.section(QStringLiteral(","),3,3);
            if (verbose) qDebug() << "sep colors" << color1 << color2 << color3 << color4;
            //runCmd("echo xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color -s " + color1 + " -s " + color2 + " -s " + color3 + " -s " + color4 + " >> undo.txt");
            undocommand = undocommand + "xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color -s " + color1 + " -s " + color2 + " -s " + color3 + " -s " + color4 + " ; ";
        } else {
            //runCmd("echo xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color -r >> undo.txt");
            undocommand = undocommand + "xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color -r ; ";
        }
    }

    //backup theme settings for xfwm4 and xsettings

    // gtk theme
    if (runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/ThemeName")).exitCode == 0) {
        undovalue = runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/ThemeName")).output;
        //runCmd("echo xfconf-query -c xsettings -p /Net/ThemeName -s " + undovalue + " >> undo.txt");
        undocommand = undocommand + "xfconf-query -c xsettings -p /Net/ThemeName -s " + undovalue + " ; ";
    } else {
        //runCmd("echo xfconf-query -c xsettings -p /Net/ThemeName -r >> undo.txt");
        undocommand = undocommand + "xfconf-query -c xsettings -p /Net/ThemeName -r ; ";
    }

    // xfwm4 theme
    if (runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/theme")).exitCode == 0) {
        undovalue = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/theme")).output;
        //runCmd("echo xfconf-query -c xfwm4 -p /general/theme -s " + undovalue + " >> undo.txt");
        undocommand = undocommand + "xfconf-query -c xfwm4 -p /general/theme -s " + undovalue + " ; ";
    } else {
        runCmd(QStringLiteral("echo xfconf-query -c xfwm4 -p /general/theme -r >> undo.txt"));
        undocommand = undocommand + "xfconf-query -c xfwm4 -p /general/theme -r ; ";
    }

    // icon theme
    if (runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/IconThemeName")).exitCode == 0) {
        undovalue = runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/IconThemeName")).output;
        //runCmd("echo xfconf-query -c xsettings -p /Net/IconThemeName -s " + undovalue + " >> undo.txt");
        undocommand = undocommand + "xfconf-query -c xsettings -p /Net/IconThemeName -s " + undovalue + " ; ";
    } else {
        //runCmd("echo xfconf-query -c xsettings -p /Net/IconThemeName -r >> undo.txt");
        undocommand = undocommand + "xfconf-query -c xsettings -p /Net/IconThemeName -r ; ";
    }

    //backup whiskermenu changes
    // if whisker-tweak.css exists, back it up
    QFileInfo fileinfo(home_path + "/.config/gtk-3.0/whisker-tweak.css");
    if (fileinfo.exists()) {
        runCmd("cp " + home_path + "/.config/gtk-3.0/whisker-tweak.css /tmp/whisker-tweak.css.undo." + whiskeriterator);
        undocommand = undocommand + "cp /tmp/whisker-tweak.css.undo." + whiskeriterator + " " + fileinfo.absoluteFilePath();
    } else {
        // delete the file
        //runCmd("echo rm -f " + fileinfo.absoluteFilePath() + " >> undo.txt" );
        undocommand = undocommand + "rm -f " + fileinfo.absoluteFilePath();
    }
    if (verbose) qDebug() << "undo command is " << undocommand;

    undotheme << undocommand;

    if (verbose) qDebug () << "undo command list is " << undotheme;

}

void defaultlook::themeundo()
{
    QString cmd = undotheme.constLast();
    system(cmd.toUtf8());
    undotheme.removeLast();
}

void defaultlook::on_buttonThemeUndo_clicked()
{
    themeundo();
    runCmd(QStringLiteral("xfce4-panel --restart"));
    if (undotheme.isEmpty()) {
        ui->buttonThemeUndo->setEnabled(false);
    }
}



void defaultlook::on_buttonConfigureCompton_clicked()
{
    this->hide();
    system("compton-conf");
    this->show();
}

void defaultlook::on_buttonCompositorApply_clicked()
{
    //disable apply button
    ui->buttonCompositorApply->setEnabled(false);

    if (ui->comboBoxCompositor->currentIndex() == 2) {
        //turn off xfce compositor
        runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/use_compositing -s false"));
        //launch compton
        system("pkill -x compton");
        system("compton-launch.sh");
        //restart apt-notifier if necessary
        CheckAptNotifierRunning();
    }
    if (ui->comboBoxCompositor->currentIndex() == 1) {
        //turn off compton
        system("pkill -x compton");
        //launch xfce compositor
        runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/use_compositing -s true"));
        //restart apt-notifier if necessary
        CheckAptNotifierRunning();
    }
    if (ui->comboBoxCompositor->currentIndex() == 0) {
        //turn off compton and xfce compositor
        //turn off xfce compositor
        runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/use_compositing -s false"));
        system("pkill -x compton");
        CheckAptNotifierRunning();

    }

    //figure out whether to autostart compton or not
    //if compton is configured in the combo box, then enable.  otherwise disable

    QString home_path = QDir::homePath();
    QFileInfo file_start(home_path + "/.config/autostart/zcompton.desktop");
    if (ui->comboBoxCompositor->currentIndex() == 2) {
        runCmd("sed -i -r s/Hidden=.*/Hidden=false/ " + file_start.absoluteFilePath());
    } else {
        runCmd("sed -i -r s/Hidden=.*/Hidden=true/ " + file_start.absoluteFilePath());
    }
    if (verbose) qDebug() << "autostart set to " << runCmd("grep Hidden= " + file_start.absoluteFilePath()).output;

    //deal with vblank setting
    if ( vblankflag ) {
        runCmd("xfconf-query -c xfwm4 -p /general/vblank_mode -t string -s " + ui->comboBoxvblank->currentText() + " --create");
        //restart xfwm4 to take advantage of the setting
        runCmd(QStringLiteral("xfwm4 --replace"));
    }
}


void defaultlook::on_buttonEditComptonConf_clicked()
{
    QString home_path = QDir::homePath();
    QFileInfo file_conf(home_path + "/.config/compton.conf");
    runCmd("xdg-open " + file_conf.absoluteFilePath());
}

void defaultlook::on_comboBoxCompositor_currentIndexChanged(const QString & /*arg1*/)
{
    if (ui->comboBoxCompositor->currentIndex() == 0) {
        ui->buttonCompositorApply->setEnabled(true);
        ui->buttonConfigureCompton->setEnabled(false);
        ui->buttonConfigureXfwm->setEnabled(false);
        ui->buttonEditComptonConf->setEnabled(false);
    }

    if (ui->comboBoxCompositor->currentIndex() == 1) {
        ui->buttonCompositorApply->setEnabled(true);
        ui->buttonConfigureCompton->setEnabled(false);
        ui->buttonConfigureXfwm->setEnabled(true);
        ui->buttonEditComptonConf->setEnabled(false);
    }

    if (ui->comboBoxCompositor->currentIndex() == 2) {
        ui->buttonCompositorApply->setEnabled(true);
        ui->buttonConfigureCompton->setEnabled(true);
        ui->buttonConfigureXfwm->setEnabled(false);
        ui->buttonEditComptonConf->setEnabled(true);
    }
}

void defaultlook::on_buttonConfigureXfwm_clicked()
{
    xfwm_compositor_settings fred;
    fred.setModal(true);
    fred.exec();
}

void defaultlook::on_checkBoxShowAllWorkspaces_clicked()
{
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}

void defaultlook::on_checkBoxMountInternalDrivesNonRoot_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_pushButtonPreview_clicked()
{
    QString themename = theme_info.value(ui->comboTheme->currentText());
    QFileInfo fileinfo(themename);

    //initialize variables
    QString file_name = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep screenshot=").output.section(QStringLiteral("=") , 1,1);
    QString path = fileinfo.absolutePath();
    QString full_file_path = path + "/" + file_name;

    QMessageBox preview_box(QMessageBox::NoIcon, file_name, QLatin1String("") , QMessageBox::Close, this);
    preview_box.setIconPixmap(QPixmap(full_file_path));
    preview_box.exec();
}

void defaultlook::on_checkBoxHibernate_clicked()
{
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}

void defaultlook::on_radioSudoUser_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}
void defaultlook::on_radioSudoRoot_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_ButtonApplyMiscDefualts_clicked()
{
    QString hibernate_option;
    hibernate_option.clear();

    if (ui->checkBoxShowAllWorkspaces->isChecked()) {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/include-all-workspaces -s true");
    } else {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/include-all-workspaces -s false");
    }

    if (ui->checkBoxSingleClick->isChecked()) {
        runCmd(QStringLiteral("xfconf-query  -c xfce4-desktop -p /desktop-icons/single-click -s true"));
    } else {
        runCmd(QStringLiteral("xfconf-query  -c xfce4-desktop -p /desktop-icons/single-click -s false"));
    }

    //thunar single click
    if (ui->checkBoxThunarSingleClick->isChecked()){
        thunarsetsingleclick(true);
    } else {
        thunarsetsingleclick(false);
    }

    //split view thunar
    if (ui->checkBoxThunarSplitView->isChecked() ){
        thunarsplitview(true);
    } else {
        thunarsplitview(false);
    }

    //split view thunar horizontal or vertical
    if (ui->checkBoxsplitviewhorizontal->isChecked()){
        thunarsplitviewhorizontal(true);
    } else {
        thunarsplitviewhorizontal(false);
    }
    //systray frame removed

    //notification percentages
    if (ui->checkBoxNotificatonPercentages->isChecked()){
        runCmd("xfconf-query -c xfce4-notifyd -p /show-text-with-gauge -t bool -s true --create");
    } else {
        runCmd("xfconf-query -c xfce4-notifyd -p /show-text-with-gauge --reset");
    }

    //set desktop zoom
    if (ui->checkBoxDesktopZoom->isChecked()) {
        runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/zoom_desktop -s true"));
    } else {
        runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/zoom_desktop -s false"));
    }

    if (ui->checkBoxThunarCAReset->isChecked()) {
        resetthunar();
    }

    //deal with gtk dialog button settings
    if (ui->checkBoxFileDialogActionButtonsPosition->isChecked()){
        runCmd("xfconf-query -c xsettings -p /Gtk/DialogsUseHeader -s false");
    } else {
        runCmd("xfconf-query -c xsettings -p /Gtk/DialogsUseHeader -s true");
    }

    //deal with no-ellipse-filenames option
    QString home_path = QDir::homePath();
    if (ui->checkboxNoEllipse->isChecked()) {
        //set desktop themeing
        QFileInfo gtk_check(home_path + "/.config/gtk-3.0/gtk.css");
        if (gtk_check.exists()) {
            if (verbose) qDebug() << "existing gtk.css found";
            QString cmd = "cat " + home_path + "/.config/gtk-3.0/gtk.css |grep -q no-ellipse-desktop-filenames.css";
            if (system(cmd.toUtf8()) == 0 ) {
                if (verbose) qDebug() << "include statement found";
            } else {
                if (verbose) qDebug() << "adding include statement";
                QString cmd = "echo '@import url(\"no-ellipse-desktop-filenames.css\");' >> " + home_path + "/.config/gtk-3.0/gtk.css";
                system(cmd.toUtf8());
            }
        } else {
            if (verbose) qDebug() << "creating simple gtk.css file";
            QString cmd = "echo '@import url(\"no-ellipse-desktop-filenames.css\");' >> " + home_path + "/.config/gtk-3.0/gtk.css";
            system(cmd.toUtf8());
        }
        //add modification config
        runCmd("cp /usr/share/mx-tweak/no-ellipse-desktop-filenames.css " + home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css ");

        //restart xfdesktop by with xfdesktop --quite && xfdesktop &

        system("xfdesktop --quit && sleep .5 && xfdesktop &");
    } else {
        QFileInfo noellipse_check(home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css");
        if (noellipse_check.exists()) {
            runCmd("rm -f " + home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css");
            runCmd("sed -i '/no-ellipse-desktop-filenames.css/d' " + home_path + "/.config/gtk-3.0/gtk.css");
            system("xfdesktop --quit && sleep .5 && xfdesktop &");
        }
    }

    //deal with hibernate
    if (ui->checkBoxHibernate->isChecked() != hibernate_flag) {
        if (ui->checkBoxHibernate->isChecked()) {
            hibernate_option =  QStringLiteral("hibernate");
            system("xfconf-query -c xfce4-session -p /shutdown/ShowHibernate -t bool -s true --create");
        } else {
            system("xfconf-query -c xfce4-session -p /shutdown/ShowHibernate -t bool -s false --create");
        }
    }

    //only do this part on MX-19.  MX-21 and later do not have uswsusp
    if ( !hibernate_option.isEmpty() ) {
        QString test = runCmd(QStringLiteral("cat /etc/mx-version")).output;
        if ( test.contains(QLatin1String("MX-19"))) {
            runCmd("x-terminal-emulator -e 'pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh " + hibernate_option + "'");
        }
    }

    setupConfigoptions();
}

void defaultlook::on_checkBoxLightdmReset_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_checkBoxDesktopZoom_clicked()
{
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}

void defaultlook::on_checkBoxThunarCAReset_clicked()
{
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}

void defaultlook::on_checkboxIntelDriver_clicked()
{
    //toggle flag for action.  this way, if box was checked initially, the action won't take place again.
    Intel_flag = true;
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_checkboxAMDtearfree_clicked()
{
    //toggle flag for action.  this way, if box was checked initially, the action won't take place again.
    amdgpuflag = true;
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_checkboxRadeontearfree_clicked()
{
    //toggle flag for action.  this way, if box was checked initially, the action won't take place again.
    radeon_flag = true;
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_pushButtontasklist_clicked()
{
    window_buttons fred;
    fred.setModal(true);
    fred.exec();
}

// Get version of the program
QString defaultlook::getVersion(const QString &name)
{
    return runCmd("dpkg-query -f '${Version}' -W " + name).output;
}

void defaultlook::on_pushButtonSettingsToThemeSet_clicked()
{
    if (isFluxbox) {
        if (QFile("/usr/bin/mxfb-look").exists()){
            this->hide();
            system("/usr/bin/mxfb-look");
            setuptheme();
            this->show();
            return;
        }
    }
    QString fileName;
    auto *dialog = new theming_to_tweak;
    int userInput = dialog->exec();
    if (userInput == QDialog::Rejected)
        return;
    fileName = dialog->nameEditor()->text();

    //declared locally to prevent an issues with other code
    auto pathAppend = [](const QString& path1, const QString& path2) {
        return QDir::cleanPath(path1 + QDir::separator() + path2);
    };

    QString panel;
    QString data = runCmd(QStringLiteral("xfconf-query -c xfce4-panel -p /panels --list")).output;
    int panelNum = 0;
    for (panelNum = 1;; panelNum++) {
        if (data.contains("panel-" + QString::number(panelNum)))
            break;
    }
    panel = "panel-" + QString::number(panelNum);

    int backgroundStyle = 0;
    data = runCmd("xfconf-query -c xfce4-panel -p /panels/" + panel + "/background-style").output;
    backgroundStyle = data.toInt(); //there may be newlines in output but qt ignores it

    QVector<double> backgroundColor;
    QString backgroundImage;
    backgroundColor.reserve(4);
    if (backgroundStyle == 1) {
        QStringList lines = runCmd("LANG=C xfconf-query -c xfce4-panel -p /panels/" + panel + "/background-rgba").output.split('\n');
        lines.removeAt(0);
        lines.removeAt(0);
        for (int i = 0; i < 4; i++)
        {
            backgroundColor << lines.at(i).toDouble();
        }
    } else if (backgroundStyle == 2) {
        backgroundImage = runCmd("xfconf-query -c /panels/" + panel + "/background-image").output;
    }

    QString iconThemeName = runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/IconThemeName")).output;
    QString themeName = runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/ThemeName")).output;
    QString windowDecorationsTheme = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/theme")).output;
    QString cursorthemename = runCmd(QStringLiteral("xfconf-query -c xsettings -p /Gtk/CursorThemeName")).output;

    QString whiskerThemeFileName = pathAppend(QDir::homePath(), QStringLiteral(".config/gtk-3.0/whisker-tweak.css"));
    QFile whiskerThemeFile(whiskerThemeFileName);
    if (!whiskerThemeFile.open(QFile::ReadOnly | QFile::Text)) {
        if (verbose) qDebug() << "Failed to fetch whisker theming from: " + whiskerThemeFileName;
    }
    QTextStream whiskerThemeFileStream(&whiskerThemeFile);
    QString whiskerThemeData = whiskerThemeFileStream.readAll();
    whiskerThemeFile.close();

    QStringList fileLines;
    fileLines << "Name=" + fileName;
    fileLines << "background-style=" + QString::number(backgroundStyle);
    if (backgroundStyle == 1) {
        QString line;
        for (double num : qAsConst(backgroundColor)) {
            line.append(QString::number(num) + ',');
        }
        if (line.endsWith(',')) line.chop(1);
        fileLines << "background-rgba=" + line;
    } else {
        fileLines << QStringLiteral("background-rgba=none");
    }
    if (backgroundStyle == 2) {
        fileLines << "background-image=" + backgroundImage;
    } else {
        fileLines << QStringLiteral("background-image=none");
    }
    fileLines << "xsettings_gtk_theme=" + themeName;
    fileLines << "xsettings_icon_theme=" + iconThemeName;
    fileLines << "xfwm4_window_decorations=" + windowDecorationsTheme;
    fileLines << "CursorThemeName=" + cursorthemename;
    fileLines << QStringLiteral("<begin_gtk_whisker_theme_code>");

    for (const QString &line : whiskerThemeData.split('\n')) {
        fileLines << line;
    }
    fileLines << QStringLiteral("<end_gtk_whisker_theme_code>");
    QFile file(pathAppend(QDir::homePath(), ".local/share/mx-tweak-data/" + fileName + ".tweak"));
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        if (verbose) qDebug() << "Failed to open file for reading: " + fileName;
        return;
    }
    QTextStream fileStream(&file);
    for (const QString &line : qAsConst(fileLines)) {
        fileStream << line + '\n';
    }
    file.close();
    //Refresh
    setupComboTheme();
}

void defaultlook::on_pushButtonRemoveUserThemeSet_clicked()
{
    remove_user_theme_set dialog;
    int result = dialog.exec();
    if (result != QDialog::Accepted)
        return;
    QString theme = dialog.themeSelector()->currentText();
    if (theme == QLatin1String("Select User Theme Set to Remove"))
        return;
    result = QMessageBox::warning(this, QStringLiteral("Remove User Theme Set"), "Are you sure you want to remove " + theme + " theme set?", QMessageBox::Ok | QMessageBox::Cancel);
    if (result != QMessageBox::Ok)
        return;
    QString file = dialog.getFilename(theme);
    file.replace(' ', QLatin1String("\\ "));
    auto cmd = runCmd("rm " + file);
    if (cmd.exitCode != 0) {
        if (verbose) qDebug() << "Removing theme set failed: exitCode: " << cmd.exitCode << " | output: " << cmd.output;
        return;
    }
    //refresh
    setupComboTheme();
}

void defaultlook::on_comboBoxvblank_activated(const QString & /*arg1*/)
{
    vblankflag = vblankinitial != ui->comboBoxvblank->currentText();
    ui->buttonCompositorApply->setEnabled(true);
}

void defaultlook::on_buttonSaveBrightness_clicked()
{
    saveBrightness();
}

void defaultlook::on_buttonGTKscaling_clicked()
{
    setgtkscaling();
}

void defaultlook::on_horizsliderhardwarebacklight_actionTriggered(int  /*action*/)
{
    setbacklight();
}

void defaultlook::on_buttonapplyresolution_clicked()
{
    setresolution();
}

void defaultlook::on_checkBoxSandbox_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
    sandboxflag = true;
}

void defaultlook::on_ApplyFluxboxResets_clicked()
{
    // toggle icons
    // flux captions - 0=on 1=off 2=On Hover
    enum FluxCaptions {On, Off, OnHover};

    if (fluxcaptionflag) {
        switch(ui->comboBoxfluxcaptions->currentIndex()) {
        case FluxCaptions::On:
            runCmd(QStringLiteral("/usr/bin/idesktoggle caption on"));
            runCmd(QStringLiteral("/usr/bin/idesktoggle CaptionOnHover off"));
            break;
        case FluxCaptions::Off:
            runCmd(QStringLiteral("/usr/bin/idesktoggle caption off"));
            break;
        case FluxCaptions::OnHover:
            runCmd(QStringLiteral("/usr/bin/idesktoggle caption on"));
            runCmd(QStringLiteral("/usr/bin/idesktoggle CaptionOnHover On"));
            break;
        }
    }

    // flux icons 0=on 1=off
    if (fluxiconflag) {
        switch(ui->comboBoxfluxIcons->currentIndex()) {
        case 0:
            runCmd(QStringLiteral("/usr/bin/idesktoggle Icon on"));
            break;
        case 1:
            runCmd(QStringLiteral("/usr/bin/idesktoggle Icon off"));
            break;
        }
    }

    //setup slit autohide
    //get slit line
    QString initline;
    QString value;
    if (ui->checkboxfluxSlitautohide->isChecked()) {
        value = QStringLiteral("true");
    } else {
        value = QStringLiteral("false");
    }
    initline = runCmd(QStringLiteral("grep screen0.slit.autoHide  $HOME/.fluxbox/init")).output;
    fluxboxchangeinitvariable(initline,value);

    //set slit placement
    //setup toolbar location
    value = ui->combofluxslitlocation->currentText();
    initline = runCmd(QStringLiteral("grep screen0.slit.placement  $HOME/.fluxbox/init")).output;
    fluxboxchangeinitvariable(initline,value);
    fluxboxchangedock();

    //setup toolbar autohide
    if (ui->checkboxfluxtoolbarautohide->isChecked()) {
        value = QStringLiteral("true");
    } else {
        value = QStringLiteral("false");
    }
    initline = runCmd(QStringLiteral("grep screen0.toolbar.autoHide  $HOME/.fluxbox/init")).output;
    fluxboxchangeinitvariable(initline,value);

    //setup toolbar visible
    if (ui->checkBoxFluxShowToolbar->isChecked()) {
        value = QStringLiteral("true");
    } else {
        value = QStringLiteral("false");
    }
    initline = runCmd(QStringLiteral("grep session.screen0.toolbar.visible  $HOME/.fluxbox/init")).output;
    fluxboxchangeinitvariable(initline,value);

    //setup toolbar location
    value = ui->combofluxtoolbarlocatoin->currentText();
    initline = runCmd(QStringLiteral("grep screen0.toolbar.placement  $HOME/.fluxbox/init")).output;
    fluxboxchangeinitvariable(initline,value);

    //setup toolbar widthpercent
    value = QString::number(ui->spinBoxFluxToolbarWidth->value(), 'G', 5);
    initline = runCmd(QStringLiteral("grep screen0.toolbar.widthPercent  $HOME/.fluxbox/init")).output;
    fluxboxchangeinitvariable(initline,value);

    //setup toolbar height
    value = QString::number(ui->spinBoxFluxToolbarHeight->value(), 'G', 5);
    initline = runCmd(QStringLiteral("grep screen0.toolbar.height  $HOME/.fluxbox/init")).output;
    fluxboxchangeinitvariable(initline,value);

    //Reset Everything
    if (ui->checkboxfluxreseteverything->isChecked()) {
        ui->checkboxfluxresetdock->setChecked(false);
        ui->checkboxfluxresetmenu->setChecked(false);
        ui->checkBoxMenuMigrate->setChecked(false);
        runCmd(QStringLiteral("/usr/bin/mxflux_install.sh"));
        runCmd(QStringLiteral("pkill wmalauncher"));
        runCmd(QStringLiteral("$HOME/.fluxbox/scripts/DefaultDock.mxdk"));
    }

    //Reset Menu
    if (ui->checkboxfluxresetmenu->isChecked() && !ui->checkboxfluxreseteverything->isChecked()) {
        //determine menu in use
        QString menumx = runCmd(QStringLiteral("grep session.menuFile $HOME/.fluxbox/init")).output.section(QStringLiteral(":"),1,1).trimmed();
        if (verbose) qDebug() << "menu mx is " << menumx;
        //backup menu
        runCmd("cp " + menumx + " " + menumx + ".$(date +%Y%m%d%H%M%S)");
        //copy menu-mx from /etc/skel/.fluxbox
        runCmd(QStringLiteral("cp /etc/skel/.fluxbox/menu-mx $HOME/.fluxbox"));
        //run localize-fluxbox-menu to generate new menu
        runCmd(QStringLiteral("localize_fluxbox_menu-mx"));
    }

    //Migrate Menu
    if (ui->checkBoxMenuMigrate->isChecked() && !ui->checkboxfluxreseteverything->isChecked()) {
        //run menu-migrate script
        runCmd(QStringLiteral("/usr/bin/menu-migrate"));
    }

    //Reset Dock
    if (ui->checkboxfluxresetdock->isChecked() && !ui->checkboxfluxreseteverything->isChecked()) {
        //copy backup dock and copy one from usr/share/mxflux/.fluxbox/scripts
        runCmd(QStringLiteral("cp $HOME/.fluxbox/scripts/DefaultDock.mxdk $HOME/.fluxbox/scripts/DefaultDock.mxdk.$(date +%Y%m%d%H%M%S)"));
        runCmd(QStringLiteral("cp /etc/skel/.fluxbox/scripts/DefaultDock.mxdk $HOME/.fluxbox/scripts/DefaultDock.mxdk"));
        runCmd(QStringLiteral("pkill wmalauncher"));
        runCmd(QStringLiteral("$HOME/.fluxbox/scripts/DefaultDock.mxdk"));
    }

    //screenblanking
    if (screenblankflag){
        //comment default line if it exists
        runCmd(QStringLiteral("sed -i 's/^[[:blank:]]*xset[[:blank:]].*dpms.*/#&/' $HOME/.fluxbox/startup"));
        //add new values to fluxbox startup menu if don't exist
        QString test = runCmd("grep '$HOME/.config/MX-Linux/screenblanking-mxtweak' $HOME/.fluxbox/startup").output;
        if (test.isEmpty()){ 
            //add comment and new config file
            runCmd(QStringLiteral("sed -i '/^exec.*/i#screenblanking added by mx-tweak' $HOME/.fluxbox/startup"));
            runCmd(QStringLiteral("sed -i '/^exec.*/i$HOME\\/.config\\/MX-Linux\\/screenblanking-mxtweak &' $HOME/.fluxbox/startup"));
            //blank space before exec
            runCmd(QStringLiteral("sed -i '/^exec.*/i\\\\' $HOME/.fluxbox/startup"));
        }
        //set new value
        int value = ui->spinBoxScreenBlankingTimeout->value() * 60;
        runCmd(QStringLiteral("echo \\#\\!/bin/bash >$HOME/.config/MX-Linux/screenblanking-mxtweak"));
        QString cmd = "xset dpms " + QString::number(value) + " " + QString::number(value) + " " + QString::number(value);
        runCmd(cmd);
        runCmd("echo " + cmd + " >>$HOME/.config/MX-Linux/screenblanking-mxtweak");
        cmd = "xset s " + QString::number(value);
        runCmd(cmd);
        runCmd("echo " + cmd + " >>$HOME/.config/MX-Linux/screenblanking-mxtweak");
        //make sure script is executable
        runCmd("chmod a+x $HOME/.config/MX-Linux/screenblanking-mxtweak");
    }

    //thunar actions
    //only if thunar installed
    if ( QFile("/usr/bin/thunar").exists()){
        //reset thunar custom actions
        if (ui->checkBoxThunarCAReset->isChecked()) {
            resetthunar();
        }
        //thunar single click
        if (ui->checkBoxThunarSingleClick_2->isChecked()){
            thunarsetsingleclick(true);
        } else {
            thunarsetsingleclick(false);
        }

        //split view thunar
        if (ui->checkBoxThunarSplitView_2->isChecked() ){
            thunarsplitview(true);
        } else {
            thunarsplitview(false);
        }

        //split view thunar horizontal or vertical
        if (ui->checkBoxsplitviewhorizontal_2->isChecked()){
            thunarsplitviewhorizontal(true);
        } else {
            thunarsplitviewhorizontal(false);
        }
    }
    //when all done, restart fluxbox
    ui->checkboxfluxresetdock->setChecked(false);
    ui->checkboxfluxresetmenu->setChecked(false);
    ui->checkboxfluxreseteverything->setChecked(false);
    ui->checkBoxMenuMigrate->setChecked(false);
    runCmd(QStringLiteral("sleep 2; /usr/bin/fluxbox-remote restart"));
    setupFluxbox();
}

void defaultlook::fluxboxchangeinitvariable(const QString &initline, const QString &value) const
{
    if (verbose) qDebug() << "checking for init value changes";
    QString initialvalue = initline.section(QStringLiteral(":"),1,1).trimmed();
    if ( initialvalue != value) {
        QString cmd = "sed -i 's/^" + initline +"/" + initline.section(QStringLiteral(":"),0,0).trimmed() + ":    " + value + "/' $HOME/.fluxbox/init";
        if (verbose) qDebug() << "init change command " << cmd;
        runCmd(cmd);
    }
}

void defaultlook::fluxboxchangedock() const
{
    if (verbose) qDebug() << "comment slit changes in mxdk files";

    if (slitflag) {
        runCmd(QStringLiteral("sed -i 's/^fluxbox-remote/#&/' $HOME/.fluxbox/scripts/*.mxdk"));
        runCmd(QStringLiteral("sed -i 's/^sed/#&/' $HOME/.fluxbox/scripts/*.mxdk"));
    }
}

void defaultlook::on_checkboxfluxresetdock_clicked()
{
    ui->ApplyFluxboxResets->setEnabled(true);
}

void defaultlook::on_checkBoxMenuMigrate_clicked()
{
    ui->ApplyFluxboxResets->setEnabled(true);
    ui->checkboxfluxresetmenu->setChecked(false);
}


void defaultlook::on_checkboxfluxresetmenu_clicked()
{
    ui->ApplyFluxboxResets->setEnabled(true);
    ui->checkBoxMenuMigrate->setChecked(false);
}

void defaultlook::on_checkboxfluxreseteverything_clicked()
{
    ui->ApplyFluxboxResets->setEnabled(true);
    ui->checkboxfluxresetdock->setChecked(false);
    ui->checkboxfluxresetmenu->setChecked(false);
    ui->checkBoxMenuMigrate->setChecked(false);
}

void defaultlook::on_combofluxtoolbarlocatoin_currentIndexChanged(int  /*index*/)
{
    ui->ApplyFluxboxResets->setEnabled(true);
}

void defaultlook::on_checkboxfluxtoolbarautohide_clicked()
{
    ui->ApplyFluxboxResets->setEnabled(true);
}

void defaultlook::on_spinBoxFluxToolbarWidth_valueChanged(int  /*arg1*/)
{
    ui->ApplyFluxboxResets->setEnabled(true);
}

void defaultlook::on_spinBoxFluxToolbarHeight_valueChanged(int  /*arg1*/)
{
    ui->ApplyFluxboxResets->setEnabled(true);
}

void defaultlook::on_combofluxslitlocation_currentIndexChanged(int  /*index*/)
{
    ui->ApplyFluxboxResets->setEnabled(true);
    slitflag = true;
}

void defaultlook::on_checkboxfluxSlitautohide_clicked()
{
    ui->ApplyFluxboxResets->setEnabled(true);
}


void defaultlook::on_comboBoxfluxIcons_currentIndexChanged(int  /*index*/)
{
    ui->ApplyFluxboxResets->setEnabled(true);
    fluxiconflag = true;
}

void defaultlook::on_comboBoxfluxcaptions_currentIndexChanged(int  /*index*/)
{
    ui->ApplyFluxboxResets->setEnabled(true);
    fluxcaptionflag = true;
}

void defaultlook::on_comboPlasmaPanelLocation_currentIndexChanged(int  /*index*/)
{
    ui->ButtonApplyPlasma->setEnabled(true);
    plasmaplacementflag = true;
}

void defaultlook::on_checkBoxPlasmaSingleClick_clicked()
{
    ui->ButtonApplyPlasma->setEnabled(true);
    plasmasingleclickflag = true;
}

void defaultlook::on_checkBoxPlasmaShowAllWorkspaces_clicked()
{
    ui->ButtonApplyPlasma->setEnabled(true);
    plasmaworkspacesflag = true;
}

void defaultlook::on_checkboxplasmaresetdock_clicked()
{
    ui->ButtonApplyPlasma->setEnabled(true);
    plasmaresetflag = true;
}

void defaultlook::on_ButtonApplyPlasma_clicked()
{
    QString home_path = QDir::homePath();
    if (plasmaresetflag) {
        plasmaplacementflag = false;
        plasmasingleclickflag = false;
        plasmaworkspacesflag = false;
        //reset plasma script Adrian
        runCmd(QStringLiteral("/usr/lib/mx-tweak/reset-kde-mx"));
    }
    if (plasmaplacementflag) {
        switch(ui->comboPlasmaPanelLocation->currentIndex()) {
        case PanelIndex::Bottom:
            writePlasmaPanelConfig(QStringLiteral("location"), QString::number(PanelLocation::Bottom));
            writePlasmaPanelConfig(QStringLiteral("formfactor"), QStringLiteral("2"));
            break;
        case PanelIndex::Left:
            writePlasmaPanelConfig(QStringLiteral("location"), QString::number(PanelLocation::Left));
            writePlasmaPanelConfig(QStringLiteral("formfactor"), QStringLiteral("3"));
            break;
        case PanelIndex::Top:
            writePlasmaPanelConfig(QStringLiteral("location"), QString::number(PanelLocation::Top));
            writePlasmaPanelConfig(QStringLiteral("formfactor"), QStringLiteral("2"));
            break;
        case PanelIndex::Right:
            writePlasmaPanelConfig(QStringLiteral("location"), QString::number(PanelLocation::Right));
            writePlasmaPanelConfig(QStringLiteral("formfactor"), QStringLiteral("3"));
            break;
        }
    }

    if (plasmasingleclickflag) {
        QString value = ui->checkBoxPlasmaSingleClick->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
        runCmd("kwriteconfig5 --group KDE --key SingleClick " + value);
        runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-kde-edit.sh " + value);
    }

    if (plasmaworkspacesflag) {
        QString value = ui->checkBoxPlasmaShowAllWorkspaces->isChecked() ? QStringLiteral("false") : QStringLiteral("true");
        writeTaskmanagerConfig(QStringLiteral("showOnlyCurrentDesktop"), value);
    }

    //plasma-discover autostart
    if (plasmadisoverautostartflag){
        QString plasmadiscoverautostart = home_path + "/.config/autostart/org.kde.discover.notifier.desktop";
        qDebug() << "discover autostart path is " << plasmadiscoverautostart;
        if (ui->checkBoxPlasmaDiscoverUpdater->isChecked()){
            //delete any Hidden=true lines to make sure its processed by xdg autostart
            runCmd("sed -i /Hidden=true/d " + plasmadiscoverautostart);
        } else {
            //copy if it doesn't exist already
            if (!QFile(plasmadiscoverautostart).exists()){
                if (QFile("/etc/xdg/autostart/org.kde.discover.notifier.desktop").exists()){
                    runCmd("cp /etc/xdg/autostart/org.kde.discover.notifier.desktop " + plasmadiscoverautostart);
                }
            }
            //remove any previous Hidden= attribute, then add Hidden=true to make it not autostart
            runCmd("sed -i /Hidden=*/d " + plasmadiscoverautostart);
            runCmd("echo Hidden=true >> " + plasmadiscoverautostart); //this also creates file if the /etc/xdg version is missing
        }
    }

    //time to reset kwin and plasmashell
    if (plasmaworkspacesflag || plasmasingleclickflag || plasmaplacementflag || plasmaresetflag || plasmasystrayiconsizeflag) {
        //restart kwin first
        runCmd(QStringLiteral("sleep 1; qdbus org.kde.KWin /KWin reconfigure"));
        //then plasma
        QString cmd =QStringLiteral("sleep 1; plasmashell --replace &");
        system(cmd.toUtf8());
    }

    setupPlasma();

}

void defaultlook::writePlasmaPanelConfig(const QString &key, const QString &value) const
{
    QString ID;
    ID = plasmaPanelId.section(QStringLiteral("["),2,2).section(QStringLiteral("]"),0,0);
    runCmd("kwriteconfig5 --file plasma-org.kde.plasma.desktop-appletsrc --group Containments --group " + ID + " --key " + key + " " + value);
}

void defaultlook::writeTaskmanagerConfig(const QString &key, const QString &value) const
{
    QString ID = plasmataskmanagerID.section(QStringLiteral("["),2,2).section(QStringLiteral("]"),0,0);
    QString Applet = plasmataskmanagerID.section(QStringLiteral("["),4,4).section(QStringLiteral("]"),0,0);
    runCmd("kwriteconfig5 --file plasma-org.kde.plasma.desktop-appletsrc --group Containments --group " + ID + " --group Applets --group " + Applet + " --key " + key + " " + value);
}

void defaultlook::on_comboBoxPlasmaSystrayIcons_currentIndexChanged(int  /*index*/)
{
    ui->ButtonApplyPlasma->setEnabled(true);
    plasmasystrayiconsizeflag = true;
}

void defaultlook::populatethemelists(const QString &value)
{
    themeflag = false;
    QString home_path = QDir::homePath();
    QString themes;
    QStringList themelist;
    QString current;

    if (value == QLatin1String("plasma")){
        themes = runCmd("LANG=C plasma-apply-desktoptheme --list-themes |grep \"*\" |cut -d\"*\" -f2").output;
        themes.append("\n");
    }
    if (value == QLatin1String("colorscheme")){
        themes = runCmd("LANG=C plasma-apply-colorscheme --list-schemes |grep \"*\" |cut -d\"*\" -f2 ").output;
        themes.append("\n");
    }
    if (value == QLatin1String("kdecursors")){
        themes = runCmd("LANG=C plasma-apply-cursortheme --list-themes | grep \"*\"").output;
        themes.append("\n");
    }
    if ( value == QLatin1String("gtk-3.0") || value == QLatin1String("xfwm4")) {
        themes = runCmd("find /usr/share/themes/*/" + value + " -maxdepth 0 2>/dev/null|cut -d\"/\" -f5").output;
        themes.append("\n");
        themes.append(runCmd("find $HOME/.themes/*/" + value + " -maxdepth 0 2>/dev/null|cut -d\"/\" -f5").output);
        themes.append("\n");
        themes.append(runCmd("find $HOME/.local/share/themes/*/" + value + " -maxdepth 0 2>/dev/null|cut -d\"/\" -f7").output);
    }

    if (value == QLatin1String("icons")){
        themes = runCmd(QStringLiteral("find /usr/share/icons/*/index.theme -maxdepth 1 2>/dev/null|cut -d\"/\" -f5")).output;
        themes.append("\n");
        themes.append(runCmd(QStringLiteral("find $HOME/.icons/*/index.theme -maxdepth 1 2>/dev/null|cut -d\"/\" -f5")).output);
        themes.append("\n");
        themes.append(runCmd(QStringLiteral("find $HOME/.local/share/icons/*/index.theme -maxdepth 1 2>/dev/null|cut -d\"/\" -f7")).output);
    }

    if ( value == QLatin1String("fluxbox")) {
        themes = runCmd("find /usr/share/fluxbox/styles/ -maxdepth 1 2>/dev/null |cut -d\"/\" -f6").output;
        themes.append("\n");
        themes.append(runCmd("find $HOME/.fluxbox/styles/ -maxdepth 1 2>/dev/null |cut -d\"/\" -f6").output);
        themes.append("\n");
    }

    if ( value == QLatin1String("cursors")){
        themes = runCmd(QStringLiteral("find /usr/share/icons/*/ -maxdepth 1 2>/dev/null |grep cursors |cut -d\"/\" -f5")).output;
        themes.append("\n");
        themes.append(runCmd(QStringLiteral("find $HOME/.icons/*/ -maxdepth 1 2>/dev/null |grep cursors |cut -d\"/\" -f5")).output);
        themes.append("\n");
        themes.append(runCmd(QStringLiteral("find $HOME/.local/share/icons/*/ -maxdepth 1 2>/dev/null |grep cursors |cut -d\"/\" -f7")).output);
        themes.append("\n");
        themes.append("default");
    }



    themelist = themes.split(QStringLiteral("\n"));
    themelist.removeDuplicates();
    themelist.removeAll(QLatin1String(""));
    themelist.sort(Qt::CaseInsensitive);

    if ( value == QLatin1String("plasma")){
        ui->listWidgetTheme->clear();
        QRegExp regex(".*current.*", Qt::CaseInsensitive);
        int index = themelist.indexOf(regex);
        themelist[index] = themelist[index].section("(",0,0);
        //index of theme in list
        if (verbose) qDebug() << "index is " << index << themelist[index];
        ui->listWidgetTheme->addItems(themelist);
        ui->listWidgetTheme->setCurrentRow(index);
    }
    if ( value == QLatin1String("colorscheme")){
        ui->listWidgetWMtheme->clear();
        QRegExp regex(".*current.*", Qt::CaseInsensitive);
        int index = themelist.indexOf(regex);
        themelist[index] = themelist[index].section("(",0,0);
        //index of theme in list
        if (verbose) qDebug() << "index is " << index << themelist[index];
        ui->listWidgetWMtheme->addItems(themelist);
        ui->listWidgetWMtheme->setCurrentRow(index);
    }
    if ( value == QLatin1String("kdecursors")){
        ui->listWidgetCursorThemes->clear();
        QRegExp regex(".*current.*", Qt::CaseInsensitive);
        int index = themelist.indexOf(regex);
        for (int i = 0; i < themelist.size(); ++i ){
            themelist[i] = themelist[i].section("[",1,1).section("]",0,0);
        }
        //index of theme in list
        if (verbose) qDebug() << "index is " << index << themelist[index];
        ui->listWidgetCursorThemes->addItems(themelist);
        ui->listWidgetCursorThemes->setCurrentRow(index);
    }

    if ( value == QLatin1String("gtk-3.0") ) {
        ui->listWidgetTheme->clear();
        ui->listWidgetTheme->addItems(themelist);
        //set current
        if (isXfce){
            current = runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/ThemeName")).output;
        } else if (isFluxbox){
            current = runCmd(QStringLiteral("grep gtk-theme-name ~/.config/gtk-3.0/settings.ini | cut -d\"=\" -f2")).output;
        }
        //index of theme in list
        ui->listWidgetTheme->setCurrentRow(themelist.indexOf(current));
    }
    if ( value == QLatin1String("xfwm4")) {

        ui->listWidgetWMtheme->clear();
        ui->listWidgetWMtheme->addItems(themelist);
        current = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/theme")).output;
        ui->listWidgetWMtheme->setCurrentRow(themelist.indexOf(current));
    }

    if ( value == QLatin1String("fluxbox")) {

        ui->listWidgetWMtheme->clear();
        ui->listWidgetWMtheme->addItems(themelist);
        current = runCmd(QStringLiteral("grep styleFile $HOME/.fluxbox/init |grep -v ^# | cut -d\"/\" -f6")).output;
        ui->listWidgetWMtheme->setCurrentRow(themelist.indexOf(current));
    }

    if ( value == QLatin1String("cursors")){
        ui->listWidgetCursorThemes->clear();
        ui->listWidgetCursorThemes->addItems(themelist);
        if (isXfce){
            current = runCmd(QStringLiteral("xfconf-query -c xsettings -p /Gtk/CursorThemeName")).output;
            if (current.isEmpty()) current = "default";
        } else if (isFluxbox){
            if (QFile(home_path + "/.icons/default/index.theme").exists()) {
                current = runCmd("grep Inherits $HOME/.icons/default/index.theme |cut -d= -f2").output;
            } else {
                  current = "default";
            }
        }
        ui->listWidgetCursorThemes->setCurrentRow(themelist.indexOf(current));
    }

    if ( value == QLatin1String("icons")) {
        if (verbose) qDebug() << "themelist" << themelist;
        QStringList iconthemelist = themelist;
        for (const QString &item : iconthemelist) {
            const QString& icontheme = item;
            if (verbose) qDebug() << "icontheme" << icontheme;
            QString test = runCmd("find /usr/share/icons/" + icontheme + " -maxdepth 1 -mindepth 1 -type d |cut -d\"/\" -f6").output;
            if ( test == QLatin1String("cursors") ) {
                themelist.removeAll(icontheme);
            }
        }
        themelist.removeAll(QStringLiteral("default.kde4"));
        themelist.removeAll(QStringLiteral("default"));
        themelist.removeAll(QStringLiteral("hicolor"));
        ui->listWidgeticons->clear();
        ui->listWidgeticons->addItems(themelist);
        //current icon set
        if (isXfce){
            current = runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/IconThemeName")).output;
        } else if (isFluxbox){
            current = runCmd(QStringLiteral("grep gtk-icon-theme-name $HOME/.config/gtk-3.0/settings.ini |grep -v ^# | cut -d\"=\" -f2")).output;
        }
        if (isKDE) {
            current = QIcon::themeName();
        }
        ui->listWidgeticons->setCurrentRow(themelist.indexOf(current));
    }

    themeflag = true;
}

void defaultlook::settheme(const QString &type, const QString &theme, const QString &desktop)
{   //set new theme
    QString cmd;
    QString cmd1;
    QString cmd2;

    if ( desktop == "XFCE" ) {
        if ( type == QLatin1String("gtk-3.0") ) {
            cmd = "xfconf-query -c xsettings -p /Net/ThemeName -s \"" + theme + "\"";
            cmd1 ="gsettings set org.gnome.desktop.interface gtk-theme \"" + theme + "\"";
            if (theme.toLower().contains("dark") || theme.contains("Blackbird")){ //blackbird special case
                cmd2="gsettings set org.gnome.desktop.interface color-scheme prefer-dark";
            } else {
                cmd2="gsettings set org.gnome.desktop.interface color-scheme default";
            }
        }
        if ( type == QLatin1String("xfwm4") ) {
            cmd = "xfconf-query -c xfwm4 -p /general/theme -s \"" + theme + "\"";
        }

        if ( type == QLatin1String("icons") ) {
            cmd = "xfconf-query -c xsettings -p /Net/IconThemeName -s \"" + theme + "\"";
        }

        if (type == QLatin1String("cursor")) {
            cmd = "xfconf-query -c xsettings -p /Gtk/CursorThemeName -s \"" + theme + "\"";
        }
        system(cmd.toUtf8());

    } else if ( desktop == "KDE" ){
        if ( type == QLatin1String("plasma") ) {
            cmd = "LANG=C plasma-apply-desktoptheme " + theme;
        }
        if ( type == QLatin1String("colorscheme") ) {
            cmd = "LANG=C plasma-apply-colorscheme " + theme;
        }

        if ( type == QLatin1String("icons") ) {
            cmd = "LANG=C /usr/lib/x86_64-linux-gnu/libexec/plasma-changeicons " + theme;
        }

        if (type == QLatin1String("kdecursor")) {
            cmd = "LANG=C plasma-apply-cursortheme " + theme;
        }
        system(cmd.toUtf8());

    } else if ( desktop == "fluxbox" ){
        QString home_path = QDir::homePath();
        if ( type == QLatin1String("gtk-3.0") ) {
            if (runCmd("grep gtk-theme-name $HOME/.config/gtk-3.0/settings.ini").exitCode == 0) {
                cmd = "sed -i 's/gtk-theme-name=.*/gtk-theme-name=" + theme + "/' $HOME/.config/gtk-3.0/settings.ini";
            } else {
                cmd = "echo gtk-theme-name=" + theme + "\" >> $HOME/.config/gtk-3.0/settings.ini";
            }
            system(cmd.toUtf8());

            if (runCmd("grep gtk-theme-name $HOME/.gtkrc-2.0").exitCode == 0) {
                cmd = "sed -i 's/gtk-theme-name=.*/gtk-theme-name=\"" + theme + "\"/' $HOME/.gtkrc-2.0";
            } else {
                cmd = "echo gtk-theme-name=\"" + theme + "\" >> $HOME/.gtkrc-2.0";
            }
            system(cmd.toUtf8());

            cmd1 ="gsettings set org.gnome.desktop.interface gtk-theme \"" + theme + "\"";
            if (theme.toLower().contains("dark") || theme.contains("Blackbird")){ //blackbird special case
                cmd2="gsettings set org.gnome.desktop.interface color-scheme prefer-dark";
                if (runCmd("grep gtk-application-prefer-dark-theme $HOME/.config/gtk-3.0/settings.ini").exitCode == 0) {
                    runCmd("sed -i 's/gtk-application-prefer-dark-theme=.*/gtk-application-prefer-dark-theme=true/' $HOME/.config/gtk-3.0/settings.ini");
                } else {
                    runCmd("echo gtk-application-prefer-dark-theme=true/' >> $HOME/.config/gtk-3.0/settings.ini");
                }
            } else {
                cmd2="gsettings set org.gnome.desktop.interface color-scheme default";
                if (runCmd("grep gtk-application-prefer-dark-theme $HOME/.config/gtk-3.0/settings.ini").exitCode == 0) {
                    runCmd("sed -i 's/gtk-application-prefer-dark-theme=.*/gtk-application-prefer-dark-theme=false/' $HOME/.config/gtk-3.0/settings.ini");
                } else {
                    runCmd("echo gtk-application-prefer-dark-theme=false/' >> $HOME/.config/gtk-3.0/settings.ini");
                }
            }

            if ( QFile("/usr/bin/preview-mx").exists()){
                cmd = "preview-mx &";
                system(cmd.toUtf8());
            }
        }
        if ( type == QLatin1String("fluxbox") ) {
            QString filepath = home_path + "/.fluxbox/styles/" + theme;
            if (QFile(filepath).exists()){
                home_path.replace("/", "\\/");
                cmd = "sed -i 's/session.styleFile:.*/session.styleFile: " + home_path + "\\/.fluxbox\\/styles\\/" + theme + "/' $HOME/.fluxbox/init && fluxbox-remote reconfigure && fluxbox-remote reloadstyle";
            } else {
                cmd = "sed -i 's/session.styleFile:.*/session.styleFile: \\/usr\\/share\\/fluxbox\\/styles\\/" + theme + "/' $HOME/.fluxbox/init && fluxbox-remote reconfigure && fluxbox-remote reloadstyle";
            }
            system(cmd.toUtf8());
        }
        //for fluxbox, edit ~/.config/gtk-3.0/settings.ini and ~/.gtkrc-2.0 has quotes
        if ( type == QLatin1String("icons") ) {

            if (runCmd("grep gtk-icon-theme-name $HOME/.config/gtk-3.0/settings.ini").exitCode == 0) {
                cmd = "sed -i 's/gtk-icon-theme-name=.*/gtk-icon-theme-name=" + theme + "/' $HOME/.config/gtk-3.0/settings.ini";
            } else {
                cmd = "echo gtk-icon-theme-name=" + theme + "\" >> $HOME/.config/gtk-3.0/settings.ini";
            }
            system(cmd.toUtf8());
            if (runCmd("grep gtk-icon-theme-name $HOME/.gtkrc-2.0").exitCode == 0) {
                cmd = "sed -i 's/gtk-icon-theme-name=.*/gtk-icon-theme-name=\"" + theme + "\"/' $HOME/.gtkrc-2.0";
            } else {
                cmd = "echo gtk-icon-theme-name=\"" + theme + "\" >> $HOME/.gtkrc-2.0";
            }
            system(cmd.toUtf8());

            if ( QFile("/usr/bin/preview-mx").exists()){
                cmd = "preview-mx &";
                system(cmd.toUtf8());
            }
        }

        //for fluxbox, edit ~/.config/gtk-3.0/settings.ini, ~/.gtkrc-2.0 has quotes, and .icons/default/index.theme (create if it doesn't exist)
        if ( type == QLatin1String("cursor") ) {

            if (runCmd("grep gtk-cursor-theme-name $HOME/.config/gtk-3.0/settings.ini").exitCode == 0) {
                cmd = "sed -i 's/gtk-cursor-theme-name=.*/gtk-cursor-theme-name=" + theme + "/' $HOME/.config/gtk-3.0/settings.ini";
            } else {
                cmd = "echo gtk-cursor-theme-name=" + theme + "\" >> $HOME/.config/gtk-3.0/settings.ini";
            }
            system(cmd.toUtf8());
            if (runCmd("grep gtk-cursor-theme-name $HOME/.gtkrc-2.0").exitCode == 0) {
                cmd = "sed -i 's/gtk-cursor-theme-name=.*/gtk-cursor-theme-name=\"" + theme + "\"/' $HOME/.gtkrc-2.0";
            } else {
                cmd = "echo gtk-cursor-theme-name=\"" + theme + "\" >> $HOME/.gtkrc-2.0";
            }
            system(cmd.toUtf8());
            if ( theme == "default"){
                runCmd("rm -R $HOME/.icons/default");
            } else {
                if ( ! QDir(home_path + "/.icons/default").exists() ){
                    runCmd("mkdir -p $HOME/.icons/default");
                }
                runCmd("echo [Icon Theme] > $HOME/.icons/default/index.theme");
                runCmd("echo Name=Default >> $HOME/.icons/default/index.theme");
                runCmd("echo Comment=Default Cursor Theme >> $HOME/.icons/default/index.theme");
                runCmd("echo Comment=Default Cursor Theme >> $HOME/.icons/default/index.theme");
                runCmd("echo Inherits=" + theme + " >> $HOME/.icons/default/index.theme");
            }
            cmd = "fluxbox-remote restart";
            system(cmd.toUtf8());
        }
    }
    if (!cmd1.isEmpty()){
        system(cmd1.toUtf8());
    }
    if (!cmd2.isEmpty()){
        system(cmd2.toUtf8());
    }
}

void defaultlook::on_listWidgetTheme_currentTextChanged(const QString &currentText)
{
    if ( themeflag ) {
        if (isXfce) {
            settheme(QStringLiteral("gtk-3.0"), currentText, "XFCE");
        } else if (isFluxbox){
            settheme(QStringLiteral("gtk-3.0"), currentText, "fluxbox");
        } else if (isKDE) {
            settheme(QStringLiteral("plasma"), currentText, "KDE");
        }
    }
}

void defaultlook::on_listWidgetWMtheme_currentTextChanged(const QString &currentText) const
{
    if ( themeflag ) {
        if (isXfce) {
            settheme(QStringLiteral("xfwm4"), currentText, "XFCE");
        } else if (isFluxbox){
            settheme(QStringLiteral("fluxbox"), currentText, "fluxbox");
        } else if (isKDE) {
            settheme(QStringLiteral("colorscheme"), currentText, "KDE");
        }
    }
}

void defaultlook::on_listWidgeticons_currentTextChanged(const QString &currentText) const
{
    if ( themeflag ) {
        if (isXfce) {
            settheme(QStringLiteral("icons"), currentText, "XFCE");
        } else if (isFluxbox){
            settheme(QStringLiteral("icons"), currentText, "fluxbox");
        } else if (isKDE) {
            settheme(QStringLiteral("icons"), currentText, "KDE");
        }
    }
}

void defaultlook::on_listWidgetCursorThemes_currentTextChanged(const QString &currentText)
{
    if ( themeflag ) {
        if (isXfce) {
            settheme(QStringLiteral("cursor"), currentText, "XFCE");
        } else if (isFluxbox){
            settheme(QStringLiteral("cursor"), currentText, "fluxbox");
        } else if (isKDE) {
            settheme(QStringLiteral("kdecursor"), currentText, "KDE");
        }
    }
}


void defaultlook::on_tabWidget_currentChanged(int /*index*/)
{
    if (!displaysetupflag) {
        if (ui->tabWidget->currentIndex() == Tab::Display) {
            setupDisplay();
            displaysetupflag = true;
        }
    }
}

void defaultlook::on_checkBoxCSD_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_pushButtonDocklikeSetttings_clicked()
{
    system("xfce4-panel --plugin-event=docklike:settings");
}

void defaultlook::on_checkBoxFileDialogActionButtonsPosition_clicked()
{
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}

void defaultlook::on_checkBoxbluetoothAutoEnable_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
    bluetoothautoenableflag = !bluetoothautoenableflag;
    if (verbose) qDebug() << "bluetooth flag is " << bluetoothautoenableflag;
}

void defaultlook::on_checkBoxFluxShowToolbar_clicked()
{
    ui->ApplyFluxboxResets->setEnabled(true);
}

void defaultlook::on_buttonManageTint2_clicked()
{
    this->hide();
    system("/usr/bin/mxfb-tint2-manager");
    this->show();

}

void defaultlook::migratepanel(const QString &date) const
{
    runCmd("tar --create --xz --file=\"$HOME/.restore/panel_backup_" + date +".tar.xz\" --directory=$HOME/.restore/.config/xfce4 panel xfconf/xfce-perchannel-xml/xfce4-panel.xml");
    runCmd("rm -R $HOME/.restore/.config/xfce4/panel $HOME/.restore/.config/xfce4/xfconf");
}

int defaultlook::validatearchive(const QString &path) const{
    QString test = runCmd("file --mime-type --brief " + path).output;
    if ( verbose ) qDebug() << test;
    //validate mime type
    if ( test != "application/x-xz"){
        return 1;
    }
    //validate contents

    test = runCmd("tar --list --file " + path).output;
    if ( verbose ) qDebug() << test;
    if (!test.contains("xfconf/xfce-perchannel-xml/xfce4-panel.xml")) {
    return 2;
    }
    return 0;
}

void defaultlook::on_lineEditBackupName_returnPressed()
{
    ui->buttonApply->setDefault(true);
}

void defaultlook::on_checkBoxInstallRecommends_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
    enable_recommendsflag = true;
}

void defaultlook::on_checkBoxThunarSplitView_clicked()
{
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}

void defaultlook::on_checkBoxsplitviewhorizontal_clicked()
{
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}

void defaultlook::thunarsplitview(bool state){

    if (state){
        runCmd(QStringLiteral("xfconf-query  -c thunar -p /misc-open-new-windows-in-split-view -t bool -s true --create"));
    } else {
        runCmd(QStringLiteral("xfconf-query  -c thunar -p /misc-open-new-windows-in-split-view --reset"));
    }
}

void defaultlook::thunarsplitviewhorizontal(bool state){
    if (state){
        runCmd(QStringLiteral("xfconf-query -c thunar -p /misc-vertical-split-pane -t bool -s true --create"));
    } else {
        runCmd(QStringLiteral("xfconf-query -c thunar -p /misc-vertical-split-pane --reset"));
    }
}

void defaultlook::thunarsetupsplitview(){
    QString test;
    //check split window status
    test = runCmd(QStringLiteral("xfconf-query  -c thunar -p /misc-open-new-windows-in-split-view")).output;
    ui->checkBoxThunarSplitView->setChecked(test == QLatin1String("true"));
    ui->checkBoxThunarSplitView_2->setChecked(test == QLatin1String("true"));

    //check split view horizontal or vertical.  default false is vertical, true is horizontal;
    test = runCmd(QStringLiteral("xfconf-query  -c thunar -p /misc-vertical-split-pane")).output;
    ui->checkBoxsplitviewhorizontal->setChecked(test == QLatin1String("true"));
    ui->checkBoxsplitviewhorizontal_2->setChecked(test == QLatin1String("true"));
}

void defaultlook::resetthunar(){
    QString cmd = QStringLiteral("cp /home/$USER/.config/Thunar/uca.xml /home/$USER/.config/Thunar/uca.xml.$(date +%Y%m%H%M%S)");
    system(cmd.toUtf8());
    runCmd(QStringLiteral("cp /etc/skel/.config/Thunar/uca.xml /home/$USER/.config/Thunar/uca.xml"));
}

void defaultlook::thunarsingleclicksetup(){
    //check single click thunar status
    QString test = runCmd(QStringLiteral("xfconf-query  -c thunar -p /misc-single-click")).output;
    ui->checkBoxThunarSingleClick->setChecked(test == QLatin1String("true"));
    ui->checkBoxThunarSingleClick_2->setChecked(test == QLatin1String("true"));

}

void defaultlook::thunarsetsingleclick(bool state){
    if (state) {
        runCmd(QStringLiteral("xfconf-query  -c thunar -p /misc-single-click -t bool -s true --create"));
    } else {
        runCmd(QStringLiteral("xfconf-query  -c thunar -p /misc-single-click -t bool -s false --create"));
    }
}

void defaultlook::on_checkBoxThunarCAReset_2_clicked()
{
    ui->ApplyFluxboxResets->setEnabled(true);
}

void defaultlook::on_checkBoxThunarSplitView_2_clicked()
{
    ui->ApplyFluxboxResets->setEnabled(true);
}

void defaultlook::on_checkBoxsplitviewhorizontal_2_clicked()
{
    ui->ApplyFluxboxResets->setEnabled(true);
}

void defaultlook::on_checkBoxThunarSingleClick_2_clicked()
{
    ui->ApplyFluxboxResets->setEnabled(true);
}

//returns first tasklist or docklike id
QString defaultlook::get_tasklistid(){
    QString tasklistID = runCmd(QStringLiteral("grep -m 1 tasklist ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    tasklistID=tasklistID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "tasklist: " << tasklistID;
    if (tasklistID == QLatin1String("")) {
        QString docklikeID = runCmd(QStringLiteral("grep -m 1 docklike ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
        docklikeID=docklikeID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
        if (verbose) qDebug() << "docklikeID: " << docklikeID;
        if (docklikeID != QLatin1String("")) {
            tasklistID = docklikeID;
            if (verbose) qDebug() << "new tasklist: " << tasklistID;
        }
    }
    return tasklistID;

}




void defaultlook::on_comboBoxTasklistPlugin_currentIndexChanged(int /*index*/)
{
    //toggle tasklistflag
    //changing tasklistflag only happens if block is actually changed

    tasklistflag = !tasklistflag;
    if (verbose) qDebug() << "tasklist flag is " << tasklistflag;
}

void defaultlook::tasklistchange(){
    //choice of tasklist
    QString tasklistchoice;

    if ( ui->comboBoxTasklistPlugin->currentIndex() == 0){
        tasklistchoice = "docklike";
    } else if ( ui->comboBoxTasklistPlugin->currentIndex() == 1){
        tasklistchoice = "tasklist";
    }
    if (verbose) qDebug() << "tasklistchoice is" << tasklistchoice;


    QString tasklistid = get_tasklistid();
    if (verbose) qDebug() << "tasklistid is " << tasklistid;

    if (tasklistchoice == "docklike"){
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + tasklistid + "/show-handle --reset");
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + tasklistid + "/show-labels --reset");
    } else if (tasklistchoice == "tasklist"){
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + tasklistid + "/show-handle -t bool -s false --create");
         QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/mode").output;
        if (test == QLatin1String("") || test == QLatin1String("0")) { //horizontal panel
            runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + tasklistid + "/show-labels -t bool -s true --create");
        } else { runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + tasklistid + "/show-labels -t bool -s false --create");  //vertical panel
        }
    }

    //switch plugin
    runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + tasklistid + " -t string -s " + tasklistchoice + " --create");


    //reset panel
    runCmd("xfce4-panel --restart");
    runCmd(QStringLiteral("sleep .5"));

}




void defaultlook::on_checkBoxDisableFluxboxMenuGeneration_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}




void defaultlook::on_spinBoxScreenBlankingTimeout_valueChanged(int arg1)
{
    screenblankflag = true;
    ui->ApplyFluxboxResets->setEnabled(true);
}


void defaultlook::on_toolButtonSuperFileBrowser_clicked()
{
    QString customcommand = QFileDialog::getOpenFileName(this, tr("Select application to run","will show in file dialog when selection an application to run"), "/usr/bin");
    QString cmd;

    //process file
    QFileInfo customcommandcheck(customcommand);
    if (customcommandcheck.fileName().endsWith(".desktop")){
        cmd = runCmd("grep Exec= " + customcommand).output.section("=",1,1).section("%",0,0).trimmed();
        cmd = runCmd("which " + cmd).output;
    } else {
        cmd = runCmd("which " + customcommand).output;
    }
    if (verbose) {
        qDebug() << "custom command is " << cmd;
    }
     ui->lineEditSuperCommand->setText(cmd);
     ui->pushButtonSuperKeyApply->setEnabled(true);

}


void defaultlook::on_pushButtonSuperKeyApply_clicked()
{
    QString home_path = QDir::homePath();
    if (!QFile(home_path + "/.config/xfce-superkey/xfce-superkey.conf").exists()){
        runCmd("mkdir -p " + home_path + "/.config/xfce-superkey/xfce-superkey.conf");
        runCmd("cp /usr/share/xfce-superkey/xfce-superkey.conf " + home_path + "/.config/xfce-superkey/xfce-superkey.conf");
    }
    QString cmd = ui->lineEditSuperCommand->text();
    //add command if no uncommented lines
    if (runCmd("grep -m1 -v -e '^#' -e '^$' $HOME/.config/xfce-superkey/xfce-superkey.conf").output.isEmpty()){
        runCmd("echo " + cmd + ">> $HOME/.config/xfce-superkey/xfce-superkey.conf");
    } else { //replace first uncommented line with new command
        runCmd("sed -i '/^[^#]/s;.*;" + cmd + ";' $HOME/.config/xfce-superkey/xfce-superkey.conf");
    }
    //restart xfce-superkey
    runCmd("pkill xfce-superkey");
    runCmd("xfce-superkey-launcher");
    setupSuperKey();
}

void defaultlook::setupSuperKey()
{
    QString test = runCmd("grep -m1 -v -e '^#' -e '^$' $HOME/.config/xfce-superkey/xfce-superkey.conf").output;
    ui->pushButtonSuperKeyApply->setEnabled(false);
    if (!test.isEmpty()){
        ui->lineEditSuperCommand->setText(test);
    }

}



void defaultlook::on_lineEditSuperCommand_textChanged(const QString &arg1)
{
    ui->pushButtonSuperKeyApply->setEnabled(true);
}


void defaultlook::on_checkBoxLiqKernelUpdates_clicked()
{
    // set action flag, actions only happen if flag is true when apply is clicked.
    if (liqKernelUpdateFlag){
        liqKernelUpdateFlag = false;
    } else {
        liqKernelUpdateFlag = true;
    }
    ui->ButtonApplyEtc->setEnabled(true);
}


void defaultlook::on_checkBoxDebianKernelUpdates_clicked()
{
    // set action flag, actions only happen if flag is true when apply is clicked.
    if (debianKernelUpdateFlag){
        liqKernelUpdateFlag = false;
    } else {
        debianKernelUpdateFlag = true;
    }
    ui->ButtonApplyEtc->setEnabled(true);
}


void defaultlook::on_spinBoxPointerSize_valueChanged(int arg1)
{
    set_cursor_size();
}


void defaultlook::on_checkBoxPlasmaDiscoverUpdater_clicked()
{
    plasmadisoverautostartflag = true;
    ui->ButtonApplyPlasma->setEnabled(true);
}


void defaultlook::on_checkBoxComputerName_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
    if (ui->checkBoxComputerName->isChecked()){
        ui->lineEditHostname->setEnabled(true);
    } else {
        ui->lineEditHostname->setEnabled(false);
    }
}

void defaultlook::on_checkBoxBluetoothBattery_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
    bluetoothbatteryflag = !bluetoothbatteryflag;
    if (verbose) qDebug() << "bluetooth battery flag is " << bluetoothbatteryflag;
}


void defaultlook::on_checkBoxDisplayManager_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}


void defaultlook::on_checkBoxKVMVirtLoad_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
    kvmflag = !kvmflag;
}

