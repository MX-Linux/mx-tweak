#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include "ui_defaultlook.h"
#include "cmd.h"
#include "tweak_misc.h"

using namespace Qt::Literals::StringLiterals;

TweakMisc::TweakMisc(Ui::defaultlook *ui, bool verbose, QObject *parent) noexcept
    : QObject(parent), ui(ui), verbose(verbose)
{
    if (!QFileInfo::exists(u"/usr/sbin/lightdm"_s)) {
        ui->checkMiscLightdmReset->hide();
    }
    // These will be shown by setup() if they make sense.
    ui->checkMiscIntelDriver->hide();
    ui->labelMiscIntelDriver->hide();
    ui->checkMiscTearfreeAMD->hide();
    ui->labelMiscTearfreeAMD->hide();
    ui->checkMiscTearfreeRadeon->hide();
    ui->labelMiscTearfreeRadeon->hide();

    setup();

    // Connections between widgets
    connect(ui->checkMiscHostName, &QCheckBox::toggled, ui->textMiscHostName, &QLineEdit::setEnabled);
    // Settings change connections
    connect(ui->checkMiscLightdmReset, &QCheckBox::clicked, this, &TweakMisc::slotSettingChanged);
    connect(ui->checkMiscCSD, &QCheckBox::clicked, this, &TweakMisc::slotSettingChanged);
    connect(ui->checkMiscDisableFluxboxMenuGeneration, &QCheckBox::clicked, this, &TweakMisc::slotSettingChanged);
    connect(ui->checkMiscDisplayManager, &QCheckBox::clicked, this, &TweakMisc::slotSettingChanged);
    connect(ui->checkMiscMountInternalDrivesNonRoot, &QCheckBox::clicked, this, &TweakMisc::slotSettingChanged);
    connect(ui->radioMiscSudoUser, &QRadioButton::clicked, this, &TweakMisc::slotSettingChanged);
    connect(ui->radioMiscSudoRoot, &QRadioButton::clicked, this, &TweakMisc::slotSettingChanged);
    connect(ui->checkMiscHostName, &QCheckBox::clicked, this, &TweakMisc::slotSettingChanged);
    // Other connections
    connect(ui->pushMiscApply, &QPushButton::clicked, this, &TweakMisc::pushMiscApply_clicked);
    connect(ui->checkMiscSandbox, &QCheckBox::clicked, this, &TweakMisc::checkMiscSandbox_clicked);
    connect(ui->checkMiscBluetoothAutoEnable, &QCheckBox::clicked, this, &TweakMisc::checkMiscBluetoothAutoEnable_clicked);
    connect(ui->checkMiscBluetoothBattery, &QCheckBox::clicked, this, &TweakMisc::checkMiscBluetoothBattery_clicked);
    connect(ui->checkMiscKVMVirtLoad, &QCheckBox::clicked, this, &TweakMisc::checkMiscKVMVirtLoad_clicked);
    connect(ui->checkMiscInstallRecommends, &QCheckBox::clicked, this, &TweakMisc::checkMiscInstallRecommends_clicked);
    connect(ui->checkMiscLiqKernelUpdates, &QCheckBox::clicked, this, &TweakMisc::checkMiscLiqKernelUpdates_clicked);
    connect(ui->checkMiscDebianKernelUpdates, &QCheckBox::clicked, this, &TweakMisc::checkMiscDebianKernelUpdates_clicked);
    connect(ui->checkMiscIntelDriver, &QCheckBox::clicked, this, &TweakMisc::checkMiscIntelDriver_clicked);
    connect(ui->checkMiscTearfreeAMD, &QCheckBox::clicked, this, &TweakMisc::checkMiscTearfreeAMD_clicked);
    connect(ui->checkMiscTearfreeRadeon, &QCheckBox::clicked, this, &TweakMisc::checkMiscTearfreeRadeon_clicked);
}

