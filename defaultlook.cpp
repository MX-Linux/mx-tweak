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

#include "defaultlook.h"
#include "ui_defaultlook.h"
#include "QDebug"
#include "QDir"
#include <QDirIterator>
#include <QFileDialog>
#include <QHash>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>

#include "xfwm_compositor_settings.h"
#include "window_buttons.h"
#include "theming_to_tweak.h"
#include "remove_user_theme_set.h"
#include "brightness_small.h"

defaultlook::defaultlook(QWidget *parent, QStringList args) :
    QDialog(parent),
    ui(new Ui::defaultlook)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    if ( args.contains("--display")) {
        displayflag = true;
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
 //   checkXFCE();
    whichpanel();
    message_flag = false;
    QString cmd = QString("test -f ~/.restore/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml");
    if (system(cmd.toUtf8()) != 0) {
        backupPanel();
        message2();
    }

    //setup theme tab
    setuptheme();
    //setup theme combo box
    setupComboTheme();
    //setup panel tab
    setuppanel();
    //setup other tab;
    setupEtc();
    //setup compositor tab
    setupCompositor();
    //set panel tab as default
    ui->tabWidget->setCurrentIndex(0);
    ui->buttonThemeUndo->setEnabled(false);
    //setup Config Options
    setupConfigoptions();
    //setup display tab
    setupDisplay();

    //copy template file to ~/.local/share/mx-tweak-data if it doesn't exist
    QString home_path = QDir::homePath();
    QDir userdir(home_path + "/.local/share/mx-tweak-data");
    QFileInfo template_file(home_path + "/.local/share/mx-tweak-data/mx.tweak.template");
    if (template_file.exists()) {
        qDebug() << "template file found";
    } else {
        if (userdir.exists()){
            runCmd("cp /usr/share/mx-tweak-data/mx.tweak.template " + userdir.absolutePath());
        } else {
            runCmd("mkdir -p " + userdir.absolutePath());
            runCmd("cp /usr/share/mx-tweak-data/mx.tweak.template " + userdir.absolutePath());
        }
    }
    version = getVersion("mx-tweak");
    if (displayflag){
        ui->tabWidget->setCurrentIndex(4);
    }
}

// Util function for getting bash command output and error code
Result defaultlook::runCmd(QString cmd)
{
    QEventLoop loop;
    proc = new QProcess(this);
    proc->setReadChannelMode(QProcess::MergedChannels);
    connect(proc, SIGNAL(finished(int)), &loop, SLOT(quit()));
    proc->start("/bin/bash", QStringList() << "-c" << cmd);
    loop.exec();
    disconnect(proc, 0, 0, 0);
    Result result = {proc->exitCode(), proc->readAll().trimmed()};
    delete proc;
    return result;
}


void defaultlook::whichpanel()
{
    // take the first panel we see as default
    QString panel_content;
    panel_content = runCmd("LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels | grep -v Value | grep -v ^$").output;
    panelIDs = panel_content.split("\n");
    panel = panelIDs.value(0);
    qDebug() << "panels found: " << panelIDs;
    qDebug() << "panel to use: " << panel;
}

