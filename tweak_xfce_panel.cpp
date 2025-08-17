#include <QProcess>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QMessageBox>
#include "ui_tweak.h"
#include "cmd.h"
#include "window_buttons.h"
#include "tweak_xfce_panel.h"

using namespace Qt::Literals::StringLiterals;

TweakXfcePanel::TweakXfcePanel(Ui::Tweak *ui, bool verbose, QObject *parent) noexcept
    : QObject{parent}, ui{ui}, verbose{verbose}
{
    // Panel orientation
    ui->comboXfcePanelPlacement->addItem(tr("Horizontal (Bottom)"), u"horz-bottom"_s);
    ui->comboXfcePanelPlacement->addItem(tr("Horizontal (Top)"), u"horz-top"_s);
    ui->comboXfcePanelPlacement->insertSeparator(2);
    ui->comboXfcePanelPlacement->addItem(tr("Vertical (Left)"), u"vert-left"_s);
    ui->comboXfcePanelPlacement->addItem(tr("Vertical (Right)"), u"vert-right"_s);
    // Tasklist plugins
    ui->comboXfcePanelTasklistPlugin->addItem(tr("docklike"), u"docklike"_s);
    ui->comboXfcePanelTasklistPlugin->addItem(tr("Window Buttons"), u"tasklist"_s);

    whichPanel();
    setup();

    connect(ui->pushXfcePanelApply, &QPushButton::clicked, this, &TweakXfcePanel::pushXfcePanelApply_clicked);
    connect(ui->comboXfcePanelPlacement, &QComboBox::currentIndexChanged, this, &TweakXfcePanel::comboXfcePanelPlacement_currentIndexChanged);
    connect(ui->spinXfcePanelPluginVolume, &QDoubleSpinBox::valueChanged, this, &TweakXfcePanel::slotPluginScaleChanged);
    connect(ui->spinXfcePanelPluginPower, &QDoubleSpinBox::valueChanged, this, &TweakXfcePanel::slotPluginScaleChanged);
    connect(ui->comboXfcePanelTasklistPlugin, &QComboBox::currentIndexChanged, this, &TweakXfcePanel::comboXfcePanelTasklistPlugin_currentIndexChanged);
    connect(ui->pushXfcePanelTasklistOptions, &QPushButton::clicked, this, &TweakXfcePanel::pushXfcePanelTasklistOptions_clicked);
    connect(ui->pushXfcePanelDocklikeOptions, &QPushButton::clicked, this, &TweakXfcePanel::pushXfcePanelDocklikeOptions_clicked);
    connect(ui->pushXfcePanelBackup, &QPushButton::clicked, this, &TweakXfcePanel::pushXfcePanelBackup_clicked);
    connect(ui->pushXfcePanelRestore, &QPushButton::clicked, this, &TweakXfcePanel::pushXfcePanelRestore_clicked);
    connect(ui->pushXfcePanelDefault, &QPushButton::clicked, this, &TweakXfcePanel::pushXfcePanelDefault_clicked);
    connect(ui->pushXfcePanelSettings, &QPushButton::clicked, this, &TweakXfcePanel::pushXfcePanelSettings_clicked);
}

