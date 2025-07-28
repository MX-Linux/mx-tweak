#include <QDir>
#include "ui_defaultlook.h"
#include "cmd.h"
#include "tweak_plasma.h"

using namespace Qt::Literals::StringLiterals;

namespace PanelIndex { // location combo index - 0=bottom, 1=left, 2=top, 3=right
enum PanelIndex {Bottom, Left, Top, Right};
}
namespace PanelLocation { // location plasma settings - 4=bottom, 3 top, 5 left, 6 right
enum {Top = 3, Bottom, Left, Right};
}

TweakPlasma::TweakPlasma(Ui::defaultlook *ui, bool verbose, QObject *parent)
    : QObject(parent), ui(ui), verbose(verbose)
{
    connect(ui->pushApplyPlasma, &QPushButton::clicked, this, &TweakPlasma::pushApplyPlasma_clicked);
    setup();
}

void TweakPlasma::setup()
{
    QString home_path = QDir::homePath();
    //get panel ID
    plasmaPanelId = runCmd(u"grep --max-count 1 -B 8 panel $HOME/.config/plasma-org.kde.plasma.desktop-appletsrc |grep Containment"_s).output;
    QString panellocation = readPlasmaPanelConfig(u"location"_s);

    switch(panellocation.toInt()) {
    case PanelLocation::Top:
        ui->comboPlasmaPanelLocation->setCurrentIndex(PanelIndex::Top);
        break;
    case PanelLocation::Bottom:
        ui->comboPlasmaPanelLocation->setCurrentIndex(PanelIndex::Bottom);
        break;
    case PanelLocation::Left:
        ui->comboPlasmaPanelLocation->setCurrentIndex(PanelIndex::Left);
        break;
    case PanelLocation::Right:
        ui->comboPlasmaPanelLocation->setCurrentIndex(PanelIndex::Right);
        break;
    default: ui->comboPlasmaPanelLocation->setCurrentIndex(PanelIndex::Bottom);
    }

    //setup plasma-discover autostart
    if (QFile::exists(u"/usr/lib/x86_64-linux-gnu/libexec/DiscoverNotifier"_s)){
        QString plasmadiscoverautostart = home_path + "/.config/autostart/org.kde.discover.notifier.desktop"_L1;
        if (runCmd("grep Hidden=true "_L1 + plasmadiscoverautostart).exitCode == 0 ){
            ui->checkPlasmaDiscoverUpdater->setChecked(false);
        } else {
            ui->checkPlasmaDiscoverUpdater->setChecked(true);
        }
    } else {
        ui->checkPlasmaDiscoverUpdater->hide();
    }

    //setup singleclick
    QString singleclick = runCmd(u"kreadconfig5 --group KDE --key SingleClick"_s).output;
    ui->checkPlasmaSingleClick->setChecked(singleclick != "false");

    //get taskmanager ID and setup showOnlyCurrentDesktop
    plasmataskmanagerID = runCmd(u"grep --max-count 1 -B 2 taskmanager $HOME/.config/plasma-org.kde.plasma.desktop-appletsrc |grep Containment"_s).output;
    QString showOnlyCurrentDesktop = readTaskmanagerConfig(u"showOnlyCurrentDesktop"_s);
    if (showOnlyCurrentDesktop == "true"_L1) {
        ui->checkPlasmaShowAllWorkspaces->setChecked(false);
    } else {
        ui->checkPlasmaShowAllWorkspaces->setChecked(true);
    }

    ui->pushApplyPlasma->setDisabled(true);

    ui->checkPlasmaResetDock->setChecked(false);

    plasmaplacementflag = false;
    plasmaworkspacesflag = false;
    plasmasingleclickflag = false;
    plasmaresetflag = false;
    plasmasystrayiconsizeflag = false;
    plasmadisoverautostartflag = false;

    connect(ui->pushApplyPlasma, &QPushButton::clicked, this, &TweakPlasma::pushApplyPlasma_clicked);
    connect(ui->comboPlasmaPanelLocation, &QComboBox::currentIndexChanged, this, &TweakPlasma::comboPlasmaPanelLocation_currentIndexChanged);
    connect(ui->checkPlasmaSingleClick, &QCheckBox::toggled, this, &TweakPlasma::checkPlasmaSingleClick_toggled);
    connect(ui->checkPlasmaShowAllWorkspaces, &QCheckBox::toggled, this, &TweakPlasma::checkPlasmaShowAllWorkspaces_toggled);
    connect(ui->checkPlasmaResetDock, &QCheckBox::toggled, this, &TweakPlasma::checkPlasmaResetDock_toggled);
    connect(ui->checkPlasmaDiscoverUpdater, &QCheckBox::toggled, this, &TweakPlasma::checkPlasmaDiscoverUpdater_toggled);
}

