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

#include "xfwm_compositor_settings.h"

defaultlook::defaultlook(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::defaultlook)
{
    ui->setupUi(this);
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
    checkXFCE();
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

    // figure out systrayID, and tasklistID

    QString systrayID = runCmd("grep \\\"systray\\\" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
    systrayID=systrayID.remove("\"").section("-",1,1).section(" ",0,0);
    qDebug() << "systray: " << systrayID;

    QString tasklistID = runCmd("grep tasklist ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
    tasklistID=tasklistID.remove("\"").section("-",1,1).section(" ",0,0);
    qDebug() << "tasklist: " << tasklistID;

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

            QString clockID = runCmd("grep clock ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml").output;
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

        //2.  if so, check second plugin is separator, if so place in front of separator

        if (switchID != "") {
            QString test = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + pluginIDs.value(1)).output;
            if (test == "separator") {
                qDebug() << "test parm" << test;
                switchID = pluginIDs.value(1);
                qDebug() << "switchID sep: " << switchID;
            }
        }

        //3.  if so, check third plugin is pager.  if so, place tasklist in front of pager

        if (switchID != ""){
            QString test = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + pluginIDs.value(2)).output;
            if (test == "pager") {
                qDebug() << "test parm" << test;
                switchID = pluginIDs.value(2);
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
    }

    //flip the panel plugins and pray for a miracle


    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel +"/plugin-ids " + cmdstring);

    //change orientation to vertical

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel + "/mode -n -t int -s 2");
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + panel + "/position -s 'p=5;x=0;y=0'");

    //change mode of tasklist labels if they exist

    if (tasklistID != "") {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + tasklistID + "/show-labels -s false");
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

    QString url = "https://mxlinux.org/wiki/help-files/help-mx-tweak";

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
        qApp->quit();
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
                             tr(" Your current panel settings have been backed up in a hidden folder called .restore in your home folder (~/.restore/)"));
}