void TweakXfcePanel::whichPanel() noexcept
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
void TweakXfcePanel::setup() noexcept
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
        if (runCmd(cmd).exitCode == 0 ) {
            if (verbose) qDebug() << "include statement found";
        } else {
            if (verbose) qDebug() << "adding include statement";
            runCmd("echo '@import url(\"xfce4-panel-tweaks.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1);
        }
    } else {
        if (verbose) qDebug() << "creating simple gtk.css file";
        runCmd("echo '@import url(\"xfce4-panel-tweaks.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1);
    }

    if (!QFileInfo::exists(home_path + "/.config/gtk-3.0/xfce4-panel-tweaks.css"_L1)) {
        runCmd("cp /usr/share/mx-tweak/xfce4-panel-tweaks.css "_L1 + home_path + "/.config/gtk-3.0/"_L1);
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
    if (populateBackups() <= 0) {
        pushXfcePanelBackup_clicked();
    }

    flags.panel = false;
    ui->pushXfcePanelApply->setEnabled(false);

    //hide tasklist setting if not present
    bool tasklist = true;
    bool docklike = true;

    if (runCmd(u"xfconf-query -c xfce4-panel -p /plugins -lv |grep tasklist"_s).exitCode != 0 ) {
        ui->pushXfcePanelTasklistOptions->hide();
        tasklist = false;
    }

    //hide docklike settings if not present
    if (runCmd(u"xfconf-query -c xfce4-panel -p /plugins -lv |grep docklike"_s).exitCode != 0 ) {
        ui->pushXfcePanelDocklikeOptions->hide();
        docklike = false;
    }

    //check status of docklike external package, hide chooser if not installed
    QString check = runCmd(u"LANG=c dpkg-query -s xfce4-docklike-plugin |grep Status"_s).output;
    if (!check.contains("installed"_L1)){
        docklike = true;
    }

    //display tasklist plugin selector if only one tasklist in use
    if ( tasklist && docklike ){
        ui->labelXfcePanelTasklist->hide();
        ui->comboXfcePanelTasklistPlugin->hide();
    } else if (tasklist) {
        ui->comboXfcePanelTasklistPlugin->setCurrentIndex(1); // index 1 is window buttons
    } else if (docklike) {
        ui->comboXfcePanelTasklistPlugin->setCurrentIndex(0); // index 0 is doclike
    } else {
        ui->groupXfcePanelTasklist->hide();
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

void TweakXfcePanel::slotPluginScaleChanged(double) noexcept
{
    ui->pushXfcePanelApply->setEnabled(true);
    flags.scales = true;
}
void TweakXfcePanel::pushXfcePanelApply_clicked() noexcept
{
    ui->pushXfcePanelApply->setEnabled(false);

    // tasklist switch
    if (flags.tasklist) {
        tasklistChange();
    }

    const bool restart = (flags.panel || flags.scales);
    if (restart) {
        runProc(u"xfce4-panel"_s, {"--quit"});
    }
    if (flags.panel) {
        flags.panel = false;
        const QString &newPlace = ui->comboXfcePanelPlacement->currentData().toString();
        if (newPlace.startsWith("horz-"_L1)) {
            QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/mode"_L1).output;
            if (test == "1"_L1 || test == "2"_L1) {
                flipToHorizontal();
            }
        } else if (newPlace.startsWith("vert-"_L1)) {
            QString test = runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel + "/mode"_L1).output;
            if (test == ""_L1 || test == "0"_L1) {
                flipToVertical();
            }
        }
        setPosition(); // Left/right (vertical) or bottom/top (horizontal)
    }
    if (flags.scales) {
        runCmd("sed -i '/xfce4-power-manager-plugin/,/\\}/ s/scale(.*)/scale("_L1
            + QString::number(ui->spinXfcePanelPluginPower->value())
            + ")/' ~/.config/gtk-3.0/xfce4-panel-tweaks.css"_L1);
        runCmd("sed -i '/pulseaudio/,/\\}/ s/scale(.*)/scale("_L1
            + QString::number(ui->spinXfcePanelPluginVolume->value())
            + ")/' ~/.config/gtk-3.0/xfce4-panel-tweaks.css"_L1);
        flags.scales = false;
    }
    if (restart) {
        QProcess::startDetached(u"xfce4-panel"_s);
    }

    setup();
}

void TweakXfcePanel::comboXfcePanelPlacement_currentIndexChanged(int) noexcept
{
    ui->pushXfcePanelApply->setEnabled(true);
    flags.panel = true;
}

void TweakXfcePanel::comboXfcePanelTasklistPlugin_currentIndexChanged(int) noexcept
{
    ui->pushXfcePanelApply->setEnabled(true);
    flags.tasklist = true;
}
void TweakXfcePanel::pushXfcePanelTasklistOptions_clicked() noexcept
{
    ui->tabWidget->setEnabled(false);
    window_buttons wb;
    wb.setModal(true);
    wb.exec();
    ui->tabWidget->setEnabled(true);
}
void TweakXfcePanel::pushXfcePanelDocklikeOptions_clicked() noexcept
{
    ui->tabWidget->setEnabled(false);
    runProc(u"xfce4-panel"_s, {u"--plugin-event=docklike:settings"_s});
    ui->tabWidget->setEnabled(true);
}

// backs up the current panel configuration
void TweakXfcePanel::pushXfcePanelBackup_clicked() noexcept
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
    populateBackups();
}
void TweakXfcePanel::pushXfcePanelRestore_clicked() noexcept
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
    sleep(500);
    whichPanel();
    setup();
}
void TweakXfcePanel::pushXfcePanelDefault_clicked() noexcept
{
    // copy template files
    runCmd(u"xfce4-panel --quit;pkill xfconfd; rm -Rf ~/.config/xfce4/panel; cp -Rf /etc/skel/.config/xfce4/panel ~/.config/xfce4; sleep 1; \
           cp -f /etc/skel/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml; \
           sleep 5; xfce4-panel &"_s);
    sleep(500);
    whichPanel();
    setup();
}
int TweakXfcePanel::populateBackups() noexcept
{
    ui->comboXfcePanelBackups->clear();
    QStringList backups = QDir(QDir::homePath() + "/.restore"_L1).entryList({u"*.tar.xz"_s},
        QDir::Files, QDir::Name | QDir::Reversed);
    backups.replaceInStrings(u".tar.xz"_s, ""_L1);
    ui->comboXfcePanelBackups->addItems(backups);
    return backups.count();
}
int TweakXfcePanel::validateArchive(const QString &path) const noexcept
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
QString TweakXfcePanel::getTasklistID() const noexcept
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
void TweakXfcePanel::tasklistChange() noexcept
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
    sleep(500);
}

