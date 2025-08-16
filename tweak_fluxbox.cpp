#include <QFile>
#include "ui_defaultlook.h"
#include "cmd.h"
#include "tweak_fluxbox.h"

using namespace Qt::Literals::StringLiterals;

TweakFluxbox::TweakFluxbox(Ui::defaultlook *ui, bool verbose, QObject *parent) noexcept
    : QObject{parent}, ui{ui}, verbose{verbose}
{
    setup();
    connect(ui->pushFluxboxApply, &QPushButton::clicked, this, &TweakFluxbox::pushFluxboxApply_clicked);
    connect(ui->checkFluxboxShowToolbar, &QCheckBox::clicked, this, &TweakFluxbox::slotSettingChanged);
    connect(ui->checkFluxboxResetDock, &QCheckBox::clicked, this, &TweakFluxbox::slotSettingChanged);
    connect(ui->checkFluxboxResetEverything, &QCheckBox::clicked, this, &TweakFluxbox::checkFluxboxResetEverything_clicked);
    connect(ui->checkFluxboxResetMenu, &QCheckBox::clicked, this, &TweakFluxbox::checkFluxboxResetMenu_clicked);
    connect(ui->checkFluxboxMenuMigrate, &QCheckBox::clicked, this, &TweakFluxbox::checkFluxboxMenuMigrate_clicked);
    connect(ui->spinFluxboxScreenIdleTime, &QSpinBox::valueChanged, this, &TweakFluxbox::spinFluxboxScreenIdleTime_valueChanged);
    connect(ui->comboFluxboxToolbarLocation, &QComboBox::currentIndexChanged, this, &TweakFluxbox::slotSettingChanged);
    connect(ui->checkFluxboxToolbarAutoHide, &QCheckBox::clicked, this, &TweakFluxbox::slotSettingChanged);
    connect(ui->checkFluxboxShowToolbar, &QCheckBox::clicked, this, &TweakFluxbox::slotSettingChanged);
    connect(ui->spinFluxboxToolbarWidth, &QSpinBox::valueChanged, this, &TweakFluxbox::slotSettingChanged);
    connect(ui->spinFluxboxToolbarHeight, &QSpinBox::valueChanged, this, &TweakFluxbox::slotSettingChanged);
    connect(ui->comboFluxboxSlitLocation, &QComboBox::currentIndexChanged, this, &TweakFluxbox::comboFluxboxSlitLocation_currentIndexChanged);
    connect(ui->checkFluxboxSlitAutoHide, &QCheckBox::clicked, this, &TweakFluxbox::slotSettingChanged);
    connect(ui->pushFluxboxManageTint2, &QPushButton::clicked, this, &TweakFluxbox::pushFluxboxManageTint2_clicked);
    connect(ui->comboFluxboxIcons, &QComboBox::currentIndexChanged, this, &TweakFluxbox::comboFluxboxIcons_currentIndexChanged);
    connect(ui->comboFluxboxCaptions, &QComboBox::currentIndexChanged, this, &TweakFluxbox::comboFluxboxCaptions_currentIndexChanged);
}

