#include <QMessageBox>
#include <QDir>
#include <QFile>
#include "ui_defaultlook.h"
#include "cmd.h"
#include "window_buttons.h"
#include "tweak_xfce.h"

using namespace Qt::Literals::StringLiterals;

TweakXfce::TweakXfce(Ui::defaultlook *ui, bool verbose, QObject *parent)
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

    /* Panel */

    if (ui->pushXfcePanelApply->icon().isNull()) {
        ui->pushXfcePanelApply->setIcon(QIcon(":/icons/dialog-ok.svg"));
    }
    // Panel orientation
    ui->comboXfcePanelPlacement->addItem(tr("Horizontal (Bottom)"), u"horz-bottom"_s);
    ui->comboXfcePanelPlacement->addItem(tr("Horizontal (Top)"), u"horz-top"_s);
    ui->comboXfcePanelPlacement->insertSeparator(2);
    ui->comboXfcePanelPlacement->addItem(tr("Vertical (Left)"), u"vert-left"_s);
    ui->comboXfcePanelPlacement->addItem(tr("Vertical (Right)"), u"vert-right"_s);
    // Tasklist plugins
    ui->comboXfcePanelTasklistPlugin->addItem(tr("docklike"), u"docklike"_s);
    ui->comboXfcePanelTasklistPlugin->addItem(tr("Window Buttons"), u"tasklist"_s);

    panelWhich();
    panelSetup();

    connect(ui->pushXfcePanelApply, &QPushButton::clicked, this, &TweakXfce::pushXfcePanelApply_clicked);
    connect(ui->comboXfcePanelPlacement, &QComboBox::currentIndexChanged, this, &TweakXfce::comboXfcePanelPlacement_currentIndexChanged);
    connect(ui->spinXfcePanelPluginVolume, &QDoubleSpinBox::valueChanged, this, &TweakXfce::slotPluginScaleChanged);
    connect(ui->spinXfcePanelPluginPower, &QDoubleSpinBox::valueChanged, this, &TweakXfce::slotPluginScaleChanged);
    connect(ui->comboXfcePanelTasklistPlugin, &QComboBox::currentIndexChanged, this, &TweakXfce::comboXfcePanelTasklistPlugin_currentIndexChanged);
    connect(ui->pushXfcePanelTasklistOptions, &QPushButton::clicked, this, &TweakXfce::pushXfcePanelTasklistOptions_clicked);
    connect(ui->pushXfcePanelBackup, &QPushButton::clicked, this, &TweakXfce::pushXfcePanelBackup_clicked);
    connect(ui->pushXfcePanelRestore, &QPushButton::clicked, this, &TweakXfce::pushXfcePanelRestore_clicked);
    connect(ui->pushXfcePanelDefault, &QPushButton::clicked, this, &TweakXfce::pushXfcePanelDefault_clicked);
}
void TweakXfce::setup()
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
    QString cmd = u"grep swap /etc/crypttab |grep -q luks"_s;
    int swaptest2 = system(cmd.toUtf8());
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
bool TweakXfce::checkXfce() const
{
    QString test = runCmd(u"pgrep xfce4-session"_s).output;
    if (verbose) qDebug() << "current xfce desktop test is " << test;
    return (!test.isEmpty());
}