void defaultlook::on_toolButtonXFCEpanelSettings_clicked()
{
    this->hide();
    system("xfce4-panel --preferences");
    system("xprop -spy -name Panel >/dev/null");
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
    QFileInfo fileinfo2(home_path + "/.local/share/mx-tweak-data/no-ellipse-desktop-filenames.rc");
    if (fileinfo2.exists()) {
        ui->checkboxNoEllipse->setChecked(true);
    } else {
        ui->checkboxNoEllipse->setChecked(false);
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
    QString cmd = "/sbin/blkid |grep -q swap";
    int swaptest = system(cmd.toUtf8());
    qDebug () << "swaptest is " << swaptest;
    if (swaptest != 0) {
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
  ui->ButtonApplyMiscDefualts->setEnabled(false);
  ui->checkBoxLightdmReset->setChecked(false);
  ui->checkBoxThunarCAReset->setChecked(false);

  //setup Intel checkbox

  if ( system("lspci |grep VGA |grep -q Intel ") == 0) {
      ui->checkboxIntelDriver->show();
      ui->labelIntel->show();
  }

  QFileInfo intelfile("/etc/X11/xorg.conf.d/20-intel.conf");
  if ( intelfile.exists()) {
      ui->checkboxIntelDriver->setChecked(true);
      Intel_flag = false ;//set false if box is checked by default.  we will set true if manually checked.
  }else {
      ui->checkboxIntelDriver->setChecked(false);
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


void defaultlook::setupComboTheme()
{
    //build theme list
    ui->comboTheme->clear();
    QStringList theme_list;
    QStringList filter("*.tweak");
    QDirIterator it("/usr/share/mx-tweak-data", filter, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFileInfo file_info(it.next());
        QString filename = file_info.absoluteFilePath();
        QString name = runCmd("cat '" + filename + "'|grep Name=").output.section("=",1,1);
        qDebug() << "filename is " << filename;
        qDebug()<< "theme combo name" << name;
        theme_list << name;
        theme_info.insert(name,filename);
        qDebug() << "theme info hash value is" << name << " " << theme_info[name];
    }
    theme_list.sort();
    theme_list.insert(0,tr("Choose a theme set"));

    //add user entries in ~/.local/share/mx-tweak-data

    QString home_path = QDir::homePath();
    QDirIterator it2(home_path + "/.local/share/mx-tweak-data", filter, QDir::Files, QDirIterator::Subdirectories);
    while (it2.hasNext()) {
        QFileInfo file_info(it2.next());
        QString filename = file_info.absoluteFilePath();
        QString name = runCmd("cat '" + filename + "'|grep Name=").output.section("=",1,1);
        qDebug() << "filename is " << filename;
        qDebug()<< "theme combo name" << name;
        theme_list << name;
        theme_info.insert(name,filename);
        qDebug() << "theme info hash value is" << name << " " << theme_info[name];
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
    QString backgroundColor = runCmd("cat '" + fileinfo.absoluteFilePath() + "' |grep background-color=").output.section("=" , 1,1);
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
    QFileInfo xsettings_theme("/usr/share/themes/" + xsettings_gtk_theme);
    if (xsettings_theme.exists()) {
        runCmd("xfconf-query -c xsettings -p /Net/ThemeName -s " + xsettings_gtk_theme);
        runCmd("sleep .5");
    }
    //set window decorations theme
    QFileInfo xfwm4_theme("/usr/share/themes/" + xfwm4_window_decorations);
    if (xfwm4_theme.exists()) {
        runCmd("xfconf-query -c xfwm4 -p /general/theme -s " + xfwm4_window_decorations);
        runCmd("sleep .5");
    }
    //set icon theme
    QFileInfo icon_theme("/usr/share/icons/" + xsettings_icon_theme);
    if (icon_theme.exists()) {
        runCmd("xfconf-query -c xsettings -p /Net/IconThemeName -s " + xsettings_icon_theme);
        runCmd("sleep .5");
    }

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
            runCmd("xfconf-query -c xfce4-panel -p /panels/panel-" + value + "/background-color -t uint -t uint -t uint -t uint -s " + color1 + " -s " + color2 + " -s " + color3 + " -s " + color4 + " --create");
        }



    }

    //set whisker themeing
    QString home_path = QDir::homePath();
    QFileInfo whisker_check(home_path + "/.gtkrc-2.0");
    if (whisker_check.exists()) {
        qDebug() << "existing gtkrc-2.0 found";
        //fix problem with original tweak setup
        runCmd("sed -i '/[[:space:]].local\\/share\\/mx-tweak-data\\/whisker-tweak.rc/d' " + whisker_check.absoluteFilePath());
        QString cmd = "cat " + home_path + "/.gtkrc-2.0 |grep -q whisker-tweak.rc";
        if (system(cmd.toUtf8()) == 0 ) {
            qDebug() << "include statement found";
        } else {
            qDebug() << "adding include statement";
            QString cmd = "echo 'include \".local/share/mx-tweak-data/whisker-tweak.rc\"' >> " + home_path + "/.gtkrc-2.0";
            system(cmd.toUtf8());
        }
    }else {
        qDebug() << "creating simple gtkrc-2.0 file";
        QString cmd = "echo 'include \".local/share/mx-tweak-data/whisker-tweak.rc\"' >> " + home_path + "/.gtkrc-2.0";
        system(cmd.toUtf8());
    }

   //add whisker info
    runCmd("awk '/<begin_gtk_whisker_theme_code>/{flag=1;next}/<end_gtk_whisker_theme_code>/{flag=0}flag' \"" +fileinfo.absoluteFilePath() +"\" > " + home_path + "/.local/share/mx-tweak-data/whisker-tweak.rc");

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
    QString cmd;
    if (ui->checkBoxMountInternalDrivesNonRoot->isChecked()) {
        if (fileinfo.exists()) {
            qDebug() << "no change to internal drive mount settings";
        } else {
            if (fileinfo.absoluteDir().exists()) {
            cmd = "gksu 'cp /usr/share/mx-tweak/50-udisks.pkla /etc/polkit-1/localauthority/50-local.d/50-udisks.pkla ;touch /etc/tweak-udisks.chk'";
            system(cmd.toUtf8());
            } else {
            cmd = "gksu 'mkdir -p /etc/polkit-1/localauthority/50-local.d ;cp /usr/share/mx-tweak/50-udisks.pkla /etc/polkit-1/localauthority/50-local.d/50-udisks.pkla ;touch /etc/tweak-udisks.chk'";
            system(cmd.toUtf8());
            }
        }
    } else {
        if (fileinfo.exists()) {
        cmd = "gksu 'rm -f /etc/polkit-1/localauthority/50-local.d/50-udisks.pkla; rm -f /etc/tweak-udisks.chk'";
        system(cmd.toUtf8());
        } else {
            qDebug() << "no change to internal drive mount settings";
        }
    }

    //deal with no-ellipse-filenames option
    QString home_path = QDir::homePath();
    if (ui->checkboxNoEllipse->isChecked()) {
        //set desktop themeing
        QFileInfo gtk_check(home_path + "/.gtkrc-2.0");
        if (gtk_check.exists()) {
            qDebug() << "existing gtkrc-2.0 found";
            QString cmd = "cat " + home_path + "/.gtkrc-2.0 |grep -q no-ellipse-desktop-filenames.rc";
            if (system(cmd.toUtf8()) == 0 ) {
                qDebug() << "include statement found";
            } else {
                qDebug() << "adding include statement";
                QString cmd = "echo 'include \".local/share/mx-tweak-data/no-ellipse-desktop-filenames.rc\"' >> " + home_path + "/.gtkrc-2.0";
                system(cmd.toUtf8());
            }
        }else {
            qDebug() << "creating simple gtkrc-2.0 file";
            QString cmd = "echo 'include \"/.local/share/mx-tweak-data/no-ellipse-desktop-filenames.rc\"' >> " + home_path + "/.gtkrc-2.0";
            system(cmd.toUtf8());
        }
        //add modification config
        runCmd("cp /usr/share/mx-tweak/no-ellipse-desktop-filenames.rc " + home_path + "/.local/share/mx-tweak-data/no-ellipse-desktop-filenames.rc ");

        //restart xfdesktop by with xfdesktop --quite && xfdesktop &

        system("xfdesktop --quit && sleep .5 && xfdesktop &");
    }else {
        QFileInfo noellipse_check(home_path + "/.local/share/mx-tweak-data/no-ellipse-desktop-filenames.rc");
        if (noellipse_check.exists()) {
            runCmd("rm -f " + home_path + "/.local/share/mx-tweak-data/no-ellipse-desktop-filenames.rc");
            system("xfdesktop --quit && sleep .5 && xfdesktop &");
        }
    }

    //deal with hibernate
    if (ui->checkBoxHibernate->isChecked() != hibernate_flag) {
        if (ui->checkBoxHibernate->isChecked()) {
            runCmd("gksu 'x-terminal-emulator -e tweak-update-initramfs.sh'");
            system("xfconf-query -c xfce4-session -p /shutdown/ShowHibernate -s true --create");
        } else {
            system("xfconf-query -c xfce4-session -p /shutdown/ShowHibernate -s false --create");
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
    // if whisker-tweak.rc exists, back it up
    QFileInfo fileinfo(home_path + "/.local/share/mx-tweak-data/whisker-tweak.rc");
    if (fileinfo.exists()) {
        runCmd("cp " + home_path + "/.local/share/mx-tweak-data/whisker-tweak.rc /tmp/whisker-tweak.rc.undo." + whiskeriterator);
        //runCmd("echo cp " + home_path + "/.local/share/mx-tweak-data/whisker-tweak.rc.undo " + fileinfo.absoluteFilePath() + " >> undo.txt");
        undocommand = undocommand + "cp /tmp/whisker-tweak.rc.undo." + whiskeriterator + " " + fileinfo.absoluteFilePath();
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

void defaultlook::on_ButtonApplyMiscDefualts_clicked()
{
    if (ui->checkBoxThunarCAReset->isChecked()) {
        QString cmd = "cp /home/$USER/.config/Thunar/uca.xml /home/$USER/.config/Thunar/uca.xml.$(date +%Y%m%H%M%S)";
        system(cmd.toUtf8());
        runCmd("cp /etc/skel/.config/Thunar/uca.xml /home/$USER/.config/Thunar/uca.xml");
    }

    if (ui->checkBoxLightdmReset->isChecked()) {
        QString cmd = "gnome-keyring-daemon -r -d";
        system(cmd.toUtf8());
        cmd = "gksu 'cp /etc/lightdm/lightdm-gtk-greeter.conf /etc/lightdm/lightdm-gtk-greeter.conf.$(date +%Y%m%H%M%S); cp /etc/lightdm/mx$(lsb_release -rs)/lightdm-gtk-greeter.conf /etc/lightdm/lightdm-gtk-greeter.conf'";
        system(cmd.toUtf8());
    }

    if ( Intel_flag ) {
        QFileInfo check_intel("/etc/X11/xorg.conf.d/20-intel.conf");
        QString cmd;
        if ( check_intel.exists()){
            //backup existing 20-intel.conf file to home folder
            cmd = "cp /etc/X11/xorg.conf.d/20-intel.conf /home/$USER/20-intel.conf.$(date +%Y%m%H%M%S)";
            system(cmd.toUtf8());
        }
        if (ui->checkboxIntelDriver->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            cmd = "gksu 'cp /usr/share/mx-tweak/20-intel.conf /etc/X11/xorg.conf.d/20-intel.conf'";
            system(cmd.toUtf8());
        } else {
            //remove 20-intel.conf
            cmd = "gksu 'rm /etc/X11/xorg.conf.d/20-intel.conf'";
            system(cmd.toUtf8());
        }
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