void TweakFluxbox::setup() noexcept
{
    //resets
    if (!QFile::exists(u"/usr/bin/menu-migrate"_s)) {
        ui->checkFluxboxMenuMigrate->setDisabled(true);
    }
    if (!QFile::exists(u"/usr/bin/mxflux_install.sh"_s)) {
        ui->checkFluxboxResetEverything->setDisabled(true);
    }
    if (!QFile::exists(u"/etc/skel/.fluxbox/scripts/DefaultDock.mxdk"_s)) {
        ui->checkFluxboxResetDock->setDisabled(true);
    }
    if (!QFile::exists(u"/etc/skel/.fluxbox/menu-mx"_s)) {
        ui->checkFluxboxResetMenu->setDisabled(true);
    }

    if (!QFile::exists(u"/usr/bin/idesktoggle"_s) || !QFile::exists(u"/usr/bin/idesk"_s)) {
        ui->comboFluxboxIcons->setDisabled(true);
        ui->comboFluxboxCaptions->setDisabled(true);
    } else {
        QString test;
        test = runCmd(u"grep \\#Caption $HOME/.idesktop/*.lnk"_s).output;
        if (test.isEmpty()) {
            ui->comboFluxboxCaptions->setCurrentIndex(0);
            test = runCmd(u"grep CaptionOnHover $HOME/.ideskrc |grep -v ToolTip"_s).output.section(u":"_s,1,1).trimmed();
            if (verbose) qDebug() << "hover test" << test;
            if (test.contains("true"_L1)) {
                ui->comboFluxboxCaptions->setCurrentIndex(2);
            }
        }
        test = runCmd(u"grep \\#Icon $HOME/.idesktop/*.lnk"_s).output;
        if (test.isEmpty()) {
            ui->comboFluxboxIcons->setCurrentIndex(0);
        }

    }

    flags.icons = false;
    flags.captions =false;

    //screenblanking value;
    const QString &itime = runCmd(u"xset q |grep timeout | awk '{print $2}'"_s).output.trimmed();
    ui->spinFluxboxScreenIdleTime->setValue(itime.toInt()/60);
    ui->spinFluxboxScreenIdleTime->setToolTip(u"set to 0 minutes to disable screensaver"_s);

    //toolbar autohide
    QString toolbarautohide = runCmd(u"grep screen0.toolbar.autoHide $HOME/.fluxbox/init"_s).output.section(u":"_s,1,1).trimmed();
    if (verbose) qDebug() << "Toolbar autohide" << toolbarautohide;
    if (toolbarautohide == "true"_L1) {
        ui->checkFluxboxToolbarAutoHide->setChecked(true);
    } else {
        ui->checkFluxboxToolbarAutoHide->setChecked(false);
    }
    QString toolbarvisible = runCmd(u"grep session.screen0.toolbar.visible: $HOME/.fluxbox/init"_s).output.section(u":"_s,1,1).trimmed();
    if (verbose) qDebug() << "Toolbar visible" << toolbarautohide;
    if (toolbarvisible == "true"_L1) {
        ui->checkFluxboxShowToolbar->setChecked(true);
    } else {
        ui->checkFluxboxShowToolbar->setChecked(false);
    }
    //toolbar location
    QString toolbarlocation = runCmd(u"grep screen0.toolbar.placement $HOME/.fluxbox/init"_s).output.section(u":"_s,1,1).trimmed();
    if (verbose) qDebug() << "Toolbar location" << toolbarlocation;
    ui->comboFluxboxToolbarLocation->setCurrentText(toolbarlocation);
    //slit location
    QString slitlocation = runCmd(u"grep screen0.slit.placement $HOME/.fluxbox/init"_s).output.section(u":"_s,1,1).trimmed();
    if (verbose) qDebug() << "Slit location" << slitlocation;
    ui->comboFluxboxSlitLocation->setCurrentText(slitlocation);
    //toolbar width;
    QString toolbarwidth = runCmd(u"grep screen0.toolbar.widthPercent $HOME/.fluxbox/init"_s).output.section(u":"_s,1,1).trimmed();
    if (verbose) qDebug() << "Toolbar width" << toolbarwidth;
    ui->spinFluxboxToolbarWidth->setValue(toolbarwidth.toInt());
    //toolbar height
    QString toolbarheight = runCmd(u"grep screen0.toolbar.height $HOME/.fluxbox/init"_s).output.section(u":"_s,1,1).trimmed();
    if (verbose) qDebug() << "Toolbar height" << toolbarheight;
    ui->spinFluxboxToolbarHeight->setValue(toolbarheight.toInt());
    //slit autohide
    QString slitautohide = runCmd(u"grep screen0.slit.autoHide $HOME/.fluxbox/init"_s).output.section(u":"_s,1,1).trimmed();
    if (verbose) qDebug() << "slit autohide" << slitautohide;
    ui->checkFluxboxSlitAutoHide->setChecked(slitautohide == "true"_L1);
    ui->pushFluxboxApply->setDisabled(true);
}