void TweakXfce::slotSettingChanged()
{
    ui->pushXfceApply->setEnabled(true);
}
void TweakXfce::pushXfceApply_clicked()
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
            if (system(cmd.toUtf8()) == 0 ) {
                if (verbose) qDebug() << "include statement found";
            } else {
                if (verbose) qDebug() << "adding include statement";
                QString cmd = "echo '@import url(\"no-ellipse-desktop-filenames.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1;
                system(cmd.toUtf8());
            }
        } else {
            if (verbose) qDebug() << "creating simple gtk.css file";
            QString cmd = "echo '@import url(\"no-ellipse-desktop-filenames.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1;
            system(cmd.toUtf8());
        }
        //add modification config
        runCmd("cp /usr/share/mx-tweak/no-ellipse-desktop-filenames.css "_L1 + home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css "_L1);

        //restart xfdesktop by with xfdesktop --quite && xfdesktop &

        system("xfdesktop --quit && sleep .5 && xfdesktop &");
    } else {
        QFileInfo noellipse_check(home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css"_L1);
        if (noellipse_check.exists()) {
            runCmd("rm -f "_L1 + home_path + "/.config/gtk-3.0/no-ellipse-desktop-filenames.css"_L1);
            runCmd("sed -i '/no-ellipse-desktop-filenames.css/d' "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1);
            system("xfdesktop --quit && sleep .5 && xfdesktop &");
        }
    }

    //deal with hibernate
    if (ui->checkXfceHibernate->isChecked() != flags.hibernate) {
        if (ui->checkXfceHibernate->isChecked()) {
            hibernate_option =  "hibernate"_L1;
            system("xfconf-query -c xfce4-session -p /shutdown/ShowHibernate -t bool -s true --create");
        } else {
            system("xfconf-query -c xfce4-session -p /shutdown/ShowHibernate -t bool -s false --create");
        }
    }

    setup();
}

/******************\
 * PANEL SETTINGS *
\******************/