bool TweakPlasma::checkPlasma() const
{
    QString test = runCmd(u"pgrep plasma"_s).output;
    if (verbose) qDebug() << test;
    return (!test.isEmpty());
}

QString TweakPlasma::readTaskmanagerConfig(const QString &key) const
{
    QString ID = plasmataskmanagerID.section(u"["_s,2,2).section(u"]"_s,0,0);
    QString Applet = plasmataskmanagerID.section(u"["_s,4,4).section(u"]"_s,0,0);
    if (verbose) qDebug() << "plasma taskmanager ID is " << ID;
    if (verbose) qDebug() << "plasma taskmanger Applet ID is " << Applet;
    //read key
    QString value = runCmd("kreadconfig5 --file plasma-org.kde.plasma.desktop-appletsrc --group Containments --group "_L1 + ID + " --group Applets --group "_L1 + Applet + " --key "_L1 + key).output;
    if (value.isEmpty()) {
        value = u"false"_s;
    }
    if (verbose) qDebug() << "key is " << value;
    return value;
}

QString TweakPlasma::readPlasmaPanelConfig(const QString &key) const
{
    QString ID = plasmaPanelId.section(u"["_s,2,2).section(u"]"_s,0,0);
    if (verbose) qDebug() << "plasma panel ID" << ID;
    //read key
    QString value = runCmd("kreadconfig5 --file plasma-org.kde.plasma.desktop-appletsrc --group Containments --group "_L1 + ID + " --key "_L1 + key).output;
    if (verbose) qDebug() << "key is " << value;
    return value;
}

void TweakPlasma::writePlasmaPanelConfig(const QString &key, const QString &value) const
{
    QString ID;
    ID = plasmaPanelId.section('[',2,2).section(']',0,0);
    runCmd("kwriteconfig5 --file plasma-org.kde.plasma.desktop-appletsrc --group Containments --group "_L1 + ID + " --key "_L1 + key + ' ' + value);
}

void TweakPlasma::writeTaskmanagerConfig(const QString &key, const QString &value) const
{
    QString ID = plasmataskmanagerID.section('[',2,2).section(']',0,0);
    QString Applet = plasmataskmanagerID.section('[',4,4).section(']',0,0);
    runCmd("kwriteconfig5 --file plasma-org.kde.plasma.desktop-appletsrc --group Containments --group "_L1 + ID + " --group Applets --group "_L1 + Applet + " --key "_L1 + key + ' ' + value);
}

void TweakPlasma::comboPlasmaPanelLocation_currentIndexChanged(int  /*index*/)
{
    ui->pushApplyPlasma->setEnabled(true);
    plasmaplacementflag = true;
}
void TweakPlasma::checkPlasmaDiscoverUpdater_toggled(bool)
{
    ui->pushApplyPlasma->setEnabled(true);
    plasmadisoverautostartflag = true;
}
void TweakPlasma::checkPlasmaSingleClick_toggled(bool)
{
    ui->pushApplyPlasma->setEnabled(true);
    plasmasingleclickflag = true;
}
void TweakPlasma::checkPlasmaShowAllWorkspaces_toggled(bool)
{
    ui->pushApplyPlasma->setEnabled(true);
    plasmaworkspacesflag = true;
}
void TweakPlasma::checkPlasmaResetDock_toggled(bool)
{
    ui->pushApplyPlasma->setEnabled(true);
    plasmaresetflag = true;
}

