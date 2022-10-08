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

#include "about.h"
#include "brightness_small.h"
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
    if ( args.contains(QStringLiteral("--display"))) {
        if (checkXFCE()) {
            displayflag = true;
        } else {
            QMessageBox::information(nullptr, tr("MX Tweak"),
                                     tr("--display switch only valid for Xfce"));
        }
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

// setup versious items first time program runs
void defaultlook::setup()
{
    this->setWindowTitle(tr("MX Tweak"));
    this->adjustSize();
    QString home_path = QDir::homePath();
    if (!checklightdm()) {
        ui->checkBoxLightdmReset->hide();
    }

    if (checkXFCE()) {
        whichpanel();
        message_flag = false;
        //setup theme tab
        setuptheme();
        ui->buttonThemeUndo->setEnabled(false);
        //setup theme combo box
        setupComboTheme();
        //setup panel tab
        setuppanel();
        //setup compositor tab
        setupCompositor();
        //setup display tab
        //setupDisplay();
        ui->tabWidget->removeTab(Tab::Plasma);
        ui->tabWidget->removeTab(Tab::Fluxbox);
        //set first tab as default
        ui->tabWidget->setCurrentIndex(Tab::Panel);
        //setup Config Options
        setupConfigoptions();
    }

    //setup fluxbox
    else if (checkFluxbox()) {
        setuptheme();
        setupFluxbox();
        ui->comboTheme->hide();
        ui->label->hide();
        ui->buttonThemeApply->hide();
        ui->buttonThemeUndo->hide();
        ui->pushButtonPreview->hide();
        ui->pushButtonRemoveUserThemeSet->hide();
        ui->tabWidget->removeTab(Tab::Plasma);
        ui->tabWidget->removeTab(Tab::Panel);
        ui->label_4->hide();
        ui->label_5->hide();
        ui->label_6->hide();
        ui->label_7->hide();
        ui->listWidgetTheme->hide();
        ui->listWidgeticons->hide();
        ui->label_28->hide();
        ui->label_30->hide();
        ui->toolButtonXFCEAppearance->hide();
        ui->toolButtonXFCEWMsettings->hide();
        ui->toolButtonXFCEpanelSettings->hide();
        ui->tabWidget->removeTab(2);
        ui->tabWidget->removeTab(2);
        ui->tabWidget->removeTab(1);
        ui->tabWidget->setCurrentIndex(1);


    }

    //setup plasma
    else if (checkPlasma()) {
        ui->label_4->hide();
        ui->label_5->hide();
        ui->label_6->hide();
        ui->label_7->hide();
        ui->toolButtonXFCEAppearance->hide();
        ui->toolButtonXFCEWMsettings->hide();
        ui->toolButtonXFCEpanelSettings->hide();
        ui->tabWidget->setCurrentIndex(Tab::Plasma);
        for (int i = 5; i >= 0; --i)
            ui->tabWidget->removeTab(i);
        setupPlasma();
    }

    //for other non-supported desktops, show only
    else {
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
    }

    //setup other tab;
    setupEtc();

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
    if (displayflag) {
        ui->tabWidget->setCurrentIndex(Tab::Display);
    }
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

    QString tasklistID = runCmd(QStringLiteral("grep tasklist ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    tasklistID=tasklistID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "tasklist: " << tasklistID;

    QString pulseaudioID = runCmd(QStringLiteral("grep pulseaudio ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    pulseaudioID=pulseaudioID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "pulseaudio: " << pulseaudioID;

    QString powerID = runCmd(QStringLiteral("grep power-manager-plugin ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    powerID=powerID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "powerID: " << powerID;

    QString workspacesID = runCmd(QStringLiteral("grep pager ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    workspacesID = workspacesID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "workspacesID: " << workspacesID;

    if (tasklistID == QLatin1String("")) {
        QString docklikeID = runCmd(QStringLiteral("grep docklike ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
        docklikeID=docklikeID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
        if (verbose) qDebug() << "docklikeID: " << docklikeID;
        if (docklikeID != QLatin1String("")) {
            tasklistID = docklikeID;
            if (verbose) qDebug() << "new tasklist: " << tasklistID;
        }
    }

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

    QString tasklistID = runCmd(QStringLiteral("grep tasklist ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    tasklistID=tasklistID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "tasklist: " << tasklistID;

    QString pulseaudioID = runCmd(QStringLiteral("grep pulseaudio ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    pulseaudioID=pulseaudioID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "pulseaudio: " << pulseaudioID;

    QString powerID = runCmd(QStringLiteral("grep power-manager-plugin ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    powerID=powerID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "powerID: " << powerID;

    QString workspacesID = runCmd(QStringLiteral("grep pager ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
    workspacesID=workspacesID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
    if (verbose) qDebug() << "workspacesID: " << workspacesID;

    if (tasklistID == QLatin1String("")) {
        QString docklikeID = runCmd(QStringLiteral("grep docklike ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml")).output;
        docklikeID=docklikeID.remove(QStringLiteral("\"")).section(QStringLiteral("-"),1,1).section(QStringLiteral(" "),0,0);
        if (verbose) qDebug() << "docklikeID: " << docklikeID;
        if (docklikeID != QLatin1String("")) {
            tasklistID = docklikeID;
            if (verbose) qDebug() << "new tasklist: " << tasklistID;
        }
    }

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
    ui->buttonApply->setEnabled(false);

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
    //read in plugin ID's
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
    setuppanel();
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
    runCmd("tar --create --xz --file=$HOME/.restore/\"" + ui->lineEditBackupName->text() + ".tar.xz\" --directory=$HOME/.config/xfce4 panel xfconf/xfce-perchannel-xml/xfce4-panel.xml");
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


    switch(validatearchive("$HOME/.restore/\"" + ui->comboBoxAvailableBackups->currentText() + "\"")){
    case 0:
        runCmd("xfce4-panel --quit; pkill xfconfd; rm -Rf ~/.config/xfce4/panel ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml; \
           tar -xf $HOME/.restore/\"" + ui->comboBoxAvailableBackups->currentText() + "\" --directory=$HOME/.config/xfce4; \
           sleep 5; xfce4-panel &");
        break;
    case 1: QMessageBox::information(nullptr, tr("MX Tweak"),
                                     tr("File is not a valid xz archive file"));
        break;
    case 2:  QMessageBox::information(nullptr, tr("MX Tweak"),
                                      tr("Archive does not contain a panel file"));
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
    }
}

void defaultlook::on_checkFirefox_clicked()
{
    ui->buttonThemeApply->setEnabled(true);
    message_flag = true;
}

void defaultlook::on_checkHexchat_clicked()
{
    ui->buttonThemeApply->setEnabled(true);
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
        ui->lineEditBackupName->show();
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
    ui->comboBoxAvailableBackups->clear();
    ui->lineEditBackupName->hide();
    ui->lineEditBackupName->setText("panel_backup_" + QDateTime::currentDateTime().toString("dd.MM.yyyy.hh.mm.ss"));
    QStringList availablebackups = QDir(home_path + "/.restore").entryList(QStringList() << "*.tar.xz",QDir::Files);
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

    if ( system("grep -q tasklist ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml") != 0 ) {
        ui->labelTasklist->hide();
        ui->pushButtontasklist->hide();
    }

    //hide docklike settings if not present

    if ( system("grep -q docklike ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml") != 0 ) {
        ui->labelDocklikeSettings->hide();
        ui->pushButtonDocklikeSetttings->hide();
    }

    //reset all checkboxes to unchecked

    ui->checkVert->setChecked(false);
    ui->checkHorz->setChecked(false);
    ui->radioBackupPanel->setChecked(false);
    ui->radioDefaultPanel->setChecked(false);
    ui->radioRestoreBackup->setChecked(false);

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

    // if backup available, make the restore backup option available

    if ( availablebackups.isEmpty()){
        ui->radioRestoreBackup->setEnabled(false);
        ui->comboBoxAvailableBackups->hide();
    }
    panelflag = true;
}

void defaultlook::setupPlasma()
{
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

    //setup icon size
    QString systrayid = runCmd(QStringLiteral("grep -B 3 extraItems $HOME/.config/plasma-org.kde.plasma.desktop-appletsrc |grep Containment")).output.section(QStringLiteral("["),2,2).section(QStringLiteral("]"),0,0);
    if (verbose) qDebug() << "systrayid is" << systrayid;
    //read in config and set combobox
    QString value = runCmd("kreadconfig5 --file ~/.config/plasma-org.kde.plasma.desktop-appletsrc --group Containments --group " + systrayid + " --group General --key iconSize").output;

    switch(value.toInt()) {
    case ValueSize::Default:
        ui->comboBoxPlasmaSystrayIcons->setCurrentIndex(IconSize::Default);
        break;
    case ValueSize::Small:
        ui->comboBoxPlasmaSystrayIcons->setCurrentIndex(IconSize::Small);
        break;
    case ValueSize::Medium:
        ui->comboBoxPlasmaSystrayIcons->setCurrentIndex(IconSize::Medium);
        break;
    case ValueSize::Large:
        ui->comboBoxPlasmaSystrayIcons->setCurrentIndex(IconSize::Large);
        break;
    case ValueSize::Larger:
        ui->comboBoxPlasmaSystrayIcons->setCurrentIndex(IconSize::Larger);
        break;
    case ValueSize::Largest:
        ui->comboBoxPlasmaSystrayIcons->setCurrentIndex(IconSize::Largest);
        break;
    default: ui->comboBoxPlasmaSystrayIcons->setCurrentIndex(IconSize::Default);
    }

    ui->ButtonApplyPlasma->setDisabled(true);

    ui->checkboxplasmaresetdock->setChecked(false);

    plasmaplacementflag = false;
    plasmaworkspacesflag = false;
    plasmasingleclickflag = false;
    plasmaresetflag = false;
    plasmasystrayiconsizeflag = false;
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
}

void defaultlook::setupEtc()
{
    QString home_path = QDir::homePath();
    QString DESKTOP = runCmd(QStringLiteral("echo $XDG_SESSION_DESKTOP")).output;
    if (verbose) qDebug() << "setupetc nocsd desktop is:" << DESKTOP;

    ui->checkBoxLightdmReset->setChecked(false);
    QString test = runCmd(QStringLiteral("pgrep lightdm")).output;
    if (test.isEmpty()) {
        ui->checkBoxLightdmReset->hide();
    }

    ui->checkboxIntelDriver->hide();
    ui->labelIntel->hide();
    ui->checkboxAMDtearfree->hide();
    ui->labelamdgpu->hide();
    ui->checkboxRadeontearfree->hide();
    ui->labelradeon->hide();

    ui->ButtonApplyEtc->setEnabled(false);
    if (ui->ButtonApplyEtc->icon().isNull()) {
        ui->ButtonApplyEtc->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }
    //set values for checkboxes

    //setup udisks option
    QFileInfo fileinfo(QStringLiteral("/etc/tweak-udisks.chk"));
    if (fileinfo.exists()) {
        ui->checkBoxMountInternalDrivesNonRoot->setChecked(true);
    } else {
        ui->checkBoxMountInternalDrivesNonRoot->setChecked(false);
    }

    //setup sudo override function

    QFileInfo sudo_override_file(QStringLiteral("/etc/polkit-1/localauthority.conf.d/55-tweak-override.conf"));
    ui->radioSudoUser->setChecked(sudo_override_file.exists());

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

    //setup bluetooth auto enable
    bluetoothautoenableflag = false;
    test = runCmd(QStringLiteral("grep ^AutoEnable /etc/bluetooth/main.conf")).output;
    test = test.section("=",1,1);
    if ( test == "true"){
        ui->checkBoxbluetoothAutoEnable->setChecked(true);
    } else {
        ui->checkBoxbluetoothAutoEnable->setChecked(false);
    }

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
    libinput_touchpadflag = false;
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

    QFileInfo libinputfile(QStringLiteral("/etc/X11/xorg.conf.d/30-touchpad.conf"));
    ui->checkBoxlibinput->setChecked(libinputfile.exists());
}

void defaultlook::setuptheme()
{
    ui->buttonThemeApply->setEnabled(false);
    if (ui->buttonThemeApply->icon().isNull()) {
        ui->buttonThemeApply->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }

    ui->pushButtonPreview->setEnabled(false);
    //reset all checkboxes to unchecked

    ui->checkFirefox->setChecked(false);
    ui->checkHexchat->setChecked(false);
    ui->checkFirefox->hide();
    ui->checkHexchat->hide();

    populatethemelists(QStringLiteral("gtk-3.0"));
    populatethemelists(QStringLiteral("icons"));

    if (checkXFCE()){
         populatethemelists(QStringLiteral("xfwm4"));
    } else if (checkFluxbox()) {
         populatethemelists(QStringLiteral("fluxbox"));
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

    //set xfce values
    if (checkXFCE()) {
        //check single click status
        QString test;
        test = runCmd(QStringLiteral("xfconf-query  -c xfce4-desktop -p /desktop-icons/single-click")).output;
        ui->checkBoxSingleClick->setChecked(test == QLatin1String("true"));

        //check single click thunar status
        test = runCmd(QStringLiteral("xfconf-query  -c thunar -p /misc-single-click")).output;
        ui->checkBoxThunarSingleClick->setChecked(test == QLatin1String("true"));

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
    //build theme list
    QString home_path = QDir::homePath();
    if (verbose) qDebug() << "home path is " << home_path;
    bool xsettings_gtk_theme_present = false;
    bool icontheme_present = false;
    bool xfwm4_theme_present = false;
    ui->comboTheme->clear();
    QStringList theme_list;
    QStringList filter(QStringLiteral("*.tweak"));
    QDirIterator it(QStringLiteral("/usr/share/mx-tweak-data"), filter, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        xsettings_gtk_theme_present = false;
        icontheme_present = false;
        xfwm4_theme_present = false;
        QFileInfo file_info(it.next());
        QString filename = file_info.absoluteFilePath();
        QString name = runCmd("cat '" + filename + "'|grep Name=").output.section(QStringLiteral("="),1,1);
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
        QString name = runCmd("cat '" + filename + "'|grep Name=").output.section(QStringLiteral("="),1,1);

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

    ui->comboTheme->addItems(theme_list);
    ui->comboTheme->setCurrentIndex(0);
}

void defaultlook::on_comboTheme_activated(const QString & /*arg1*/)
{
    if (ui->comboTheme->currentIndex() != 0) {
        ui->buttonThemeApply->setEnabled(true);
        ui->pushButtonPreview->setEnabled(true);
    }
}

void defaultlook::on_buttonThemeApply_clicked()
{
    themeflag = false;
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

    //  use xfconf system to change values

    message_flag = true;

    //set gtk theme
    runCmd("xfconf-query -c xsettings -p /Net/ThemeName -s " + xsettings_gtk_theme);
    runCmd(QStringLiteral("sleep .5"));

    //set window decorations theme
    runCmd("xfconf-query -c xfwm4 -p /general/theme -s " + xfwm4_window_decorations);
    runCmd(QStringLiteral("sleep .5"));

    //set icon theme
    runCmd("xfconf-query -c xsettings -p /Net/IconThemeName -s " + xsettings_icon_theme);
    runCmd(QStringLiteral("sleep .5"));

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

    //check theme overrides

    if (ui->checkFirefox->isChecked()) {
        runCmd(QStringLiteral("touch /home/$USER/.config/FirefoxDarkThemeOverride.check"));
    } else {
        runCmd(QStringLiteral("rm /home/$USER/.config/FirefoxDarkThemeOverride.check"));
    }

    //deal with hexchat
    QFileInfo file_hexchat(home_path + "/.config/hexchat/hexchat.conf");
    if (ui->checkHexchat->isChecked()) {
        if (file_hexchat.exists()) {
            //replace setting
            runCmd("sed -i -r 's/gui_input_style = 1/gui_input_style = 0/' " + file_hexchat.absoluteFilePath());
        } else {
            //copy a config file into user directory
            runCmd("mkdir -p " + home_path + "/.config/hexchat");
            runCmd("cp /usr/share/mx-tweak/hexchat.conf " + file_hexchat.absoluteFilePath());
        }
    } else {
        if (file_hexchat.exists()) {
            //replace setting
            runCmd("sed -i -r 's/gui_input_style = 0/gui_input_style = 1/' " + file_hexchat.absoluteFilePath());
        }
    }

    // message that we are done if a theme change was made

    //if (message_flag == true) {
    //   message();
    //  message_flag = false;
    //}

    // reset gui
    setuptheme();
}

void defaultlook::on_ButtonApplyEtc_clicked()
{
    QString intel_option;
    QString amd_option;
    QString radeon_option;
    QString lightdm_option;
    QString libinput_option;
    QString bluetooth_option;
    QString DESKTOP = runCmd(QStringLiteral("echo $XDG_SESSION_DESKTOP")).output;
    QString home_path = QDir::homePath();
    ui->ButtonApplyEtc->setEnabled(false);

    intel_option.clear();
    lightdm_option.clear();
    libinput_option.clear();
    bluetooth_option.clear();

    //deal with udisks option
    QFileInfo fileinfo(QStringLiteral("/etc/tweak-udisks.chk"));
    QFileInfo sudo_override(QStringLiteral("/etc/polkit-1/localauthority.conf.d/55-tweak-override.conf"));
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

    if (ui->checkBoxLightdmReset->isChecked()) {
        lightdm_option = QStringLiteral("lightdm_reset");
    }

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
        } else {
            bluetooth_option = QStringLiteral("disable_bluetooth");
            //blueman
            if (QFile::exists("/usr/bin/blueman")) {
                runCmd(QStringLiteral("gsettings set org.blueman.plugins.powermanager auto-power-on false"));
            }

        }
    }

    //libinput_touchpad

    if ( libinput_touchpadflag ) {
        QFileInfo check_libinput(QStringLiteral("/etc/X11/xorg.conf.d/30-touchpad.conf"));
        if ( check_libinput.exists()) {
            //backup existing 30-touchpad.conf file to home folder
            cmd = QStringLiteral("cp /etc/X11/xorg.conf.d/30-touchpad.conf /home/$USER/30-touchpad.conf.$(date +%Y%m%H%M%S)");
            system(cmd.toUtf8());
        }
        if (ui->checkBoxlibinput->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            libinput_option = QStringLiteral("enable_libinput_touchpad");
        } else {
            //remove 20-radeon.conf
            libinput_option = QStringLiteral("disable_libinput_touchpad");
        }
    }

    //deal with sudo override

    if (ui->radioSudoUser->isChecked()) {
        if (sudo_override.exists()) {
            if (verbose) qDebug() << "no change to admin password settings";
        } else {
            sudo_override_option = QStringLiteral("enable_sudo_override");
        }
    } else {
        if (sudo_override.exists()) {
            sudo_override_option = QStringLiteral("disable_sudo_override");
        } else {
            if (verbose) qDebug() << "no change to admin password settings";
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

    if ( ! udisks_option.isEmpty() || ! sudo_override_option.isEmpty() || ! user_name_space_override_option.isEmpty() || ! intel_option.isEmpty() || ! lightdm_option.isEmpty() || ! amd_option.isEmpty() || ! radeon_option.isEmpty() || ! libinput_option.isEmpty() || !bluetooth_option.isEmpty()) {
        runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh " + udisks_option + " " + sudo_override_option + " " + user_name_space_override_option + " " + intel_option + " " + amd_option + " " + radeon_option + " " + libinput_option + " " + bluetooth_option + " " + lightdm_option);
    }
    //reset gui
    setupEtc();
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

    qDebug () << "undo command list is " << undotheme;

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
    QString cmd;
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

    if (ui->checkBoxThunarSingleClick->isChecked()) {
        runCmd(QStringLiteral("xfconf-query  -c thunar -p /misc-single-click -s true"));
    } else {
        runCmd(QStringLiteral("xfconf-query  -c thunar -p /misc-single-click -s false"));
    }

    //systray frame removed

    //set desktop zoom
    if (ui->checkBoxDesktopZoom->isChecked()) {
        runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/zoom_desktop -s true"));
    } else {
        runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/zoom_desktop -s false"));
    }

    if (ui->checkBoxThunarCAReset->isChecked()) {
        cmd = QStringLiteral("cp /home/$USER/.config/Thunar/uca.xml /home/$USER/.config/Thunar/uca.xml.$(date +%Y%m%H%M%S)");
        system(cmd.toUtf8());
        runCmd(QStringLiteral("cp /etc/skel/.config/Thunar/uca.xml /home/$USER/.config/Thunar/uca.xml"));
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
            system("xfconf-query -c xfce4-session -p /shutdown/ShowHibernate -s true --create");
        } else {
            system("xfconf-query -c xfce4-session -p /shutdown/ShowHibernate -s false --create");
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
    if (checkFluxbox()) {
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
        runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-kde-edit.sh \"kwriteconfig5 --file /root/.config/kdeglobals --group KDE --key SingleClick " + value + "\"");
    }

    if (plasmasystrayiconsizeflag) {
        int index = ui->comboBoxPlasmaSystrayIcons->currentIndex();
        QString systrayiconvalue;
        switch(index) {
        case IconSize::Default:
            systrayiconvalue = QString::number(ValueSize::Default);
            break;
        case IconSize::Small:
            systrayiconvalue = QString::number(ValueSize::Small);
            break;
        case IconSize::Medium:
            systrayiconvalue = QString::number(ValueSize::Medium);
            break;
        case IconSize::Large:
            systrayiconvalue = QString::number(ValueSize::Large);
            break;
        case IconSize::Larger:
            systrayiconvalue = QString::number(ValueSize::Larger);
            break;
        case IconSize::Largest:
            systrayiconvalue = QString::number(ValueSize::Largest);
            break;
        default: systrayiconvalue = QString::number(ValueSize::Default);
        }
        QString systrayid = runCmd(QStringLiteral("grep -B 3 extraItems $HOME/.config/plasma-org.kde.plasma.desktop-appletsrc |grep Containment")).output.section(QStringLiteral("["),2,2).section(QStringLiteral("]"),0,0);
        runCmd("kwriteconfig5 --file ~/.config/plasma-org.kde.plasma.desktop-appletsrc --group Containments --group " + systrayid + " --group General --key iconSize " + systrayiconvalue);
    }

    if (plasmaworkspacesflag) {
        QString value = ui->checkBoxPlasmaShowAllWorkspaces->isChecked() ? QStringLiteral("false") : QStringLiteral("true");
        writeTaskmanagerConfig(QStringLiteral("showOnlyCurrentDesktop"), value);
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
    QString themes;
    QStringList themelist;
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



    themelist = themes.split(QStringLiteral("\n"));
    themelist.removeDuplicates();
    themelist.removeAll(QLatin1String(""));
    themelist.sort(Qt::CaseInsensitive);
    QString current;
    if ( value == QLatin1String("gtk-3.0") ) {
        ui->listWidgetTheme->clear();
        ui->listWidgetTheme->addItems(themelist);
        //set current
        if (checkXFCE()){
            current = runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/ThemeName")).output;
        } else if (checkFluxbox()){
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
        QString current = runCmd(QStringLiteral("grep styleFile $HOME/.fluxbox/init |grep -v ^# | cut -d\"/\" -f6")).output;
        ui->listWidgetWMtheme->setCurrentRow(themelist.indexOf(current));
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
        if (checkXFCE()){
            current = runCmd(QStringLiteral("xfconf-query -c xsettings -p /Net/IconThemeName")).output;
        } else if (checkFluxbox()){
            current = runCmd(QStringLiteral("grep gtk-icon-theme-name $HOME/.config/gtk-3.0/settings.ini |grep -v ^# | cut -d\"=\" -f2")).output;
        }
        ui->listWidgeticons->setCurrentRow(themelist.indexOf(current));
    }

    themeflag = true;
}

void defaultlook::settheme(const QString &type, const QString &theme, const QString &desktop)
{   //set new theme
    QString cmd;
    if ( desktop == "XFCE" ) {
        if ( type == QLatin1String("gtk-3.0") ) {
            cmd = "xfconf-query -c xsettings -p /Net/ThemeName -s \"" + theme + "\"";
        }
        if ( type == QLatin1String("xfwm4") ) {
            cmd = "xfconf-query -c xfwm4 -p /general/theme -s \"" + theme + "\"";
        }

        if ( type == QLatin1String("icons") ) {
            cmd = "xfconf-query -c xsettings -p /Net/IconThemeName -s \"" + theme + "\"";
        }
    } else if ( desktop == "fluxbox" ){
        QString home_path = QDir::homePath();
        if ( type == QLatin1String("gtk-3.0") ) {
            cmd = "sed -i 's/gtk-theme-name=.*/gtk-theme-name=" + theme + "/' $HOME/.config/gtk-3.0/settings.ini";
            system(cmd.toUtf8());
            cmd = "yad --form --title \"Preview\"  --button:gtk-ok --field=Button:FBTN --field=Combobox:CBE --field=Checkbox:CHK --close-on-unfocus";
            system(cmd.toUtf8());
            cmd = "sed -i 's/gtk-theme-name=\".*/gtk-theme-name=\"" + theme + "\"/' $HOME/.gtkrc-2.0";
        }
        if ( type == QLatin1String("fluxbox") ) {
            QString filepath = home_path + "/.fluxbox/styles/" + theme;
            if (QFile(filepath).exists()){
                home_path.replace("/", "\\/");
                cmd = "sed -i 's/session.styleFile:.*/session.styleFile: " + home_path + "\\/.fluxbox\\/styles\\/" + theme + "/' $HOME/.fluxbox/init && fluxbox-remote reconfigure";
            } else {
                cmd = "sed -i 's/session.styleFile:.*/session.styleFile: \\/usr\\/share\\/fluxbox\\/styles\\/" + theme + "/' $HOME/.fluxbox/init && fluxbox-remote reconfigure";
            }
        }
        //for fluxbox, edit ~/.config/gtk-3.0/settings.ini and ~/.gtkrc-2.0 has quotes
        if ( type == QLatin1String("icons") ) {
            cmd = "sed -i 's/gtk-icon-theme-name=.*/gtk-icon-theme-name=" + theme + "/' $HOME/.config/gtk-3.0/settings.ini";
            system(cmd.toUtf8());
            cmd = cmd = "yad --form --title \"Preview\"  --button:gtk-ok --field=Button:FBTN --field=Combobox:CBE --field=Checkbox:CHK --close-on-unfocus";
            system(cmd.toUtf8());
            cmd = "sed -i 's/gtk-icon-theme-name=\".*/gtk-icon-theme-name=\"" + theme + "\"/' $HOME/.gtkrc-2.0";
        }        
    }
    system(cmd.toUtf8());
}

void defaultlook::on_listWidgetTheme_currentTextChanged(const QString &currentText)
{
    if ( themeflag ) {
        QString desktop;
        if (checkXFCE()){
            settheme(QStringLiteral("gtk-3.0"), currentText, "XFCE");
        } else if (checkFluxbox()){
            settheme(QStringLiteral("gtk-3.0"), currentText, "fluxbox");
        }
    }
}

void defaultlook::on_listWidgetWMtheme_currentTextChanged(const QString &currentText) const
{
    if ( themeflag ) {
        QString desktop;
        if (checkXFCE()){
            settheme(QStringLiteral("xfwm4"), currentText, "XFCE");
        } else if (checkFluxbox()){
            settheme(QStringLiteral("fluxbox"), currentText, "fluxbox");
        }
    }
}


void defaultlook::on_listWidgeticons_currentTextChanged(const QString &currentText) const
{
    if ( themeflag ) {
        if (checkXFCE()){
            settheme(QStringLiteral("icons"), currentText, "XFCE");
        } else if (checkFluxbox()){
            settheme(QStringLiteral("icons"), currentText, "fluxbox");
        }
    }
}

void defaultlook::on_checkBoxlibinput_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
    libinput_touchpadflag = true;

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
    if (!bluetoothautoenableflag) {
        bluetoothautoenableflag = true;
    } else {
        bluetoothautoenableflag = false;
    }
    qDebug() << "bluetooth flag is " << bluetoothautoenableflag;
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
    QString test = runCmd("file --mime-type --brief \"" + path + "\"").output;
    if ( verbose ) qDebug() << test;
    //validate mime type
    if ( test != "application/x-xz"){
        return 1;
    }
    //validate contents

    test = runCmd("tar --list --file \"" + path + "\"").output;
    if ( verbose ) qDebug() << test;
    if (!test.contains("xfconf/xfce-perchannel-xml/xfce4-panel.xml")) {
    return 2;
    }
    return 0;
}
