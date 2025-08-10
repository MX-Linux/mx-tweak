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

#include <cassert>
#include <utility>
#include <QDebug>
#include <QDir>
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
#include <QRegularExpression>
#include <QSettings>

#include "about.h"
#include "cmd.h"
#include "tweak_plasma.h"
#include "tweak_xfce.h"
#include "tweak_fluxbox.h"
#include "defaultlook.h"
#include "remove_user_theme_set.h"
#include "theming_to_tweak.h"
#include "ui_defaultlook.h"
#include "version.h"
#include "xfwm_compositor_settings.h"

using namespace Qt::Literals::StringLiterals;

defaultlook::defaultlook(QWidget *parent, const QStringList &args) :
    QDialog(parent),
    ui(new Ui::defaultlook)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    // check session type
    checkSession();
    if ( args.contains(u"--display"_s)) {
        if (isXfce) {
            displayflag = true;
        } else {
            QMessageBox::information(nullptr, tr("MX Tweak"),
                                     tr("--display switch only valid for Xfce"));
        }
    }

    if (args.contains(u"--theme"_s)){
        themetabflag = true;
    }

    if (args.contains(u"--other"_s)){
        othertabflag = true;
    }
    if (args.contains(u"--verbose"_s)) {
        verbose = true;
    }

    connect(ui->pushAbout, &QPushButton::clicked, this, &defaultlook::pushAbout_clicked);
    connect(ui->pushHelp, &QPushButton::clicked, this, &defaultlook::pushHelp_clicked);
    connect(ui->pushClose, &QPushButton::clicked, this, &defaultlook::close);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &defaultlook::tabWidget_currentChanged);

    setup();
}

defaultlook::~defaultlook()
{
    if (tweakPlasma) {
        delete tweakPlasma;
    }
    if (tweakXfce) {
        delete tweakXfce;
    }
    if (tweakFluxbox) {
        delete tweakFluxbox;
    }
    delete ui;
}

void defaultlook::checkSession() {
    QString test = runCmd(u"ps -aux |grep  -E \'plasmashell|xfce4-session|fluxbox|lightdm|xfce-superkey\' |grep -v grep"_s).output;
    if (test.contains("xfce4-session"_L1)){
        isXfce = true;
    } else if (test.contains("plasmashell"_L1)) {
        isKDE = true;
    } else if (test.contains("fluxbox"_L1)){
        isFluxbox = true;
    }
    if (test.contains("lightdm"_L1)){
        isLightdm = true;
    }
    if (test.contains("xfce-superkey"_L1)){
        isSuperkey = true;
    }
    if (verbose) qDebug() << "isXfce is " << isXfce << "isKDE is " << isKDE << "isFluxbox is " << isFluxbox << "isLightdm is " << isLightdm << "isSuperkey is" << isSuperkey;

}
// setup versious items first time program runs
void defaultlook::setup()
{
    QString home_path = QDir::homePath();
    if (!checklightdm()) {
        ui->checkBoxLightdmReset->hide();
    }
    //load saved settings, for now only fluxbox legacy styles checkbox
    loadSettings();

    if (themetabflag) ui->tabWidget->setCurrentIndex(Tab::Theme);
    if (othertabflag) ui->tabWidget->setCurrentIndex(Tab::Others);
    if (displayflag) ui->tabWidget->setCurrentIndex(Tab::Display);
    ui->checkBoxFluxboxLegacyStyles->hide();

    if (isXfce) {
        ui->pushXFCEPanelSettings->setIcon(QIcon::fromTheme(u"org.xfce.panel"_s));
        ui->pushXFCEAppearance->setIcon(QIcon::fromTheme(u"org.xfce.settings.appearance"_s));
        ui->pushXFCEWMsettings->setIcon(QIcon::fromTheme(u"org.xfce.xfwm4"_s));
        message_flag = false;
        //setup theme tab
        ui->pushButtonPreview->hide();
        //set first tab as default
        if (!displayflag && !themetabflag && !othertabflag) {
            ui->tabWidget->setCurrentIndex(Tab::Panel);
        }
        ui->tabWidget->removeTab(Tab::Plasma);
        ui->tabWidget->removeTab(Tab::Fluxbox);
        tweakXfce = new TweakXfce(ui, verbose, this);
        //setup panel tab
        setuptheme();
        //setup compositor tab
        setupCompositor();
        //setup theme combo box
        setupComboTheme();
        //setup other tab;
        setupEtc();
        if (!isSuperkey || ! QFile(u"/usr/bin/xfce-superkey-launcher"_s).exists()){
            ui->tabWidget->removeTab(Tab::Superkey);
        } else {
            setupSuperKey();
        }

        connect(ui->pushXFCEPanelSettings, &QPushButton::clicked, this, &defaultlook::pushXFCEPanelSettings_clicked);
        connect(ui->pushXFCEAppearance, &QPushButton::clicked, this, &defaultlook::pushXFCEAppearance_clicked);
        connect(ui->pushXFCEWMsettings, &QPushButton::clicked, this, &defaultlook::pushXFCEWMsettings_clicked);
    }

    //setup fluxbox
    else if (isFluxbox) {
        ui->comboTheme->hide();
        ui->labelTheme->hide();
        ui->buttonThemeApply->hide();
        ui->pushButtonPreview->hide();
        ui->pushButtonRemoveUserThemeSet->hide();
        ui->groupXFCESettings->hide();
        if (!themetabflag && !othertabflag) {
            ui->tabWidget->setCurrentIndex(Tab::Fluxbox);
        }
        ui->tabWidget->removeTab(Tab::Superkey);
        ui->tabWidget->removeTab(Tab::Plasma);
        ui->tabWidget->removeTab(Tab::Config);
        ui->tabWidget->removeTab(Tab::Display);
        ui->tabWidget->removeTab(Tab::Panel);
        ui->checkBoxFluxboxLegacyStyles->show();
        tweakFluxbox = new TweakFluxbox(ui, verbose, this);

        setupCompositor();
        setuptheme();
        //setup other tab;
        setupEtc();

        connect(ui->pushManageTint2, &QPushButton::clicked, this, &defaultlook::pushManageTint2_clicked);
    }
//Panel, Theme, Compositor, Display, Config, Fluxbox, Plasma, Superkey, Others
    //setup plasma
    else if (isKDE) {
        ui->pushButtonPreview->hide();
        ui->pushButtonRemoveUserThemeSet->hide();
        ui->groupXFCESettings->hide();
        if (!themetabflag && !othertabflag) {
            ui->tabWidget->setCurrentIndex(Tab::Plasma);
        }
        ui->tabWidget->removeTab(Tab::Superkey);
        ui->tabWidget->removeTab(Tab::Fluxbox);
        ui->tabWidget->removeTab(Tab::Config);
        ui->tabWidget->removeTab(Tab::Display);
        ui->tabWidget->removeTab(Tab::Compositor);
        ui->tabWidget->removeTab(Tab::Panel);
        tweakPlasma = new TweakPlasma(ui, verbose, this);
        setuptheme();
        setupComboTheme();
        //setup other tab;
        setupEtc();


    //for other non-supported desktops, show only
    } else {
        ui->groupXFCESettings->hide();
        ui->tabWidget->setCurrentIndex(Tab::Others);
        for (int i = 6; i >= 0; --i) {
            ui->tabWidget->removeTab(i);
        }
        //setup other tab;
        setupEtc();
    }

    // Thunar (on MX installed with XFCE or Fluxbox by default)
    if (QFile::exists(u"/usr/bin/thunar"_s)) {
        if (isFluxbox) {
            ui->layoutFluxboxTab->replaceWidget(ui->widgetFluxboxThunar, ui->groupThunar);
            connect(ui->pushFluxboxApply, &QCheckBox::clicked, this, &defaultlook::applyThunar);
        } else if (isXfce) {
            connect(ui->pushXfceApply, &QPushButton::clicked, this, &defaultlook::applyThunar);
        }
        connect(ui->checkThunarSingleClick, &QCheckBox::clicked, this, &defaultlook::slotThunarChanged);
        connect(ui->checkThunarResetCustomActions, &QCheckBox::clicked, this, &defaultlook::slotThunarChanged);
        connect(ui->checkThunarSplitView, &QCheckBox::clicked, this, &defaultlook::slotThunarChanged);
        connect(ui->checkThunarSplitViewHorizontal, &QCheckBox::clicked, this, &defaultlook::slotThunarChanged);
        setupThunar();
    }

    //copy template file to ~/.local/share/mx-tweak-data if it doesn't exist
    QDir userdir(home_path + "/.local/share/mx-tweak-data"_L1);
    QFileInfo template_file(home_path + "/.local/share/mx-tweak-data/mx.tweak.template"_L1);
    if (template_file.exists()) {
        if (verbose) qDebug() << "template file found";
    } else {
        if (userdir.exists()) {
            runCmd("cp /usr/share/mx-tweak-data/mx.tweak.template "_L1 + userdir.absolutePath());
        } else {
            runCmd("mkdir -p "_L1 + userdir.absolutePath());
            runCmd("cp /usr/share/mx-tweak-data/mx.tweak.template "_L1 + userdir.absolutePath());
        }
    }
    setupflag=true;
    version = getVersion(u"mx-tweak"_s);
    this->adjustSize();
}

void defaultlook::pushAbout_clicked()
{
    this->hide();
    displayAboutMsgBox(tr("About MX Tweak"),
                       "<p align=\"center\"><b><h2>"_L1 + tr("MX Tweak") + "</h2></b></p><p align=\"center\">"_L1 +
                       tr("Version: ") + VERSION"</p><p align=\"center\"><h3>"_L1 +
                       tr("App for quick default ui theme changes and tweaks") +
                       "</h3></p><p align=\"center\"><a href=\"http://mxlinux.org\">http://mxlinux.org</a><br /></p>"_L1
                       "<p align=\"center\">"_L1 + tr("Copyright (c) MX Linux") + "<br /><br /></p>"_L1,
                       u"/usr/share/doc/mx-tweak/license.html"_s, tr("%1 License").arg(this->windowTitle()));
    this->show();
}

void defaultlook::pushHelp_clicked()
{
    QLocale locale;
    QString lang = locale.bcp47Name();

    QString url = u"file:///usr/share/doc/mx-tweak/mx-tweak.html"_s;

    if (lang.startsWith("fr"_L1)) {
        url = u"https://mxlinux.org/wiki/help-files/help-tweak-ajustements"_s;
    }
    displayDoc(url, tr("%1 Help").arg(tr("MX Tweak")));
}

void defaultlook::message() const
{
    QString cmd = u"ps -aux |grep -v grep|grep firefox"_s;
    if ( system(cmd.toUtf8()) != 0 ) {
        if (verbose) qDebug() << "Firefox not running" ;
    } else {
        QMessageBox::information(nullptr, tr("MX Tweak"),
                                 tr("Finished! Firefox may require a restart for changes to take effect"));
    }
}

bool defaultlook::checklightdm()
{
    QFileInfo test(u"/etc/lightdm/lightdm-gtk-greeter.conf"_s);
    return (test.exists());
}

void defaultlook::message2()
{
    QMessageBox::information(nullptr, tr("Panel settings"),
                             tr("Your current panel settings have been backed up in a hidden folder called .restore in your home folder (~/.restore/)"));
}