void TweakXfce::panelWhich()
{
    // take the first panel we see as default
    const QString &panel_content = runCmd(u"LC_ALL=en_US.UTF-8 xfconf-query"_s
        u" -c xfce4-panel -p /panels | grep -v Value | grep -v ^$"_s).output;
    panelIDs = panel_content.split(u"\n"_s);
    panel = panelIDs.value(0);
    if (verbose) {
        qDebug() << "panels found: " << panelIDs;
        qDebug() << "panel to use: " << panel;
    }
}
void TweakXfce::panelSetup()
{
    QString home_path = QDir::homePath();
    // Migrate existing older backup directory to multi-backup structure.
    QFileInfo backuppanel(home_path + "/.restore/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_L1);
    if (backuppanel.exists()) {
        runCmd("tar --create --xz --file=\"$HOME/.restore/panel_backup_"_L1
            + backuppanel.lastModified().toString(u"dd.MM.yyyy.hh.mm.ss"_s) + ".tar.xz\""_L1
            " --directory=$HOME/.restore/.config/xfce4 panel xfconf/xfce-perchannel-xml/xfce4-panel.xml"_L1);
        runCmd(u"rm -R $HOME/.restore/.config/xfce4/panel $HOME/.restore/.config/xfce4/xfconf"_s);
    }

    //setup pulseaudio plugin scale functoin
    //get files setup if they don't exist
    if (QFileInfo::exists(home_path + "/.config/gtk-3.0/gtk.css"_L1)) {
        if (verbose) qDebug() << "existing gtk.css found";
        QString cmd = "cat "_L1 + home_path + "/.config/gtk-3.0/gtk.css |grep -q xfce4-panel-tweaks.css"_L1;
        if (system(cmd.toUtf8()) == 0 ) {
            if (verbose) qDebug() << "include statement found";
        } else {
            if (verbose) qDebug() << "adding include statement";
            QString cmd = "echo '@import url(\"xfce4-panel-tweaks.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1;
            system(cmd.toUtf8());
        }
    } else {
        if (verbose) qDebug() << "creating simple gtk.css file";
        QString cmd = "echo '@import url(\"xfce4-panel-tweaks.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1;
        system(cmd.toUtf8());
    }

    if (!QFileInfo::exists(home_path + "/.config/gtk-3.0/xfce4-panel-tweaks.css"_L1)) {
        QString cmd = "cp /usr/share/mx-tweak/xfce4-panel-tweaks.css "_L1 + home_path + "/.config/gtk-3.0/"_L1;
        system(cmd.toUtf8());
    }
    //check for existence of plugins before running these commands, hide buttons and labels if not present.
    //Get value of scale
    QString plugins = runCmd("grep plugin "_L1 + home_path + "/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_L1).output;
    bool volumeplugin;
    bool powerplugin;
    if (plugins.contains("pulseaudio"_L1)){
        ui->spinXfcePanelPluginVolume->setValue(runCmd("grep -A 1 pulseaudio "_L1 + home_path + "/.config/gtk-3.0/xfce4-panel-tweaks.css |grep scale |cut -d'(' -f2 |cut -d')' -f1"_L1).output.toDouble());
        volumeplugin = true;
    } else {
        ui->spinXfcePanelPluginVolume->hide();
        ui->labelXfcePanelPluginVolume->hide();
        volumeplugin = false;
    }
    if (plugins.contains("power-manager-plugin"_L1)){
        ui->spinXfcePanelPluginPower->setValue(runCmd("grep -A 1 xfce4-power-manager-plugin "_L1 + home_path + "/.config/gtk-3.0/xfce4-panel-tweaks.css |grep scale |cut -d'(' -f2 |cut -d')' -f1"_L1).output.toDouble());
        powerplugin = true;
    } else {
        ui->spinXfcePanelPluginPower->hide();
        ui->labelXfcePanelPluginPower->hide();
        powerplugin = false;
    }

    if (! volumeplugin && ! powerplugin){
        ui->groupXfcePanelPluginScales->hide();
    }

    // if backup available, make the restore backup option available
    if (populatePanelBackups() <= 0) {
        pushXfcePanelBackup_clicked();
    }

    flags.panel = false;
    ui->pushXfcePanelApply->setEnabled(false);

    //hide tasklist setting if not present
    bool tasklist = true;
    bool docklike = true;

    if ( system("xfconf-query -c xfce4-panel -p /plugins -lv |grep tasklist") != 0 ) {
        ui->groupXfcePanelTasklist->hide();
        tasklist = false;
    }

    //hide docklike settings if not present
    if ( system("xfconf-query -c xfce4-panel -p /plugins -lv |grep docklike") != 0 ) {
        docklike = false;
    }

    //check status of docklike external package, hide chooser if not installed
    QString check = runCmd(u"LANG=c dpkg-query -s xfce4-docklike-plugin |grep Status"_s).output;
    if (!check.contains("installed"_L1)){
        docklike = true;
    }

    //display tasklist plugin selector if only one tasklist in use
    if ( tasklist && docklike ){
        ui->groupXfcePanelTasklist->hide();
    } else if (tasklist) {
        ui->comboXfcePanelTasklistPlugin->setCurrentIndex(1); // index 1 is window buttons
    } else if (docklike) {
        ui->comboXfcePanelTasklistPlugin->setCurrentIndex(0); // index 0 is doclike
    }

    flags.tasklist = false;

    //only enable options that make sense

    //if panel is already horizontal, set vertical option available, and vice versa  "" and "0" are horizontal

    QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/mode"_L1).output;
    QString test2 = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/position"_L1).output.section(u";"_s, 0,0);
    if (verbose) qDebug() << "test2" << test2;

    int posItem = -1;
    if (test.isEmpty() || test == "0"_L1) {
        if (test2 == "p=11"_L1 || test2 == "p=6"_L1 || test2 == "p=2"_L1) {
            posItem = ui->comboXfcePanelPlacement->findData("horz-top");
        } else {
            posItem = ui->comboXfcePanelPlacement->findData("horz-bottom");
        }
    } else if (test == "1"_L1 || test == "2"_L1) {
        if (test2 == "p=1"_L1) {
            posItem = ui->comboXfcePanelPlacement->findData("vert-right");
        } else {
            posItem = ui->comboXfcePanelPlacement->findData("vert-left");
        }
    }
    ui->comboXfcePanelPlacement->setCurrentIndex(posItem);
}

void TweakXfce::slotPluginScaleChanged(double)
{
    ui->pushXfcePanelApply->setEnabled(true);
    flags.scales = true;
}
void TweakXfce::pushXfcePanelApply_clicked()
{
    ui->pushXfcePanelApply->setEnabled(false);

    // tasklist switch
    if (flags.tasklist) {
        tasklistChange();
    }

    //flip panels
    if (flags.panel) {
        flags.panel = false;
        const QString &newPlace = ui->comboXfcePanelPlacement->currentData().toString();
        if (newPlace.startsWith("horz-"_L1)) {
            QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/mode"_L1).output;
            if (test == "1"_L1 || test == "2"_L1) {
                panelFlipToHorizontal();
            }
        } else if (newPlace.startsWith("vert-"_L1)) {
            QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/mode"_L1).output;
            if (test == ""_L1 || test == "0"_L1) {
                panelFlipToVertical();
            }
        }
        panelSetPosition(); // Left/right (vertical) or bottom/top (horizontal)
        //runCmd(u"xfce4-panel --restart"_s);
        //runCmd(u"sleep .5"_s);
    }

    if (flags.scales) {
        runCmd("sed -i '/xfce4-power-manager-plugin/,/\\}/ s/scale(.*)/scale("_L1
            + QString::number(ui->spinXfcePanelPluginPower->value())
            + ")/' ~/.config/gtk-3.0/xfce4-panel-tweaks.css"_L1);
        runCmd("sed -i '/pulseaudio/,/\\}/ s/scale(.*)/scale("_L1
            + QString::number(ui->spinXfcePanelPluginVolume->value())
            + ")/' ~/.config/gtk-3.0/xfce4-panel-tweaks.css"_L1);
        runCmd(u"xfce4-panel --restart"_s);
        flags.scales = false;
    }

    panelSetup();
}

void TweakXfce::comboXfcePanelPlacement_currentIndexChanged(int)
{
    ui->pushXfcePanelApply->setEnabled(true);
    flags.panel = true;
}

void TweakXfce::comboXfcePanelTasklistPlugin_currentIndexChanged(int)
{
    ui->pushXfcePanelApply->setEnabled(true);
    flags.tasklist = true;
}
void TweakXfce::pushXfcePanelTasklistOptions_clicked()
{
    ui->tabWidget->setEnabled(false);
    if (ui->comboXfcePanelTasklistPlugin->currentData() == "tasklist"_L1) {
        window_buttons wb;
        wb.setModal(true);
        wb.exec();
    } else {
        system("xfce4-panel --plugin-event=docklike:settings");
    }
    ui->tabWidget->setEnabled(true);
}

// backs up the current panel configuration
void TweakXfce::pushXfcePanelBackup_clicked()
{
    //ensure .restore folder exists
    QString home_path = QDir::homePath();
    if ( ! QDir(home_path + "/.restore/"_L1).exists()){
        runCmd("mkdir -p "_L1 + home_path + "/.restore/"_L1);
    }
    // Auto-generate file name if empty.
    if (ui->textXfcePanelBackupName->text().isEmpty()) {
        const QString &stamp = QDateTime::currentDateTime().toString(u"dd.MM.yyyy.hh.mm.ss"_s);
        ui->textXfcePanelBackupName->setText("panel_backup_"_L1 + stamp);
    }
    //validate file name
    qDebug() << "ui file name " << ui->textXfcePanelBackupName->text();
    //QRegExp rx("(@|\\$|%|\\&|\\*|(|)|{|}|[|]|/|\\|\\?");
    QRegularExpression rx(u"\\$|@|%|\\&|\\*|\\(|\\)|\\[|\\]|\\{|\\}|\\||\\?"_s);
    QRegularExpressionMatch match = rx.match(ui->textXfcePanelBackupName->text());
    int rxtest = match.hasMatch() ? match.capturedStart(0) : -1;
    qDebug() << "rxtest" << rxtest;
    if ( rxtest > 0 ){
        QMessageBox::information(nullptr, tr("MX Tweak"),
            tr("Plese remove special characters") + "@,$,%,&,*,(,),[,],{,},|,\\,?"_L1 + tr("from file name"));
    } else {
        QString path = home_path + "/.restore/"_L1 + ui->textXfcePanelBackupName->text() + ".tar.xz"_L1;
        qDebug() << path;
        //check filename existence
        if (QFileInfo::exists(path)) {
            QMessageBox::information(nullptr, tr("MX Tweak"), tr("File name already exists.  Choose another name"));
        } else {
            runCmd("tar --create --xz --file=\""_L1 + path + "\" --directory=$HOME/.config/xfce4 panel xfconf/xfce-perchannel-xml/xfce4-panel.xml"_L1);
        }
    }
    populatePanelBackups();
}
void TweakXfce::pushXfcePanelRestore_clicked()
{
    //validate file first
    switch(validateArchive("$HOME/.restore/\""_L1 + ui->comboXfcePanelBackups->currentText() + ".tar.xz\""_L1)){
    case 0:
        runCmd("xfce4-panel --quit; pkill xfconfd; rm -Rf ~/.config/xfce4/panel ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml; "_L1
           "tar -xf $HOME/.restore/\""_L1 + ui->comboXfcePanelBackups->currentText() + ".tar.xz\" --directory=$HOME/.config/xfce4; "_L1
           "sleep 5; xfce4-panel &"_L1);
        break;
    case 1:
        QMessageBox::information(nullptr, tr("MX Tweak"),
            tr("File is not a valid tar.xz archive file"));
        break;
    case 2:
        QMessageBox::information(nullptr, tr("MX Tweak"),
            tr("Archive does not contain a panel config"));
        break;
    }
    runCmd(u"sleep .5"_s);
    panelWhich();
    panelSetup();
}
void TweakXfce::pushXfcePanelDefault_clicked()
{
    // copy template files
    runCmd(u"xfce4-panel --quit;pkill xfconfd; rm -Rf ~/.config/xfce4/panel; cp -Rf /etc/skel/.config/xfce4/panel ~/.config/xfce4; sleep 1; \
           cp -f /etc/skel/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml; \
           sleep 5; xfce4-panel &"_s);
    runCmd(u"sleep .5"_s);
    panelWhich();
    panelSetup();
}
int TweakXfce::populatePanelBackups()
{
    ui->comboXfcePanelBackups->clear();
    QStringList backups = QDir(QDir::homePath() + "/.restore"_L1).entryList({u"*.tar.xz"_s},
        QDir::Files, QDir::Name | QDir::Reversed);
    backups.replaceInStrings(u".tar.xz"_s, ""_L1);
    ui->comboXfcePanelBackups->addItems(backups);
    return backups.count();
}
int TweakXfce::validateArchive(const QString &path) const
{
    QString test = runCmd("file --mime-type --brief "_L1 + path).output;
    if ( verbose ) qDebug() << test;
    //validate mime type
    if ( test != "application/x-xz"_L1){
        return 1;
    }
    //validate contents
    test = runCmd("tar --list --file "_L1 + path).output;
    if ( verbose ) qDebug() << test;
    if (!test.contains("xfconf/xfce-perchannel-xml/xfce4-panel.xml"_L1)) {
        return 2;
    }
    return 0;
}

//returns first tasklist or docklike id
QString TweakXfce::getTasklistID() const
{
    QString id = runCmd(u"grep -m 1 tasklist ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
    id = id.remove('"').section('-',1,1).section(' ',0,0);
    if (verbose) qDebug() << "tasklist: " << id;
    if (id.isEmpty()) {
        id = runCmd(u"grep -m 1 docklike ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
        id = id.remove('"').section('-',1,1).section(' ',0,0);
        if (verbose) qDebug() << "docklike: " << id;
    }
    return id;
}
void TweakXfce::tasklistChange()
{
    //choice of tasklist
    const QString &choice = ui->comboXfcePanelTasklistPlugin->currentData().toString();
    QString tasklistid = getTasklistID();
    if (verbose) {
        qDebug() << "tasklist choice:" << choice;
        qDebug() << "tasklist ID:" << tasklistid;
    }

    if (choice == "docklike"_L1){
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistid + "/show-handle --reset"_L1);
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistid + "/show-labels --reset"_L1);
    } else if (choice == "tasklist"_L1){
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistid + "/show-handle -t bool -s false --create"_L1);
        QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel +"/mode"_L1).output;
        if (test.isEmpty() || test == "0"_L1) { //horizontal panel
            runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistid + "/show-labels -t bool -s true --create"_L1);
        } else {
            runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistid + "/show-labels -t bool -s false --create"_L1);  //vertical panel
        }
    }

    //switch plugin
    runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistid + " -t string -s "_L1 + choice + " --create"_L1);
    //reset panel
    runCmd(u"xfce4-panel --restart"_s);
    runCmd(u"sleep .5"_s);
}