void TweakMisc::setup() noexcept
{
    if (!QFileInfo::exists(u"/etc/lightdm/lightdm-gtk-greeter.conf"_s)) {
        ui->checkMiscLightdmReset->hide();
    }
    QString home_path = QDir::homePath();
    QString DESKTOP = runCmd(u"echo $XDG_SESSION_DESKTOP"_s).output;
    if (verbose) qDebug() << "setupetc nocsd desktop is:" << DESKTOP;

    ui->checkMiscLightdmReset->setChecked(false);
    QString test;

    ui->pushMiscApply->setEnabled(false);
    if (ui->pushMiscApply->icon().isNull()) {
        ui->pushMiscApply->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }
    //set values for checkboxes

    //fluxbox menu auto generation on package install, removal, and upgrades
    if ( QFile::exists(u"/usr/bin/mxfb-menu-generator"_s)){
        if (QFile::exists(home_path + "/.fluxbox/mxfb-menu-generator-disabled.chk"_L1)){
            ui->checkMiscDisableFluxboxMenuGeneration->setChecked(false);
        } else {
            ui->checkMiscDisableFluxboxMenuGeneration->setChecked(true);
        }
    } else {
        ui->checkMiscDisableFluxboxMenuGeneration->hide();
    }

    //setup udisks option
    QFileInfo fileinfo(u"/etc/tweak-udisks.chk"_s);
    if (fileinfo.exists()) {
        ui->checkMiscMountInternalDrivesNonRoot->setChecked(true);
    } else {
        ui->checkMiscMountInternalDrivesNonRoot->setChecked(false);
    }

    //setup sudo override function

    int rootest = runCmd(u"pkexec /usr/lib/mx-tweak/mx-tweak-rootcheck.sh"_s).exitCode;
    if ( rootest == 0 ){
        ui->radioMiscSudoRoot->setChecked(true);
    } else {
        ui->radioMiscSudoUser->setChecked(true);
    }

    //if root accout disabled, disable root authentication changes
    test = runCmd(u"pkexec /usr/lib/mx-tweak/mx-tweak-check.sh"_s).output;
    if (test.contains("NP"_L1)) {
        ui->radioMiscSudoRoot->setEnabled(false);
        ui->radioMiscSudoUser->setEnabled(false);
        ui->labelSudo->setEnabled(false);
    }

    //setup user namespaces option (99-sandbox-mx.conf)
    flags.sandbox = false;
    QString userns_clone = runCmd(u"/usr/sbin/sysctl -n kernel.unprivileged_userns_clone"_s).output;
    //QString yama_ptrace = runCmd("/usr/sbin/sysctl -n kernel.yama.ptrace_scope").output;
    if (verbose) qDebug() << "userns_clone is: " << userns_clone;
    if (userns_clone == "0"_L1 || userns_clone == "1"_L1) {
        //if (userns_clone == "1"_L1 && yama_ptrace == "1"_L1) {
        ui->checkMiscSandbox->setChecked(userns_clone == "1"_L1);
    } else {
        ui->checkMiscSandbox->hide();
    }

    //setup bluetooth auto enable, hide box if config file doesn't exist
    if (QFile::exists(u"/etc/bluetooth/main.conf"_s)){
        flags.bluetoothAutoEnable = false;
        test = runCmd(u"grep ^AutoEnable /etc/bluetooth/main.conf"_s).output;
        test = test.section('=',1,1);
        ui->checkMiscBluetoothAutoEnable->setChecked(test == "true"_L1);
    } else {
        ui->checkMiscBluetoothAutoEnable->hide();
    }

    //setup bluetooth battery info
    if (QFile::exists(u"/etc/bluetooth/main.conf"_s)){
        flags.bluetoothBattery = false;
        test = runCmd(u"grep -E '^(#\\s*)?Experimental' /etc/bluetooth/main.conf"_s).output;
        if (verbose) qDebug() << "bluetooth battery " << test;
        if (test.contains('#')){
            ui->checkMiscBluetoothBattery->setChecked(false);
        } else if ( test.contains("true"_L1) ){
            ui->checkMiscBluetoothBattery->setChecked(true);
        } else if ( test.contains("false"_L1)) {
            ui->checkMiscBluetoothBattery->setChecked(false);
        }
    } else {
        ui->checkMiscBluetoothBattery->hide();
    }

    //setup early KVM module loading
    test = runCmd(u"LANG=C grep \"enable_virt_at_load=0\" /etc/modprobe.d/* | grep kvm"_s).output;
    if (!test.isEmpty()){
        if (!test.section(':',1,1).startsWith('#')){
            ui->checkMiscKVMVirtLoad->setChecked(true);
            kvmConfFile=test.section(':',0,0);
            if (verbose) qDebug() << "kvm conf file is " << kvmConfFile;
        } else {
            ui->checkMiscKVMVirtLoad->setChecked(false);
            kvmConfFile = "/etc/modprobe.d/kvm.conf"_L1;
        }
    } else {
        ui->checkMiscKVMVirtLoad->setChecked(false);
        kvmConfFile = "/etc/modprobe.d/kvm.conf"_L1;
    }
    //set flag false so future changes processed, but not an unchanged checkbox
    flags.kvm=false;

    //setup apt install_recommends
    //enable checkbox only if Install-Recommends is set to 1. default is 0 or no if no existanct apt.conf
    if (QFile::exists(u"/etc/apt/apt.conf"_s)){
        test = runCmd(u"grep Install-Recommends /etc/apt/apt.conf"_s).output;
        if ( test.contains('1')){
            ui->checkMiscInstallRecommends->setChecked(true);
        } else {
            ui->checkMiscInstallRecommends->setChecked(false);
        }
    } else {
        ui->checkMiscInstallRecommends->setChecked(false);
    }

    //setup kernel auto updates

    if (runCmd(u"LC_ALL=C dpkg --status linux-image-amd64 linux-image-686 linux-image-686-pae 2>/dev/null |grep 'ok installed'"_s).output.isEmpty()){
        ui->checkMiscDebianKernelUpdates->setChecked(false);
        ui->checkMiscDebianKernelUpdates->hide();
    } else {
        ui->checkMiscDebianKernelUpdates->setChecked(true);
    }
    if (runCmd(u"LC_ALL=C dpkg --status linux-image-liquorix-amd64 2>/dev/null |grep 'ok installed'"_s).output.isEmpty()){
        ui->checkMiscLiqKernelUpdates->setChecked(false);
        ui->checkMiscLiqKernelUpdates->hide();
    } else {
        ui->checkMiscLiqKernelUpdates->setChecked(true);
    }
    QString autoupdate = runCmd(u"apt-mark showhold"_s).output;
    if ( autoupdate.contains("linux-image-686"_L1) || autoupdate.contains("linux-image-amd64"_L1) ){
        ui->checkMiscDebianKernelUpdates->setChecked(false);
    }

    if ( autoupdate.contains("linux-image-liquorix-amd64"_L1) ){
        ui->checkMiscLiqKernelUpdates->setChecked(false);
    }
    flags.updateKernelDebian = false;
    flags.updateKernelLiquorix = false;

    //hostname
    ui->checkMiscHostName->setChecked(false);
    ui->textMiscHostName->setEnabled(false);
    ui->textMiscHostName->setText(runProc(u"hostname"_s).output);

    //setup NOCSD GTK3 option
    if (!QFileInfo::exists(u"/usr/bin/gtk3-nocsd"_s)) {
        if (verbose) qDebug() << "gtk3-nocsd not found";
        ui->checkMiscCSD->hide();
    } else {
        if (verbose) qDebug() << "gtk3-nocsd found";
    }
    if (verbose) {
        qDebug() << "home path nocsd is"_L1 << home_path + "/.config/MX-Linux/nocsd/"_L1 + DESKTOP;
    }
    if (QFileInfo::exists(home_path + "/.config/MX-Linux/nocsd/"_L1 + DESKTOP)) {
        ui->checkMiscCSD->setChecked(false);
    } else {
        ui->checkMiscCSD->setChecked(true);
    }

    flags.intel = false;
    flags.amdgpu = false;
    flags.radeon =false;
    flags.enableRecommends = false;
    //setup Intel checkbox

    QString partcheck = runCmd(uR"(for i in $(lspci -n | awk '{print $2,$1}' | grep -E '^(0300|0302|0380)' | cut -f2 -d\ ); do lspci -kns "$i"; done)"_s).output;
    if (verbose) qDebug()<< "partcheck = " << partcheck;

    if ( partcheck.contains("i915"_L1)) {
        ui->checkMiscIntelDriver->show();
        ui->labelMiscIntelDriver->show();
    }

    if ( partcheck.contains("Kernel driver in use: amdgpu"_L1)) {
        ui->checkMiscTearfreeAMD->show();
        ui->labelMiscTearfreeAMD->show();
    }

    if ( partcheck.contains("Kernel driver in use: radeon"_L1)) {
        ui->checkMiscTearfreeRadeon->show();
        ui->labelMiscTearfreeRadeon->show();
    }

    QFileInfo intelfile(u"/etc/X11/xorg.conf.d/20-intel.conf"_s);
    ui->checkMiscIntelDriver->setChecked(intelfile.exists());

    QFileInfo amdfile(u"/etc/X11/xorg.conf.d/20-amd.conf"_s);
    ui->checkMiscTearfreeAMD->setChecked(amdfile.exists());

    QFileInfo radeonfile(u"/etc/X11/xorg.conf.d/20-radeon.conf"_s);
    ui->checkMiscTearfreeRadeon->setChecked(radeonfile.exists());

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
                    currentDisplayManager=in.readAll().section('/',3,3).remove('\n');
                    defaultdisplay.close();
                }
            } else {
                currentDisplayManager = runCmd(u"ps -aux |grep  -E '/usr/.*bin/sddm|/usr/.*bin*/gdm3|.*bin*/lightdm|.*bin*/slim|.*bin*/slimski|.*bin*/xdm.*bin*/wdm.*bin*/lxdm.*bin*/nodm' |grep -v grep | awk '{print $11}'"_s).output.section('/', 3,3);
            }
            if (verbose) qDebug() << "current display manager is " << currentDisplayManager;
            ui->comboBoxDisplayManager->setCurrentText(currentDisplayManager);
        }
    } else {
        ui->checkMiscDisplayManager->hide();
        ui->comboBoxDisplayManager->hide();
    }
}