bool TweakFluxbox::checkFluxbox() const noexcept
{
    QString test = runCmd(u"pgrep fluxbox"_s).output;
    if (verbose) qDebug() << "current fluxbox test is" << test;
    return (!test.isEmpty());
}

void TweakFluxbox::changeInitVariable(const QString &initline, const QString &value) const noexcept
{
    if (verbose) qDebug() << "checking for init value changes";
    QString initialvalue = initline.section(':',1,1).trimmed();
    if ( initialvalue != value) {
        QString cmd = "sed -i 's/^"_L1 + initline +'/' + initline.section(':',0,0).trimmed() + ":    "_L1 + value + "/' $HOME/.fluxbox/init"_L1;
        if (verbose) qDebug() << "init change command " << cmd;
        runCmd(cmd);
    }
}

void TweakFluxbox::changeDock() const noexcept
{
    if (verbose) qDebug() << "comment slit changes in mxdk files";

    if (flags.slit) {
        runCmd(u"sed -i 's/^fluxbox-remote/#&/' $HOME/.fluxbox/scripts/*.mxdk"_s);
        runCmd(u"sed -i 's/^sed/#&/' $HOME/.fluxbox/scripts/*.mxdk"_s);
    }
}

void TweakFluxbox::slotSettingChanged() noexcept
{
    ui->pushFluxboxApply->setEnabled(true);
}
void TweakFluxbox::pushFluxboxApply_clicked() noexcept
{
    ui->tabFluxbox->setEnabled(false);

    // toggle icons
    // flux captions - 0=on 1=off 2=On Hover
    enum FluxCaptions {On, Off, OnHover};

    if (flags.captions) {
        switch(ui->comboFluxboxCaptions->currentIndex()) {
        case FluxCaptions::On:
            runCmd(u"/usr/bin/idesktoggle caption on"_s);
            runCmd(u"/usr/bin/idesktoggle CaptionOnHover off"_s);
            break;
        case FluxCaptions::Off:
            runCmd(u"/usr/bin/idesktoggle caption off"_s);
            break;
        case FluxCaptions::OnHover:
            runCmd(u"/usr/bin/idesktoggle caption on"_s);
            runCmd(u"/usr/bin/idesktoggle CaptionOnHover On"_s);
            break;
        }
    }

    // flux icons 0=on 1=off
    if (flags.icons) {
        switch(ui->comboFluxboxIcons->currentIndex()) {
        case 0:
            runCmd(u"/usr/bin/idesktoggle Icon on"_s);
            break;
        case 1:
            runCmd(u"/usr/bin/idesktoggle Icon off"_s);
            break;
        }
    }

    //setup slit autohide
    //get slit line
    QString initline;
    QString value;
    if (ui->checkFluxboxSlitAutoHide->isChecked()) {
        value = u"true"_s;
    } else {
        value = u"false"_s;
    }
    initline = runCmd(u"grep screen0.slit.autoHide  $HOME/.fluxbox/init"_s).output;
    changeInitVariable(initline,value);

    //set slit placement
    //setup toolbar location
    value = ui->comboFluxboxSlitLocation->currentText();
    initline = runCmd(u"grep screen0.slit.placement  $HOME/.fluxbox/init"_s).output;
    changeInitVariable(initline,value);
    changeDock();

    //setup toolbar autohide
    if (ui->checkFluxboxToolbarAutoHide->isChecked()) {
        value = u"true"_s;
    } else {
        value = u"false"_s;
    }
    initline = runCmd(u"grep screen0.toolbar.autoHide  $HOME/.fluxbox/init"_s).output;
    changeInitVariable(initline,value);

    //setup toolbar visible
    if (ui->checkFluxboxShowToolbar->isChecked()) {
        value = u"true"_s;
    } else {
        value = u"false"_s;
    }
    initline = runCmd(u"grep session.screen0.toolbar.visible  $HOME/.fluxbox/init"_s).output;
    changeInitVariable(initline,value);

    //setup toolbar location
    value = ui->comboFluxboxToolbarLocation->currentText();
    initline = runCmd(u"grep screen0.toolbar.placement  $HOME/.fluxbox/init"_s).output;
    changeInitVariable(initline,value);

    //setup toolbar widthpercent
    value = QString::number(ui->spinFluxboxToolbarWidth->value(), 'G', 5);
    initline = runCmd(u"grep screen0.toolbar.widthPercent  $HOME/.fluxbox/init"_s).output;
    changeInitVariable(initline,value);

    //setup toolbar height
    value = QString::number(ui->spinFluxboxToolbarHeight->value(), 'G', 5);
    initline = runCmd(u"grep screen0.toolbar.height  $HOME/.fluxbox/init"_s).output;
    changeInitVariable(initline,value);

    //Reset Everything
    if (ui->checkFluxboxResetEverything->isChecked()) {
        ui->checkFluxboxResetDock->setChecked(false);
        ui->checkFluxboxResetMenu->setChecked(false);
        ui->checkFluxboxMenuMigrate->setChecked(false);
        runCmd(u"/usr/bin/mxflux_install.sh"_s);
        runCmd(u"pkill wmalauncher"_s);
        runCmd(u"$HOME/.fluxbox/scripts/DefaultDock.mxdk"_s);
    }

    //Reset Menu
    if (ui->checkFluxboxResetMenu->isChecked() && !ui->checkFluxboxResetEverything->isChecked()) {
        //determine menu in use
        QString menumx = runCmd(u"grep session.menuFile $HOME/.fluxbox/init"_s).output.section(u":"_s,1,1).trimmed();
        if (verbose) qDebug() << "menu mx is " << menumx;
        //backup menu
        runCmd("cp "_L1 + menumx + ' ' + menumx + ".$(date +%Y%m%d%H%M%S)"_L1);
        //copy menu-mx from /etc/skel/.fluxbox
        runCmd(u"cp /etc/skel/.fluxbox/menu-mx $HOME/.fluxbox"_s);
        //run localize-fluxbox-menu to generate new menu
        runCmd(u"localize_fluxbox_menu-mx"_s);
    }

    //Migrate Menu
    if (ui->checkFluxboxMenuMigrate->isChecked() && !ui->checkFluxboxResetEverything->isChecked()) {
        //run menu-migrate script
        runCmd(u"/usr/bin/menu-migrate"_s);
    }

    //Reset Dock
    if (ui->checkFluxboxResetDock->isChecked() && !ui->checkFluxboxResetEverything->isChecked()) {
        //copy backup dock and copy one from usr/share/mxflux/.fluxbox/scripts
        runCmd(u"cp $HOME/.fluxbox/scripts/DefaultDock.mxdk $HOME/.fluxbox/scripts/DefaultDock.mxdk.$(date +%Y%m%d%H%M%S)"_s);
        runCmd(u"cp /etc/skel/.fluxbox/scripts/DefaultDock.mxdk $HOME/.fluxbox/scripts/DefaultDock.mxdk"_s);
        runCmd(u"pkill wmalauncher"_s);
        runCmd(u"$HOME/.fluxbox/scripts/DefaultDock.mxdk"_s);
    }

    //screenblanking
    if (flags.screenIdle){
        //comment default line if it exists
        runCmd(u"sed -i 's/^[[:blank:]]*xset[[:blank:]].*dpms.*/#&/' $HOME/.fluxbox/startup"_s);
        //add new values to fluxbox startup menu if don't exist
        QString test = runCmd(u"grep '$HOME/.config/MX-Linux/screenblanking-mxtweak' $HOME/.fluxbox/startup"_s).output;
        if (test.isEmpty()){
            //add comment and new config file
            runCmd(u"sed -i '/^exec.*/i#screenblanking added by mx-tweak' $HOME/.fluxbox/startup"_s);
            runCmd(u"sed -i '/^exec.*/i$HOME\\/.config\\/MX-Linux\\/screenblanking-mxtweak &' $HOME/.fluxbox/startup"_s);
            //blank space before exec
            runCmd(u"sed -i '/^exec.*/i\\\\' $HOME/.fluxbox/startup"_s);
        }
        //set new value
        int value = ui->spinFluxboxScreenIdleTime->value() * 60;
        runCmd(u"echo \\#\\!/bin/bash >$HOME/.config/MX-Linux/screenblanking-mxtweak"_s);
        QString cmd = "xset dpms "_L1 + QString::number(value) + ' ' + QString::number(value) + ' ' + QString::number(value);
        runCmd(cmd);
        runCmd("echo "_L1 + cmd + " >>$HOME/.config/MX-Linux/screenblanking-mxtweak"_L1);
        cmd = "xset s "_L1 + QString::number(value);
        runCmd(cmd);
        runCmd("echo "_L1 + cmd + " >>$HOME/.config/MX-Linux/screenblanking-mxtweak"_L1);
        //make sure script is executable
        runCmd(u"chmod a+x $HOME/.config/MX-Linux/screenblanking-mxtweak"_s);
    }

    //when all done, restart fluxbox
    ui->checkFluxboxResetDock->setChecked(false);
    ui->checkFluxboxResetMenu->setChecked(false);
    ui->checkFluxboxResetEverything->setChecked(false);
    ui->checkFluxboxMenuMigrate->setChecked(false);
    runCmd(u"sleep 2; /usr/bin/fluxbox-remote restart"_s);
    setup();

    ui->tabFluxbox->setEnabled(true);
}