/* Panel orientation switching (horizontal vs vertical) */

void TweakXfce::panelFlipToHorizontal()
{
    QString file_content;
    QStringList pluginIDs;
    file_content = runCmd("LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel +"/plugin-ids | grep -v Value | grep -v ^$"_L1).output;
    pluginIDs = file_content.split(u"\n"_s);
    if (verbose) qDebug() << pluginIDs;

    // figure out moving the systray, if it exists

    // figure out systrayID, pusleaudio plugin, and tasklistID
    QString systrayID = runCmd(uR"(grep \"systray\" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml)"_s).output;
    systrayID=systrayID.remove(u"\""_s).section(u"-"_s,1,1).section(u" "_s,0,0);
    if (verbose) qDebug() << "systray: " << systrayID;

    QString tasklistID = getTasklistID();

    QString pulseaudioID = runCmd(u"grep pulseaudio ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
    pulseaudioID=pulseaudioID.remove(u"\""_s).section(u"-"_s,1,1).section(u" "_s,0,0);
    if (verbose) qDebug() << "pulseaudio: " << pulseaudioID;

    QString powerID = runCmd(u"grep power-manager-plugin ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
    powerID=powerID.remove(u"\""_s).section(u"-"_s,1,1).section(u" "_s,0,0);
    if (verbose) qDebug() << "powerID: " << powerID;

    QString workspacesID = runCmd(u"grep pager ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
    workspacesID = workspacesID.remove(u"\""_s).section(u"-"_s,1,1).section(u" "_s,0,0);
    if (verbose) qDebug() << "workspacesID: " << workspacesID;

    // if systray exists, do a bunch of stuff to relocate it a list of plugins.  If not present, do nothing to list
    if (!systrayID.isEmpty()) {

        //get tasklist index in list
        int tasklistindex = pluginIDs.indexOf(tasklistID);
        if (verbose) qDebug() << "tasklistIDindex 1" << tasklistindex;

        //check next plugin in list to see if its an expanding separator
        int expsepindex = tasklistindex + 1;
        if (verbose) qDebug() << "expsepindex" << expsepindex;
        QString expsepID = pluginIDs.value(expsepindex);
        if (verbose) qDebug() << "expsepID to test" << expsepID;
        QString test = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + expsepID + "/expand"_L1).output;
        if (verbose) qDebug() << "test parm" << test;

        //move the notification area (systray) to above window buttons (tasklist) in the list if tasklist exists

        if (!tasklistID.isEmpty()) {
            pluginIDs.removeAll(systrayID);
            tasklistindex = pluginIDs.indexOf(tasklistID);
            if (verbose) qDebug() << "tasklistIDindex 2" << tasklistindex;
            pluginIDs.insert(tasklistindex, systrayID);
            if (verbose) qDebug() << "reordered list" << pluginIDs;

            //move the expanding separator

            if (test == "true"_L1) {
                pluginIDs.removeAll(expsepID);
                tasklistindex = pluginIDs.indexOf(tasklistID);
                if (verbose) qDebug() << "tasklistIDindex 2" << tasklistindex;
                pluginIDs.insert(tasklistindex, expsepID);
                if (verbose) qDebug() << "reordered list" << pluginIDs;
            }
        }

        //if the tasklist isn't present, try to make a decision about where to put the systray

        if (tasklistID.isEmpty()) {

            //try to move to in front of clock if present

            QString clockID = runCmd(uR"(grep -m1 "clock\|datetime" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml)"_s).output;
            QString switchID;
            clockID=clockID.remove(u"\""_s).section(u"-"_s,1,1).section(u" "_s,0,0);
            if (verbose) qDebug() << "clockID: " << clockID;
            if (!clockID.isEmpty()) {
                switchID = clockID;

                //if clock found check if next plugin down is a separator and if so put it there

                int clocksepindex = pluginIDs.indexOf(clockID) + 1;
                QString clocksepcheck = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + pluginIDs.value(clocksepindex)).output;
                if (verbose) qDebug() << "clocksepcheck: " << clocksepcheck;
                if (clocksepcheck == "separator"_L1) {
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
        if (!pulseaudioID.isEmpty()) {
            int switchIDindex = 0;
            pluginIDs.removeAll(pulseaudioID);
            switchIDindex = pluginIDs.indexOf(systrayID) + 1;
            pluginIDs.insert(switchIDindex, pulseaudioID);
            if (verbose) qDebug() << "reorderd PA list" << pluginIDs;
        }
        //if power-manager plugin is present, move it to in behind of systray
        if (!powerID.isEmpty()) {
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
        cmdstring = QString(cmdstring + "-s "_L1 + value + " "_L1);
        if (verbose) qDebug() << cmdstring;
    }

    //flip the panel plugins and hold on, it could be a bumpy ride
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel +"/plugin-ids "_L1 + cmdstring);

    //change orientation to horizontal
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel +"/mode -s 0"_L1);

    //change mode of tasklist labels if it exists
    if (!tasklistID.isEmpty()) {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistID + "/show-labels -s true"_L1);
    }

    //change mode of pager if exists
    //xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspaceID + "/rows --type int --set 1"
    //check current workspaces rows
    QString workspacesrows = runCmd("xfconf-query -c xfce4-panel --property /plugins/plugin-"_L1 + workspacesID + "/rows"_L1).output;
    if ( workspacesrows == "1"_L1 || workspacesrows == "2"_L1) {
        runCmd("xfconf-query -c xfce4-panel --property /plugins/plugin-"_L1 + workspacesID + "/rows --type int --set 1"_L1);
    }
}
void TweakXfce::panelFlipToVertical()
{
    QString file_content;
    QStringList pluginIDs;
    file_content = runCmd("LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel +"/plugin-ids | grep -v Value | grep -v ^$"_L1).output;
    pluginIDs = file_content.split(u"\n"_s);
    if (verbose) qDebug() << pluginIDs;

    // figure out moving the systray, if it exists

    QString systrayID = runCmd(uR"(grep \"systray\" ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml)"_s).output;
    systrayID=systrayID.remove(u"\""_s).section(u"-"_s,1,1).section(u" "_s,0,0);
    if (verbose) qDebug() << "systray: " << systrayID;

    QString tasklistID = getTasklistID();

    QString pulseaudioID = runCmd(u"grep pulseaudio ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
    pulseaudioID=pulseaudioID.remove(u"\""_s).section(u"-"_s,1,1).section(u" "_s,0,0);
    if (verbose) qDebug() << "pulseaudio: " << pulseaudioID;

    QString powerID = runCmd(u"grep power-manager-plugin ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
    powerID=powerID.remove(u"\""_s).section(u"-"_s,1,1).section(u" "_s,0,0);
    if (verbose) qDebug() << "powerID: " << powerID;

    QString workspacesID = runCmd(u"grep pager ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
    workspacesID=workspacesID.remove(u"\""_s).section(u"-"_s,1,1).section(u" "_s,0,0);
    if (verbose) qDebug() << "workspacesID: " << workspacesID;

    //if systray exists, do a bunch of stuff to try to move it in a logical way

    if (!systrayID.isEmpty()) {
        // figure out whiskerID, appmenuID, systrayID, tasklistID, and pagerID
        QString whiskerID = runCmd(u"grep whisker ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
        whiskerID=whiskerID.remove(u"\""_s).section(u"-"_s,1,1).section(u" "_s,0,0);
        if (verbose) qDebug() << "whisker: " << whiskerID;

        QString pagerID = runCmd(u"grep pager ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
        pagerID=pagerID.remove(u"\""_s).section(u"-"_s,1,1).section(u" "_s,0,0);
        if (verbose) qDebug() << "pager: " << pagerID;

        QString appmenuID = runCmd(u"grep applicationsmenu ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml"_s).output;
        appmenuID=appmenuID.remove(u"\""_s).section(u"-"_s,1,1).section(u" "_s,0,0);
        if (verbose) qDebug() << "appmenuID: " << appmenuID;

        //get tasklist index in list
        int tasklistindex = pluginIDs.indexOf(tasklistID);
        if (verbose) qDebug() << "tasklistIDindex 1" << tasklistindex;

        //check next plugin in list to see if its an expanding separator
        int expsepindex = tasklistindex + 1;
        if (verbose) qDebug() << "expsepindex" << expsepindex;
        QString expsepID = pluginIDs.value(expsepindex);
        if (verbose) qDebug() << "expsepID to test" << expsepID;
        QString testexpandsep = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + expsepID + "/expand"_L1).output;
        if (verbose) qDebug() << "test parm" << testexpandsep;

        //move the notification area (systray) to an appropriate area.

        //1.  determine if menu is present, place in front of menu

        QString switchID;
        if (!whiskerID.isEmpty()) {
            switchID = whiskerID;
            if (verbose) qDebug() << "switchID whisker: " << switchID;
        } else {
            if (!appmenuID.isEmpty()) {
                switchID = appmenuID;
                if (verbose) qDebug() << "switchID appmenu: " << switchID;
            }
        }

        //        //2.  if so, check second plugin is separator, if so place in front of separator

        //        if (switchID != "") {
        //            QString test = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-" + pluginIDs.value(1)).output;
        //            if (test == "separator"_L1) {
        //                if (verbose) qDebug() << "test parm" << test;
        //                switchID = pluginIDs.value(1);
        //                if (verbose) qDebug() << "switchID sep: " << switchID;
        //            }
        //        }

        //3.  if so, check third plugin is pager.  if so, place tasklist in front of pager

        if (!switchID.isEmpty()) {
            QString test = runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + pluginIDs.value(1)).output;
            if (test == "pager"_L1) {
                if (verbose) qDebug() << "test parm" << test;
                switchID = pluginIDs.value(1);
                if (verbose) qDebug() << "switchID pager: " << switchID;
            }
        }

        // if the menu doesn't exist, give a default value that is sane but might not be correct
        if (switchID.isEmpty()) {
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
        if (!pulseaudioID.isEmpty()) {
            int switchIDindex = 0;
            pluginIDs.removeAll(pulseaudioID);
            switchIDindex = pluginIDs.indexOf(systrayID) + 1;
            pluginIDs.insert(switchIDindex, pulseaudioID);
            if (verbose) qDebug() << "reorderd PA list" << pluginIDs;
        }

        //if powerID plugin is present, move it to in behind of systray
        if (!powerID.isEmpty()) {
            int switchIDindex = 0;
            pluginIDs.removeAll(powerID);
            switchIDindex = pluginIDs.indexOf(systrayID);
            pluginIDs.insert(switchIDindex, powerID);
            if (verbose) qDebug() << "reorderd PA list" << pluginIDs;
        }
        //move the expanding separator
        if (testexpandsep == "true"_L1) {
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
        cmdstring = QString(cmdstring + "-s "_L1 + value + " "_L1);
        if (verbose) qDebug() << cmdstring;
    }
    //flip the panel plugins and pray for a miracle
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel +"/plugin-ids "_L1 + cmdstring);

    //change orientation to vertical
    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/mode -n -t int -s 2"_L1);

    //change mode of tasklist labels if they exist
    if (!tasklistID.isEmpty()) {
        runCmd("xfconf-query -c xfce4-panel -p /plugins/plugin-"_L1 + tasklistID + "/show-labels -s false"_L1);
    }

    //change mode of pager if exists
    //xfconf-query -c xfce4-panel --property /plugins/plugin-" + workspaceID + "/rows --type int --set 2"
    //check current workspaces rows
    QString workspacesrows = runCmd("xfconf-query -c xfce4-panel --property /plugins/plugin-"_L1 + workspacesID + "/rows"_L1).output;
    if ( workspacesrows == "1"_L1 || workspacesrows == "2"_L1) {
        runCmd("xfconf-query -c xfce4-panel --property /plugins/plugin-"_L1 + workspacesID + "/rows --type int --set 2"_L1);
    }
}
void TweakXfce::panelSetPosition()
{
    //move to user selected top or bottom border per mx-16 defaults
    QString newPos = ui->comboXfcePanelPlacement->currentData().toString();
    if (newPos == "horz-bottom"_L1) {
        newPos = u"12"_s;
    } else if (newPos == "horz-top"_L1) {
        newPos = u"11"_s;
    } else if (newPos == "vert-left"_L1) {
        newPos = u"5"_s;
    } else if (newPos == "vert-right"_L1) {
        newPos = u"1"_s;
    }

    if (verbose) {
        qDebug() << "position:" << newPos << ui->comboXfcePanelPlacement->currentData();
    }

    runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/position -s 'p="_L1 + newPos + ";x=0;y=0'"_L1);
}