bool TweakMisc::validateHostName(const QString &hostname) const noexcept
{
    // see if name is reasonable
    if (hostname.isEmpty()) {
        QMessageBox::critical(ui->tabWidget, QString(),
            tr("Please enter a computer name.", "question to enter a name for the computer hostname"));
        return false;
    } else if (hostname.contains(QRegularExpression(u"[^0-9a-zA-Z-.]|^[.-]|[.-]$|\\.\\."_s))) {
        QMessageBox::critical(ui->tabWidget, QString(),
            tr("Sorry, your computer name contains invalid characters.\nYou'll have to select a different\nname before proceeding.", "unacceptable characters are found in hostname, pick a new name"));
        return false;
    }
    return true;
}

void TweakMisc::slotSettingChanged() noexcept
{
    ui->pushMiscApply->setEnabled(true);
}
void TweakMisc::pushMiscApply_clicked() noexcept
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
    ui->pushMiscApply->setEnabled(false);

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
    if (ui->checkMiscCSD->isChecked()) {
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
        if (! ui->checkMiscDisableFluxboxMenuGeneration->isChecked()){
            runCmd("echo '#this file is used to disable automatic updating of the All Apps menu' > "_L1 + home_path + "/.fluxbox/mxfb-menu-generator-disabled.chk"_L1);
        } else {
            runCmd("rm "_L1 + home_path + "/.fluxbox/mxfb-menu-generator-disabled.chk"_L1);
        }
    }

    //internal drive mounting for non root users
    if (ui->checkMiscMountInternalDrivesNonRoot->isChecked()) {
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
    if (ui->checkMiscLightdmReset->isChecked()) {
        lightdm_option = u"lightdm_reset"_s;
    }

    //graphics driver overrides

    if (flags.intel) {
        QFileInfo check_intel(u"/etc/X11/xorg.conf.d/20-intel.conf"_s);
        if ( check_intel.exists()) {
            //backup existing 20-intel.conf file to home folder
            runCmd(u"cp /etc/X11/xorg.conf.d/20-intel.conf /home/$USER/20-intel.conf.$(date +%Y%m%H%M%S)"_s);
        }
        if (ui->checkMiscIntelDriver->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            intel_option = u"enable_intel"_s;
        } else {
            //remove 20-intel.conf
            intel_option = u"disable_intel"_s;
        }
    }

    if (flags.amdgpu) {
        QFileInfo check_amd(u"/etc/X11/xorg.conf.d/20-amd.conf"_s);
        if ( check_amd.exists()) {
            //backup existing 20-amd.conf file to home folder
            runCmd(u"cp /etc/X11/xorg.conf.d/20-amd.conf /home/$USER/20-amd.conf.$(date +%Y%m%H%M%S)"_s);
        }
        if (ui->checkMiscTearfreeAMD->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            amd_option = u"enable_amd"_s;
        } else {
            //remove 20-amd.conf
            amd_option = u"disable_amd"_s;
        }
    }

    if (flags.radeon) {
        QFileInfo check_radeon(u"/etc/X11/xorg.conf.d/20-radeon.conf"_s);
        if ( check_radeon.exists()) {
            //backup existing 20-radeon.conf file to home folder
            runCmd(u"cp /etc/X11/xorg.conf.d/20-radeon.conf /home/$USER/20-radeon.conf.$(date +%Y%m%H%M%S)"_s);
        }
        if (ui->checkMiscTearfreeRadeon->isChecked()) {
            //copy mx-tweak version to xorg.conf.d directory
            radeon_option = u"enable_radeon"_s;
        } else {
            //remove 20-radeon.conf
            radeon_option = u"disable_radeon"_s;
        }
    }

    //bluetooth auto enable
    if (flags.bluetoothAutoEnable) {
        if (ui->checkMiscBluetoothAutoEnable->isChecked()) {
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
    if (flags.bluetoothBattery){
        if (ui->checkMiscBluetoothBattery->isChecked()){
            runCmd(u"pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh bluetooth_battery true"_s);
        } else {
            runCmd(u"pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh bluetooth_battery false"_s);
        }
    }

    //install recommends option
    if (flags.enableRecommends){
        if ( ui->checkMiscInstallRecommends->isChecked()){
            recommends_option = "install_recommends"_L1;
        } else
            recommends_option = "noinstall_recommends"_L1;
    }

    //deal with sudo override

    if (ui->radioMiscSudoUser->isChecked()) {
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
    if (flags.sandbox) {
        if (ui->checkMiscSandbox->isChecked()) {
            user_name_space_override_option = u"enable_sandbox"_s;
        } else {
            user_name_space_override_option = u"disable_sandbox"_s;
        }
    }

    //debian kernel updates
    if (flags.updateKernelDebian){
        if ( ui->checkMiscDebianKernelUpdates->isChecked()){
            debian_kernel_updates_option = "unhold_debian_kernel_updates"_L1;
            qDebug() << "debian update option" << debian_kernel_updates_option;
        } else {
            debian_kernel_updates_option = "hold_debian_kernel_updates"_L1;
        }
    }
    //liquorix kernel updates
    if (flags.updateKernelLiquorix){
        if (ui->checkMiscLiqKernelUpdates->isChecked()){
            liq_kernel_updates_option = "unhold_liquorix_kernel_updates"_L1;
        } else {
            liq_kernel_updates_option = "hold_liquorix_kernel_updates"_L1;
        }
    }

    //hostname setting
    //if name doesn't validate, don't make any changes to any options, and don't reset gui.
    if (ui->checkMiscHostName->isChecked()){
        if (validateHostName(ui->textMiscHostName->text())){
            runProc(u"pkexec"_s, {u"/usr/lib/mx-tweak/mx-tweak-lib.sh"_s,
                u"hostname"_s, ui->textMiscHostName->text()});
        } else {
            return;
        }
    }
    // display manager change
    if (ui->checkMiscDisplayManager->isChecked()){
        //don't do anything if selection is still default
        if (ui->comboBoxDisplayManager->currentText() != currentDisplayManager) {
            runProc(u"pkexec"_s, {u"/usr/lib/mx-tweak/mx-tweak-lib.sh"_s,
                u"displaymanager"_s, ui->comboBoxDisplayManager->currentText()});
        }
        ui->checkMiscDisplayManager->setChecked(false);
    }

    //kvm_early_switch
    if (flags.kvm) {
        runProc(u"pkexec"_s, {u"/usr/lib/mx-tweak/mx-tweak-lib.sh"_s, u"kvm_early_switch"_s,
            (ui->checkMiscKVMVirtLoad->isChecked() ? u"on"_s : u"off"_s), kvmConfFile});
    }

    //checkbox options
    if ( ! udisks_option.isEmpty() || ! sudo_override_option.isEmpty() || ! user_name_space_override_option.isEmpty() || ! intel_option.isEmpty() || ! lightdm_option.isEmpty() || ! amd_option.isEmpty() || ! radeon_option.isEmpty() || !bluetooth_option.isEmpty() || !recommends_option.isEmpty() || !debian_kernel_updates_option.isEmpty() || !liq_kernel_updates_option.isEmpty()){
        runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-lib.sh "_L1 + udisks_option + ' ' + sudo_override_option + ' ' + user_name_space_override_option + ' ' + intel_option + ' ' + amd_option + ' ' + radeon_option + ' ' + bluetooth_option + ' ' + recommends_option + ' ' + lightdm_option + ' ' + debian_kernel_updates_option + ' ' + liq_kernel_updates_option);
    }
    //reset gui
    setup();
}

void TweakMisc::checkMiscSandbox_clicked() noexcept
{
    ui->pushMiscApply->setEnabled(true);
    flags.sandbox = true;
}

void TweakMisc::checkMiscBluetoothAutoEnable_clicked() noexcept
{
    ui->pushMiscApply->setEnabled(true);
    flags.bluetoothAutoEnable = true;
}
void TweakMisc::checkMiscBluetoothBattery_clicked() noexcept
{
    ui->pushMiscApply->setEnabled(true);
    flags.bluetoothBattery = true;
}

void TweakMisc::checkMiscKVMVirtLoad_clicked() noexcept
{
    ui->pushMiscApply->setEnabled(true);
    flags.kvm = true;
}

void TweakMisc::checkMiscInstallRecommends_clicked() noexcept
{
    ui->pushMiscApply->setEnabled(true);
    flags.enableRecommends = true;
}

void TweakMisc::checkMiscLiqKernelUpdates_clicked() noexcept
{
    flags.updateKernelLiquorix = true;
    ui->pushMiscApply->setEnabled(true);
}
void TweakMisc::checkMiscDebianKernelUpdates_clicked() noexcept
{
    flags.updateKernelDebian = true;
    ui->pushMiscApply->setEnabled(true);
}

void TweakMisc::checkMiscIntelDriver_clicked() noexcept
{
    //toggle flag for action.  this way, if box was checked initially, the action won't take place again.
    flags.intel = true;
    ui->pushMiscApply->setEnabled(true);
}
void TweakMisc::checkMiscTearfreeAMD_clicked() noexcept
{
    //toggle flag for action.  this way, if box was checked initially, the action won't take place again.
    flags.amdgpu = true;
    ui->pushMiscApply->setEnabled(true);
}
void TweakMisc::checkMiscTearfreeRadeon_clicked() noexcept
{
    //toggle flag for action.  this way, if box was checked initially, the action won't take place again.
    flags.radeon = true;
    ui->pushMiscApply->setEnabled(true);
}