void defaultlook::pushXFCEPanelSettings_clicked()
{
    this->hide();
    system("xfce4-panel --preferences");
    system("xprop -spy -name \"Panel Preferences\" >/dev/null");
    this->show();
    QString test;
    bool flag = false;

    //restart panel if background style of any panel is 1 - solid color, affects transparency
    QStringList panelproperties = runCmd(u"xfconf-query -c xfce4-panel --list |grep background-style"_s).output.split('\n');

    QStringListIterator changeIterator(panelproperties);

    while (changeIterator.hasNext()) {
        QString value = changeIterator.next();
        test = runCmd("xfconf-query -c xfce4-panel -p "_L1 + value).output;
        if (test == "1"_L1) {
            flag = true;
        }
    }

    if (flag) {
        system("xfce4-panel --restart");
    }

    assert(isXfce && tweakXfce != nullptr);
    tweakXfce->panelSetup();
}

void defaultlook::pushXFCEAppearance_clicked()
{
    this->hide();
    system("xfce4-appearance-settings");
    this->show();
}

void defaultlook::pushXFCEWMsettings_clicked()
{
    this->hide();
    system("xfwm4-settings");
    this->show();
}

void defaultlook::setupEtc()
{
    QString home_path = QDir::homePath();
    QString DESKTOP = runCmd(u"echo $XDG_SESSION_DESKTOP"_s).output;
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
    if ( QFile::exists(u"/usr/bin/mxfb-menu-generator"_s)){
        if (QFile::exists(home_path + "/.fluxbox/mxfb-menu-generator-disabled.chk"_L1)){
            ui->checkBoxDisableFluxboxMenuGeneration->setChecked(false);
        } else {
            ui->checkBoxDisableFluxboxMenuGeneration->setChecked(true);
        }
    } else {
        ui->checkBoxDisableFluxboxMenuGeneration->hide();
    }

    //setup udisks option
    QFileInfo fileinfo(u"/etc/tweak-udisks.chk"_s);
    if (fileinfo.exists()) {
        ui->checkBoxMountInternalDrivesNonRoot->setChecked(true);
    } else {
        ui->checkBoxMountInternalDrivesNonRoot->setChecked(false);
    }

    //setup sudo override function

    int rootest = runCmd(u"pkexec /usr/lib/mx-tweak/mx-tweak-rootcheck.sh"_s).exitCode;
    if ( rootest == 0 ){
        ui->radioSudoRoot->setChecked(true);
    } else {
        ui->radioSudoUser->setChecked(true);
    }

    //if root accout disabled, disable root authentication changes
    test = runCmd(u"pkexec /usr/lib/mx-tweak/mx-tweak-check.sh"_s).output;
    if (test.contains("NP"_L1)) {
        ui->radioSudoRoot->setEnabled(false);
        ui->radioSudoUser->setEnabled(false);
        ui->labelSudo->setEnabled(false);
    }

    //setup user namespaces option (99-sandbox-mx.conf)
    sandboxflag = false;
    QString userns_clone = runCmd(u"/usr/sbin/sysctl -n kernel.unprivileged_userns_clone"_s).output;
    //QString yama_ptrace = runCmd("/usr/sbin/sysctl -n kernel.yama.ptrace_scope").output;
    if (verbose) qDebug() << "userns_clone is: " << userns_clone;
    if (userns_clone == "0"_L1 || userns_clone == "1"_L1) {
        //if (userns_clone == "1"_L1 && yama_ptrace == "1"_L1) {
        ui->checkBoxSandbox->setChecked(userns_clone == "1"_L1);
    } else {
        ui->checkBoxSandbox->hide();
    }

    //setup bluetooth auto enable, hide box if config file doesn't exist
    if (QFile::exists(u"/etc/bluetooth/main.conf"_s)){
        bluetoothautoenableflag = false;
        test = runCmd(u"grep ^AutoEnable /etc/bluetooth/main.conf"_s).output;
        test = test.section('=',1,1);
        if ( test == "true"_L1){
            ui->checkBoxbluetoothAutoEnable->setChecked(true);
        } else {
            ui->checkBoxbluetoothAutoEnable->setChecked(false);
        }
    } else {
        ui->checkBoxbluetoothAutoEnable->hide();
    }

    //setup bluetooth battery info
    if (QFile::exists(u"/etc/bluetooth/main.conf"_s)){
        bluetoothbatteryflag = false;
        test = runCmd(u"grep -E '^(#\\s*)?Experimental' /etc/bluetooth/main.conf"_s).output;
        if (verbose) qDebug() << "bluetooth battery " << test;
        if (test.contains('#')){
            ui->checkBoxBluetoothBattery->setChecked(false);
        } else if ( test.contains("true"_L1) ){
            ui->checkBoxBluetoothBattery->setChecked(true);
        } else if ( test.contains("false"_L1)) {
            ui->checkBoxBluetoothBattery->setChecked(false);
        }
    } else {
        ui->checkBoxBluetoothBattery->hide();
    }

    //setup early KVM module loading
    test = runCmd(u"LANG=C grep \"enable_virt_at_load=0\" /etc/modprobe.d/* | grep kvm"_s).output;
    if (!test.isEmpty()){
        if (!test.section(':',1,1).startsWith('#')){
        ui->checkBoxKVMVirtLoad->setChecked(true);
        kvmconffile=test.section(':',0,0);
        if (verbose) qDebug() << "kvm conf file is " << kvmconffile;
        } else {
            ui->checkBoxKVMVirtLoad->setChecked(false);
            kvmconffile = "/etc/modprobe.d/kvm.conf"_L1;
        }
    } else {
        ui->checkBoxKVMVirtLoad->setChecked(false);
        kvmconffile = "/etc/modprobe.d/kvm.conf"_L1;
    }
    //set flag false so future changes processed, but not an unchanged checkbox
    kvmflag=false;

    //setup apt install_recommends
    //enable checkbox only if Install-Recommends is set to 1. default is 0 or no if no existanct apt.conf
    if (QFile::exists(u"/etc/apt/apt.conf"_s)){
        test = runCmd(u"grep Install-Recommends /etc/apt/apt.conf"_s).output;
        if ( test.contains('1')){
            ui->checkBoxInstallRecommends->setChecked(true);
        } else {
            ui->checkBoxInstallRecommends->setChecked(false);
        }
    } else {
        ui->checkBoxInstallRecommends->setChecked(false);
    }

    //setup kernel auto updates

    if (runCmd(u"LC_ALL=C dpkg --status linux-image-amd64 linux-image-686 linux-image-686-pae 2>/dev/null |grep 'ok installed'"_s).output.isEmpty()){
        ui->checkBoxDebianKernelUpdates->setChecked(false);
        ui->checkBoxDebianKernelUpdates->hide();
    } else {
        ui->checkBoxDebianKernelUpdates->setChecked(true);
    }
    if (runCmd(u"LC_ALL=C dpkg --status linux-image-liquorix-amd64 2>/dev/null |grep 'ok installed'"_s).output.isEmpty()){
        ui->checkBoxLiqKernelUpdates->setChecked(false);
        ui->checkBoxLiqKernelUpdates->hide();
    } else {
        ui->checkBoxLiqKernelUpdates->setChecked(true);
    }
    QString autoupdate = runCmd(u"apt-mark showhold"_s).output;
    if ( autoupdate.contains("linux-image-686"_L1) || autoupdate.contains("linux-image-amd64"_L1) ){
        ui->checkBoxDebianKernelUpdates->setChecked(false);
    }

    if ( autoupdate.contains("linux-image-liquorix-amd64"_L1) ){
        ui->checkBoxLiqKernelUpdates->setChecked(false);
    }
    debianKernelUpdateFlag = false;
    liqKernelUpdateFlag = false;

    //hostname
    ui->checkBoxComputerName->setChecked(false);
    ui->lineEditHostname->setEnabled(false);
    originalhostname = runCmd(u"hostname"_s).output;
    ui->lineEditHostname->setText(originalhostname);

    //setup NOCSD GTK3 option
    if (!QFileInfo::exists(u"/usr/bin/gtk3-nocsd"_s)) {
        if (verbose) qDebug() << "gtk3-nocsd not found";
        ui->checkBoxCSD->hide();
    } else {
        if (verbose) qDebug() << "gtk3-nocsd found";
    }
    if (verbose) {
        qDebug() << "home path nocsd is"_L1 << home_path + "/.config/MX-Linux/nocsd/"_L1 + DESKTOP;
    }
    if (QFileInfo::exists(home_path + "/.config/MX-Linux/nocsd/"_L1 + DESKTOP)) {
        ui->checkBoxCSD->setChecked(false);
    } else {
        ui->checkBoxCSD->setChecked(true);
    }

    Intel_flag = false;
    amdgpuflag = false;
    radeon_flag =false;
    enable_recommendsflag = false;
    //setup Intel checkbox

    QString partcheck = runCmd(uR"(for i in $(lspci -n | awk '{print $2,$1}' | grep -E '^(0300|0302|0380)' | cut -f2 -d\ ); do lspci -kns "$i"; done)"_s).output;
    if (verbose) qDebug()<< "partcheck = " << partcheck;

    if ( partcheck.contains("i915"_L1)) {
        ui->checkboxIntelDriver->show();
        ui->labelIntel->show();
    }

    if ( partcheck.contains("Kernel driver in use: amdgpu"_L1)) {
        ui->checkboxAMDtearfree->show();
        ui->labelamdgpu->show();
    }

    if ( partcheck.contains("Kernel driver in use: radeon"_L1)) {
        ui->checkboxRadeontearfree->show();
        ui->labelradeon->show();
    }

    QFileInfo intelfile(u"/etc/X11/xorg.conf.d/20-intel.conf"_s);
    ui->checkboxIntelDriver->setChecked(intelfile.exists());

    QFileInfo amdfile(u"/etc/X11/xorg.conf.d/20-amd.conf"_s);
    ui->checkboxAMDtearfree->setChecked(amdfile.exists());

    QFileInfo radeonfile(u"/etc/X11/xorg.conf.d/20-radeon.conf"_s);
    ui->checkboxRadeontearfree->setChecked(radeonfile.exists());

    //setup display manager combo box
    QString displaymanagers=runCmd(u"dpkg --list sddm gdm3 lightdm slim slimski xdm wdm lxdm nodm 2>/dev/null |grep ii | awk '{print $2}'"_s).output;
    QStringList displaymanagerlist = displaymanagers.split(u"\n"_s);
    if ( displaymanagerlist.count() > 1) {
        //only add items once
        if (ui->comboBoxDisplayManager->count() == 0) {
            ui->comboBoxDisplayManager->addItems(displaymanagerlist);
            //set default selection to current display manager
            //read from /etc/X11/default-display-manager if it exits, else use running dm
            QFile defaultdisplay(u"/etc/X11/default-display-manager"_s);
            if (defaultdisplay.exists()){
                if (defaultdisplay.open(QIODevice::ReadOnly | QIODevice::Text)){
                    QTextStream in(&defaultdisplay);
                    currentdisplaymanager=in.readAll().section('/',3,3).remove('\n');
                    defaultdisplay.close();
                }
            } else {
                currentdisplaymanager = runCmd(u"ps -aux |grep  -E '/usr/.*bin/sddm|/usr/.*bin*/gdm3|.*bin*/lightdm|.*bin*/slim|.*bin*/slimski|.*bin*/xdm.*bin*/wdm.*bin*/lxdm.*bin*/nodm' |grep -v grep | awk '{print $11}'"_s).output.section('/', 3,3);
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
    populatethemelists(u"gtk-3.0"_s);
    populatethemelists(u"icons"_s);
    populatethemelists(u"cursors"_s);
    get_cursor_size();
    cursor_size_flag = true;
    }

    if (isXfce){
         populatethemelists(u"xfwm4"_s);
    } else if (isFluxbox) {
         populatethemelists(u"fluxbox"_s);
    }

    if (isKDE) {
        ui->labelThemes->setText("<b>"_L1 + tr("Plasma Widget Themes","theme style of the kde plasma widgets") + "</b>"_L1);
        ui->labelWMThemes->setText("<b>"_L1 + tr("Color Schemes", "plasma widget color schemes") + "</b>"_L1);
        ui->labelTheme->setText("<b>"_L1 + tr("Plasma Look & Feel Global Themes", "plasma global themes") + "</b>"_L1);
        populatethemelists(u"plasma"_s);
        populatethemelists(u"colorscheme"_s);
        populatethemelists(u"kdecursors"_s);
        populatethemelists(u"icons"_s);
        ui->pushButtonSettingsToThemeSet->hide();
        ui->spinBoxPointerSize->hide();
        ui->labelPointerSize->hide();
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
//        if (code == "gui_input_style = 0"_L1) {
//            ui->checkHexchat->setChecked(true);
//        }
//    }
}

void defaultlook::get_cursor_size() {
    QString home_path = QDir::homePath();
    QString size = u"0"_s;
    QString sizecheck;

    //check .Xresources
    if (isFluxbox){
        QString file = home_path + "/.Xresources"_L1;
        if ( QFile(file).exists()){
            sizecheck = runCmd("grep Xcursor.size "_L1 + file).output.section(':',1,1).simplified();
            if ( sizecheck.isEmpty()){
                size = '0';
            } else {
                size = sizecheck;
            }
        }
        ui->spinBoxPointerSize->setValue(size.toInt());
    }

    //check Xfce
    if (isXfce){
        sizecheck = runCmd(u"xfconf-query --channel xsettings --property /Gtk/CursorThemeSize"_s).output.simplified();
        if ( sizecheck.isEmpty()){
            size = '0';
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
            QString file = home_path + "/.Xresources"_L1;
            if (runCmd("grep Xcursor.size "_L1 + file).exitCode == 0) {
                cmd = "sed -i 's/Xcursor.size:.*/Xcursor.size: "_L1 + size + "/' "_L1 + file;
            } else {
                cmd = "echo Xcursor.size: "_L1 + size + " >"_L1 + file;
            }
        }

        //check Xfce
        if (isXfce){
            cmd = ("xfconf-query --channel xsettings --property /Gtk/CursorThemeSize -t int -s "_L1 + size + " --create"_L1);
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
    connect(ui->comboCompositor, &QComboBox::currentIndexChanged, this, &defaultlook::comboCompositor_currentIndexChanged);
    //set comboboxvblank to current setting

    vblankflag = false;
    vblankinitial = runCmd(u"xfconf-query -c xfwm4 -p /general/vblank_mode"_s).output;
    if (verbose) qDebug() << "vblank = " << vblankinitial;
    ui->comboBoxvblank->setCurrentText(vblankinitial);

    //deal with compositors

    QString cmd = u"ps -aux |grep -v grep |grep -q compiz"_s;
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
        CheckComptonRunning();
    }
}

void defaultlook::CheckComptonRunning()
{
    //Index for combo box:  0=none, 1=xfce, 2=picom (formerly compton)

    if ( system("ps -ax -o comm,pid |grep -w ^picom") == 0 ) {
        if (verbose) qDebug() << "picom is running";
        const int i = ui->comboCompositor->findData(u"picom"_s);
        ui->comboCompositor->setCurrentIndex(i);
    } else {
        if (verbose) qDebug() << "picom is NOT running";

        //check if picom is present on system, remove from choices if not
        QFileInfo picom(u"/usr/bin/picom"_s);
        //hide picom settings
        if ( !picom.exists() ) {
            const int i = ui->comboCompositor->findData(u"picom"_s);
            ui->comboCompositor->removeItem(i);
            ui->buttonConfigureCompton->hide();
            ui->buttonEditComptonConf->hide();
        }
    }

    //check if xfce compositor is enabled
    const QString &test = runCmd(u"xfconf-query -c xfwm4 -p /general/use_compositing"_s).output;
    if (verbose) qDebug() << "etc test is "<< test;
    int compIndex = ui->comboCompositor->findData(u"xfwm"_s);
    if (test == "true"_L1) {
        ui->buttonConfigureXfwm->setEnabled(true);
    } else {
        ui->comboCompositor->removeItem(compIndex);
        compIndex = ui->comboCompositor->findData(QVariant());
    }
    ui->comboCompositor->setCurrentIndex(compIndex);
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
    QString xscale = u"1"_s;
    QString yscale = u"1"_s;
    double scale = 1;

    //get active profile
    QString activeprofile = runCmd(u"LANG=C xfconf-query --channel displays -p /ActiveProfile"_s).output;
    //get scales for display show in combobox
    int exitcode = runCmd("LANG=C xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + ui->comboBoxDisplay->currentText() +"/Scale/X"_L1).exitCode;
    if ( exitcode == 0 ) {
        xscale = runCmd("LANG=C xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + ui->comboBoxDisplay->currentText() +"/Scale/X"_L1).output;
    }
    exitcode = runCmd("LANG=C xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + ui->comboBoxDisplay->currentText() +"/Scale/Y"_L1).exitCode;
    if ( exitcode == 0 ) {
        yscale = runCmd("LANG=C xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + ui->comboBoxDisplay->currentText() + "/Scale/Y"_L1).output;
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
    QString resolution = runCmd("xrandr |grep "_L1 + ui->comboBoxDisplay->currentText() + " |cut -d' ' -f3 |cut -d'+' -f1"_L1).output;
    if (verbose) qDebug() << "resolution is : " << resolution;
    QString scalestring = QString::number(scale, 'G', 5);
    QString activeprofile = runCmd(u"LANG=C xfconf-query --channel displays -p /ActiveProfile"_s).output;

    //set missing variables
    setmissingxfconfvariables(activeprofile, resolution);

    //set scale value
    QString cmd = "xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + ui->comboBoxDisplay->currentText() +"/Scale/Y -t double -s "_L1 + scalestring + " --create"_L1;
    if (verbose) qDebug() << "cmd is " << cmd;
    runCmd(cmd);
    cmd = "xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + ui->comboBoxDisplay->currentText() +"/Scale/X -t double -s "_L1 + scalestring + " --create"_L1;
    runCmd(cmd);
    if (verbose) qDebug() << "cmd is " << cmd;

    //set initial scale with xrandr
    QString cmd2 = "xrandr --output "_L1 + ui->comboBoxDisplay->currentText() + " --scale "_L1 + scalestring + 'x' + scalestring;
    runCmd(cmd2);
}

void defaultlook::on_buttonApplyDisplayScaling_clicked()
{
    setscale();
}

void defaultlook::on_comboBoxDisplay_currentIndexChanged(int  /*index*/)
{
    if (setupflag){
        setupBrightness();
        setupscale();
        setupresolutions();
        setupGamma();
    }
}

void defaultlook::setupDisplay()
{
    if (setupflag){
        //populate combobox
        QString displaydata = runCmd(u"LANG=C xrandr |grep -w connected | cut -d' ' -f1"_s).output;
        QStringList displaylist = displaydata.split(u"\n"_s);
        ui->comboBoxDisplay->clear();
        ui->comboBoxDisplay->addItems(displaylist);
        setupBrightness();
        setupGamma();
        setupscale();
        setupbacklight();
        setupresolutions();
        brightnessflag = true;

        //get gtk scaling value
        QString GTKScale = runCmd(u"LANG=C xfconf-query --channel xsettings -p /Gdk/WindowScalingFactor"_s).output;
        ui->spinBoxgtkscaling->setValue(GTKScale.toInt());
        //disable resolution stuff
    }
}

void defaultlook::setupresolutions()
{
    QString display = ui->comboBoxDisplay->currentText();
    ui->comboBoxresolutions->clear();
    QString cmd = "LANG=C /usr/lib/mx-tweak/mx-tweak-lib-randr.sh "_L1 + display + " resolutions"_L1;
    if (verbose) qDebug() << "get resolution command is :" << cmd;
    QString resolutions = runCmd(cmd).output;
    if (verbose) qDebug() << "resolutions are :" << resolutions;
    QStringList resolutionslist = resolutions.split(u"\n"_s);
    ui->comboBoxresolutions->addItems(resolutionslist);
    //set current resolution as default
    QString resolution = runCmd("xrandr |grep "_L1 + ui->comboBoxDisplay->currentText() + " |cut -d+ -f1 |grep -oE '[^ ]+$'"_L1).output;
    if (verbose) qDebug() << "resolution is : " << resolution;
    ui->comboBoxresolutions->setCurrentText(resolution);
}

void defaultlook::setresolution()
{
    QString activeprofile = runCmd(u"LANG=C xfconf-query --channel displays -p /ActiveProfile"_s).output;
    QString display = ui->comboBoxDisplay->currentText();
    QString resolution = ui->comboBoxresolutions->currentText();
    QString cmd = "xrandr --output "_L1 + display + " --mode "_L1 + resolution;
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
    runCmd("xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + ui->comboBoxDisplay->currentText() + " -t string -s "_L1 + ui->comboBoxDisplay->currentText() + " --create"_L1);

    //set resolution
    runCmd("xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + ui->comboBoxDisplay->currentText() + "/Resolution -t string -s "_L1 + resolution.simplified() + " --create"_L1);

    //set active profile
    runCmd("xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + ui->comboBoxDisplay->currentText() + "/Active -t bool -s true --create"_L1);
}

void defaultlook::setrefreshrate(const QString &display, const QString &resolution, const QString &activeprofile) const
{
    //set refreshrate too
    QString refreshrate = runCmd("/usr/lib/mx-tweak/mx-tweak-lib-randr.sh "_L1 + display + " refreshrate"_L1).output;
    refreshrate=refreshrate.simplified();
    QStringList refreshratelist = refreshrate.split(QRegularExpression(u"\\s"_s));
    refreshratelist.removeAll(resolution);
    if (verbose) qDebug() << "defualt refreshreate list is :" << refreshratelist.at(0).section('*',0,0);
    runCmd("xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + display + "/RefreshRate -t double -s "_L1 + refreshratelist.at(0).section('*',0,0) + " --create; sleep 1"_L1);
}

void defaultlook::setupbacklight()
{
    //check for backlights
    QString test = runCmd(u"ls /sys/class/backlight"_s).output;
    if ( ! test.isEmpty()) {
        //get backlight value for currently
        QString backlight=runCmd(u"sudo /usr/lib/mx-tweak/backlight-brightness -g"_s).output;
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
    QString cmd = "sudo /usr/lib/mx-tweak/backlight-brightness -s "_L1 + backlight;
    ui->backlight_label->setText(backlight);
    system(cmd.toUtf8());
}

void defaultlook::setgtkscaling()
{
    runCmd("xfconf-query --channel xsettings -p /Gdk/WindowScalingFactor -t int -s "_L1 + QString::number(ui->spinBoxgtkscaling->value()));
    runCmd(u"xfce4-panel --restart"_s);
}

void defaultlook::setupBrightness()
{
    //get brightness value for currently shown display
    QString brightness=runCmd("LANG=C xrandr --verbose | awk '/"_L1 + ui->comboBoxDisplay->currentText() +"/{flag=1;next}/Clones/{flag=0}flag'|grep Brightness|cut -d' ' -f2"_L1).output;
    int brightness_slider_value = static_cast<int>(brightness.toFloat() * 100);
    ui->horizontalSliderBrightness->setValue(brightness_slider_value);
    if (verbose) qDebug() << "brightness string is " << brightness;
    if (verbose) qDebug() << " brightness_slider_value is " << brightness_slider_value;
    ui->horizontalSliderBrightness->setToolTip(QString::number(ui->horizontalSliderBrightness->value()));
}

void defaultlook::setupGamma()
{
    QString gamma = runCmd("/usr/lib/mx-tweak/mx-tweak-lib-randr.sh "_L1 + ui->comboBoxDisplay->currentText() + " gamma"_L1).output;
    gamma=gamma.simplified();
    gamma = gamma.section(':',1,3).simplified();
    double gamma1 = 1.0 / gamma.section(':',0,0).toDouble();
    double gamma2 = 1.0 / gamma.section(':',1,1).toDouble();
    double gamma3 = 1.0 / gamma.section(':',2,2).toDouble();
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
    cmd = "xrandr --output "_L1 + ui->comboBoxDisplay->currentText() + " --brightness "_L1 + brightness + " --gamma "_L1 + g1 + ':' + g2 + ':' +g3;
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
    QString config_file_path = home_path + "/.config/MX-Linux/MX-Tweak/brightness"_L1;
    if ( ! QFileInfo::exists(config_file_path)) {
        runCmd("mkdir -p "_L1 + config_file_path);
    }
    //save config in file named after the display
    runCmd("echo 'xrandr --output "_L1 + ui->comboBoxDisplay->currentText() + " --brightness "_L1 + brightness + " --gamma "_L1 + g1 + ':' + g2 + ':' + g3 + "'>"_L1 + config_file_path + '/' + ui->comboBoxDisplay->currentText());
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
        QStringList filter(u"*.tweak"_s);
        QDirIterator it(u"/usr/share/mx-tweak-data"_s, filter, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            xsettings_gtk_theme_present = false;
            icontheme_present = false;
            xfwm4_theme_present = false;
            QFileInfo file_info(it.next());
            QString filename = file_info.absoluteFilePath();
            QString name = runCmd("cat '"_L1 + filename + "'|grep ^Name="_L1).output.section('=',1,1);
            QString xsettings_gtk_theme = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xsettings_gtk_theme="_L1).output.section('=',1,1);
            if (verbose) qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
            QString xsettings_icon_theme = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xsettings_icon_theme="_L1).output.section('=',1,1);
            if (verbose) qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
            QString xfwm4_window_decorations = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xfwm4_window_decorations="_L1).output.section('=',1,1);
            if (verbose) qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;

            //check theme existence, only list if all 3 elements present
            QFileInfo xsettings_theme("/usr/share/themes/"_L1 + xsettings_gtk_theme);
            QFileInfo xfwm4_theme("/usr/share/themes/"_L1 + xfwm4_window_decorations);
            QFileInfo icon_theme("/usr/share/icons/"_L1 + xsettings_icon_theme);
            QFileInfo xsettings_theme_home(home_path + "/.themes/"_L1 + xsettings_gtk_theme);
            QFileInfo xfwm4_theme_home(home_path + "/.themes/"_L1 + xfwm4_window_decorations);
            QFileInfo icon_theme_home(home_path + "/.icons/"_L1 + xsettings_icon_theme);
            QFileInfo xsettings_theme_home_alt(home_path + "/.local/share/themes/"_L1 + xsettings_gtk_theme);
            QFileInfo xfwm4_theme_home_alt(home_path + "/.local/share/themes/"_L1 + xfwm4_window_decorations);
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

        QDirIterator it2(home_path + "/.local/share/mx-tweak-data"_L1, filter, QDir::Files, QDirIterator::Subdirectories);
        while (it2.hasNext()) {
            xsettings_gtk_theme_present = false;
            icontheme_present = false;
            xfwm4_theme_present = false;
            QString home_path = QDir::homePath();
            QFileInfo file_info(it2.next());
            QString filename = file_info.absoluteFilePath();
            QString name = runCmd("cat '"_L1 + filename + "'|grep ^Name="_L1).output.section('=',1,1);

            QString xsettings_gtk_theme = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xsettings_gtk_theme="_L1).output.section('=',1,1);
            if (verbose) qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
            QString xsettings_icon_theme = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xsettings_icon_theme="_L1).output.section('=',1,1);
            if (verbose) qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
            QString xfwm4_window_decorations = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xfwm4_window_decorations="_L1).output.section('=',1,1);
            if (verbose) qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;

            //check theme existence, only list if all 3 elements present
            QFileInfo xsettings_theme("/usr/share/themes/"_L1 + xsettings_gtk_theme);
            QFileInfo xfwm4_theme("/usr/share/themes/"_L1 + xfwm4_window_decorations);
            QFileInfo icon_theme("/usr/share/icons/"_L1 + xsettings_icon_theme);
            QFileInfo xsettings_theme_home(home_path + "/.themes/"_L1 + xsettings_gtk_theme);
            QFileInfo xfwm4_theme_home(home_path + "/.themes/"_L1 + xfwm4_window_decorations);
            QFileInfo icon_theme_home(home_path + "/.icons/"_L1 + xsettings_icon_theme);
            QFileInfo xsettings_theme_home_alt(home_path + "/.local/share/themes/"_L1 + xsettings_gtk_theme);
            QFileInfo xfwm4_theme_home_alt(home_path + "/.local/share/themes/"_L1 + xfwm4_window_decorations);
            QFileInfo icon_theme_home_alt(home_path + "/.local/share/icons/"_L1 + xsettings_icon_theme);
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
        QString themes = runCmd(u"plasma-apply-lookandfeel --list"_s).output;
        themes.append("\n");
        theme_list = themes.split(u"\n"_s);
        current = runCmd(u"grep LookAndFeelPackage $HOME/.config/kdeglobals"_s).output.section('=',1,1);
        if (verbose) qDebug() << "current is " << current;
    }

    ui->comboTheme->addItems(theme_list);
    if (current.isEmpty()){
            ui->comboTheme->setCurrentIndex(0);
          } else {
            ui->comboTheme->setCurrentText(current);
    }
}

void defaultlook::on_comboTheme_activated(const int /*arg1*/)
{
    if (setupflag){
        qDebug() << "combo box activated";
        if (isXfce) {
            if (ui->comboTheme->currentIndex() != 0) {
                ui->buttonThemeApply->setEnabled(true);
                ui->pushButtonPreview->setEnabled(true);
            }
        } else if (isKDE) {
            ui->buttonThemeApply->setEnabled(true);
        }
    }
}

void defaultlook::on_buttonThemeApply_clicked()
{
    themeflag = false;
    if (isXfce){
        ui->buttonThemeApply->setEnabled(false);
        QString themename = theme_info.value(ui->comboTheme->currentText());
        QFileInfo fileinfo(themename);
        //initialize variables
        QString backgroundColor = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep background-rgba="_L1).output.section('=' , 1,1);
        if (verbose) qDebug() << "backgroundColor = " << backgroundColor;
        QString color1 = backgroundColor.section(',',0,0);
        QString color2 = backgroundColor.section(',', 1, 1);
        QString color3 = backgroundColor.section(',',2,2);
        QString color4 = backgroundColor.section(',',3,3);
        if (verbose) qDebug() << "sep colors" << color1 << color2 << color3 << color4;
        QString background_image = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep background-image="_L1).output.section('=',1,1);
        if (verbose) qDebug() << "backgroundImage = " << background_image;
        QString background_style = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep background-style="_L1).output.section('=',1,1);
        if (verbose) qDebug() << "backgroundstyle = " << background_style;
        QString xsettings_gtk_theme = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep xsettings_gtk_theme="_L1).output.section('=',1,1);
        if (verbose) qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
        QString xsettings_icon_theme = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep xsettings_icon_theme="_L1).output.section('=',1,1);
        if (verbose) qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
        QString xfwm4_window_decorations = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep xfwm4_window_decorations="_L1).output.section('=',1,1);
        if (verbose) qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;
        QString cursorthemename = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep CursorThemeName="_L1).output.section('=',1,1);
        if (verbose) qDebug() << "CursorThemeName = " << cursorthemename;
        //  use xfconf system to change values

        message_flag = true;

        //set gtk theme
        runCmd("xfconf-query -c xsettings -p /Net/ThemeName -s "_L1 + xsettings_gtk_theme);
        runCmd(u"sleep .5"_s);
        runCmd("gsettings set org.gnome.desktop.interface gtk-theme \""_L1 + xsettings_gtk_theme + '"');
        if (xsettings_gtk_theme.contains("dark"_L1, Qt::CaseInsensitive)){
            runCmd(u"gsettings set org.gnome.desktop.interface color-scheme prefer-dark"_s);
        } else {
            runCmd(u"gsettings set org.gnome.desktop.interface color-scheme default"_s);
        }

        //set window decorations theme
        runCmd("xfconf-query -c xfwm4 -p /general/theme -s "_L1 + xfwm4_window_decorations);
        runCmd(u"sleep .5"_s);

        //set icon theme
        runCmd("xfconf-query -c xsettings -p /Net/IconThemeName -s "_L1 + xsettings_icon_theme);
        runCmd(u"sleep .5"_s);

        //set cursor theme if exists
        if ( ! cursorthemename.isEmpty()){
            runCmd("xfconf-query -c xsettings -p /Gtk/CursorThemeName -s "_L1 + cursorthemename);
        }

        //deal with panel customizations for each panel

        QStringListIterator changeIterator(tweakXfce->panelIDs);
        while (changeIterator.hasNext()) {
            QString value = changeIterator.next();

            //set panel background mode

            if (background_style == "1"_L1 || background_style == "2"_L1 || background_style == "0"_L1) {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + value + "/background-style -t int -s "_L1 + background_style + " --create"_L1);
            } else {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + value + "/background-style -t int -s 0 --create"_L1);
            }

            //set panel background image

            QFileInfo image(background_image);

            if (image.exists()) {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + value + "/background-image -t string -s "_L1 + background_image + " --create"_L1);
            } else {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + value + "/background-image --reset"_L1);
            }

            //set panel color

            if (!backgroundColor.isEmpty()) {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + value + "/background-rgba -t double -t double -t double -t double -s "_L1 + color1 + " -s "_L1 + color2 + " -s "_L1 + color3 + " -s "_L1 + color4 + " --create"_L1);
            }
        }

        //set whisker themeing
        QString home_path = QDir::homePath();
        QFileInfo whisker_check(home_path + "/.config/gtk-3.0/gtk.css"_L1);
        if (whisker_check.exists()) {
            if (verbose) qDebug() << "existing gtk.css found";
            QString cmd = "cat "_L1 + home_path + "/.config/gtk-3.0/gtk.css |grep -q whisker-tweak.css"_L1;
            if (system(cmd.toUtf8()) == 0 ) {
                if (verbose) qDebug() << "include statement found";
            } else {
                if (verbose) qDebug() << "adding include statement";
                QString cmd = "echo '@import url(\"whisker-tweak.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1;
                system(cmd.toUtf8());
            }
        } else {
            if (verbose) qDebug() << "creating simple gtk.css file";
            QString cmd = "echo '@import url(\"whisker-tweak.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1;
            system(cmd.toUtf8());
        }

        //add whisker info
        runCmd("awk '/<begin_gtk_whisker_theme_code>/{flag=1;next}/<end_gtk_whisker_theme_code>/{flag=0}flag' \""_L1 +fileinfo.absoluteFilePath() +"\" > "_L1 + home_path + "/.config/gtk-3.0/whisker-tweak.css"_L1);

        //restart xfce4-panel

        system("xfce4-panel --restart");
    }

    if (isKDE){
        runCmd("plasma-apply-lookandfeel --apply "_L1 + ui->comboTheme->currentText());
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
    QString DESKTOP = runCmd(u"echo $XDG_SESSION_DESKTOP"_s).output;
    QString home_path = QDir::homePath();
    ui->ButtonApplyEtc->setEnabled(false);

    intel_option.clear();
    lightdm_option.clear();
    bluetooth_option.clear();
    recommends_option.clear();

    //deal with udisks option
    QFileInfo fileinfo(u"/etc/tweak-udisks.chk"_s);
    int sudooverride = runCmd(u"pkexec /usr/lib/mx-tweak/mx-tweak-rootcheck.sh"_s).exitCode;
    QString udisks_option;
    QString sudo_override_option;
    QString user_name_space_override_option;
    udisks_option.clear();

    if (verbose) qDebug() << "applyetc DESKTOP is " << DESKTOP;
    if (verbose) qDebug() << "home path applyetc is " << home_path + "/.config/MX-Linux/nocsd/"_L1 + DESKTOP;
    if (ui->checkBoxCSD->isChecked()) {
        if ( QFileInfo::exists(home_path + "/.config/MX-Linux/nocsd/"_L1 + DESKTOP)) {
            runCmd("rm "_L1 + home_path + "/.config/MX-Linux/nocsd/"_L1 + DESKTOP);
        }
    } else {
        int test = runCmd("mkdir -p "_L1 + home_path + "/.config/MX-Linux/nocsd/"_L1).exitCode;
        if ( test != 0 ) {
            if (verbose) qDebug() << "could not make directory";
        }
        test = runCmd("touch "_L1 + home_path + "/.config/MX-Linux/nocsd/"_L1 + DESKTOP ).exitCode;
        if ( test != 0 ) {
            if (verbose) qDebug() << "could not write nocsd desktop file";
        }
    }
    //fluxbox menu autogeneration
    if (QFile::exists(u"/usr/bin/mxfb-menu-generator"_s)){
        if (! ui->checkBoxDisableFluxboxMenuGeneration->isChecked()){
            runCmd("echo '#this file is used to disable automatic updating of the All Apps menu' > "_L1 + home_path + "/.fluxbox/mxfb-menu-generator-disabled.chk"_L1);
        } else {
            runCmd("rm "_L1 + home_path + "/.fluxbox/mxfb-menu-generator-disabled.chk"_L1);
        }
    }

    //internal drive mounting for non root users
    if (ui->checkBoxMountInternalDrivesNonRoot->isChecked()) {
        if (fileinfo.exists()) {
            if (verbose) qDebug() << "no change to internal drive mount settings";
        } else {
            udisks_option = u"enable_user_mount"_s;
        }
    } else {
        if (fileinfo.exists()) {
            udisks_option = u"disable_user_mount"_s;
        } else {
            if (verbose) qDebug() << "no change to internal drive mount settings";
        }
    }

    //reset lightdm greeter config
    if (ui->checkBoxLightdmReset->isChecked()) {
        lightdm_option = u"lightdm_reset"_s;
    }

    //graphics driver overrides

    if ( Intel_flag ) {
        QFileInfo check_intel(u"/etc/X11/xorg.conf.d/20-intel.conf"_s);
        if ( check_intel.exists()) {
            //backup existing 20-intel.conf file to home folder
            system("cp /etc/X11/xorg.conf.d/20-intel.conf /home/$USER/20-intel.conf.$(date +%Y%m%H%M%S)");
        }
        if (ui->checkboxIntelDriver->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            intel_option = u"enable_intel"_s;
        } else {
            //remove 20-intel.conf
            intel_option = u"disable_intel"_s;
        }
    }

    if ( amdgpuflag ) {
        QFileInfo check_amd(u"/etc/X11/xorg.conf.d/20-amd.conf"_s);
        if ( check_amd.exists()) {
            //backup existing 20-amd.conf file to home folder
            system("cp /etc/X11/xorg.conf.d/20-amd.conf /home/$USER/20-amd.conf.$(date +%Y%m%H%M%S)");
        }
        if (ui->checkboxAMDtearfree->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            amd_option = u"enable_amd"_s;
        } else {
            //remove 20-amd.conf
            amd_option = u"disable_amd"_s;
        }
    }

    if ( radeon_flag ) {
        QFileInfo check_radeon(u"/etc/X11/xorg.conf.d/20-radeon.conf"_s);
        if ( check_radeon.exists()) {
            //backup existing 20-radeon.conf file to home folder
            system("cp /etc/X11/xorg.conf.d/20-radeon.conf /home/$USER/20-radeon.conf.$(date +%Y%m%H%M%S)");
        }
        if (ui->checkboxRadeontearfree->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            radeon_option = u"enable_radeon"_s;
        } else {
            //remove 20-radeon.conf
            radeon_option = u"disable_radeon"_s;
        }
    }

    //bluetooth auto enable
    if (bluetoothautoenableflag) {
        if (ui->checkBoxbluetoothAutoEnable->isChecked()) {
            bluetooth_option = u"enable_bluetooth"_s;
            //blueman
            if (QFile::exists(u"/usr/bin/blueman"_s)) {
                runCmd(u"gsettings set org.blueman.plugins.powermanager auto-power-on true"_s);
            }
            //kde bluedevil
            if (QFile::exists(u"/usr/bin/kwriteconfig5"_s)) {
                runCmd(u"kwriteconfig5 --file kded5rc --group Module-bluedevil --key autoload true"_s);
            }
        } else {
            bluetooth_option = u"disable_bluetooth"_s;
            //blueman
            if (QFile::exists(u"/usr/bin/blueman"_s)) {
                runCmd(u"gsettings set org.blueman.plugins.powermanager auto-power-on false"_s);
            }
            //kde bluedevil
            if (QFile::exists(u"/usr/bin/kwriteconfig5"_s)) {
                runCmd(u"kwriteconfig5 --file kded5rc --group Module-bluedevil --key autoload false"_s);
            }

        }
    }

    //bluetooth battery info
    if (bluetoothbatteryflag){
        if (ui->checkBoxBluetoothBattery->isChecked()){
            runCmd(u"pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh bluetooth_battery true"_s);
        } else {
            runCmd(u"pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh bluetooth_battery false"_s);
        }
    }

    //install recommends option
    if ( enable_recommendsflag ){
        if ( ui->checkBoxInstallRecommends->isChecked()){
            recommends_option = "install_recommends"_L1;
        } else
            recommends_option = "noinstall_recommends"_L1;
    }

    //deal with sudo override

    if (ui->radioSudoUser->isChecked()) {
        if (sudooverride == 0) {
            sudo_override_option = u"enable_sudo_override"_s;
        } else {
            if (verbose) qDebug() << "no change to admin password settings";
        }
    } else {
        if (sudooverride == 0) {
            if (verbose) qDebug() << "no change to admin password settings";
        } else {
            sudo_override_option = u"disable_sudo_override"_s;
        }
    }

    //deal with user namespace override
    if (sandboxflag) {
        if (ui->checkBoxSandbox->isChecked()) {
            user_name_space_override_option = u"enable_sandbox"_s;
        } else {
            user_name_space_override_option = u"disable_sandbox"_s;
        }
    }

    //debian kernel updates
    if (debianKernelUpdateFlag){
        if ( ui->checkBoxDebianKernelUpdates->isChecked()){
            debian_kernel_updates_option = "unhold_debian_kernel_updates"_L1;
            qDebug() << "debian update option" << debian_kernel_updates_option;
        } else {
            debian_kernel_updates_option = "hold_debian_kernel_updates"_L1;
        }
    }
    //liquorix kernel updates
    if (liqKernelUpdateFlag){
        if (ui->checkBoxLiqKernelUpdates->isChecked()){
            liq_kernel_updates_option = "unhold_liquorix_kernel_updates"_L1;
        } else {
            liq_kernel_updates_option = "hold_liquorix_kernel_updates"_L1;
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
           kvm_early_switch(u"on"_s, kvmconffile);
        } else {
            kvm_early_switch(u"off"_s, kvmconffile);
        }
    }

    //checkbox options
    if ( ! udisks_option.isEmpty() || ! sudo_override_option.isEmpty() || ! user_name_space_override_option.isEmpty() || ! intel_option.isEmpty() || ! lightdm_option.isEmpty() || ! amd_option.isEmpty() || ! radeon_option.isEmpty() || !bluetooth_option.isEmpty() || !recommends_option.isEmpty() || !debian_kernel_updates_option.isEmpty() || !liq_kernel_updates_option.isEmpty()){
        runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh "_L1 + udisks_option + ' ' + sudo_override_option + ' ' + user_name_space_override_option + ' ' + intel_option + ' ' + amd_option + ' ' + radeon_option + ' ' + bluetooth_option + ' ' + recommends_option + ' ' + lightdm_option + ' ' + debian_kernel_updates_option + ' ' + liq_kernel_updates_option);

    }
    //reset gui
    setupEtc();
}

void defaultlook::changecomputername(const QString &hostname)
{
    runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh hostname "_L1 + hostname);
}

void defaultlook::kvm_early_switch(const QString &action, const QString &file)
{
    if (verbose) qDebug() << "kvm flag is " << kvmflag << "action is " << action << " file is " << file;
    runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh kvm_early_switch "_L1 + action + ' ' + file);
}

void defaultlook::changedisplaymanager(const QString &dm)
{
    runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh displaymanager "_L1 + dm);
}

bool defaultlook::validatecomputername(const QString &hostname)
{
    // see if name is reasonable
    if (hostname.isEmpty()) {
        QMessageBox::critical(this, this->windowTitle(), tr("Please enter a computer name.", "question to enter a name for the computer hostname"));
        return false;
    } else if (hostname.contains(QRegularExpression(u"[^0-9a-zA-Z-.]|^[.-]|[.-]$|\\.\\."_s))) {
        QMessageBox::critical(this, this->windowTitle(),
            tr("Sorry, your computer name contains invalid characters.\nYou'll have to select a different\nname before proceeding.", "unacceptable characters are found in hostname, pick a new name"));
        return false;
    }
    return true;
}

void defaultlook::on_buttonConfigureCompton_clicked()
{
    this->hide();
    system("picom-conf");
    this->show();
}

void defaultlook::on_buttonCompositorApply_clicked()
{
    //disable apply button
    ui->buttonCompositorApply->setEnabled(false);

    if (ui->comboCompositor->currentData() == "picom"_L1) {
        //turn off xfce compositor
        runCmd(u"xfconf-query -c xfwm4 -p /general/use_compositing -s false"_s);
        //launch picom
        system("pkill -x picom");
        system("picom-launch.sh");
        //restart apt-notifier if necessary
        CheckAptNotifierRunning();
    } else if (ui->comboCompositor->currentData() == "xfwm"_L1) {
        //turn off picom
        system("pkill -x picom");
        //launch xfce compositor
        runCmd(u"xfconf-query -c xfwm4 -p /general/use_compositing -s true"_s);
        //restart apt-notifier if necessary
        CheckAptNotifierRunning();
    } else {
        //turn off picom and xfce compositor
        //turn off xfce compositor
        runCmd(u"xfconf-query -c xfwm4 -p /general/use_compositing -s false"_s);
        system("pkill -x picom");
        CheckAptNotifierRunning();
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
    if ( vblankflag ) {
        runCmd("xfconf-query -c xfwm4 -p /general/vblank_mode -t string -s "_L1 + ui->comboBoxvblank->currentText() + " --create"_L1);
        //restart xfwm4 to take advantage of the setting
        runCmd(u"xfwm4 --replace"_s);
    }
}


void defaultlook::on_buttonEditComptonConf_clicked()
{
    QString home_path = QDir::homePath();
    QFileInfo file_conf(home_path + "/.config/picom.conf"_L1);
    runCmd("xdg-open "_L1 + file_conf.absoluteFilePath());
}

void defaultlook::comboCompositor_currentIndexChanged(const int)
{
    if (setupflag){
        if (!ui->comboCompositor->currentData().isValid()) {
            ui->buttonConfigureCompton->setEnabled(false);
            ui->buttonConfigureXfwm->setEnabled(false);
            ui->buttonEditComptonConf->setEnabled(false);
        } else if (ui->comboCompositor->currentData() == "xfwm"_L1) {
            ui->buttonConfigureCompton->setEnabled(false);
            ui->buttonConfigureXfwm->setEnabled(true);
            ui->buttonEditComptonConf->setEnabled(false);
        } else if (ui->comboCompositor->currentData() == "picom"_L1) {
            ui->buttonConfigureCompton->setEnabled(true);
            ui->buttonConfigureXfwm->setEnabled(false);
            ui->buttonEditComptonConf->setEnabled(true);
        }
        ui->buttonCompositorApply->setEnabled(true);
    }
}

void defaultlook::on_buttonConfigureXfwm_clicked()
{
    xfwm_compositor_settings fred;
    fred.setModal(true);
    fred.exec();
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
    QString file_name = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep screenshot="_L1).output.section('=' , 1,1);
    QString path = fileinfo.absolutePath();
    QString full_file_path = path + '/' + file_name;

    QMessageBox preview_box(QMessageBox::NoIcon, file_name, QString(), QMessageBox::Close, this);
    preview_box.setIconPixmap(QPixmap(full_file_path));
    preview_box.exec();
}

void defaultlook::on_radioSudoUser_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}
void defaultlook::on_radioSudoRoot_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_checkBoxLightdmReset_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
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

// Get version of the program
QString defaultlook::getVersion(const QString &name)
{
    return runCmd("dpkg-query -f '${Version}' -W "_L1 + name).output;
}

void defaultlook::on_pushButtonSettingsToThemeSet_clicked()
{
    if (isFluxbox) {
        if (QFile::exists(u"/usr/bin/mxfb-look"_s)){
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
    QString data = runCmd(u"xfconf-query -c xfce4-panel -p /panels --list"_s).output;
    int panelNum = 0;
    for (panelNum = 1;; panelNum++) {
        if (data.contains("panel-"_L1 + QString::number(panelNum)))
            break;
    }
    panel = "panel-"_L1 + QString::number(panelNum);

    int backgroundStyle = 0;
    data = runCmd("xfconf-query -c xfce4-panel -p /panels/"_L1 + panel + "/background-style"_L1).output;
    backgroundStyle = data.toInt(); //there may be newlines in output but qt ignores it

    QVector<double> backgroundColor;
    QString backgroundImage;
    backgroundColor.reserve(4);
    if (backgroundStyle == 1) {
        QStringList lines = runCmd("LANG=C xfconf-query -c xfce4-panel -p /panels/"_L1 + panel + "/background-rgba"_L1).output.split('\n');
        lines.removeAt(0);
        lines.removeAt(0);
        for (int i = 0; i < 4; i++)
        {
            backgroundColor << lines.at(i).toDouble();
        }
    } else if (backgroundStyle == 2) {
        backgroundImage = runCmd("xfconf-query -c /panels/"_L1 + panel + "/background-image"_L1).output;
    }

    QString iconThemeName = runCmd(u"xfconf-query -c xsettings -p /Net/IconThemeName"_s).output;
    QString themeName = runCmd(u"xfconf-query -c xsettings -p /Net/ThemeName"_s).output;
    QString windowDecorationsTheme = runCmd(u"xfconf-query -c xfwm4 -p /general/theme"_s).output;
    QString cursorthemename = runCmd(u"xfconf-query -c xsettings -p /Gtk/CursorThemeName"_s).output;

    QString whiskerThemeFileName = pathAppend(QDir::homePath(), u".config/gtk-3.0/whisker-tweak.css"_s);
    QFile whiskerThemeFile(whiskerThemeFileName);
    if (!whiskerThemeFile.open(QFile::ReadOnly | QFile::Text)) {
        if (verbose) qDebug() << "Failed to fetch whisker theming from: "_L1 + whiskerThemeFileName;
    }
    QTextStream whiskerThemeFileStream(&whiskerThemeFile);
    QString whiskerThemeData = whiskerThemeFileStream.readAll();
    whiskerThemeFile.close();

    QStringList fileLines;
    fileLines << "Name="_L1 + fileName;
    fileLines << "background-style="_L1 + QString::number(backgroundStyle);
    if (backgroundStyle == 1) {
        QString line;
        for (double num : std::as_const(backgroundColor)) {
            line.append(QString::number(num) + ',');
        }
        if (line.endsWith(',')) line.chop(1);
        fileLines << "background-rgba="_L1 + line;
    } else {
        fileLines << u"background-rgba=none"_s;
    }
    if (backgroundStyle == 2) {
        fileLines << "background-image="_L1 + backgroundImage;
    } else {
        fileLines << u"background-image=none"_s;
    }
    fileLines << "xsettings_gtk_theme="_L1 + themeName;
    fileLines << "xsettings_icon_theme="_L1 + iconThemeName;
    fileLines << "xfwm4_window_decorations="_L1 + windowDecorationsTheme;
    fileLines << "CursorThemeName="_L1 + cursorthemename;
    fileLines << u"<begin_gtk_whisker_theme_code>"_s;

    {
        const QStringList &themeSplit = whiskerThemeData.split('\n');
        for (const QString &line : themeSplit) {
            fileLines << line;
        }
    }
    fileLines << u"<end_gtk_whisker_theme_code>"_s;
    QFile file(pathAppend(QDir::homePath(), ".local/share/mx-tweak-data/"_L1 + fileName + ".tweak"_L1));
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        if (verbose) qDebug() << "Failed to open file for reading: "_L1 + fileName;
        return;
    }
    QTextStream fileStream(&file);
    for (const QString &line : std::as_const(fileLines)) {
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
    if (theme == "Select User Theme Set to Remove"_L1)
        return;
    result = QMessageBox::warning(this, u"Remove User Theme Set"_s, "Are you sure you want to remove "_L1 + theme + " theme set?"_L1, QMessageBox::Ok | QMessageBox::Cancel);
    if (result != QMessageBox::Ok)
        return;
    QString file = dialog.getFilename(theme);
    file.replace(' ', "\\ "_L1);
    auto cmd = runCmd("rm "_L1 + file);
    if (cmd.exitCode != 0) {
        if (verbose) qDebug() << "Removing theme set failed: exitCode: " << cmd.exitCode << " | output: " << cmd.output;
        return;
    }
    //refresh
    setupComboTheme();
}

void defaultlook::on_comboBoxvblank_activated(int)
{
    if (setupflag){
        vblankflag = vblankinitial != ui->comboBoxvblank->currentText();
        ui->buttonCompositorApply->setEnabled(true);
    }
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

void defaultlook::populatethemelists(const QString &value)
{
    themeflag = false;
    QString home_path = QDir::homePath();
    QString themes;
    QStringList themelist;
    QString current;

    if (value == "plasma"_L1){
        themes = runCmd(u"LANG=C plasma-apply-desktoptheme --list-themes |grep \"*\" |cut -d\"*\" -f2"_s).output;
        themes.append("\n");
    }
    if (value == "colorscheme"_L1){
        themes = runCmd(u"LANG=C plasma-apply-colorscheme --list-schemes |grep \"*\" |cut -d\"*\" -f2 "_s).output;
        themes.append("\n");
    }
    if (value == "kdecursors"_L1){
        themes = runCmd(u"LANG=C plasma-apply-cursortheme --list-themes | grep \"*\""_s).output;
        themes.append("\n");
    }
    if (value == "gtk-3.0"_L1 || value == "xfwm4"_L1) {
        themes = runCmd("find /usr/share/themes/*/"_L1 + value + " -maxdepth 0 2>/dev/null|cut -d\"/\" -f5"_L1).output;
        themes.append("\n");
        themes.append(runCmd("find $HOME/.themes/*/"_L1 + value + " -maxdepth 0 2>/dev/null|cut -d\"/\" -f5"_L1).output);
        themes.append("\n");
        themes.append(runCmd("find $HOME/.local/share/themes/*/"_L1 + value + " -maxdepth 0 2>/dev/null|cut -d\"/\" -f7"_L1).output);
    }

    if (value == "icons"_L1){
        themes = runCmd(u"find /usr/share/icons/*/index.theme -maxdepth 1 2>/dev/null|cut -d\"/\" -f5"_s).output;
        themes.append("\n");
        themes.append(runCmd(u"find $HOME/.icons/*/index.theme -maxdepth 1 2>/dev/null|cut -d\"/\" -f5"_s).output);
        themes.append("\n");
        themes.append(runCmd(u"find $HOME/.local/share/icons/*/index.theme -maxdepth 1 2>/dev/null|cut -d\"/\" -f7"_s).output);
    }

    if (value == "fluxbox"_L1) {
        themes = runCmd(u"find /usr/share/mxflux/styles/ ! -name *attribution* -maxdepth 1 2>/dev/null |cut -d\"/\" -f6"_s).output;
        themes.append("\n");
        themes.append(runCmd(u"find $HOME/.fluxbox/styles/ ! -name *attribution* -maxdepth 1 2>/dev/null |cut -d\"/\" -f6"_s).output);
        themes.append("\n");
        if (ui->checkBoxFluxboxLegacyStyles->isChecked()){
            themes.append(runCmd(u"find /usr/share/fluxbox/styles/ ! -name *attribution* -maxdepth 1 2>/dev/null |cut -d\"/\" -f6"_s).output);
            themes.append("\n");
        }
    }

    if (value == "cursors"_L1){
        themes = runCmd(u"find /usr/share/icons/*/ -maxdepth 1 2>/dev/null |grep cursors |cut -d\"/\" -f5"_s).output;
        themes.append("\n");
        themes.append(runCmd(u"find $HOME/.icons/*/ -maxdepth 1 2>/dev/null |grep cursors |cut -d\"/\" -f5"_s).output);
        themes.append("\n");
        themes.append(runCmd(u"find $HOME/.local/share/icons/*/ -maxdepth 1 2>/dev/null |grep cursors |cut -d\"/\" -f7"_s).output);
        themes.append("\n");
        themes.append("default");
    }



    themelist = themes.split(u"\n"_s);
    themelist.removeDuplicates();
    themelist.removeAll("");
    themelist.sort(Qt::CaseInsensitive);

    if (value == "plasma"_L1){
        ui->listWidgetTheme->clear();
        QRegularExpression regex(u".*current.*"_s, QRegularExpression::CaseInsensitiveOption);
        int index = themelist.indexOf(regex);
        themelist[index] = themelist[index].section('(',0,0);
        //index of theme in list
        if (verbose) qDebug() << "index is " << index << themelist[index];
        ui->listWidgetTheme->addItems(themelist);
        ui->listWidgetTheme->setCurrentRow(index);
    }
    if (value == "colorscheme"_L1){
        ui->listWidgetWMtheme->clear();
        QRegularExpression regex(u".*current.*"_s, QRegularExpression::CaseInsensitiveOption);
        int index = themelist.indexOf(regex);
        themelist[index] = themelist[index].section('(',0,0);
        //index of theme in list
        if (verbose) qDebug() << "index is " << index << themelist[index];
        ui->listWidgetWMtheme->addItems(themelist);
        ui->listWidgetWMtheme->setCurrentRow(index);
    }
    if (value == "kdecursors"_L1){
        ui->listWidgetCursorThemes->clear();
        QRegularExpression regex(u".*current.*"_s, QRegularExpression::CaseInsensitiveOption);
        int index = themelist.indexOf(regex);
        for (int i = 0; i < themelist.size(); ++i ){
            themelist[i] = themelist[i].section('[',1,1).section(']',0,0);
        }
        //index of theme in list
        if (verbose) qDebug() << "index is " << index << themelist[index];
        ui->listWidgetCursorThemes->addItems(themelist);
        ui->listWidgetCursorThemes->setCurrentRow(index);
    }

    if ( value == "gtk-3.0"_L1 ) {
        ui->listWidgetTheme->clear();
        ui->listWidgetTheme->addItems(themelist);
        //set current
        if (isXfce){
            current = runCmd(u"xfconf-query -c xsettings -p /Net/ThemeName"_s).output;
        } else if (isFluxbox){
            current = runCmd(u"grep gtk-theme-name ~/.config/gtk-3.0/settings.ini | cut -d\"=\" -f2"_s).output;
        }
        //index of theme in list
        ui->listWidgetTheme->setCurrentRow(themelist.indexOf(current));
    }
    if ( value == "xfwm4"_L1) {

        ui->listWidgetWMtheme->clear();
        ui->listWidgetWMtheme->addItems(themelist);
        current = runCmd(u"xfconf-query -c xfwm4 -p /general/theme"_s).output;
        ui->listWidgetWMtheme->setCurrentRow(themelist.indexOf(current));
    }

    if ( value == "fluxbox"_L1) {

        ui->listWidgetWMtheme->clear();
        ui->listWidgetWMtheme->addItems(themelist);
        current = runCmd(u"basename $(grep styleFile $HOME/.fluxbox/init |grep -v ^# |grep -oE '/[^ ]+')"_s).output;
        qDebug() << "current style is " << current;
        ui->listWidgetWMtheme->setCurrentRow(themelist.indexOf(current));
    }

    if ( value == "cursors"_L1){
        ui->listWidgetCursorThemes->clear();
        ui->listWidgetCursorThemes->addItems(themelist);
        if (isXfce){
            current = runCmd(u"xfconf-query -c xsettings -p /Gtk/CursorThemeName"_s).output;
            if (current.isEmpty()) current = "default"_L1;
        } else if (isFluxbox){
            if (QFile::exists(home_path + "/.icons/default/index.theme"_L1)) {
                current = runCmd(u"grep Inherits $HOME/.icons/default/index.theme |cut -d= -f2"_s).output;
            } else {
                  current = "default"_L1;
            }
        }
        ui->listWidgetCursorThemes->setCurrentRow(themelist.indexOf(current));
    }

    if ( value == "icons"_L1) {
        if (verbose) qDebug() << "themelist" << themelist;
        QStringList iconthemelist = themelist;
        for (const QString &item : iconthemelist) {
            const QString& icontheme = item;
            if (verbose) qDebug() << "icontheme" << icontheme;
            QString test = runCmd("find /usr/share/icons/"_L1 + icontheme + " -maxdepth 1 -mindepth 1 -type d |cut -d\"/\" -f6"_L1).output;
            if ( test == "cursors"_L1 ) {
                themelist.removeAll(icontheme);
            }
        }
        themelist.removeAll(u"default.kde4"_s);
        themelist.removeAll(u"default"_s);
        themelist.removeAll(u"hicolor"_s);
        ui->listWidgeticons->clear();
        ui->listWidgeticons->addItems(themelist);
        //current icon set
        if (isXfce){
            current = runCmd(u"xfconf-query -c xsettings -p /Net/IconThemeName"_s).output;
        } else if (isFluxbox){
            current = runCmd(u"grep gtk-icon-theme-name $HOME/.config/gtk-3.0/settings.ini |grep -v ^# | cut -d\"=\" -f2"_s).output;
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

    if ( desktop == "XFCE"_L1 ) {
        if ( type == "gtk-3.0"_L1 ) {
            cmd = "xfconf-query -c xsettings -p /Net/ThemeName -s \""_L1 + theme + '"';
            cmd1 ="gsettings set org.gnome.desktop.interface gtk-theme \""_L1 + theme + '"';
            if (theme.contains("dark"_L1, Qt::CaseInsensitive) || theme.contains("Blackbird"_L1)){ //blackbird special case
                cmd2="gsettings set org.gnome.desktop.interface color-scheme prefer-dark"_L1;
            } else {
                cmd2="gsettings set org.gnome.desktop.interface color-scheme default"_L1;
            }
        }
        if ( type == "xfwm4"_L1 ) {
            cmd = "xfconf-query -c xfwm4 -p /general/theme -s \""_L1 + theme + '"';
        }

        if ( type == "icons"_L1 ) {
            cmd = "xfconf-query -c xsettings -p /Net/IconThemeName -s \""_L1 + theme + '"';
        }

        if (type == "cursor"_L1) {
            cmd = "xfconf-query -c xsettings -p /Gtk/CursorThemeName -s \""_L1 + theme + '"';
        }
        system(cmd.toUtf8());

    } else if ( desktop == "KDE"_L1 ){
        if ( type == "plasma"_L1 ) {
            cmd = "LANG=C plasma-apply-desktoptheme "_L1 + theme;
        }
        if ( type == "colorscheme"_L1 ) {
            cmd = "LANG=C plasma-apply-colorscheme "_L1 + theme;
        }

        if ( type == "icons"_L1 ) {
            cmd = "LANG=C /usr/lib/x86_64-linux-gnu/libexec/plasma-changeicons "_L1 + theme;
        }

        if (type == "kdecursor"_L1) {
            cmd = "LANG=C plasma-apply-cursortheme "_L1 + theme;
        }
        system(cmd.toUtf8());

    } else if ( desktop == "fluxbox"_L1 ){
        QString home_path = QDir::homePath();
        if ( type == "gtk-3.0"_L1 ) {
            if (runCmd(u"grep gtk-theme-name $HOME/.config/gtk-3.0/settings.ini"_s).exitCode == 0) {
                cmd = "sed -i 's/gtk-theme-name=.*/gtk-theme-name="_L1 + theme + "/' $HOME/.config/gtk-3.0/settings.ini"_L1;
            } else {
                cmd = "echo gtk-theme-name="_L1 + theme + "\" >> $HOME/.config/gtk-3.0/settings.ini"_L1;
            }
            system(cmd.toUtf8());

            if (runCmd(u"grep gtk-theme-name $HOME/.gtkrc-2.0"_s).exitCode == 0) {
                cmd = "sed -i 's/gtk-theme-name=.*/gtk-theme-name=\""_L1 + theme + "\"/' $HOME/.gtkrc-2.0"_L1;
            } else {
                cmd = "echo gtk-theme-name=\""_L1 + theme + "\" >> $HOME/.gtkrc-2.0"_L1;
            }
            system(cmd.toUtf8());

            cmd1 ="gsettings set org.gnome.desktop.interface gtk-theme \""_L1 + theme + '"';
            if (theme.contains("dark"_L1, Qt::CaseInsensitive) || theme.contains("Blackbird"_L1)){ //blackbird special case
                cmd2="gsettings set org.gnome.desktop.interface color-scheme prefer-dark"_L1;
                if (runCmd(u"grep gtk-application-prefer-dark-theme $HOME/.config/gtk-3.0/settings.ini"_s).exitCode == 0) {
                    runCmd(u"sed -i 's/gtk-application-prefer-dark-theme=.*/gtk-application-prefer-dark-theme=true/' $HOME/.config/gtk-3.0/settings.ini"_s);
                } else {
                    runCmd(u"echo gtk-application-prefer-dark-theme=true/' >> $HOME/.config/gtk-3.0/settings.ini"_s);
                }
            } else {
                cmd2="gsettings set org.gnome.desktop.interface color-scheme default"_L1;
                if (runCmd(u"grep gtk-application-prefer-dark-theme $HOME/.config/gtk-3.0/settings.ini"_s).exitCode == 0) {
                    runCmd(u"sed -i 's/gtk-application-prefer-dark-theme=.*/gtk-application-prefer-dark-theme=false/' $HOME/.config/gtk-3.0/settings.ini"_s);
                } else {
                    runCmd(u"echo gtk-application-prefer-dark-theme=false/' >> $HOME/.config/gtk-3.0/settings.ini"_s);
                }
            }

            if (QFile::exists(u"/usr/bin/preview-mx"_s)){
                cmd = "preview-mx &"_L1;
                system(cmd.toUtf8());
            }
        }
        if ( type == "fluxbox"_L1 ) {
            //always take home folder version, then mx-fluxbox version, then fluxbox version if conflicts arise
            QString filepath = home_path + "/.fluxbox/styles/"_L1 + theme;
            if (QFile(filepath).exists()){
                home_path.replace('/', "\\/"_L1);
                cmd = "sed -i 's/session.styleFile:.*/session.styleFile: "_L1 + home_path + "\\/.fluxbox\\/styles\\/"_L1 + theme + "/' $HOME/.fluxbox/init && fluxbox-remote reconfigure && fluxbox-remote reloadstyle"_L1;
            } else {
                if (QFile::exists("/usr/share/fluxbox/styles/"_L1 + theme)){
                    cmd = "sed -i 's/session.styleFile:.*/session.styleFile: \\/usr\\/share\\/fluxbox\\/styles\\/"_L1 + theme + "/' $HOME/.fluxbox/init && fluxbox-remote reconfigure && fluxbox-remote reloadstyle"_L1;
                }
                if (QFile::exists("/usr/share/mxflux/styles/"_L1 + theme)){
                    cmd = "sed -i 's/session.styleFile:.*/session.styleFile: \\/usr\\/share\\/mxflux\\/styles\\/"_L1 + theme + "/' $HOME/.fluxbox/init && fluxbox-remote reconfigure && fluxbox-remote reloadstyle"_L1;
                }
            }
            system(cmd.toUtf8());
        }
        //for fluxbox, edit ~/.config/gtk-3.0/settings.ini and ~/.gtkrc-2.0 has quotes
        if ( type == "icons"_L1 ) {

            if (runCmd(u"grep gtk-icon-theme-name $HOME/.config/gtk-3.0/settings.ini"_s).exitCode == 0) {
                cmd = "sed -i 's/gtk-icon-theme-name=.*/gtk-icon-theme-name="_L1 + theme + "/' $HOME/.config/gtk-3.0/settings.ini"_L1;
            } else {
                cmd = "echo gtk-icon-theme-name="_L1 + theme + "\" >> $HOME/.config/gtk-3.0/settings.ini"_L1;
            }
            system(cmd.toUtf8());
            if (runCmd(u"grep gtk-icon-theme-name $HOME/.gtkrc-2.0"_s).exitCode == 0) {
                cmd = "sed -i 's/gtk-icon-theme-name=.*/gtk-icon-theme-name=\""_L1 + theme + "\"/' $HOME/.gtkrc-2.0"_L1;
            } else {
                cmd = "echo gtk-icon-theme-name=\""_L1 + theme + "\" >> $HOME/.gtkrc-2.0"_L1;
            }
            system(cmd.toUtf8());

            if (QFile::exists(u"/usr/bin/preview-mx"_s)){
                cmd = "preview-mx &"_L1;
                system(cmd.toUtf8());
            }
        }

        //for fluxbox, edit ~/.config/gtk-3.0/settings.ini, ~/.gtkrc-2.0 has quotes, and .icons/default/index.theme (create if it doesn't exist)
        if ( type == "cursor"_L1 ) {

            if (runCmd(u"grep gtk-cursor-theme-name $HOME/.config/gtk-3.0/settings.ini"_s).exitCode == 0) {
                cmd = "sed -i 's/gtk-cursor-theme-name=.*/gtk-cursor-theme-name="_L1 + theme + "/' $HOME/.config/gtk-3.0/settings.ini"_L1;
            } else {
                cmd = "echo gtk-cursor-theme-name="_L1 + theme + "\" >> $HOME/.config/gtk-3.0/settings.ini"_L1;
            }
            system(cmd.toUtf8());
            if (runCmd(u"grep gtk-cursor-theme-name $HOME/.gtkrc-2.0"_s).exitCode == 0) {
                cmd = "sed -i 's/gtk-cursor-theme-name=.*/gtk-cursor-theme-name=\""_L1 + theme + "\"/' $HOME/.gtkrc-2.0"_L1;
            } else {
                cmd = "echo gtk-cursor-theme-name=\""_L1 + theme + "\" >> $HOME/.gtkrc-2.0"_L1;
            }
            system(cmd.toUtf8());
            if ( theme == "default"_L1){
                runCmd(u"rm -R $HOME/.icons/default"_s);
            } else {
                if (!QDir(home_path + "/.icons/default"_L1).exists() ){
                    runCmd(u"mkdir -p $HOME/.icons/default"_s);
                }
                runCmd(u"echo [Icon Theme] > $HOME/.icons/default/index.theme"_s);
                runCmd(u"echo Name=Default >> $HOME/.icons/default/index.theme"_s);
                runCmd(u"echo Comment=Default Cursor Theme >> $HOME/.icons/default/index.theme"_s);
                runCmd(u"echo Comment=Default Cursor Theme >> $HOME/.icons/default/index.theme"_s);
                runCmd("echo Inherits="_L1 + theme + " >> $HOME/.icons/default/index.theme"_L1);
            }
            cmd = "fluxbox-remote restart"_L1;
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
            settheme(u"gtk-3.0"_s, currentText, u"XFCE"_s);
        } else if (isFluxbox){
            settheme(u"gtk-3.0"_s, currentText, u"fluxbox"_s);
        } else if (isKDE) {
            settheme(u"plasma"_s, currentText, u"KDE"_s);
        }
    }
}

void defaultlook::on_listWidgetWMtheme_currentTextChanged(const QString &currentText) const
{
    if ( themeflag ) {
        if (isXfce) {
            settheme(u"xfwm4"_s, currentText, u"XFCE"_s);
        } else if (isFluxbox){
            settheme(u"fluxbox"_s, currentText, u"fluxbox"_s);
        } else if (isKDE) {
            settheme(u"colorscheme"_s, currentText, u"KDE"_s);
        }
    }
}

void defaultlook::on_listWidgeticons_currentTextChanged(const QString &currentText) const
{
    if ( themeflag ) {
        if (isXfce) {
            settheme(u"icons"_s, currentText, u"XFCE"_s);
        } else if (isFluxbox){
            settheme(u"icons"_s, currentText, u"fluxbox"_s);
        } else if (isKDE) {
            settheme(u"icons"_s, currentText, u"KDE"_s);
        }
    }
}

void defaultlook::on_listWidgetCursorThemes_currentTextChanged(const QString &currentText)
{
    if ( themeflag ) {
        if (isXfce) {
            settheme(u"cursor"_s, currentText, u"XFCE"_s);
        } else if (isFluxbox){
            settheme(u"cursor"_s, currentText, u"fluxbox"_s);
        } else if (isKDE) {
            settheme(u"kdecursor"_s, currentText, u"KDE"_s);
        }
    }
}


void defaultlook::tabWidget_currentChanged(int index)
{
    if (!displaysetupflag) {
        if (index == Tab::Display) {
            setupDisplay();
            displaysetupflag = true;
        }
    }
}

void defaultlook::on_checkBoxCSD_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_checkBoxbluetoothAutoEnable_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
    bluetoothautoenableflag = !bluetoothautoenableflag;
    if (verbose) qDebug() << "bluetooth flag is " << bluetoothautoenableflag;
}

void defaultlook::pushManageTint2_clicked()
{
    this->hide();
    system("/usr/bin/mxfb-tint2-manager");
    this->show();

}

void defaultlook::on_checkBoxInstallRecommends_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
    enable_recommendsflag = true;
}

void defaultlook::setupThunar()
{
    ui->checkThunarResetCustomActions->setChecked(false);
    QString test;

    //check single click thunar status
    test = runCmd(u"xfconf-query  -c thunar -p /misc-single-click"_s).output;
    ui->checkThunarSingleClick->setChecked(test == "true"_L1);

    //check split window status
    test = runCmd(u"xfconf-query  -c thunar -p /misc-open-new-windows-in-split-view"_s).output;
    ui->checkThunarSplitView->setChecked(test == "true"_L1);
    //check split view horizontal or vertical.  default false is vertical, true is horizontal;
    test = runCmd(u"xfconf-query  -c thunar -p /misc-vertical-split-pane"_s).output;
    ui->checkThunarSplitViewHorizontal->setChecked(test == "true"_L1);
}
void defaultlook::applyThunar()
{
    // Single-click
    if (ui->checkThunarSingleClick->isChecked()) {
        runCmd(u"xfconf-query  -c thunar -p /misc-single-click -t bool -s true --create"_s);
    } else {
        runCmd(u"xfconf-query  -c thunar -p /misc-single-click -t bool -s false --create"_s);
    }
    // Reset right-click custom actions
    if (ui->checkThunarResetCustomActions->isChecked()) {
        system("cp /home/$USER/.config/Thunar/uca.xml /home/$USER/.config/Thunar/uca.xml.$(date +%Y%m%H%M%S)");
        runCmd(u"cp /etc/skel/.config/Thunar/uca.xml /home/$USER/.config/Thunar/uca.xml"_s);
    }
    // Split view
    if (ui->checkThunarSplitView->isChecked()) {
        runCmd(u"xfconf-query  -c thunar -p /misc-open-new-windows-in-split-view -t bool -s true --create"_s);
    } else {
        runCmd(u"xfconf-query  -c thunar -p /misc-open-new-windows-in-split-view --reset"_s);
    }
    // Split view thunar horizontal or vertical
    if (ui->checkThunarSplitViewHorizontal->isChecked()) {
        runCmd(u"xfconf-query -c thunar -p /misc-vertical-split-pane -t bool -s true --create"_s);
    } else {
        runCmd(u"xfconf-query -c thunar -p /misc-vertical-split-pane --reset"_s);
    }

    setupThunar();
}
void defaultlook::slotThunarChanged()
{
    if (ui->tabFluxbox->isVisible()) {
        ui->pushFluxboxApply->setEnabled(true);
    } else {
        ui->pushXfceApply->setEnabled(true);
    }
}

void defaultlook::on_checkBoxDisableFluxboxMenuGeneration_clicked()
{
    ui->ButtonApplyEtc->setEnabled(true);
}

void defaultlook::on_toolButtonSuperFileBrowser_clicked()
{
    QString customcommand = QFileDialog::getOpenFileName(this, tr("Select application to run","will show in file dialog when selection an application to run"), u"/usr/bin"_s);
    QString cmd;

    //process file
    QFileInfo customcommandcheck(customcommand);
    if (customcommandcheck.fileName().endsWith(".desktop"_L1)){
        cmd = runCmd("grep Exec= "_L1 + customcommand).output.section('=',1,1).section('%',0,0).trimmed();
        cmd = runCmd("which "_L1 + cmd).output;
    } else {
        cmd = runCmd("which "_L1 + customcommand).output;
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
    if (!QFile(home_path + "/.config/xfce-superkey/xfce-superkey.conf"_L1).exists()){
        runCmd("mkdir -p "_L1 + home_path + "/.config/xfce-superkey/xfce-superkey.conf"_L1);
        runCmd("cp /usr/share/xfce-superkey/xfce-superkey.conf "_L1 + home_path + "/.config/xfce-superkey/xfce-superkey.conf"_L1);
    }
    QString cmd = ui->lineEditSuperCommand->text();
    //add command if no uncommented lines
    if (runCmd(u"grep -m1 -v -e '^#' -e '^$' $HOME/.config/xfce-superkey/xfce-superkey.conf"_s).output.isEmpty()){
        runCmd("echo "_L1 + cmd + ">> $HOME/.config/xfce-superkey/xfce-superkey.conf"_L1);
    } else { //replace first uncommented line with new command
        runCmd("sed -i '/^[^#]/s;.*;"_L1 + cmd + ";' $HOME/.config/xfce-superkey/xfce-superkey.conf"_L1);
    }
    //restart xfce-superkey
    runCmd(u"pkill xfce-superkey"_s);
    runCmd(u"xfce-superkey-launcher"_s);
    setupSuperKey();
}

void defaultlook::setupSuperKey()
{
    QString test = runCmd(u"grep -m1 -v -e '^#' -e '^$' $HOME/.config/xfce-superkey/xfce-superkey.conf"_s).output;
    ui->pushButtonSuperKeyApply->setEnabled(false);
    if (!test.isEmpty()){
        ui->lineEditSuperCommand->setText(test);
    }

}

void defaultlook::on_lineEditSuperCommand_textChanged(const QString &)
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

void defaultlook::on_spinBoxPointerSize_valueChanged(int)
{
    set_cursor_size();
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

void defaultlook::on_checkBoxFluxboxLegacyStyles_stateChanged(int)
{
    populatethemelists(u"fluxbox"_s);
    saveSettings();
}

void defaultlook::saveSettings()
{
    QSettings settings(u"MX-Linux"_s, u"MX-Tweak"_s);
    settings.setValue("checkbox_state", ui->checkBoxFluxboxLegacyStyles->checkState());
}

void defaultlook::loadSettings()
{
    QSettings settings(u"MX-Linux"_s, u"MX-Tweak"_s);
    bool checked = settings.value("checkbox_state", false).toBool();
    ui->checkBoxFluxboxLegacyStyles->setChecked(checked);
}
