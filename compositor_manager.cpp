/**********************************************************************
 *  compositor_manager.cpp
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

#include "compositor_manager.h"
#include "cmd.h"
#include "ui_defaultlook.h"
#include "xfwm_compositor_settings.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QIcon>

using namespace Qt::Literals::StringLiterals;

CompositorManager::CompositorManager(Ui::defaultlook *ui, bool verbose, QDialog *parent)
    : BaseManager(ui, verbose, parent)
{
}

void CompositorManager::setup()
{
    setupVblankSettings();
    setupCompositorButtons();

    QString cmd = u"ps -aux |grep -v grep |grep -q compiz"_s;
    if (system(cmd.toUtf8()) != 0) {
        checkComptonRunning();
        setupFlag = true;
    }

    emit setupCompleted();
}

void CompositorManager::setupVblankSettings()
{
    vblankFlag = false;
    vblankInitial = runCmd(u"xfconf-query -c xfwm4 -p /general/vblank_mode"_s).output;
    if (verbose) {
        qDebug() << "vblank = " << vblankInitial;
    }
    ui->comboBoxvblank->setCurrentText(vblankInitial);
}

void CompositorManager::setupCompositorButtons()
{
    ui->buttonCompositorApply->setEnabled(false);
    if (ui->buttonCompositorApply->icon().isNull()) {
        ui->buttonCompositorApply->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }
    ui->buttonConfigureCompton->setEnabled(false);
    ui->buttonConfigureXfwm->setEnabled(false);
    ui->buttonEditComptonConf->setEnabled(false);

    QString home_path = QDir::homePath();
    if (verbose) {
        qDebug() << "Home Path =" << home_path;
    }

    QFileInfo file_start(home_path + "/.config/autostart/zpicom.desktop"_L1);
    if (file_start.exists()) {
        if (verbose) {
            qDebug() << "picom startup file exists";
        }
    } else {
        if (verbose) {
            qDebug() << "picom startup file does not exist";
        }
    }
}

void CompositorManager::checkComptonRunning()
{
    // Index for combo box: 0=none, 1=xfce, 2=picom (formerly compton)
    if (system("ps -ax -o comm,pid |grep -w ^picom") == 0) {
        if (verbose) {
            qDebug() << "picom is running";
        }
        ui->comboBoxCompositor->setCurrentIndex(2);
    } else {
        if (verbose) {
            qDebug() << "picom is NOT running";
        }

        QFileInfo picom(u"/usr/bin/picom"_s);
        if (!picom.exists()) {
            ui->comboBoxCompositor->removeItem(2);
            ui->buttonConfigureCompton->hide();
            ui->buttonEditComptonConf->hide();
        }
    }

    QString test = runCmd(u"xfconf-query -c xfwm4 -p /general/use_compositing"_s).output;
    if (verbose) {
        qDebug() << "xfce compositor test is " << test;
    }

    if (test == "true"_L1) {
        ui->comboBoxCompositor->setCurrentIndex(1);
        ui->buttonConfigureXfwm->setEnabled(true);
    } else {
        ui->comboBoxCompositor->setCurrentIndex(0);
    }
}

void CompositorManager::checkAptNotifierRunning() const
{
    QString cmd = u"ps -aux |grep -v grep |grep -q apt-notifier"_s;
    if (system(cmd.toUtf8()) == 0) {
        system("pkill apt-notifier; sleep 1; /usr/bin/nohup /usr/bin/apt-notifier.py &");
    }
}

void CompositorManager::enableCompositorByIndex(int index)
{
    switch (index) {
    case 0: // None
        runCmd(u"xfconf-query -c xfwm4 -p /general/use_compositing -s false"_s);
        system("pkill -x picom");
        checkAptNotifierRunning();
        break;

    case 1: // XFCE
        system("pkill -x picom");
        runCmd(u"xfconf-query -c xfwm4 -p /general/use_compositing -s true"_s);
        checkAptNotifierRunning();
        break;

    case 2: // Picom
        runCmd(u"xfconf-query -c xfwm4 -p /general/use_compositing -s false"_s);
        system("pkill -x picom");
        system("picom-launch.sh");
        checkAptNotifierRunning();
        break;
    }
}

// Slot implementations
void CompositorManager::onButtonCompositorApplyClicked()
{
    ui->buttonCompositorApply->setEnabled(false);
    enableCompositorByIndex(ui->comboBoxCompositor->currentIndex());
}

void CompositorManager::onButtonConfigureComptonClicked()
{
    parent->hide();
    system("picom-conf");
    parent->show();
}

void CompositorManager::onButtonConfigureXfwmClicked()
{
    xfwm_compositor_settings dialog;
    dialog.setModal(true);
    dialog.exec();
}

void CompositorManager::onButtonEditComptonConfClicked()
{
    QString cmd = u"featherpad ~/.config/picom.conf &"_s;
    system(cmd.toUtf8());
}

void CompositorManager::onComboBoxCompositorCurrentIndexChanged(int index)
{
    if (setupFlag) {
        ui->buttonCompositorApply->setEnabled(true);

        switch (index) {
        case 0: // None
            ui->buttonConfigureCompton->setEnabled(false);
            ui->buttonConfigureXfwm->setEnabled(false);
            ui->buttonEditComptonConf->setEnabled(false);
            break;

        case 1: // XFCE
            ui->buttonConfigureCompton->setEnabled(false);
            ui->buttonConfigureXfwm->setEnabled(true);
            ui->buttonEditComptonConf->setEnabled(false);
            break;

        case 2: // Picom
            ui->buttonConfigureCompton->setEnabled(true);
            ui->buttonConfigureXfwm->setEnabled(false);
            ui->buttonEditComptonConf->setEnabled(true);
            break;
        }
    }
}

void CompositorManager::onComboBoxVblankActivated(int index)
{
    vblankFlag = true;

    QString vblank_value;
    switch (index) {
    case 0:
        vblank_value = "off"_L1;
        break;
    case 1:
        vblank_value = "auto"_L1;
        break;
    case 2:
        vblank_value = "xpresent"_L1;
        break;
    case 3:
        vblank_value = "glx"_L1;
        break;
    }

    runCmd("xfconf-query -c xfwm4 -p /general/vblank_mode -s "_L1 + vblank_value);

    if (verbose) {
        qDebug() << "vblank set to" << vblank_value;
    }
}