void TweakFluxbox::checkFluxboxMenuMigrate_clicked() noexcept
{
    ui->pushFluxboxApply->setEnabled(true);
    ui->checkFluxboxResetMenu->setChecked(false);
}
void TweakFluxbox::checkFluxboxResetMenu_clicked() noexcept
{
    ui->pushFluxboxApply->setEnabled(true);
    ui->checkFluxboxMenuMigrate->setChecked(false);
}
void TweakFluxbox::checkFluxboxResetEverything_clicked() noexcept
{
    ui->pushFluxboxApply->setEnabled(true);
    ui->checkFluxboxResetDock->setChecked(false);
    ui->checkFluxboxResetMenu->setChecked(false);
    ui->checkFluxboxMenuMigrate->setChecked(false);
}
void TweakFluxbox::spinFluxboxScreenIdleTime_valueChanged(int) noexcept
{
    flags.screenIdle = true;
    ui->pushFluxboxApply->setEnabled(true);
}
void TweakFluxbox::comboFluxboxSlitLocation_currentIndexChanged(int  /*index*/) noexcept
{
    ui->pushFluxboxApply->setEnabled(true);
    flags.slit = true;
}
void TweakFluxbox::pushFluxboxManageTint2_clicked() noexcept
{
    ui->tabWidget->setEnabled(false);
    runCmd(u"/usr/bin/mxfb-tint2-manager"_s);
    ui->tabWidget->setEnabled(true);
}
void TweakFluxbox::comboFluxboxIcons_currentIndexChanged(int  /*index*/) noexcept
{
    ui->pushFluxboxApply->setEnabled(true);
    flags.icons = true;
}
void TweakFluxbox::comboFluxboxCaptions_currentIndexChanged(int  /*index*/) noexcept
{
    ui->pushFluxboxApply->setEnabled(true);
    flags.captions = true;
}