void defaultlook::fliptohorizontal()
{
    QString file_content;
    QStringList pluginIDs;
    file_content = runCmd("LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/plugin-ids | grep -v Value | grep -v ^$").output;
    pluginIDs = file_content.split("\n");
    qDebug() << pluginIDs;

    // figure out moving the systray, if it exists

    // figure out systrayID, pusleaudio plugin, and tasklistID

    QString systrayID = runCmd("grep \\\"systray\\\" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
    systrayID=systrayID.remove("\"").section("-",1,1).section(" ",0,0);
    qDebug() << "systray: " << systrayID;

    QString tasklistID = runCmd("grep tasklist ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
    tasklistID=tasklistID.remove("\"").section("-",1,1).section(" ",0,0);
    qDebug() << "tasklist: " << tasklistID;

    QString pulseaudioID = runCmd("grep pulseaudio ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
    pulseaudioID=pulseaudioID.remove("\"").section("-",1,1).section(" ",0,0);
    qDebug() << "pulseaudio: " << pulseaudioID;

    QString workspacesID = runCmd("grep pager ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
    workspacesID=workspacesID.remove("\"").section("-",1,1).section(" ",0,0);
    qDebug() << "workspacesID: " << workspacesID;

    // if systray exists, do a bunch of stuff to relocate it a list of plugins.  If not present, do nothing to list

    if (systrayID !=""){

        //get tasklist index in list
        int tasklistindex = pluginIDs.indexOf(tasklistID);
        qDebug() << "tasklistIDindex 1" << tasklistindex;

        //check next plugin in list to see if its an expanding separator
        int expsepindex = tasklistindex + 1;
        qDebug() << "expsepindex" << expsepindex;
        QString expsepID = pluginIDs.value(expsepindex);
        qDebug() << "expsepID to test" << expsepID;
        QString test = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + expsepID + "/expand").output;
        qDebug() << "test parm" << test;


        //move the notification area (systray) to above window buttons (tasklist) in the list if tasklist exists

        if (tasklistID !="") {
            pluginIDs.removeAll(systrayID);
            tasklistindex = pluginIDs.indexOf(tasklistID);
            qDebug() << "tasklistIDindex 2" << tasklistindex;
            pluginIDs.insert(tasklistindex, systrayID);
            qDebug() << "reordered list" << pluginIDs;

            //move the expanding separator

            if (test == "true") {
                pluginIDs.removeAll(expsepID);
                tasklistindex = pluginIDs.indexOf(tasklistID);
                qDebug() << "tasklistIDindex 2" << tasklistindex;
                pluginIDs.insert(tasklistindex, expsepID);
                qDebug() << "reordered list" << pluginIDs;
            }
        }

        //if the tasklist isn't present, try to make a decision about where to put the systray

        if (tasklistID == "") {


            //try to move to in front of clock if present

            QString clockID = runCmd("grep -m1 \"clock\\|datetime\" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
            QString switchID;
            clockID=clockID.remove("\"").section("-",1,1).section(" ",0,0);
            qDebug() << "clockID: " << clockID;
            if (clockID != "") {
                switchID = clockID;

                //if clock found check if next plugin down is a separator and if so put it there

                int clocksepindex = pluginIDs.indexOf(clockID) + 1;
                QString clocksepcheck = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + pluginIDs.value(clocksepindex)).output;
                qDebug() << "clocksepcheck: " << clocksepcheck;
                if (clocksepcheck == "separator") {
                    switchID = pluginIDs.value(clocksepindex);
                }

                // if there is no clock, put it near the end and hope for the best

            } else {
                switchID = pluginIDs.value(1);
            }

            // move the systray

            int switchIDindex;
            pluginIDs.removeAll(systrayID);
            switchIDindex = pluginIDs.indexOf(switchID) + 1;
            qDebug() << "switchIDindex 2" << switchIDindex;
            pluginIDs.insert(switchIDindex, systrayID);
            qDebug() << "reordered list" << pluginIDs;
        }

        //if pulsaudio plugin is present, move it to in front of systray
        if (pulseaudioID != "") {
            int switchIDindex;
            pluginIDs.removeAll(pulseaudioID);
            switchIDindex = pluginIDs.indexOf(systrayID) + 1;
            pluginIDs.insert(switchIDindex, pulseaudioID);
            qDebug() << "reorderd PA list" << pluginIDs;


        }
    }


    //now reverse the list

    std::reverse(pluginIDs.begin(), pluginIDs.end());
    qDebug() << "reversed list" << pluginIDs;

    //now build xfconf command

    QStringListIterator changeIterator(pluginIDs);
    QString cmdstring;
    while (changeIterator.hasNext()) {
        QString value = changeIterator.next();
        cmdstring = QString(cmdstring + "-s " + value + " ");
        qDebug() << cmdstring;
    }

    //flip the panel plugins and hold on, it could be a bumpy ride

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/plugin-ids " + cmdstring);

    //change orientation to horizontal

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/mode -s 0");

    //change mode of tasklist labels if it exists

    if (tasklistID != "") {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + tasklistID + "/show-labels -s true");
    }

    //change mode of pager if exists
    //xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspaceID + "/rows --type int --set 1"
    //check current workspaces rows
        QString workspacesrows = runCmd("xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspacesID + "/rows").output;
        if ( workspacesrows == "1" || workspacesrows == "2") {
            runCmd("xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspacesID + "/rows --type int --set 1");
        }

    //deteremine top or bottom horizontal placement
    top_or_bottom();

    runCmd("xfce4-panel --restart");
}

void defaultlook::fliptovertical()
{
    QString file_content;
    QStringList pluginIDs;
    file_content = runCmd("LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/plugin-ids | grep -v Value | grep -v ^$").output;
    pluginIDs = file_content.split("\n");
    qDebug() << pluginIDs;


    // figure out moving the systray, if it exists

    QString systrayID = runCmd("grep \\\"systray\\\" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
    systrayID=systrayID.remove("\"").section("-",1,1).section(" ",0,0);
    qDebug() << "systray: " << systrayID;

    QString tasklistID = runCmd("grep tasklist ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
    tasklistID=tasklistID.remove("\"").section("-",1,1).section(" ",0,0);
    qDebug() << "tasklist: " << tasklistID;

    QString pulseaudioID = runCmd("grep pulseaudio ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
    pulseaudioID=pulseaudioID.remove("\"").section("-",1,1).section(" ",0,0);
    qDebug() << "pulseaudio: " << pulseaudioID;

    QString workspacesID = runCmd("grep pager ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
    workspacesID=workspacesID.remove("\"").section("-",1,1).section(" ",0,0);
    qDebug() << "workspacesID: " << workspacesID;

    //if systray exists, do a bunch of stuff to try to move it in a logical way

    if (systrayID !=""){

        // figure out whiskerID, appmenuID, systrayID, tasklistID, and pagerID

        QString whiskerID = runCmd("grep whisker ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
        whiskerID=whiskerID.remove("\"").section("-",1,1).section(" ",0,0);
        qDebug() << "whisker: " << whiskerID;

        QString pagerID = runCmd("grep pager ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
        pagerID=pagerID.remove("\"").section("-",1,1).section(" ",0,0);
        qDebug() << "pager: " << pagerID;

        QString appmenuID = runCmd("grep applicationsmenu ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
        appmenuID=appmenuID.remove("\"").section("-",1,1).section(" ",0,0);
        qDebug() << "appmenuID: " << appmenuID;

        //get tasklist index in list
        int tasklistindex = pluginIDs.indexOf(tasklistID);
        qDebug() << "tasklistIDindex 1" << tasklistindex;

        //check next plugin in list to see if its an expanding separator
        int expsepindex = tasklistindex + 1;
        qDebug() << "expsepindex" << expsepindex;
        QString expsepID = pluginIDs.value(expsepindex);
        qDebug() << "expsepID to test" << expsepID;
        QString testexpandsep = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + expsepID + "/expand").output;
        qDebug() << "test parm" << testexpandsep;


        //move the notification area (systray) to an appropriate area.

        //1.  determine if menu is present, place in front of menu

        QString switchID;
        if (whiskerID != "") {
            switchID = whiskerID;
            qDebug() << "switchID whisker: " << switchID;
        } else {
            if (appmenuID != "") {
                switchID = appmenuID;
                qDebug() << "switchID appmenu: " << switchID;
            }
        }

//        //2.  if so, check second plugin is separator, if so place in front of separator

//        if (switchID != "") {
//            QString test = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + pluginIDs.value(1)).output;
//            if (test == "separator") {
//                qDebug() << "test parm" << test;
//                switchID = pluginIDs.value(1);
//                qDebug() << "switchID sep: " << switchID;
//            }
//        }

        //3.  if so, check third plugin is pager.  if so, place tasklist in front of pager

        if (switchID != ""){
            QString test = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + pluginIDs.value(1)).output;
            if (test == "pager") {
                qDebug() << "test parm" << test;
                switchID = pluginIDs.value(1);
                qDebug() << "switchID pager: " << switchID;
            }
        }

        // if the menu doesn't exist, give a default value that is sane but might not be correct

        if (switchID == "") {
            switchID = pluginIDs.value(1);
            qDebug() << "switchID default: " << switchID;
        }


        //4.  move the systray

        pluginIDs.removeAll(systrayID);
        int switchindex = pluginIDs.indexOf(switchID) + 1;
        qDebug() << "switchindex" << switchindex;
        pluginIDs.insert(switchindex, systrayID);
        qDebug() << "reordered list" << pluginIDs;


        //if pulsaudio plugin is present, move it to in front of systray
        if (pulseaudioID != "") {
            int switchIDindex;
            pluginIDs.removeAll(pulseaudioID);
            switchIDindex = pluginIDs.indexOf(systrayID) + 1;
            pluginIDs.insert(switchIDindex, pulseaudioID);
            qDebug() << "reorderd PA list" << pluginIDs;


        }
        //move the expanding separator

        if (testexpandsep == "true") {
            pluginIDs.removeAll(expsepID);
            tasklistindex = pluginIDs.indexOf(tasklistID);
            qDebug() << "tasklistIDindex 2" << tasklistindex;
            pluginIDs.insert(tasklistindex, expsepID);
            qDebug() << "reordered list" << pluginIDs;
        }
    }

    //now reverse the list

    std::reverse(pluginIDs.begin(), pluginIDs.end());
    qDebug() << "reversed list" << pluginIDs;

    //now build xfconf command

    QStringListIterator changeIterator(pluginIDs);
    QString cmdstring;
    while (changeIterator.hasNext()) {
        QString value = changeIterator.next();
        cmdstring = QString(cmdstring + "-s " + value + " ");
        qDebug() << cmdstring;
    }QString switchID;

    //flip the panel plugins and pray for a miracle


    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/plugin-ids " + cmdstring);

    //change orientation to vertical

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel + "/mode -n -t int -s 2");
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel + "/position -s 'p=5;x=0;y=0'");

    //change mode of tasklist labels if they exist

    if (tasklistID != "") {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + tasklistID + "/show-labels -s false");
    }

    //change mode of pager if exists
    //xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspaceID + "/rows --type int --set 2"
    //check current workspaces rows
        QString workspacesrows = runCmd("xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspacesID + "/rows").output;
        if ( workspacesrows == "1" || workspacesrows == "2") {
            runCmd("xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspacesID + "/rows --type int --set 2");
        }

    //determine left_or_right placement
    left_or_right();

    //restart xfce4-panel

    system("xfce4-panel --restart");
}

//// slots ////

//Apply Button
void defaultlook::on_buttonApply_clicked()
{
    ui->buttonApply->setEnabled(false);

    //backups and default panel
    if (ui->radioDefaultPanel->isChecked()) {
        restoreDefaultPanel();
        runCmd("sleep .5");
        whichpanel();
    }

    if (ui->radioRestoreBackup->isChecked()) {
        restoreBackup();
        runCmd("sleep .5");
        whichpanel();
    }

    if (ui->radioBackupPanel->isChecked()) {
        backupPanel();

    }
    //read in plugin ID's
    if (ui->checkHorz->isChecked()) {
        QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/mode").output;

        if (test == "1" || test =="2") {
            fliptohorizontal();
        }


        runCmd("sleep .5");
    }

    if (ui->checkVert->isChecked()) {
        QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/mode").output;

        if (test == "" || test =="0") {
            fliptovertical();
        }
        runCmd("sleep .5");
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
    QMessageBox msgBox(QMessageBox::NoIcon,
                       tr("About MX Tweak"), "<p align=\"center\"><b><h2>" +
                       tr("MX Tweak") + "</h2></b></p><p align=\"center\">" + tr("Version: ") + version + "</p><p align=\"center\"><h3>" +
                       tr("App for quick default ui theme changes and tweaks") +
                       "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p><p align=\"center\">" +
                       tr("Copyright (c) MX Linux") + "<br /><br /></p>", 0, this);
    msgBox.addButton(tr("License"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
    if (msgBox.exec() == QMessageBox::AcceptRole) {
        system("mx-viewer file:///usr/share/doc/mx-tweak/license.html '" + tr("MX Tweak").toUtf8() + " " + tr("License").toUtf8() + "'");
    }
    this->show();
}

// Help button clicked
void defaultlook::on_buttonHelp_clicked()
{
    QLocale locale;
    QString lang = locale.bcp47Name();

    QString url = "file:///usr/share/doc/mx-tweak/help/mx-tweak.html";

    if (lang.startsWith("fr")) {
        url = "https://mxlinux.org/wiki/help-files/help-tweak-ajustements";
    }

    QString cmd = QString("mx-viewer %1 '%2' &").arg(url).arg(tr("MX Tweak"));
    system(cmd.toUtf8());

}

void defaultlook::message()
{
    QString cmd = "ps -aux |grep -v grep|grep firefox";
    if ( system(cmd.toUtf8()) != 0 ) {
        qDebug() << "Firefox not running" ;
    } else {
        QMessageBox::information(0, tr("MX Tweak"),
                             tr("Finished! Firefox may require a restart for changes to take effect"));
    }
}

void defaultlook::checkXFCE()
{
    QString test = runCmd("echo $XDG_CURRENT_DESKTOP").output;
    qDebug() << test;
    if ( test != "XFCE") {
        QMessageBox::information(0, tr("MX Tweak"),
                                 tr("This app is Xfce-only"));
    }
}

// backs up the current panel configuration
void defaultlook::backupPanel()
{
    runCmd("rm -Rf ~/.restore; mkdir -p ~/.restore/.config/xfce4; \
           mkdir -p ~/.restore/.config/xfce4/xfconf/xfce-perchannel-xml; \
            cp -Rf ~/.config/xfce4/panel ~/.restore/.config/xfce4; \
    cp -f ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml ~/.restore/.config/xfce4/xfconf/xfce-perchannel-xml/");
}

void defaultlook::restoreDefaultPanel()
{
    // copy template files
    runCmd("xfce4-panel --quit;pkill xfconfd; rm -Rf ~/.config/xfce4/panel; cp -Rf /etc/skel/.config/xfce4/panel ~/.config/xfce4; sleep 1; \
           cp -f /etc/skel/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml; \
             sleep 5; xfce4-panel");
}

void defaultlook::restoreBackup()
{
    runCmd("xfce4-panel --quit; pkill xfconfd; rm -Rf ~/.config/xfce4/panel; cp -Rf ~/.restore/.config/xfce4/panel ~/.config/xfce4; sleep 1; \
           cp -f ~/.restore/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml ~/.config/xfce4/xfconf/xfce-perchannel-xml; \
            sleep 5; xfce4-panel ");
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
    }

}

void defaultlook::top_or_bottom()
{
    //move to user selected top or bottom border per mx-16 defaults  p=11 is top, p=12 is bottom

    QString top_bottom;
    if (ui->comboboxHorzPostition->currentIndex() == 0) {
        top_bottom = "12";
    }

    if (ui->comboboxHorzPostition->currentIndex() == 1) {
        top_bottom = "11";
    }

    qDebug() << "position index is : " << ui->comboboxHorzPostition->currentIndex();
    qDebug() << "position is :" << top_bottom;

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel + "/position -s 'p=" + top_bottom + ";x=0;y=0'");

}

void defaultlook::left_or_right()
{
    //move to user selected top or bottom border per mx-16 defaults  p=5 is left, p=1 is right

    QString left_right;
    if (ui->comboboxVertpostition->currentIndex() == 0) {
        left_right = "5";
    }

    if (ui->comboboxVertpostition->currentIndex() == 1) {
        left_right = "1";
    }

    qDebug() << "position index is : " << ui->comboboxVertpostition->currentIndex();
    qDebug() << "position is :" << left_right;

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel + "/position -s 'p=" + left_right + ";x=0;y=0'");

}

void defaultlook::message2()
{
    QMessageBox::information(0, tr("Panel settings"),
                             tr("Your current panel settings have been backed up in a hidden folder called .restore in your home folder (~/.restore/)"));
}

void defaultlook::on_toolButtonXFCEpanelSettings_clicked()
{
    this->hide();
    system("xfce4-panel --preferences");
    system("xprop -spy -name \"Panel Preferences\" >/dev/null");
    this->show();
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

void defaultlook::on_comboboxHorzPostition_currentIndexChanged(const QString &arg1)
{
    qDebug() << "top or bottom output " << ui->comboboxHorzPostition->currentText();
    QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/mode").output;
    qDebug() << "test value, blank or 0 runs left_or_right" << test;
    if (test == "") {
        top_or_bottom();
    }
    if (test == "0") {
        top_or_bottom();
    }
}


void defaultlook::on_comboboxVertpostition_currentIndexChanged(const QString &arg1)
{
    qDebug() << "left or right output " << ui->comboboxVertpostition->currentText();
    QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/mode").output;
    qDebug() << "test value, 1 or 2 runs top_or_bottom" << test;
    if (test == "1") {
        left_or_right();
    }
    if (test == "2") {
        left_or_right();
    }
}
void defaultlook::setuppanel()
{
    ui->buttonApply->setEnabled(false);
    if (ui->buttonApply->icon().isNull()) {
        ui->buttonApply->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }

    //hide tasklist setting if not present

    if ( system("grep -q tasklist ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml") != 0 ) {
        ui->labelTasklist->hide();
        ui->pushButtontasklist->hide();
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
     QString test2 = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel + "/position").output.section(";", 0,0);
    qDebug() << "test2" << test2;

    if (test == "" || test == "0") {
        ui->checkVert->setEnabled(true);
        ui->checkHorz->setChecked(true);
        if (test2 == "p=11" || test2 == "p=6" || test2 == "p=2") {
            ui->comboboxHorzPostition->setCurrentIndex(1);
        }

    }

    if (test == "1" || test == "2") {
        ui->checkVert->setChecked(true);
        ui->checkHorz->setEnabled(true);
    }

    // if backup available, make the restore backup option available

    QString cmd = QString("test -f ~/.restore/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml");
    if (system(cmd.toUtf8()) == 0) {
        ui->radioRestoreBackup->setEnabled(true);
    } else {
        ui->radioRestoreBackup->setEnabled(false);
    }

}

void defaultlook::setupEtc()
{
    QString home_path = QDir::homePath();
    ui->ButtonApplyEtc->setEnabled(false);
    if (ui->ButtonApplyEtc->icon().isNull()) {
        ui->ButtonApplyEtc->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }
    //set values for checkboxes


    //check single click status
    QString test;
    test = runCmd("xfconf-query  -c xfce4-desktop -p /desktop-icons/single-click").output;
    if ( test == "true") {
        ui->checkBoxSingleClick->setChecked(true);
    } else {
        ui->checkBoxSingleClick->setChecked(false);
    }

    //check single click thunar status

    test = runCmd("xfconf-query  -c thunar -p /misc-single-click").output;
    if ( test == "true") {
        ui->checkBoxThunarSingleClick->setChecked(true);
    } else {
        ui->checkBoxThunarSingleClick->setChecked(false);
    }

    //check systray frame status

    pluginidsystray = runCmd("cat ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml | grep \\\"systray\\\"|cut -d '=' -f2 | cut -d '' -f1| cut -d '\"' -f2").output;
    qDebug() << "systray is " << pluginidsystray;
    test = runCmd("xfconf-query -c xfce4-panel -p /plugins/" + pluginidsystray + "/show-frame").output;
    if ( test == "true") {
        ui->checkBoxSystrayFrame->setChecked(true);
    } else {
        ui->checkBoxSystrayFrame->setChecked(false);
    }

    plugintasklist = runCmd("cat ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml | grep \\\"tasklist\\\"|cut -d '=' -f2 | cut -d '' -f1| cut -d '\"' -f2").output;
    qDebug() << "tasklist is " << plugintasklist;
    test = runCmd("xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/include-all-workspaces").output;
    if ( test == "true") {
        ui->checkBoxShowAllWorkspaces->setChecked(true);
    } else {
        ui->checkBoxShowAllWorkspaces->setChecked(false);
    }

    //setup udisks option
    QFileInfo fileinfo("/etc/tweak-udisks.chk");
    if (fileinfo.exists()) {
        ui->checkBoxMountInternalDrivesNonRoot->setChecked(true);
    } else {
        ui->checkBoxMountInternalDrivesNonRoot->setChecked(false);
    }

    //setup no-ellipse option
    QFileInfo fileinfo2(home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css");
    if (fileinfo2.exists()) {
        ui->checkboxNoEllipse->setChecked(true);
    } else {
        ui->checkboxNoEllipse->setChecked(false);
    }

    //setup sudo override function
    QFileInfo sudo_override_file("/etc/polkit-1/localauthority.conf.d/55-tweak-override.conf");
    if (sudo_override_file.exists()) {
        ui->radioSudoUser->setChecked(true);
    } else {
        ui->radioSudoRoot->setChecked(true);
    }

    //setup user namespaces option (99-sandbox-mx.conf)
    QFileInfo user_namespace_override("/etc/sysctl.d/99-sandbox-mx.conf");
    if (user_namespace_override.exists()){
        ui->checkBoxSandbox->setChecked(true);
    } else {
        ui->checkBoxSandbox->setChecked(false);
}

    //setup hibernate switch
    //first, hide if running live
    test = runCmd("df -T / |tail -n1 |awk '{print $2}'").output;
    qDebug() << test;
    if ( test == "aufs" || test == "overlay" ) {
        ui->checkBoxHibernate->hide();
        ui->label_hibernate->hide();
    }

    //hide hibernate if there is no swap
    QString swaptest = runCmd("/usr/sbin/swapon --show").output;
    qDebug() << "swaptest swap present is " << swaptest;
    if (swaptest.isEmpty()) {
        ui->checkBoxHibernate->hide();
        ui->label_hibernate->hide();
    }

    // also hide hibernate if /etc/uswsusp.conf is missing
    QFileInfo file("/etc/uswsusp.conf");
    if (file.exists()) {
        qDebug() << "uswsusp.conf found";
    }else {
        ui->checkBoxHibernate->hide();
        ui->label_hibernate->hide();
    }

    //and hide hibernate if swap is encrypted
    QString cmd = "grep swap /etc/crypttab |grep -q luks";
    int swaptest2 = system(cmd.toUtf8());
    qDebug() << "swaptest encrypted is " << swaptest2;
    if (swaptest2 == 0) {
        ui->checkBoxHibernate->hide();
        ui->label_hibernate->hide();
    }

    //set checkbox
    test = runCmd("xfconf-query -c xfce4-session -p /shutdown/ShowHibernate").output;
    if ( test == "true") {
        ui->checkBoxHibernate->setChecked(true);
        hibernate_flag = true;
    } else {
        ui->checkBoxHibernate->setChecked(false);
        hibernate_flag = false;
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

    ui->checkFirefox->setChecked(false);
    ui->checkHexchat->setChecked(false);

    //only enable options that make sense

    // check theme overrides

    QString home_path = QDir::homePath();
    QFileInfo file(home_path + "/.config/FirefoxDarkThemeOverride.check");
    if (file.exists()) {
        ui->checkFirefox->setChecked(true);
    }

    //check status of hex chat dark tweak
    QFileInfo file_hexchat(home_path + "/.config/hexchat/hexchat.conf");
    if (file_hexchat.exists()) {
        //check for absolutePath()setting
       QString code = runCmd("grep 'gui_input_style = 0' " + file_hexchat.absoluteFilePath()).output;
        qDebug() << "hexchat command :" << code;
        if (code == "gui_input_style = 0") {
            ui->checkHexchat->setChecked(true);
        }
    }
}

void defaultlook::setupCompositor()
{
    //set comboboxvblank to current setting

    vblankflag = false;
    vblankinitial = runCmd("xfconf-query -c xfwm4 -p /general/vblank_mode").output;
    qDebug() << "vblank = " << vblankinitial;
    ui->comboBoxvblank->setCurrentText(vblankinitial);

    //deal with compositors

    QString cmd = "ps -aux |grep -v grep |grep -q compiz";
    if (system(cmd.toUtf8()) == 0) {
        ui->tabWidget->removeTab(2);
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
        qDebug() << "Home Path =" << home_path;
        QFileInfo file_start(home_path + "/.config/autostart/zcompton.desktop");
        //check to see if compton.desktop startup file exists
        if (file_start.exists()) {
            qDebug() << "compton startup file exists";
        } else {
            //copy in a startup file, startup initially disabled
            runCmd("cp /usr/share/mx-tweak/zcompton.desktop " + file_start.absoluteFilePath());
        }

        //check to see if existing compton.conf file
        QFileInfo file_conf(home_path + "/.config/compton.conf");
        if (file_conf.exists()) {
            qDebug() << "Found existing conf file";
        } else {
            runCmd("cp /usr/share/mx-tweak/compton.conf " + file_conf.absoluteFilePath());
        }
        CheckComptonRunning();
    }

}

void defaultlook::setupConfigoptions()
{
  ui->checkboxIntelDriver->hide();
  ui->labelIntel->hide();
  ui->checkboxAMDtearfree->hide();
  ui->labelamdgpu->hide();
  ui->checkboxRadeontearfree->hide();
  ui->labelradeon->hide();
  ui->ButtonApplyMiscDefualts->setEnabled(false);
  ui->checkBoxLightdmReset->setChecked(false);
  ui->checkBoxThunarCAReset->setChecked(false);
  Intel_flag = false;
  amdgpuflag = false;
  radeon_flag =false;
  //setup Intel checkbox

  QString partcheck = runCmd("for i in $(lspci -n | awk '{print $2,$1}' | grep -E '^(0300|0302|0380)' | cut -f2 -d\\ ); do lspci -kns \"$i\"; done").output;
  qDebug()<< "partcheck = " << partcheck;

  if ( partcheck.contains("i915")) {
      ui->checkboxIntelDriver->show();
      ui->labelIntel->show();
  }

  if ( partcheck.contains("Kernel driver in use: amdgpu")) {
      ui->checkboxAMDtearfree->show();
      ui->labelamdgpu->show();
  }

  if ( partcheck.contains("Kernel driver in use: radeon")) {
      ui->checkboxRadeontearfree->show();
      ui->labelradeon->show();
  }

  QFileInfo intelfile("/etc/X11/xorg.conf.d/20-intel.conf");
  if ( intelfile.exists()) {
      ui->checkboxIntelDriver->setChecked(true);
  }else {
      ui->checkboxIntelDriver->setChecked(false);
  }

  QFileInfo amdfile("/etc/X11/xorg.conf.d/20-amd.conf");
  if ( amdfile.exists()) {
      ui->checkboxAMDtearfree->setChecked(true);
  }else {
      ui->checkboxAMDtearfree->setChecked(false);
  }

  QFileInfo radeonfile("/etc/X11/xorg.conf.d/20-radeon.conf");
  if ( radeonfile.exists()) {
      ui->checkboxRadeontearfree->setChecked(true);
  }else {
      ui->checkboxRadeontearfree->setChecked(false);
  }
}

void defaultlook::CheckComptonRunning()
{
    //Index for combo box:  0=none, 1=xfce, 2=compton

    if ( system("ps -ax -o comm,pid |grep -w ^compton") == 0 ) {
        qDebug() << "Compton is running";
        ui->comboBoxCompositor->setCurrentIndex(2);
    } else {
        qDebug() << "Compton is NOT running";

        //check if compton is present on system, remove from choices if not
        QFileInfo compton("/usr/bin/compton");

        //adjust for picom
        if (compton.readLink() == "/usr/bin/picom" ){
            QFileInfo picom(compton.readLink());
            ui->comboBoxCompositor->setItemText(2,picom.baseName());
            ui->buttonConfigureCompton->setText(picom.baseName() + " " + tr("settings"));
        } else {
            //hide compton settings
            if ( !compton.exists() ){
                ui->comboBoxCompositor->removeItem(2);
                ui->buttonConfigureCompton->hide();
                ui->buttonEditComptonConf->hide();
            }
        }

        //check if xfce compositor is enabled
        QString test;
        test = runCmd("xfconf-query -c xfwm4 -p /general/use_compositing").output;
        qDebug() << "etc test is "<< test;
        if (test == "true") {
            ui->comboBoxCompositor->setCurrentIndex(1);
        }else{
            ui->comboBoxCompositor->setCurrentIndex(0);
        }
    }
}

void defaultlook::CheckAptNotifierRunning()
{
    if ( system("ps -aux |grep -v grep| grep python |grep --quiet apt-notifier") == 0 ) {
        qDebug() << "apt-notifier is running";
        //check if icon is supposed to be hidden by user
        if ( system("cat /home/$USER/.config/apt-notifierrc |grep --quiet DontShowIcon") == 0 ) {
            qDebug() << "apt-notifier set to hide icon, do not restart";
        } else {
            qDebug() << "unhide apt-notifier icon";
            system("/usr/bin/apt-notifier-unhide-Icon");
        }
    } else {
        qDebug() << "apt-notifier not running, do NOT restart";
    }
}

void defaultlook::setupscale()
{
    //setup scale for currently shown display and in the active profile
    QString xscale = "1";
    QString yscale = "1";
    double scale = 1;

    //get active profile
    QString activeprofile = runCmd("LANG=C xfconf-query --channel displays -p /ActiveProfile").output;
    //get scales for display show in combobox
    int exitcode = runCmd("LANG=C xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() +"/Scale/X").exitCode;
    if ( exitcode == 0 ){
        xscale = runCmd("LANG=C xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() +"/Scale/X").output;
    }
    exitcode = runCmd("LANG=C xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() +"/Scale/Y").exitCode;
    if ( exitcode == 0 ){
        yscale = runCmd("LANG=C xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() + "/Scale/Y").output;
    }
    // since we want scales equal, set the scale spin box to xscale.  invert so that 2 = .5

    scale = 1 / xscale.toDouble();
    qDebug() << "active profile is: " << activeprofile << " xscale is " << xscale << " yscale is " << yscale << " scale is: " << scale;

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
    qDebug() << "resolution is : " << resolution;
    QString scalestring = QString::number(scale, 'G', 5);
    QString activeprofile = runCmd("LANG=C xfconf-query --channel displays -p /ActiveProfile").output;

    //set missing variables
    setmissingxfconfvariables(activeprofile, resolution);

    //set scale value
    QString cmd = "xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() +"/Scale/Y -t double -s " + scalestring + " --create";
    qDebug() << "cmd is " << cmd;
    runCmd(cmd);
    cmd = "xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() +"/Scale/X -t double -s " + scalestring + " --create";
    runCmd(cmd);
    qDebug() << "cmd is " << cmd;

    //set initial scale with xrandr
    QString cmd2 = "xrandr --output " + ui->comboBoxDisplay->currentText() + " --scale " + scalestring + "x" + scalestring;
    runCmd(cmd2);
}

void defaultlook::on_buttonApplyDisplayScaling_clicked()
{
    setscale();
}

void defaultlook::on_comboBoxDisplay_currentIndexChanged(int index)
{
    setupBrightness();
    setupscale();
    setupresolutions();
    setupGamma();
}

void defaultlook::setupDisplay()
{
    //populate combobox
    QString displaydata = runCmd("LANG=C xrandr |grep -w connected | cut -d' ' -f1").output;
    QStringList displaylist = displaydata.split("\n");
    ui->comboBoxDisplay->clear();
    ui->comboBoxDisplay->addItems(displaylist);
    setupBrightness();
    setupGamma();
    setupscale();
    setupbacklight();
    setupresolutions();
    brightnessflag = true;

    //get gtk scaling value
    QString GTKScale = runCmd("LANG=C xfconf-query --channel xsettings -p /Gdk/WindowScalingFactor").output;
    ui->spinBoxgtkscaling->setValue(GTKScale.toInt());
    //disable resolution stuff
}

void defaultlook::setupresolutions()
{
    QString display = ui->comboBoxDisplay->currentText();
    ui->comboBoxresolutions->clear();
    QString cmd = "LANG=C /usr/lib/mx-tweak/mx-tweak-lib-randr.sh " + display + " resolutions";
    qDebug() << "get resolution command is :" << cmd;
    QString resolutions = runCmd(cmd).output;
    qDebug() << "resolutions are :" << resolutions;
    QStringList resolutionslist = resolutions.split("\n");
    ui->comboBoxresolutions->addItems(resolutionslist);
    //set current resolution as default
    QString resolution = runCmd("xrandr |grep " + ui->comboBoxDisplay->currentText() + " |cut -d' ' -f3 |cut -d'+' -f1").output;
    qDebug() << "resolution is : " << resolution;
    ui->comboBoxresolutions->setCurrentText(resolution);
}

void defaultlook::setresolution()
{
    QString activeprofile = runCmd("LANG=C xfconf-query --channel displays -p /ActiveProfile").output;
    QString display = ui->comboBoxDisplay->currentText();
    QString resolution = ui->comboBoxresolutions->currentText();
    QString cmd = "xrandr --output " + display + " --mode " + resolution;
    qDebug() << "resolution change command is " << cmd;
    runCmd(cmd);
    //setmissingvariables
    setmissingxfconfvariables(activeprofile, resolution);
    //set refresh rate
    setrefreshrate(display, resolution, activeprofile);
}

void defaultlook::setmissingxfconfvariables(QString activeprofile, QString resolution)
{
    //set resolution, set active, set scales, set display name

    //set display name
    runCmd("xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() + " -t string -s " + ui->comboBoxDisplay->currentText() + " --create");

    //set resolution
    runCmd("xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() + "/Resolution -t string -s " + resolution.simplified() + " --create");

    //set active profile
    runCmd("xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() + "/Active -t bool -s true --create");
}

void defaultlook::setrefreshrate(QString display, QString resolution, QString activeprofile)
{
    //set refreshrate too
    QString refreshrate = runCmd("/usr/lib/mx-tweak/mx-tweak-lib-randr.sh " + display + " refreshrate").output;
    refreshrate=refreshrate.simplified();
    QStringList refreshratelist = refreshrate.split(QRegExp("\\s"));
    refreshratelist.removeAll(resolution);
    qDebug() << "defualt refreshreate list is :" << refreshratelist.at(0).section("*",0,0);
    runCmd("xfconf-query --channel displays -p /" + activeprofile + "/" + display + "/RefreshRate -t double -s " + refreshratelist.at(0).section("*",0,0) + " --create; sleep 1");
}

void defaultlook::setupbacklight()
{
    //check for backlights
    QString test = runCmd("ls /sys/class/backlight").output;
    if ( ! test.isEmpty()) {
        //get backlight value for currently
        QString backlight=runCmd("sudo /usr/lib/mx-tweak/backlight-brightness -g").output;
        int backlight_slider_value = backlight.toInt();
        ui->horizsliderhardwarebacklight->setValue(backlight_slider_value);
        ui->horizsliderhardwarebacklight->setToolTip(backlight);
        ui->backlight_label->setText(backlight);
        qDebug() << "backlight string is " << backlight;
        qDebug() << " backlight_slider_value is " << backlight_slider_value;
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
    runCmd("xfce4-panel --restart");
}

void defaultlook::setupBrightness()
{
    //get brightness value for currently shown display
    QString brightness=runCmd("LANG=C xrandr --verbose | awk '/" + ui->comboBoxDisplay->currentText() +"/{flag=1;next}/CONNECTOR_ID/{flag=0}flag'|grep Brightness|cut -d' ' -f2").output;
    int brightness_slider_value = brightness.toFloat() * 100;
    ui->horizontalSliderBrightness->setValue(brightness_slider_value);
    qDebug() << "brightness string is " << brightness;
    qDebug() << " brightness_slider_value is " << brightness_slider_value;
    ui->horizontalSliderBrightness->setToolTip(QString::number(ui->horizontalSliderBrightness->value()));
}

void defaultlook::setupGamma()
{
    QString gamma = runCmd("/usr/lib/mx-tweak/mx-tweak-lib-randr.sh " + ui->comboBoxDisplay->currentText() + " gamma").output;
    gamma=gamma.simplified();
    gamma = gamma.section(":",1,3).simplified();
    double gamma1 = 1.0 / gamma.section(":",0,0).toDouble();
    double gamma2 = 1.0 / gamma.section(":",1,1).toDouble();
    double gamma3 = 1.0 / gamma.section(":",2,2).toDouble();
    g1 = QString::number(gamma1,'G', 3);
    g2 = QString::number(gamma2,'G', 3);
    g3 = QString::number(gamma3,'G', 3);
    qDebug() << "gamma is " << g1 << " " << g2 << " " << g3;
}

void defaultlook::on_horizontalSliderBrightness_valueChanged(int value)
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
    qDebug() << "num is :" << num;
    QString brightness = QString::number(num, 'G', 5);
    qDebug() << "changed brightness is :" << brightness;
    cmd = "xrandr --output " + ui->comboBoxDisplay->currentText() + " --brightness " + brightness + " --gamma " + g1 + ":" + g2 + ":" +g3;
    system(cmd.toUtf8());
}

void defaultlook::saveBrightness()
{
    //save cmd used in user's home file under .config
    //make directory when its not present
    double num = ui->horizontalSliderBrightness->value() / 100.0;
    qDebug() << "num is :" << num;
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
    qDebug() << "home path is " << home_path;
    bool xsettings_gtk_theme_present = false;
    bool icontheme_present = false;
    bool xfwm4_theme_present = false;
    ui->comboTheme->clear();
    QStringList theme_list;
    QStringList filter("*.tweak");
    QDirIterator it("/usr/share/mx-tweak-data", filter, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFileInfo file_info(it.next());
        QString filename = file_info.absoluteFilePath();
        QString name = runCmd("cat '" + filename + "'|grep Name=").output.section("=",1,1);
        QString xsettings_gtk_theme = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xsettings_gtk_theme=").output.section("=",1,1);
        qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
        QString xsettings_icon_theme = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xsettings_icon_theme=").output.section("=",1,1);
        qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
        QString xfwm4_window_decorations = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xfwm4_window_decorations=").output.section("=",1,1);
        qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;

        //check theme existence, only list if all 3 elements present
        QFileInfo xsettings_theme("/usr/share/themes/" + xsettings_gtk_theme);
        QFileInfo xfwm4_theme("/usr/share/themes/" + xfwm4_window_decorations);
        QFileInfo icon_theme("/usr/share/icons/" + xsettings_icon_theme);
        QFileInfo xsettings_theme_home(home_path + "/.themes/" + xsettings_gtk_theme);
        QFileInfo xfwm4_theme_home("" + home_path + "/.themes/" + xfwm4_window_decorations);
        QFileInfo icon_theme_home("" + home_path + "/.icons/" + xsettings_icon_theme);
        qDebug() << "xsettings_theme_home path" << xsettings_theme_home.absoluteFilePath();

        if (xsettings_theme.exists() || xsettings_theme_home.exists() ) {
            xsettings_gtk_theme_present = true;
        }

        if (xfwm4_theme.exists() || xfwm4_theme_home.exists()) {
            xfwm4_theme_present = true;
        }

        if (icon_theme.exists() || icon_theme_home.exists()) {
            icontheme_present = true;
        }

        if (xsettings_gtk_theme_present && xfwm4_theme_present && icontheme_present) {
            qDebug() << "filename is " << filename;
            qDebug()<< "theme combo name" << name;
            theme_list << name;
            theme_info.insert(name,filename);
            qDebug() << "theme info hash value is" << name << " " << theme_info[name];
        }
    }
    theme_list.sort();
    theme_list.insert(0,tr("Choose a theme set"));

    //add user entries in ~/.local/share/mx-tweak-data

    QDirIterator it2(home_path + "/.local/share/mx-tweak-data", filter, QDir::Files, QDirIterator::Subdirectories);
    while (it2.hasNext()) {
        QString home_path = QDir::homePath();
        QFileInfo file_info(it2.next());
        QString filename = file_info.absoluteFilePath();
        QString name = runCmd("cat '" + filename + "'|grep Name=").output.section("=",1,1);

        QString xsettings_gtk_theme = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xsettings_gtk_theme=").output.section("=",1,1);
        qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
        QString xsettings_icon_theme = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xsettings_icon_theme=").output.section("=",1,1);
        qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
        QString xfwm4_window_decorations = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xfwm4_window_decorations=").output.section("=",1,1);
        qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;

        //check theme existence, only list if all 3 elements present
        QFileInfo xsettings_theme("/usr/share/themes/" + xsettings_gtk_theme);
        QFileInfo xfwm4_theme("/usr/share/themes/" + xfwm4_window_decorations);
        QFileInfo icon_theme("/usr/share/icons/" + xsettings_icon_theme);
        QFileInfo xsettings_theme_home(home_path + "/.themes/" + xsettings_gtk_theme);
        QFileInfo xfwm4_theme_home("" + home_path + "/.themes/" + xfwm4_window_decorations);
        QFileInfo icon_theme_home("" + home_path + "/.icons/" + xsettings_icon_theme);
        qDebug() << "xsettings_theme_home path" << xsettings_theme_home.absoluteFilePath();

        if (xsettings_theme.exists() || xsettings_theme_home.exists() ) {
            xsettings_gtk_theme_present = true;
        }

        if (xfwm4_theme.exists() || xfwm4_theme_home.exists()) {
            xfwm4_theme_present = true;
        }

        if (icon_theme.exists() || icon_theme_home.exists()) {
            icontheme_present = true;
        }

        if (xsettings_gtk_theme_present && xfwm4_theme_present && icontheme_present) {
            qDebug() << "filename is " << filename;
            qDebug()<< "theme combo name" << name;
            theme_list << name;
            theme_info.insert(name,filename);
            qDebug() << "theme info hash value is" << name << " " << theme_info[name];
        }
    }

    ui->comboTheme->addItems(theme_list);
    ui->comboTheme->setCurrentIndex(0);
}

void defaultlook::on_comboTheme_activated(const QString &arg1)
{
    if (ui->comboTheme->currentIndex() != 0) {
        ui->buttonThemeApply->setEnabled(true);
        ui->pushButtonPreview->setEnabled(true);
    }
}

void defaultlook::on_buttonThemeApply_clicked()
{

    savethemeundo();
    ui->buttonThemeApply->setEnabled(false);
    ui->buttonThemeUndo->setEnabled(true);
    QString themename = theme_info[ui->comboTheme->currentText()];
    QFileInfo fileinfo(themename);
    //initialize variables
    QString backgroundColor = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep background-rgba=").output.section("=" , 1,1);
    qDebug() << "backgroundColor = " << backgroundColor;
    QString color1 = backgroundColor.section(",",0,0);
    QString color2 = backgroundColor.section(",", 1, 1);
    QString color3 = backgroundColor.section(",",2,2);
    QString color4 = backgroundColor.section(",",3,3);
    qDebug() << "sep colors" << color1 << color2 << color3 << color4;
    QString background_image = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep background-image=").output.section("=",1,1);
    qDebug() << "backgroundImage = " << background_image;
    QString background_style = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep background-style=").output.section("=",1,1);
    qDebug() << "backgroundstyle = " << background_style;
    QString xsettings_gtk_theme = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep xsettings_gtk_theme=").output.section("=",1,1);
    qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
    QString xsettings_icon_theme = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep xsettings_icon_theme=").output.section("=",1,1);
    qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
    QString xfwm4_window_decorations = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep xfwm4_window_decorations=").output.section("=",1,1);
    qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;

    //  use xfconf system to change values

    message_flag = true;

    //set gtk theme
        runCmd("xfconf-query -c xsettings -p /Net/ThemeName -s " + xsettings_gtk_theme);
        runCmd("sleep .5");

    //set window decorations theme
        runCmd("xfconf-query -c xfwm4 -p /general/theme -s " + xfwm4_window_decorations);
        runCmd("sleep .5");

    //set icon theme
        runCmd("xfconf-query -c xsettings -p /Net/IconThemeName -s " + xsettings_icon_theme);
        runCmd("sleep .5");

    //deal with panel customizations for each panel

    QStringListIterator changeIterator(panelIDs);
    while (changeIterator.hasNext()) {
        QString value = changeIterator.next();


        //set panel background mode

        if (background_style == "1" || background_style == "2" || background_style == "0") {
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

        if (backgroundColor != "") {
            runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-rgba -t double -t double -t double -t double -s " + color1 + " -s " + color2 + " -s " + color3 + " -s " + color4 + " --create");
        }
    }

    //set whisker themeing
    QString home_path = QDir::homePath();
    QFileInfo whisker_check(home_path + "/.config/gtk-3.0/gtk.css");
    if (whisker_check.exists()) {
        qDebug() << "existing gtk.css found";
        QString cmd = "cat " + home_path + "/.config/gtk-3.0/gtk.css |grep -q whisker-tweak.css";
        if (system(cmd.toUtf8()) == 0 ) {
            qDebug() << "include statement found";
        } else {
            qDebug() << "adding include statement";
            QString cmd = "echo '@import url(\"whisker-tweak.css\");' >> " + home_path + "/.config/gtk-3.0/gtk.css";
            system(cmd.toUtf8());
        }
    }else {
        qDebug() << "creating simple gtk.css file";
        QString cmd = "echo '@import url(\"whisker-tweak.css\");' >> " + home_path + "/.config/gtk-3.0/gtk.css";
        system(cmd.toUtf8());
    }

   //add whisker info
    runCmd("awk '/<begin_gtk_whisker_theme_code>/{flag=1;next}/<end_gtk_whisker_theme_code>/{flag=0}flag' \"" +fileinfo.absoluteFilePath() +"\" > " + home_path + "/.config/gtk-3.0/whisker-tweak.css");

    //restart xfce4-panel

    system("xfce4-panel --restart");

    //check theme overrides

    if (ui->checkFirefox->isChecked()) {
        runCmd("touch /home/$USER/.config/FirefoxDarkThemeOverride.check");
    } else {
        runCmd("rm /home/$USER/.config/FirefoxDarkThemeOverride.check");
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

    if (message_flag == true) {
        message();
        message_flag = false;
    }

    // reset gui
    setuptheme();
}

void defaultlook::on_ButtonApplyEtc_clicked()
{
    ui->ButtonApplyEtc->setEnabled(false);

    if (ui->checkBoxShowAllWorkspaces->isChecked()) {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/include-all-workspaces -s true");
    } else {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/include-all-workspaces -s false");
    }

    if (ui->checkBoxSingleClick->isChecked()) {
        runCmd("xfconf-query  -c xfce4-desktop -p /desktop-icons/single-click -s true");
    }else {
        runCmd("xfconf-query  -c xfce4-desktop -p /desktop-icons/single-click -s false");
    }

    if (ui->checkBoxThunarSingleClick->isChecked()) {
        runCmd("xfconf-query  -c thunar -p /misc-single-click -s true");
    } else {
        runCmd("xfconf-query  -c thunar -p /misc-single-click -s false");
    }

    if (ui->checkBoxSystrayFrame->isChecked()) {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/" + pluginidsystray + "/show-frame -s true");
    } else {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/" + pluginidsystray + "/show-frame -s false");
    }

    //deal with udisks option
    QFileInfo fileinfo("/etc/tweak-udisks.chk");
    QFileInfo sudo_override("/etc/polkit-1/localauthority.conf.d/55-tweak-override.conf");
    QFileInfo user_namespace_override("/etc/sysctl.d/99-sandbox-mx.conf");
    QString cmd;
    QString udisks_option;
    QString hibernate_option;
    QString sudo_override_option;
    QString user_name_space_override_option;
    udisks_option.clear();
    hibernate_option.clear();

    if (ui->checkBoxMountInternalDrivesNonRoot->isChecked()) {
        if (fileinfo.exists()) {
            qDebug() << "no change to internal drive mount settings";
        } else {
            udisks_option = "enable_user_mount";
        }
    } else {
        if (fileinfo.exists()) {
            udisks_option = "disable_user_mount";
        } else {
            qDebug() << "no change to internal drive mount settings";
        }
    }

    //deal with no-ellipse-filenames option
    QString home_path = QDir::homePath();
    if (ui->checkboxNoEllipse->isChecked()) {
        //set desktop themeing
        QFileInfo gtk_check(home_path + "/.config/gtk-3.0/gtk.css");
        if (gtk_check.exists()) {
            qDebug() << "existing gtk.css found";
            QString cmd = "cat " + home_path + "/.config/gtk-3.0/gtk.css |grep -q no-ellipse-desktop-filenames.css";
            if (system(cmd.toUtf8()) == 0 ) {
                qDebug() << "include statement found";
            } else {
                qDebug() << "adding include statement";
                QString cmd = "echo '@import url(\"no-ellipse-desktop-filenames.css\");' >> " + home_path + "/.config/gtk-3.0/gtk.css";
                system(cmd.toUtf8());
            }
        }else {
            qDebug() << "creating simple gtk.css file";
            QString cmd = "echo '@import url(\"no-ellipse-desktop-filenames.css\");' >> " + home_path + "/.config/gtk-3.0/gtk.css";
            system(cmd.toUtf8());
        }
        //add modification config
        runCmd("cp /usr/share/mx-tweak/no-ellipse-desktop-filenames.css " + home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css ");

        //restart xfdesktop by with xfdesktop --quite && xfdesktop &

        system("xfdesktop --quit && sleep .5 && xfdesktop &");
    }else {
        QFileInfo noellipse_check(home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css");
        if (noellipse_check.exists()) {
            runCmd("rm -f " + home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css");
            runCmd("sed -i '/no-ellipse-desktop-filenames.css/d' " + home_path + "/.config/gtk-3.0/gtk.css");
            system("xfdesktop --quit && sleep .5 && xfdesktop &");
        }
    }

    //deal with sudo override

    if (ui->radioSudoUser->isChecked()) {
        if (sudo_override.exists()) {
            qDebug() << "no change to admin password settings";
        } else {
            sudo_override_option = "enable_sudo_override";
        }
    } else {
        if (sudo_override.exists()) {
            sudo_override_option = "disable_sudo_override";
        } else {
            qDebug() << "no change to admin password settings";
        }
    }

    //deal with user namespace override
    if (ui->checkBoxSandbox->isChecked()){
        if (user_namespace_override.exists()) {
            qDebug() << "no change to user namespace override";
        } else {
            user_name_space_override_option = "enable_sandbox";
        }
    } else {
        if (user_namespace_override.exists()){
            user_name_space_override_option = "disable_sandbox";
        } else {
            qDebug() << "no change to user namespace override";
        }
    }

    //deal with hibernate
    if (ui->checkBoxHibernate->isChecked() != hibernate_flag) {
        if (ui->checkBoxHibernate->isChecked()) {
            hibernate_option =  "hibernate";
            system("xfconf-query -c xfce4-session -p /shutdown/ShowHibernate -s true --create");
        } else {
            system("xfconf-query -c xfce4-session -p /shutdown/ShowHibernate -s false --create");
        }
    }


    if ( ! hibernate_option.isEmpty() || ! udisks_option.isEmpty() || ! sudo_override_option.isEmpty() || ! user_name_space_override_option.isEmpty()) {
        if ( hibernate_option.isEmpty()) {
            runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh " + udisks_option + " " + sudo_override_option + " " + user_name_space_override_option);
        } else {
            runCmd("x-terminal-emulator -e 'pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh " + udisks_option + " " + sudo_override_option + + " " + user_name_space_override_option + " " + hibernate_option + "'");
        }
    }

    //reset gui
    setupEtc();
}

void defaultlook::on_checkBoxSingleClick_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_checkBoxThunarSingleClick_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_checkBoxSystrayFrame_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_checkboxNoEllipse_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::savethemeundo()
{
    QString home_path = QDir::homePath();
    system("rm -f undo.txt");
    QString undovalue;
    QStringListIterator changeIterator(panelIDs);
    QString undocommand;
    QString whiskeriterator = QString::number(undotheme.count());
    qDebug() << "whisker interator is " << whiskeriterator;

    while (changeIterator.hasNext()) {
        QString value = changeIterator.next();


        //backup panel background mode
        if (runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style").exitCode == 0) {
            undovalue = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style").output;
            //runCmd("echo xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style -s " + undovalue + " >> undo.txt");
            undocommand = undocommand + "xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style -s "+ undovalue + " ; ";
        }else{
            //runCmd("echo xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style -r >> undo.txt");
            undocommand = undocommand + "xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-style -r ; ";
        }

        //backup panel background image

        if (runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image").exitCode == 0) {
            undovalue = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image").output;
            //runCmd("echo xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image -s " + undovalue + " >> undo.txt");
            undocommand = undocommand + "xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image -s " + undovalue +" ; ";
        }else{
            //runCmd("echo xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image -r >> undo.txt");
            undocommand = undocommand + " xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-image -r ; ";
        }

        //backup panel color

        if (runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color").exitCode == 0) {
            undovalue = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color |cut -d ':' -f2").output.trimmed();
            undovalue.replace("\n", ",");
            qDebug() << "backup color test" << undovalue;
            QString color1 = undovalue.section(",",0,0);
            QString color2 = undovalue.section(",", 1, 1);
            QString color3 = undovalue.section(",",2,2);
            QString color4 = undovalue.section(",",3,3);
            qDebug() << "sep colors" << color1 << color2 << color3 << color4;
            //runCmd("echo xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color -s " + color1 + " -s " + color2 + " -s " + color3 + " -s " + color4 + " >> undo.txt");
            undocommand = undocommand + "xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color -s " + color1 + " -s " + color2 + " -s " + color3 + " -s " + color4 + " ; ";
        } else {
            //runCmd("echo xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color -r >> undo.txt");
            undocommand = undocommand + "xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color -r ; ";
        }
    }

    //backup theme settings for xfwm4 and xsettings

    // gtk theme
    if (runCmd("xfconf-query -c xsettings -p /Net/ThemeName").exitCode == 0) {
        undovalue = runCmd("xfconf-query -c xsettings -p /Net/ThemeName").output;
        //runCmd("echo xfconf-query -c xsettings -p /Net/ThemeName -s " + undovalue + " >> undo.txt");
        undocommand = undocommand + "xfconf-query -c xsettings -p /Net/ThemeName -s " + undovalue + " ; ";
    }else{
        //runCmd("echo xfconf-query -c xsettings -p /Net/ThemeName -r >> undo.txt");
        undocommand = undocommand + "xfconf-query -c xsettings -p /Net/ThemeName -r ; ";
    }

    // xfwm4 theme
    if (runCmd("xfconf-query -c xfwm4 -p /general/theme").exitCode == 0) {
        undovalue = runCmd("xfconf-query -c xfwm4 -p /general/theme").output;
        //runCmd("echo xfconf-query -c xfwm4 -p /general/theme -s " + undovalue + " >> undo.txt");
        undocommand = undocommand + "xfconf-query -c xfwm4 -p /general/theme -s " + undovalue + " ; ";
    }else{
        runCmd("echo xfconf-query -c xfwm4 -p /general/theme -r >> undo.txt");
        undocommand = undocommand + "xfconf-query -c xfwm4 -p /general/theme -r ; ";
    }

    // icon theme
    if (runCmd("xfconf-query -c xsettings -p /Net/IconThemeName").exitCode == 0) {
        undovalue = runCmd("xfconf-query -c xsettings -p /Net/IconThemeName").output;
        //runCmd("echo xfconf-query -c xsettings -p /Net/IconThemeName -s " + undovalue + " >> undo.txt");
        undocommand = undocommand + "xfconf-query -c xsettings -p /Net/IconThemeName -s " + undovalue + " ; ";
    }else{
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
qDebug() << "undo command is " << undocommand;

undotheme << undocommand;

qDebug () << "undo command list is " << undotheme;

}

void defaultlook::themeundo()
{
   QString cmd = undotheme.last();
   system(cmd.toUtf8());
   undotheme.removeLast();
}

void defaultlook::on_buttonThemeUndo_clicked()
{
    themeundo();
    runCmd("xfce4-panel --restart");
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
        runCmd("xfconf-query -c xfwm4 -p /general/use_compositing -s false");
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
        runCmd("xfconf-query -c xfwm4 -p /general/use_compositing -s true");
        //restart apt-notifier if necessary
        CheckAptNotifierRunning();
    }
    if (ui->comboBoxCompositor->currentIndex() == 0) {
        //turn off compton and xfce compositor
        //turn off xfce compositor
        runCmd("xfconf-query -c xfwm4 -p /general/use_compositing -s false");
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
    qDebug() << "autostart set to " << runCmd("grep Hidden= " + file_start.absoluteFilePath()).output;

    //deal with vblank setting
    if ( vblankflag ) {
        runCmd("xfconf-query -c xfwm4 -p /general/vblank_mode -t string -s " + ui->comboBoxvblank->currentText() + " --create");
        //restart xfwm4 to take advantage of the setting
        runCmd("xfwm4 --replace");
    }
}


void defaultlook::on_buttonEditComptonConf_clicked()
{
    QString home_path = QDir::homePath();
        QFileInfo file_conf(home_path + "/.config/compton.conf");
        runCmd("xdg-open " + file_conf.absoluteFilePath());
}

void defaultlook::on_comboBoxCompositor_currentIndexChanged(const QString &arg1)
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
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_checkBoxMountInternalDrivesNonRoot_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}



void defaultlook::on_pushButtonPreview_clicked()
{
    QString themename = theme_info[ui->comboTheme->currentText()];
    QFileInfo fileinfo(themename);

    //initialize variables
    QString file_name = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep screenshot=").output.section("=" , 1,1);
    QString path = fileinfo.absolutePath();
    QString full_file_path = path + "/" + file_name;

    QMessageBox preview_box(QMessageBox::NoIcon, file_name, "" , QMessageBox::Close, this);
    preview_box.setIconPixmap(QPixmap(full_file_path));
    preview_box.exec();

}

void defaultlook::on_checkBoxHibernate_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
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
    QString intel_option;
    QString amd_option;
    QString radeon_option;
    QString lightdm_option;

    intel_option.clear();
    lightdm_option.clear();

    if (ui->checkBoxThunarCAReset->isChecked()) {
        cmd = "cp /home/$USER/.config/Thunar/uca.xml /home/$USER/.config/Thunar/uca.xml.$(date +%Y%m%H%M%S)";
        system(cmd.toUtf8());
        runCmd("cp /etc/skel/.config/Thunar/uca.xml /home/$USER/.config/Thunar/uca.xml");
    }

    if (ui->checkBoxLightdmReset->isChecked()) {
        lightdm_option = "lightdm_reset";
    }

    if ( Intel_flag ) {
        QFileInfo check_intel("/etc/X11/xorg.conf.d/20-intel.conf");
        if ( check_intel.exists()){
            //backup existing 20-intel.conf file to home folder
            cmd = "cp /etc/X11/xorg.conf.d/20-intel.conf /home/$USER/20-intel.conf.$(date +%Y%m%H%M%S)";
            system(cmd.toUtf8());
        }
        if (ui->checkboxIntelDriver->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            intel_option = "enable_intel";
        } else {
            //remove 20-intel.conf
            intel_option = "disable_intel";
        }
    }

    if ( amdgpuflag ) {
        QFileInfo check_amd("/etc/X11/xorg.conf.d/20-amd.conf");
        if ( check_amd.exists()){
            //backup existing 20-amd.conf file to home folder
            cmd = "cp /etc/X11/xorg.conf.d/20-amd.conf /home/$USER/20-amd.conf.$(date +%Y%m%H%M%S)";
            system(cmd.toUtf8());
        }
        if (ui->checkboxAMDtearfree->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            amd_option = "enable_amd";
        } else {
            //remove 20-amd.conf
            amd_option = "disable_amd";
        }
    }

    if ( radeon_flag ) {
        QFileInfo check_radeon("/etc/X11/xorg.conf.d/20-radeon.conf");
        if ( check_radeon.exists()){
            //backup existing 20-radeon.conf file to home folder
            cmd = "cp /etc/X11/xorg.conf.d/20-radeon.conf /home/$USER/20-radeon.conf.$(date +%Y%m%H%M%S)";
            system(cmd.toUtf8());
        }
        if (ui->checkboxRadeontearfree->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            radeon_option = "enable_radeon";
        } else {
            //remove 20-radeon.conf
            radeon_option = "disable_radeon";
        }
    }
    if ( ! intel_option.isEmpty() || ! lightdm_option.isEmpty() || ! amd_option.isEmpty() || ! radeon_option.isEmpty() ) {
    cmd = "pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh " + intel_option + " " + amd_option + " " + radeon_option + " " + lightdm_option;
    qDebug() << "cmd is " << cmd;
    system(cmd.toUtf8());
    }


    setupConfigoptions();
}

void defaultlook::on_checkBoxLightdmReset_clicked()
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
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}

void defaultlook::on_checkboxAMDtearfree_clicked()
{
    //toggle flag for action.  this way, if box was checked initially, the action won't take place again.
    amdgpuflag = true;
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}

void defaultlook::on_checkboxRadeontearfree_clicked()
{
    //toggle flag for action.  this way, if box was checked initially, the action won't take place again.
    radeon_flag = true;
    ui->ButtonApplyMiscDefualts->setEnabled(true);
}

void defaultlook::on_pushButtontasklist_clicked()
{
    window_buttons fred;
    fred.setModal(true);
    fred.exec();
}

// Get version of the program
QString defaultlook::getVersion(QString name)
{
    return runCmd("dpkg-query -f '${Version}' -W " + name).output;
}

void defaultlook::on_pushButtonSettingsToThemeSet_clicked()
{
    QString fileName;
    theming_to_tweak* dialog = new theming_to_tweak;
    int userInput = dialog->exec();
    if(userInput == QDialog::Rejected)
        return;
    fileName = dialog->nameEditor()->text();

    //declared locally to prevent an issues with other code
    auto pathAppend = [](const QString& path1, const QString& path2)
    {
        return QDir::cleanPath(path1 + QDir::separator() + path2);
    };

    QString panel;
    QString data = runCmd("xfconf-query -c xfce4-panel -p /panels --list").output;
    int panelNum;
    for(panelNum = 1;; panelNum++)
    {
        if(data.contains("panel-" + QString::number(panelNum)))
            break;
    }
    panel = "panel-" + QString::number(panelNum);

    int backgroundStyle;
    data = runCmd("xfconf-query -c xfce4-panel -p /panels/" + panel + "/background-style").output;
    backgroundStyle = data.toInt(); //there may be newlines in output but qt ignores it

    QVector<double> backgroundColor;
    QString backgroundImage;
    if(backgroundStyle == 1)
    {
        QStringList lines = runCmd("LANG=C xfconf-query -c xfce4-panel -p /panels/" + panel + "/background-rgba").output.split('\n');
        lines.removeAt(0);
        lines.removeAt(0);
        for(int i = 0; i < 4; i++)
        {
            backgroundColor << lines.at(i).toDouble();
        }
    }
    else if(backgroundStyle == 2)
    {
        backgroundImage = runCmd("xfconf-query -c /panels/" + panel + "/background-image").output;
    }

    QString iconThemeName = runCmd("xfconf-query -c xsettings -p /Net/IconThemeName").output;
    QString themeName = runCmd("xfconf-query -c xsettings -p /Net/ThemeName").output;
    QString windowDecorationsTheme = runCmd("xfconf-query -c xfwm4 -p /general/theme").output;

    QString whiskerThemeFileName = pathAppend(QDir::homePath(), ".config/gtk-3.0/whisker-tweak.css");
    QFile whiskerThemeFile(whiskerThemeFileName);
    if(!whiskerThemeFile.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "Failed to fetch whisker theming from: " + whiskerThemeFileName;
    }
    QTextStream whiskerThemeFileStream(&whiskerThemeFile);
    QString whiskerThemeData = whiskerThemeFileStream.readAll();
    whiskerThemeFile.close();

    QStringList fileLines;
    fileLines << "Name=" + fileName;
    fileLines << "background-style=" + QString::number(backgroundStyle);
    if(backgroundStyle == 1)
    {
        QString line;
        for(double num : backgroundColor)
        {
            line.append(QString::number(num) + ',');
        }
        if(line.endsWith(',')) line.chop(1);
        fileLines << "background-rgba=" + line;
    }
    else
    {
        fileLines << "background-rgba=none";
    }
    if(backgroundStyle == 2)
    {
        fileLines << "background-image=" + backgroundImage;
    }
    else
    {
        fileLines << "background-image=none";
    }
    fileLines << "xsettings_gtk_theme=" + themeName;
    fileLines << "xsettings_icon_theme=" + iconThemeName;
    fileLines << "xfwm4_window_decorations=" + windowDecorationsTheme;
    fileLines << "<begin_gtk_whisker_theme_code>";
    for(QString line : whiskerThemeData.split('\n'))
    {
        fileLines << line;
    }
    fileLines << "<end_gtk_whisker_theme_code>";
    QFile file(pathAppend(QDir::homePath(), ".local/share/mx-tweak-data/" + fileName + ".tweak"));
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug() << "Failed to open file for reading: " + fileName;
        return;
    }
    QTextStream fileStream(&file);
    for(QString line : fileLines)
    {
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
    if(result != QDialog::Accepted)
        return;
    QString theme = dialog.themeSelector()->currentText();
    if(theme == "Select User Theme Set to Remove")
        return;
    result = QMessageBox::warning(this, "Remove User Theme Set", "Are you sure you want to remove " + theme + " theme set?", QMessageBox::Ok | QMessageBox::Cancel);
    if(result != QMessageBox::Ok)
        return;
    QString file = dialog.getFilename(theme);
    file.replace(' ', "\\ ");
    auto cmd = runCmd("rm " + file);
    if(cmd.exitCode != 0)
    {
        qDebug() << "Removing theme set failed: exitCode: " << cmd.exitCode << " | output: " << cmd.output;
        return;
    }
    //refresh
    setupComboTheme();
}

void defaultlook::on_comboBoxvblank_activated(const QString &arg1)
{
    if ( vblankinitial == ui->comboBoxvblank->currentText()){
        vblankflag = false;
    } else {
        vblankflag = true;
    }
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

void defaultlook::on_horizsliderhardwarebacklight_actionTriggered(int action)
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
}
