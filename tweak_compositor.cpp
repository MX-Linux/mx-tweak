#include <QMessageBox>
#include <QDir>
#include <QFile>
#include "ui_defaultlook.h"
#include "cmd.h"
#include "window_buttons.h"
#include "xfwm_compositor_settings.h"
#include "tweak_compositor.h"

using namespace Qt::Literals::StringLiterals;

TweakCompositor::TweakCompositor(Ui::defaultlook *ui, bool verbose, QObject *parent) noexcept
    : QObject{parent}, ui{ui}, verbose{verbose}
{
    setup();
    connect(ui->comboCompositor, &QComboBox::currentIndexChanged, this, &TweakCompositor::comboCompositor_currentIndexChanged);
    connect(ui->comboCompositorVBlank, &QComboBox::activated, this, &TweakCompositor::comboCompositorVBlank_activated);
    connect(ui->pushCompositorXfwmSettings, &QPushButton::clicked, this, &TweakCompositor::pushCompositorXfwmSettings_clicked);
    connect(ui->pushCompositorPicomSettings, &QPushButton::clicked, this, &TweakCompositor::pushCompositorPicomSettings_clicked);
    connect(ui->pushCompositorEditPicomConf, &QPushButton::clicked, this, &TweakCompositor::pushCompositorEditPicomConf_clicked);
    connect(ui->pushCompositorApply, &QPushButton::clicked, this, &TweakCompositor::pushCompositorApply_clicked);
}

bool TweakCompositor::check() noexcept
{
    return (runCmd("ps -aux |grep -v grep |grep -q compiz").exitCode != 0);
}
void TweakCompositor::setup() noexcept
{
    //set comboCompositorVBlank to current setting

    flags.vblank = false;
    initVBlank = runCmd(u"xfconf-query -c xfwm4 -p /general/vblank_mode"_s).output;
    if (verbose) qDebug() << "vblank = " << initVBlank;
    ui->comboCompositorVBlank->setCurrentText(initVBlank);

    ui->pushCompositorApply->setEnabled(false);
    if (ui->pushCompositorApply->icon().isNull()) {
        ui->pushCompositorApply->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }
    ui->pushCompositorPicomSettings->setEnabled(false);
    ui->pushCompositorXfwmSettings->setEnabled(false);
    ui->pushCompositorEditPicomConf->setEnabled(false);

    // check to see if compton is enabled
    QString home_path = QDir::homePath();
    if (verbose) qDebug() << "Home Path =" << home_path;
    QFileInfo file_start(home_path + "/.config/autostart/zpicom.desktop"_L1);
    //check to see if picom.desktop startup file exists
    if (file_start.exists()) {
        if (verbose) qDebug() << "picom startup file exists";
    } else {
        //copy in a startup file, startup initially disabled
        runCmd("cp /usr/share/mx-tweak/zpicom.desktop "_L1 + file_start.absoluteFilePath());
    }

    //check to see if existing picom.conf file
    QFileInfo file_conf(home_path + "/.config/picom.conf"_L1);
    if (file_conf.exists()) {
        if (verbose) qDebug() << "Found existing conf file";
    } else {
        runCmd("cp /usr/share/mx-tweak/picom.conf "_L1 + file_conf.absoluteFilePath());
    }

    ui->comboCompositor->addItem(tr("None"));
    ui->comboCompositor->addItem(tr("Xfwm (Xfce) Compositor"), u"xfwm"_s);
    ui->comboCompositor->addItem(tr("Picom"), u"picom"_s);
    checkPicomRunning();
}

void TweakCompositor::checkPicomRunning() noexcept
{
    //Index for combo box:  0=none, 1=xfce, 2=picom (formerly compton)

    if (runCmd(u"ps -ax -o comm,pid |grep -w ^picom"_s).exitCode == 0) {
        if (verbose) qDebug() << "picom is running";
        const int i = ui->comboCompositor->findData(u"picom"_s);
        ui->comboCompositor->setCurrentIndex(i);
    } else {
        if (verbose) qDebug() << "picom is NOT running";

        //check if picom is present on system, remove from choices if not
        if (!QFileInfo::exists(u"/usr/bin/picom"_s)) {
            const int i = ui->comboCompositor->findData(u"picom"_s);
            ui->comboCompositor->removeItem(i);
            ui->pushCompositorPicomSettings->hide();
            ui->pushCompositorEditPicomConf->hide();
        }
    }

    //check if xfce compositor is enabled
    const QString &test = runCmd(u"xfconf-query -c xfwm4 -p /general/use_compositing"_s).output;
    if (verbose) qDebug() << "etc test is "<< test;
    int compIndex = ui->comboCompositor->findData(u"xfwm"_s);
    if (test == "true"_L1) {
        ui->pushCompositorXfwmSettings->setEnabled(true);
    } else {
        ui->comboCompositor->removeItem(compIndex);
        compIndex = ui->comboCompositor->findData(QVariant());
    }
    ui->comboCompositor->setCurrentIndex(compIndex);
}

