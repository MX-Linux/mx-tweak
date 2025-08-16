#include <QMessageBox>
#include <QDir>
#include <QFile>
#include "ui_tweak.h"
#include "cmd.h"
#include "window_buttons.h"
#include "tweak_xfce.h"

using namespace Qt::Literals::StringLiterals;

TweakXfce::TweakXfce(Ui::Tweak *ui, bool verbose, QObject *parent) noexcept
    : QObject{parent}, ui{ui}, verbose{verbose}
{
    setup();
    connect(ui->pushXfceApply, &QPushButton::clicked, this, &TweakXfce::pushXfceApply_clicked);
    connect(ui->checkXfceSingleClick, &QCheckBox::clicked, this, &TweakXfce::slotSettingChanged);
    connect(ui->checkXfceDesktopZoom, &QCheckBox::clicked, this, &TweakXfce::slotSettingChanged);
    connect(ui->checkXfceShowAllWorkspaces, &QCheckBox::clicked, this, &TweakXfce::slotSettingChanged);
    connect(ui->checkXfceNoEllipseFileNames, &QCheckBox::clicked, this, &TweakXfce::slotSettingChanged);
    connect(ui->checkXfceNotificationPercentages, &QCheckBox::clicked, this, &TweakXfce::slotSettingChanged);
    connect(ui->checkXfceFileDialogActionButtonsAtBottom, &QCheckBox::clicked, this, &TweakXfce::slotSettingChanged);
    connect(ui->checkXfceHibernate, &QCheckBox::clicked, this, &TweakXfce::slotSettingChanged);
    connect(ui->pushXfceAppearance, &QPushButton::clicked, this, &TweakXfce::pushXfceAppearance_clicked);
    connect(ui->pushXfceWindowManager, &QPushButton::clicked, this, &TweakXfce::pushXfceWindowManager_clicked);
}
void TweakXfce::setup() noexcept
{
    ui->pushXfceApply->setEnabled(false);
    float versioncheck = 4.18;

    QString XfceVersion = runCmd(u"dpkg-query --show xfce4-session | awk '{print $2}'"_s).output.section('.',0,1);
    if (verbose) qDebug() << "XfceVersion = " << XfceVersion.toFloat();
    if ( XfceVersion.toFloat() < versioncheck ){
        ui->labelXfceCSD->hide();
    }

    //check single click status
    QString test;
    test = runCmd(u"xfconf-query  -c xfce4-desktop -p /desktop-icons/single-click"_s).output;
    ui->checkXfceSingleClick->setChecked(test == "true"_L1);

    //check systray frame status
    //frame has been removed from systray

    // show all workspaces - tasklist/show buttons feature
    pluginTaskList = runCmd(uR"(grep \"tasklist\" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml |cut -d '=' -f2 | cut -d '' -f1| cut -d '"' -f2)"_s).output;
    if (verbose) qDebug() << "tasklist is " << pluginTaskList;
    if ( ! pluginTaskList.isEmpty()) {
        test = runCmd("xfconf-query -c xfce4-panel -p /plugins/"_L1 + pluginTaskList + "/include-all-workspaces"_L1).output;
        ui->checkXfceShowAllWorkspaces->setChecked(test == "true"_L1);
    } else {
        ui->checkXfceShowAllWorkspaces->setEnabled(false);
    }

    // set percentages in notifications
    test = runCmd(u"xfconf-query -c xfce4-notifyd -p /show-text-with-gauge"_s).output;
    ui->checkXfceNotificationPercentages->setChecked(test == "true"_L1);

    //switch zoom_desktop

    test = runCmd(u"xfconf-query -c xfwm4 -p /general/zoom_desktop"_s).output;
    ui->checkXfceDesktopZoom->setChecked(test == "true"_L1);

    //setup no-ellipse option
    const bool noEllipseFileNames = QFile::exists(QDir::homePath()
        + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css"_L1);
    ui->checkXfceNoEllipseFileNames->setChecked(noEllipseFileNames);

    //setup classic file dialog buttons
    test = runCmd(u"xfconf-query -c xsettings -p /Gtk/DialogsUseHeader"_s).output;
    ui->checkXfceFileDialogActionButtonsAtBottom->setChecked(test == "false"_L1);

    //setup hibernate switch
    //first, hide if running live
    test = runCmd(u"df -T / |tail -n1 |awk '{print $2}'"_s).output;
    if (verbose) qDebug() << test;
    if ( test == "aufs"_L1 || test == "overlay"_L1 ) {
        ui->checkXfceHibernate->hide();
        ui->labelXfceHibernate->hide();
    }

    //hide hibernate if there is no swap
    QString swaptest = runCmd(u"/usr/sbin/swapon --show"_s).output;
    if (verbose) qDebug() << "swaptest swap present is " << swaptest;
    if (swaptest.isEmpty()) {
        ui->checkXfceHibernate->hide();
        ui->labelXfceHibernate->hide();
    }

    //and hide hibernate if swap is encrypted
    int swaptest2 = runCmd(u"grep swap /etc/crypttab |grep -q luks"_s).exitCode;
    if (verbose) qDebug() << "swaptest encrypted is " << swaptest2;
    if (swaptest2 == 0) {
        ui->checkXfceHibernate->hide();
        ui->labelXfceHibernate->hide();
    }

    //set checkbox
    test = runCmd(u"xfconf-query -c xfce4-session -p /shutdown/ShowHibernate"_s).output;
    if ( test == "true"_L1) {
        ui->checkXfceHibernate->setChecked(true);
        flags.hibernate = true;
    } else {
        ui->checkXfceHibernate->setChecked(false);
        flags.hibernate = false;
    }
}
bool TweakXfce::checkXfce() const noexcept
{
    QString test = runCmd(u"pgrep xfce4-session"_s).output;
    if (verbose) qDebug() << "current xfce desktop test is " << test;
    return (!test.isEmpty());
}

void TweakXfce::slotSettingChanged() noexcept
{
    ui->pushXfceApply->setEnabled(true);
}
void TweakXfce::pushXfceApply_clicked() noexcept
{
    QString hibernate_option;
    hibernate_option.clear();

    if (ui->checkXfceShowAllWorkspaces->isChecked()) {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/"_L1 + pluginTaskList + "/include-all-workspaces -s true"_L1);
    } else {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/"_L1 + pluginTaskList + "/include-all-workspaces -s false"_L1);
    }

    if (ui->checkXfceSingleClick->isChecked()) {
        runCmd(u"xfconf-query  -c xfce4-desktop -p /desktop-icons/single-click -s true"_s);
    } else {
        runCmd(u"xfconf-query  -c xfce4-desktop -p /desktop-icons/single-click -s false"_s);
    }
    //systray frame removed

    //notification percentages
    if (ui->checkXfceNotificationPercentages->isChecked()){
        runCmd(u"xfconf-query -c xfce4-notifyd -p /show-text-with-gauge -t bool -s true --create"_s);
    } else {
        runCmd(u"xfconf-query -c xfce4-notifyd -p /show-text-with-gauge --reset"_s);
    }

    //set desktop zoom
    if (ui->checkXfceDesktopZoom->isChecked()) {
        runCmd(u"xfconf-query -c xfwm4 -p /general/zoom_desktop -s true"_s);
    } else {
        runCmd(u"xfconf-query -c xfwm4 -p /general/zoom_desktop -s false"_s);
    }

    //deal with gtk dialog button settings
    if (ui->checkXfceFileDialogActionButtonsAtBottom->isChecked()){
        runCmd(u"xfconf-query -c xsettings -p /Gtk/DialogsUseHeader -s false"_s);
    } else {
        runCmd(u"xfconf-query -c xsettings -p /Gtk/DialogsUseHeader -s true"_s);
    }

    //deal with no-ellipse-filenames option
    QString home_path = QDir::homePath();
    if (ui->checkXfceNoEllipseFileNames->isChecked()) {
        //set desktop themeing
        QFileInfo gtk_check(home_path + "/.config/gtk-3.0/gtk.css"_L1);
        if (gtk_check.exists()) {
            if (verbose) qDebug() << "existing gtk.css found";
            QString cmd = "cat "_L1 + home_path + "/.config/gtk-3.0/gtk.css |grep -q no-ellipse-desktop-filenames.css"_L1;
            if (runCmd(cmd).exitCode == 0 ) {
                if (verbose) qDebug() << "include statement found";
            } else {
                if (verbose) qDebug() << "adding include statement";
                runCmd("echo '@import url(\"no-ellipse-desktop-filenames.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1);
            }
        } else {
            if (verbose) qDebug() << "creating simple gtk.css file";
            runCmd("echo '@import url(\"no-ellipse-desktop-filenames.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1);
        }
        //add modification config
        runCmd("cp /usr/share/mx-tweak/no-ellipse-desktop-filenames.css "_L1 + home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css "_L1);

        //restart xfdesktop by with xfdesktop --quite && xfdesktop &
        runCmd(u"xfdesktop --quit && sleep .5 && xfdesktop &"_s);
    } else {
        QFileInfo noellipse_check(home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css"_L1);
        if (noellipse_check.exists()) {
            runCmd("rm -f "_L1 + home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css"_L1);
            runCmd("sed -i '/no-ellipse-desktop-filenames.css/d' "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1);
            runCmd(u"xfdesktop --quit && sleep .5 && xfdesktop &"_s);
        }
    }

    //deal with hibernate
    if (ui->checkXfceHibernate->isChecked() != flags.hibernate) {
        QString param = u"false"_s;
        if (ui->checkXfceHibernate->isChecked()) {
            hibernate_option =  "hibernate"_L1;
            param = u"true"_s;
        }
        runProc(u"xfconf-query"_s, {u"-c"_s, u"xfce4-session"_s, u"-p"_s,
            u"/shutdown/ShowHibernate"_s, u"-t"_s, u"bool"_s, u"-s"_s, param, u"--create"_s});
    }

    setup();
}

void TweakXfce::pushXfceAppearance_clicked() noexcept
{
    ui->tabWidget->setEnabled(false);
    runProc(u"xfce4-appearance-settings"_s);
    ui->tabWidget->setEnabled(true);
}
void TweakXfce::pushXfceWindowManager_clicked() noexcept
{
    ui->tabWidget->setEnabled(false);
    runProc(u"xfwm4-settings"_s);
    ui->tabWidget->setEnabled(true);
}