void TweakPlasma::pushApplyPlasma_clicked()
{
    QString home_path = QDir::homePath();
    if (plasmaresetflag) {
        plasmaplacementflag = false;
        plasmasingleclickflag = false;
        plasmaworkspacesflag = false;
        //reset plasma script Adrian
        runCmd(u"/usr/lib/mx-tweak/reset-kde-mx"_s);
    }
    if (plasmaplacementflag) {
        switch(ui->comboPlasmaPanelLocation->currentIndex()) {
        case PanelIndex::Bottom:
            writePlasmaPanelConfig(u"location"_s, QString::number(PanelLocation::Bottom));
            writePlasmaPanelConfig(u"formfactor"_s, u"2"_s);
            break;
        case PanelIndex::Left:
            writePlasmaPanelConfig(u"location"_s, QString::number(PanelLocation::Left));
            writePlasmaPanelConfig(u"formfactor"_s, u"3"_s);
            break;
        case PanelIndex::Top:
            writePlasmaPanelConfig(u"location"_s, QString::number(PanelLocation::Top));
            writePlasmaPanelConfig(u"formfactor"_s, u"2"_s);
            break;
        case PanelIndex::Right:
            writePlasmaPanelConfig(u"location"_s, QString::number(PanelLocation::Right));
            writePlasmaPanelConfig(u"formfactor"_s, u"3"_s);
            break;
        }
    }

    if (plasmasingleclickflag) {
        QString value = ui->checkPlasmaSingleClick->isChecked() ? u"true"_s : u"false"_s;
        runCmd("kwriteconfig5 --group KDE --key SingleClick "_L1 + value);
        runCmd("pkexec /usr/lib/mx-tweak/mx-tweak-kde-edit.sh "_L1 + value);
    }

    if (plasmaworkspacesflag) {
        QString value = ui->checkPlasmaShowAllWorkspaces->isChecked() ? u"false"_s : u"true"_s;
        writeTaskmanagerConfig(u"showOnlyCurrentDesktop"_s, value);
    }

    //plasma-discover autostart
    if (plasmadisoverautostartflag){
        QString plasmadiscoverautostart = home_path + "/.config/autostart/org.kde.discover.notifier.desktop"_L1;
        qDebug() << "discover autostart path is " << plasmadiscoverautostart;
        if (ui->checkPlasmaDiscoverUpdater->isChecked()){
            //delete any Hidden=true lines to make sure its processed by xdg autostart
            runCmd("sed -i /Hidden=true/d "_L1 + plasmadiscoverautostart);
        } else {
            //copy if it doesn't exist already
            if (!QFile(plasmadiscoverautostart).exists()){
                if (QFile::exists(u"/etc/xdg/autostart/org.kde.discover.notifier.desktop"_s)){
                    runCmd("cp /etc/xdg/autostart/org.kde.discover.notifier.desktop "_L1 + plasmadiscoverautostart);
                }
            }
            //remove any previous Hidden= attribute, then add Hidden=true to make it not autostart
            runCmd("sed -i /Hidden=*/d "_L1 + plasmadiscoverautostart);
            runCmd("echo Hidden=true >> "_L1 + plasmadiscoverautostart); //this also creates file if the /etc/xdg version is missing
        }
    }

    //time to reset kwin and plasmashell
    if (plasmaworkspacesflag || plasmasingleclickflag || plasmaplacementflag || plasmaresetflag || plasmasystrayiconsizeflag) {
        //restart kwin first
        runCmd(u"sleep 1; qdbus org.kde.KWin /KWin reconfigure"_s);
        //then plasma
        system("sleep 1; plasmashell --replace &");
    }

    setup();
}