/* Panel orientation switching (horizontal vs vertical) */

void TweakXfcePanel::flipToHorizontal() noexcept
{
    QString file_content;
    QStringList pluginIDs;
    file_content = runCmd("LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel +"/plugin-ids | grep -v Value | grep -v ^$"_L1).output;
    pluginIDs = file_content.split(u"\n"_s);
    if (verbose) qDebug() << "Flip to Horizontal" << pluginIDs;

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
void TweakXfcePanel::flipToVertical() noexcept
{
    QString file_content;
    QStringList pluginIDs;
    file_content = runCmd("LC_ALL=en_US.UTF-8 xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + panel +"/plugin-ids | grep -v Value | grep -v ^$"_L1).output;
    pluginIDs = file_content.split(u"\n"_s);
    if (verbose) qDebug() << "Flip to Vertical" << pluginIDs;

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
void TweakXfcePanel::setPosition() noexcept
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

void TweakXfcePanel::pushXfcePanelSettings_clicked() noexcept
{
    ui->tabWidget->setEnabled(false);
    runProc(u"xfce4-panel"_s, {u"--preferences"_s});
    runProc(u"xprop"_s, {u"-spy"_s, u"-name"_s, u"Panel Preferences"_s});

    //restart panel if background style of any panel is 1 - solid color, affects transparency
    const QStringList &properties = runCmd(u"xfconf-query -c xfce4-panel --list"_s
        u" |grep background-style"_s).output.split('\n', Qt::SkipEmptyParts);

    bool flag = false;
    for (const QString &value : properties) {
        const QString &test = runCmd("xfconf-query -c xfce4-panel -p "_L1 + value).output;
        if (test == "1"_L1) {
            flag = true;
        }
    }
    if (flag) {
        runProc(u"xfce4-panel"_s, {u"--restart"_s});
    }

    setup();
    ui->tabWidget->setEnabled(true);
}

void TweakXfcePanel::sleep(int msec) noexcept
{
    QTimer timer(this);
    QEventLoop loop(this);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(msec);
    loop.exec();
}
