/**********************************************************************
 *  panel_manager.cpp
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

#include "panel_manager.h"
#include "cmd.h"
#include "ui_defaultlook.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegularExpression>

using namespace Qt::Literals::StringLiterals;

PanelManager::PanelManager(Ui::defaultlook *ui, bool verbose, QDialog *parent)
    : BaseManager(ui, verbose, parent)
{
}

void PanelManager::setup()
{
    QString home_path = QDir::homePath();
    QFileInfo backuppanel(home_path + "/.restore/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_L1);
    if (backuppanel.exists()) {
        migratePanel(backuppanel.lastModified().toString(u"dd.MM.yyyy.hh.mm.ss"_s));
    }

    setupPanelBackupRestore();
    setupPanelOrientation();
    setupTasklistPlugin();

    emit setupCompleted();
}

void PanelManager::setupPanelBackupRestore()
{
    QString home_path = QDir::homePath();

    if (QFileInfo::exists(home_path + "/.config/gtk-3.0/gtk.css"_L1)) {
        if (verbose) {
            qDebug() << "existing gtk.css found";
        }
        QString cmd = "cat "_L1 + home_path + "/.config/gtk-3.0/gtk.css |grep -q xfce4-panel-tweaks.css"_L1;
        if (system(cmd.toUtf8()) == 0) {
            if (verbose) {
                qDebug() << "include statement found";
            }
        } else {
            if (verbose) {
                qDebug() << "adding include statement";
            }
            QString cmd
                = "echo '@import url(\"xfce4-panel-tweaks.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1;
            system(cmd.toUtf8());
        }
    } else {
        if (verbose) {
            qDebug() << "creating simple gtk.css file";
        }
        QString cmd
            = "echo '@import url(\"xfce4-panel-tweaks.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1;
        system(cmd.toUtf8());
    }

    if (!QFileInfo::exists(home_path + "/.config/gtk-3.0/xfce4-panel-tweaks.css"_L1)) {
        QString cmd = "cp /usr/share/mx-tweak/xfce4-panel-tweaks.css "_L1 + home_path + "/.config/gtk-3.0/"_L1;
        system(cmd.toUtf8());
    }
}

void PanelManager::setupPanelOrientation()
{
    QString home_path = QDir::homePath();
    QString plugins
        = runCmd("grep plugin "_L1 + home_path + "/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_L1).output;

    if (plugins.contains("pulseaudio"_L1)) {
        ui->doubleSpinBoxpaplugin->setValue(
            runCmd("grep -A 1 pulseaudio "_L1 + home_path
                   + "/.config/gtk-3.0/xfce4-panel-tweaks.css |grep scale |cut -d'(' -f2 |cut -d')' -f1"_L1)
                .output.toDouble());
    } else {
        ui->doubleSpinBoxpaplugin->hide();
        ui->Label_Volume_plugin->hide();
    }

    if (plugins.contains("power-manager-plugin"_L1)) {
        ui->doubleSpinBoxpmplugin->setValue(
            runCmd("grep -A 1 xfce4-power-manager-plugin "_L1 + home_path
                   + "/.config/gtk-3.0/xfce4-panel-tweaks.css |grep scale |cut -d'(' -f2 |cut -d')' -f1"_L1)
                .output.toDouble());
    } else {
        ui->doubleSpinBoxpmplugin->hide();
        ui->Label_power_manager_plugin->hide();
    }
}

void PanelManager::setupTasklistPlugin()
{
    QString home_path = QDir::homePath();
    QString testTasklist
        = runCmd("grep tasklist "_L1 + home_path + "/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_L1)
              .output;
    QString testDocklike
        = runCmd("grep docklike "_L1 + home_path + "/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_L1)
              .output;

    if (!testTasklist.isEmpty()) {
        ui->comboBoxTasklistPlugin->setCurrentIndex(1);
    } else if (!testDocklike.isEmpty()) {
        ui->comboBoxTasklistPlugin->setCurrentIndex(0);
    }

    ui->radioButtonTasklist->setEnabled(false);
    ui->radioButtonSetPanelPluginScales->setEnabled(false);
    ui->doubleSpinBoxpaplugin->setEnabled(false);
    ui->doubleSpinBoxpmplugin->setEnabled(false);
    ui->Label_Volume_plugin->setEnabled(false);
    ui->Label_power_manager_plugin->setEnabled(false);
}

void PanelManager::whichPanel()
{
    QString panel_content
        = runCmd(u"LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels | grep -v Value | grep -v ^$"_s).output;
    panelIDs = panel_content.split(u"\n"_s);
    panel = panelIDs.value(0);
    if (verbose) {
        qDebug() << "panels found: " << panelIDs;
    }
    if (verbose) {
        qDebug() << "panel to use: " << panel;
    }
}

void PanelManager::flipToHorizontal()
{
    QString file_content = runCmd("LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel
                                  + "/plugin-ids | grep -v Value | grep -v ^$"_L1)
                               .output;
    QStringList pluginIDs = file_content.split(u"\n"_s);
    if (verbose) {
        qDebug() << pluginIDs;
    }

    QString systrayID
        = runCmd(uR"(grep \"systray\" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml)"_s).output;
    systrayID = systrayID.remove(u"\""_s).section(u"-"_s, 1, 1).section(u" "_s, 0, 0);
    if (verbose) {
        qDebug() << "systray: " << systrayID;
    }

    QString tasklistID = getTasklistId();
    QString pulseaudioID
        = runCmd(u"grep pulseaudio ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
    pulseaudioID = pulseaudioID.remove(u"\""_s).section(u"-"_s, 1, 1).section(u" "_s, 0, 0);
    if (verbose) {
        qDebug() << "pulseaudio: " << pulseaudioID;
    }

    if (!systrayID.isEmpty()) {
        int tasklistindex = pluginIDs.indexOf(tasklistID);
        if (verbose) {
            qDebug() << "tasklistIDindex 1" << tasklistindex;
        }

        int expsepindex = tasklistindex + 1;
        if (verbose) {
            qDebug() << "expsepindex" << expsepindex;
        }
        QString expsepID = pluginIDs.value(expsepindex);
        if (verbose) {
            qDebug() << "expsepID to test" << expsepID;
        }
        QString test = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + expsepID + "/expand"_L1).output;
        if (verbose) {
            qDebug() << "test parm" << test;
        }

        if (!tasklistID.isEmpty()) {
            pluginIDs.removeAll(systrayID);
            tasklistindex = pluginIDs.indexOf(tasklistID);
            if (verbose) {
                qDebug() << "tasklistIDindex 2" << tasklistindex;
            }
            pluginIDs.insert(tasklistindex, systrayID);
            if (verbose) {
                qDebug() << "new plugin list" << pluginIDs;
            }

            runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/plugin-ids -a -t int "_L1
                   + pluginIDs.join(" -t int "_L1));
        }
    }

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/mode -t int -s 0"_L1);
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/position -t string -s 'p=6;x=683;y=738'"_L1);
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/position-locked -t bool -s true"_L1);
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/size -t int -s 30"_L1);
    runCmd(u"xfce4-panel --restart"_s);
}

void PanelManager::flipToVertical()
{
    QString file_content = runCmd("LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel
                                  + "/plugin-ids | grep -v Value | grep -v ^$"_L1)
                               .output;
    QStringList pluginIDs = file_content.split(u"\n"_s);
    if (verbose) {
        qDebug() << pluginIDs;
    }

    QString systrayID
        = runCmd(uR"(grep \"systray\" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml)"_s).output;
    systrayID = systrayID.remove(u"\""_s).section(u"-"_s, 1, 1).section(u" "_s, 0, 0);
    if (verbose) {
        qDebug() << "systray: " << systrayID;
    }

    QString tasklistID = getTasklistId();

    if (!systrayID.isEmpty()) {
        QString whiskerID = runCmd(u"grep whisker ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
        whiskerID = whiskerID.remove(u"\""_s).section(u"-"_s, 1, 1).section(u" "_s, 0, 0);
        if (verbose) {
            qDebug() << "whisker: " << whiskerID;
        }

        int tasklistindex = pluginIDs.indexOf(tasklistID);
        if (verbose) {
            qDebug() << "tasklistIDindex 1" << tasklistindex;
        }

        if (!tasklistID.isEmpty()) {
            pluginIDs.removeAll(systrayID);
            int lastindex = pluginIDs.size();
            pluginIDs.insert(lastindex, systrayID);
            if (verbose) {
                qDebug() << "new plugin list" << pluginIDs;
            }

            runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/plugin-ids -a -t int "_L1
                   + pluginIDs.join(" -t int "_L1));
        }
    }

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/mode -t int -s 1"_L1);
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/position -t string -s 'p=5;x=1896;y=384'"_L1);
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/position-locked -t bool -s true"_L1);
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/size -t int -s 45"_L1);
    runCmd(u"xfce4-panel --restart"_s);
}

void PanelManager::topOrBottom()
{
    if (ui->comboboxHorzPostition->currentIndex() == 0) {
        runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel
               + "/position -t string -s 'p=8;x=683;y=0'"_L1);
    } else {
        runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel
               + "/position -t string -s 'p=10;x=683;y=738'"_L1);
    }
    runCmd(u"xfce4-panel --restart"_s);
}

void PanelManager::leftOrRight()
{
    if (ui->comboboxVertpostition->currentIndex() == 0) {
        runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel
               + "/position -t string -s 'p=3;x=0;y=384'"_L1);
    } else {
        runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel
               + "/position -t string -s 'p=5;x=1896;y=384'"_L1);
    }
    runCmd(u"xfce4-panel --restart"_s);
}

void PanelManager::backupPanel()
{
    QString home_path = QDir::homePath();
    if (!QDir(home_path + "/.restore/"_L1).exists()) {
        runCmd("mkdir -p "_L1 + home_path + "/.restore/"_L1);
    }

    QRegularExpression rx(u"\\$|@|%|\\&|\\*|\\(|\\)|\\[|\\]|\\{|\\}|\\||\\?"_s);
    QRegularExpressionMatch match = rx.match(ui->lineEditBackupName->text());
    int rxtest = match.hasMatch() ? match.capturedStart(0) : -1;

    if (rxtest > 0) {
        QMessageBox::information(nullptr, tr("MX Tweak"),
                                 tr("Please remove special characters") + "@,$,%,&,*,(,),[,],{,},|,\\,?"_L1
                                     + tr("from file name"));
        validateFlag = true;
    } else {
        QString path = home_path + "/.restore/"_L1 + ui->lineEditBackupName->text() + ".tar.xz"_L1;

        if (QFileInfo::exists(path)) {
            QMessageBox::information(nullptr, tr("MX Tweak"), tr("File name already exists. Choose another name"));
        } else {
            path = "$HOME/.restore/\""_L1 + ui->lineEditBackupName->text() + ".tar.xz\""_L1;
            runCmd("tar --create --xz --file="_L1 + path
                   + " --directory=$HOME/.config/xfce4 panel xfconf/xfce-perchannel-xml/xfce4-panel.xml"_L1);
        }
    }
}

void PanelManager::restoreDefaultPanel()
{
    runCmd(
        u"xfce4-panel --quit;pkill xfconfd; rm -Rf ~/.config/xfce4/panel; cp -Rf /etc/skel/.config/xfce4/panel ~/.config/xfce4; sleep 1; \
           cp -f /etc/skel/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml; \
           sleep 5; xfce4-panel &"_s);
}

void PanelManager::restoreBackup()
{
    switch (validateArchive("$HOME/.restore/\""_L1 + ui->comboBoxAvailableBackups->currentText() + ".tar.xz\""_L1)) {
    case 0:
        runCmd(
            "xfce4-panel --quit; pkill xfconfd; rm -Rf ~/.config/xfce4/panel ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml; "_L1
            "tar -xf $HOME/.restore/\""_L1
            + ui->comboBoxAvailableBackups->currentText()
            + ".tar.xz\" --directory=$HOME/.config/xfce4; "_L1
              "sleep 5; xfce4-panel &"_L1);
        break;
    case 1:
        QMessageBox::information(nullptr, tr("MX Tweak"), tr("File is not a valid tar.xz archive file"));
        break;
    case 2:
        QMessageBox::information(nullptr, tr("MX Tweak"), tr("Archive does not contain a panel config"));
        break;
    }
}

void PanelManager::migratePanel(const QString &date) const
{
    runCmd("tar --create --xz --file=\"$HOME/.restore/panel_backup_"_L1 + date
           + ".tar.xz\" --directory=$HOME/.restore/.config/xfce4 panel xfconf/xfce-perchannel-xml/xfce4-panel.xml"_L1);
    runCmd(u"rm -R $HOME/.restore/.config/xfce4/panel $HOME/.restore/.config/xfce4/xfconf"_s);
}

int PanelManager::validateArchive(const QString &path) const
{
    QString test = runCmd("file --mime-type --brief "_L1 + path).output;
    if (verbose) {
        qDebug() << test;
    }

    if (test != "application/x-xz"_L1) {
        return 1;
    }

    test = runCmd("tar --list --file "_L1 + path).output;
    if (verbose) {
        qDebug() << test;
    }
    if (!test.contains("xfconf/xfce-perchannel-xml/xfce4-panel.xml"_L1)) {
        return 2;
    }
    return 0;
}

QString PanelManager::getTasklistId()
{
    QString tasklistID
        = runCmd(u"grep -m 1 tasklist ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
    tasklistID = tasklistID.remove(u"\""_s).section(u"-"_s, 1, 1).section(u" "_s, 0, 0);
    if (verbose) {
        qDebug() << "tasklist: " << tasklistID;
    }

    if (tasklistID.isEmpty()) {
        QString docklikeID
            = runCmd(u"grep -m 1 docklike ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
        docklikeID = docklikeID.remove(u"\""_s).section(u"-"_s, 1, 1).section(u" "_s, 0, 0);
        if (verbose) {
            qDebug() << "docklikeID: " << docklikeID;
        }
        if (!docklikeID.isEmpty()) {
            tasklistID = docklikeID;
            if (verbose) {
                qDebug() << "new tasklist: " << tasklistID;
            }
        }
    }
    return tasklistID;
}

void PanelManager::tasklistChange()
{
    QString tasklistchoice;
    if (ui->comboBoxTasklistPlugin->currentIndex() == 0) {
        tasklistchoice = "docklike"_L1;
    } else if (ui->comboBoxTasklistPlugin->currentIndex() == 1) {
        tasklistchoice = "tasklist"_L1;
    }

    if (verbose) {
        qDebug() << "tasklistchoice is" << tasklistchoice;
    }
    QString tasklistid = getTasklistId();
    if (verbose) {
        qDebug() << "tasklistid is " << tasklistid;
    }

    if (tasklistchoice == "docklike"_L1) {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistid + "/show-handle --reset"_L1);
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistid + "/show-labels --reset"_L1);
    } else if (tasklistchoice == "tasklist"_L1) {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistid
               + "/show-handle -t bool -s false --create"_L1);
        QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/mode"_L1).output;
        if (test.isEmpty() || test == "0"_L1) {
            runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistid
                   + "/show-labels -t bool -s true --create"_L1);
        } else {
            runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistid
                   + "/show-labels -t bool -s false --create"_L1);
        }
    }

    runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistid + " -t string -s "_L1 + tasklistchoice
           + " --create"_L1);
    runCmd(u"xfce4-panel --restart"_s);
    runCmd(u"sleep .5"_s);
}

// Slot implementations
void PanelManager::onApplyButtonClicked()
{
    if (ui->checkHorz->isChecked()) {
        flipToHorizontal();
        topOrBottom();
    }
    if (ui->checkVert->isChecked()) {
        flipToVertical();
        leftOrRight();
    }
    if (ui->radioDefaultPanel->isChecked()) {
        restoreDefaultPanel();
    }
    if (ui->radioBackupPanel->isChecked()) {
        backupPanel();
    }
    if (ui->radioRestoreBackup->isChecked()) {
        restoreBackup();
    }
    if (ui->radioButtonTasklist->isChecked()) {
        tasklistChange();
    }
    if (ui->radioButtonSetPanelPluginScales->isChecked()) {
        runCmd("sed -i '/pulseaudio/,/\\}/ s/scale(.*)/scale("_L1 + QString::number(ui->doubleSpinBoxpaplugin->value())
               + ")/' ~/.config/gtk-3.0/xfce4-panel-tweaks.css"_L1);
        runCmd(u"xfce4-panel --restart"_s);
        setup();
    }
    if (!validateFlag) {
        setup();
    }
}

void PanelManager::onCheckHorzClicked()
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

void PanelManager::onCheckVertClicked()
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

void PanelManager::onRadioDefaultPanelClicked()
{
    ui->buttonApply->setEnabled(true);
    ui->checkHorz->setChecked(false);
    ui->checkVert->setChecked(false);
    ui->radioBackupPanel->setChecked(false);
    ui->radioRestoreBackup->setChecked(false);
    ui->radioButtonTasklist->setChecked(false);
    ui->radioButtonSetPanelPluginScales->setChecked(false);
    ui->doubleSpinBoxpaplugin->setEnabled(false);
    ui->doubleSpinBoxpmplugin->setEnabled(false);
    ui->Label_Volume_plugin->setEnabled(false);
    ui->Label_power_manager_plugin->setEnabled(false);
}

void PanelManager::onRadioBackupPanelClicked()
{
    ui->buttonApply->setEnabled(true);
    ui->checkHorz->setChecked(false);
    ui->checkVert->setChecked(false);
    ui->radioDefaultPanel->setChecked(false);
    ui->radioRestoreBackup->setChecked(false);
    ui->radioButtonTasklist->setChecked(false);
    ui->radioButtonSetPanelPluginScales->setChecked(false);
    ui->doubleSpinBoxpaplugin->setEnabled(false);
    ui->doubleSpinBoxpmplugin->setEnabled(false);
    ui->Label_Volume_plugin->setEnabled(false);
    ui->Label_power_manager_plugin->setEnabled(false);
}

void PanelManager::onRadioRestoreBackupClicked()
{
    ui->buttonApply->setEnabled(true);
    ui->checkHorz->setChecked(false);
    ui->checkVert->setChecked(false);
    ui->radioDefaultPanel->setChecked(false);
    ui->radioBackupPanel->setChecked(false);
    ui->radioButtonTasklist->setChecked(false);
    ui->radioButtonSetPanelPluginScales->setChecked(false);
    ui->doubleSpinBoxpaplugin->setEnabled(false);
    ui->doubleSpinBoxpmplugin->setEnabled(false);
    ui->Label_Volume_plugin->setEnabled(false);
    ui->Label_power_manager_plugin->setEnabled(false);
}

void PanelManager::onRadioButtonTasklistClicked()
{
    ui->buttonApply->setEnabled(true);
    ui->checkHorz->setChecked(false);
    ui->checkVert->setChecked(false);
    ui->radioDefaultPanel->setChecked(false);
    ui->radioBackupPanel->setChecked(false);
    ui->radioRestoreBackup->setChecked(false);
    ui->radioButtonSetPanelPluginScales->setChecked(false);
    ui->doubleSpinBoxpaplugin->setEnabled(false);
    ui->doubleSpinBoxpmplugin->setEnabled(false);
    ui->Label_Volume_plugin->setEnabled(false);
    ui->Label_power_manager_plugin->setEnabled(false);
    tasklistFlag = true;
}

void PanelManager::onRadioButtonSetPanelPluginScalesClicked()
{
    ui->buttonApply->setEnabled(true);
    ui->checkHorz->setChecked(false);
    ui->checkVert->setChecked(false);
    ui->radioDefaultPanel->setChecked(false);
    ui->radioBackupPanel->setChecked(false);
    ui->radioRestoreBackup->setChecked(false);
    ui->radioButtonTasklist->setChecked(false);
    ui->doubleSpinBoxpaplugin->setEnabled(true);
    ui->doubleSpinBoxpmplugin->setEnabled(true);
    ui->Label_Volume_plugin->setEnabled(true);
    ui->Label_power_manager_plugin->setEnabled(true);
}

void PanelManager::onComboboxHorzPositionChanged(int /*index*/)
{
    if (ui->checkHorz->isChecked()) {
        topOrBottom();
    }
}

void PanelManager::onComboboxVertPositionChanged(int /*index*/)
{
    if (ui->checkVert->isChecked()) {
        leftOrRight();
    }
}

void PanelManager::onComboBoxTasklistPluginChanged(int /*index*/)
{
    if (tasklistFlag) {
        tasklistChange();
        tasklistFlag = false;
        ui->radioButtonTasklist->setChecked(false);
        ui->buttonApply->setEnabled(false);
    }
}

void PanelManager::onLineEditBackupNameReturnPressed()
{
    ui->buttonApply->setDefault(true);
}