void TweakCompositor::checkAptNotifierRunning() const noexcept
{
    if (runCmd("ps -aux |grep -v grep| grep python |grep --quiet apt-notifier").exitCode == 0) {
        if (verbose) qDebug() << "apt-notifier is running";
        //check if icon is supposed to be hidden by user
        if (runCmd(u"cat /home/$USER/.config/apt-notifierrc |grep --quiet DontShowIcon"_s).exitCode == 0) {
            if (verbose) qDebug() << "apt-notifier set to hide icon, do not restart";
        } else {
            if (verbose) qDebug() << "unhide apt-notifier icon";
            runCmd(u"/usr/bin/apt-notifier-unhide-Icon"_s);
        }
    } else {
        if (verbose) qDebug() << "apt-notifier not running, do NOT restart";
    }
}

void TweakCompositor::comboCompositor_currentIndexChanged(const int) noexcept
{
    if (!ui->comboCompositor->currentData().isValid()) {
        ui->pushCompositorPicomSettings->setEnabled(false);
        ui->pushCompositorXfwmSettings->setEnabled(false);
        ui->pushCompositorEditPicomConf->setEnabled(false);
    } else if (ui->comboCompositor->currentData() == "xfwm"_L1) {
        ui->pushCompositorPicomSettings->setEnabled(false);
        ui->pushCompositorXfwmSettings->setEnabled(true);
        ui->pushCompositorEditPicomConf->setEnabled(false);
    } else if (ui->comboCompositor->currentData() == "picom"_L1) {
        ui->pushCompositorPicomSettings->setEnabled(true);
        ui->pushCompositorXfwmSettings->setEnabled(false);
        ui->pushCompositorEditPicomConf->setEnabled(true);
    }
    ui->pushCompositorApply->setEnabled(true);
}

void TweakCompositor::comboCompositorVBlank_activated(int) noexcept
{
    flags.vblank = initVBlank != ui->comboCompositorVBlank->currentText();
    ui->pushCompositorApply->setEnabled(true);
}

void TweakCompositor::pushCompositorApply_clicked() noexcept
{
    //disable apply button
    ui->pushCompositorApply->setEnabled(false);

    if (ui->comboCompositor->currentData() == "picom"_L1) {
        //turn off xfce compositor
        runCmd(u"xfconf-query -c xfwm4 -p /general/use_compositing -s false"_s);
        //launch picom
        runProc(u"pkill"_s, {u"-x"_s, u"picom"_s});
        runCmd(u"picom-launch.sh"_s);
        //restart apt-notifier if necessary
        checkAptNotifierRunning();
    } else if (ui->comboCompositor->currentData() == "xfwm"_L1) {
        //turn off picom
        runProc(u"pkill"_s, {u"-x"_s, u"picom"_s});
        //launch xfce compositor
        runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s,
            u"/general/use_compositing"_s, u"-s"_s, u"true"_s});
        //restart apt-notifier if necessary
        checkAptNotifierRunning();
    } else {
        //turn off picom and xfce compositor
        //turn off xfce compositor
        runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s,
            u"/general/use_compositing"_s, u"-s"_s, u"false"_s});
        runProc(u"pkill"_s, {u"-x"_s, u"picom"_s});
        checkAptNotifierRunning();
    }

    //figure out whether to autostart picom or not
    //if picom is configured in the combo box, then enable.  otherwise disable

    QString home_path = QDir::homePath();
    QFileInfo file_start(home_path + "/.config/autostart/zpicom.desktop"_L1);
    if (ui->comboCompositor->currentIndex() == 2) {
        runCmd("sed -i -r s/Hidden=.*/Hidden=false/ "_L1 + file_start.absoluteFilePath());
    } else {
        runCmd("sed -i -r s/Hidden=.*/Hidden=true/ "_L1 + file_start.absoluteFilePath());
    }
    if (verbose) qDebug() << "autostart set to " << runCmd("grep Hidden= "_L1 + file_start.absoluteFilePath()).output;

    //deal with vblank setting
    if ( flags.vblank ) {
        runCmd("xfconf-query -c xfwm4 -p /general/vblank_mode -t string -s "_L1 + ui->comboCompositorVBlank->currentText() + " --create"_L1);
        //restart xfwm4 to take advantage of the setting
        runCmd(u"xfwm4 --replace"_s);
    }
}

void TweakCompositor::pushCompositorXfwmSettings_clicked() noexcept
{
    xfwm_compositor_settings fred;
    fred.setModal(true);
    fred.exec();
}

void TweakCompositor::pushCompositorPicomSettings_clicked() noexcept
{
    ui->tabWidget->setEnabled(false);
    runProc(u"picom-conf"_s);
    ui->tabWidget->setEnabled(true);
}
void TweakCompositor::pushCompositorEditPicomConf_clicked() noexcept
{
    QString home_path = QDir::homePath();
    QFileInfo file_conf(home_path + "/.config/picom.conf"_L1);
    runCmd("xdg-open "_L1 + file_conf.absoluteFilePath());
}
