#include "QDebug"

#include "cmd.h"
#include "defaultlook.h"
#include "ui_window_buttons.h"
#include "window_buttons.h"

window_buttons::window_buttons(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::window_buttons)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    setup();
}

window_buttons::~window_buttons()
{
    delete ui;
}

void window_buttons::setup()
{
    setWindowTitle(tr("Window Buttons"));

    plugintasklist.clear();
    plugintasklist = runCmd(QStringLiteral(R"(cat ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml | grep \"tasklist\"|cut -d '=' -f2 | cut -d '' -f1| cut -d '"' -f2)")).output;
    qDebug() << "tasklist is " << plugintasklist;

    //setup initial values

    // use Xfce defaults for "do not exist" values"

    QString test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/show-labels").output;
    qDebug() << "show button labels is: " << test;

    ui->checkBoxbuttonlabels->setChecked(test.contains(QLatin1String("does not exist")) || test == QLatin1String("true"));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/flat-buttons").output;
    qDebug() << "flatbuttons is: " << test;

    ui->checkBoxshowflatbuttons->setChecked(test == QLatin1String("true") && !test.contains(QLatin1String("does not exist")));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/show-handle").output;
    qDebug() << "show handles is: " << test;

    ui->checkBoxshowhandle->setChecked(test.contains(QLatin1String("does not exist")) || test == QLatin1String("true"));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/sort-order").output;
    qDebug() << "sort order is: " << test;

    if ( test.contains(QLatin1String("does not exist"))) {
        ui->comboBoxsortingorder->setCurrentIndex(1);
    } else {
        ui->comboBoxsortingorder->setCurrentIndex(test.toInt());
    }

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/grouping").output;
    qDebug() << "grouping is: " << test;

    if ( test.contains(QLatin1String("does not exist"))) {
        ui->comboBoxwindowgrouping->setCurrentIndex(0);
    } else {
        ui->comboBoxwindowgrouping->setCurrentIndex(test.toInt());
    }


    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/middle-click").output;
    qDebug() << "middle-click is: " << test;

    if ( test.contains(QLatin1String("does not exist"))) {
        ui->comboBoxmiddleclickaction->setCurrentIndex(0);
    } else {
        ui->comboBoxmiddleclickaction->setCurrentIndex(test.toInt());
    }

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/switch-workspace-on-unminimize").output;
    qDebug() << "restore minimize is: " << test;

    ui->checkBoxrestoreminwindows->setChecked(test == QLatin1String("true") && !test.contains(QLatin1String("does not exist")));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/show-wireframes").output;
    qDebug() << "draw window frames is: " << test;

    ui->checkBoxdrawframes->setChecked(test == QLatin1String("true") && !test.contains(QLatin1String("does not exist")));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/window-scrolling").output;
    qDebug() << "window scrolling is: " << test;
    ui->checkBoxswitchwindowsmousewheel->setChecked(test.contains(QLatin1String("does not exist")) || test == QLatin1String("true"));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/include-all-workspaces").output;
    qDebug() << "include-all-workspaces is: " << test;

    ui->checkBoxwindowsallworkspaces->setChecked(test == QLatin1String("true") && !test.contains(QLatin1String("does not exist")));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/show-only-minimized").output;
    qDebug() << "show-only-minimized is: " << test;

    ui->checkBoxonlyminwindows->setChecked(test == QLatin1String("true") && !test.contains(QLatin1String("does not exist")));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/include-all-monitors").output;
    qDebug() << "include-all-monitors is: " << test;

    ui->checkBoxwindowsallmonitors->setChecked(test.contains(QLatin1String("does not exist")) || test == QLatin1String("true"));
}

void window_buttons::on_pushButton_clicked()
{
    close();
}

void window_buttons::on_checkBoxbuttonlabels_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxbuttonlabels->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/show-labels -t bool" + " -s " + param  + " --create";
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxshowflatbuttons_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxshowflatbuttons->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/flat-buttons -t bool" + " -s " + param  + " --create";
    system(cmd.toUtf8());
}


void window_buttons::on_checkBoxshowhandle_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxshowhandle->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/show-handle -t bool" + " -s " + param  + " --create";
    system(cmd.toUtf8());
}

void window_buttons::on_comboBoxsortingorder_currentIndexChanged(int index) const
{
    QString param = QString::number(index);
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/sort-order -t int" + " -s " + param  + " --create";
    system(cmd.toUtf8());
}


void window_buttons::on_comboBoxwindowgrouping_currentIndexChanged(int index) const
{
    QString param = QString::number(index);
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/grouping -t int" + " -s " + param  + " --create";
    system(cmd.toUtf8());
}

void window_buttons::on_comboBoxmiddleclickaction_currentIndexChanged(int index) const
{
    QString param = QString::number(index);
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/middle-click -t int" + " -s " + param  + " --create";
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxrestoreminwindows_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxrestoreminwindows->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/switch-workspace-on-unminimize -t bool" + " -s " + param  + " --create";
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxdrawframes_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxdrawframes->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/show-wireframes -t bool" + " -s " + param  + " --create";
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxswitchwindowsmousewheel_toggled(bool /*checked*/)
{
    QString param = ui->checkBoxswitchwindowsmousewheel->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/window-scrolling -t bool" + " -s " + param  + " --create";
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxwindowsallworkspaces_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxwindowsallworkspaces->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/include-all-workspaces boolean -t bool" + " -s " + param  + " --create";
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxonlyminwindows_toggled(bool  /*checked*/)
{
    QString param =ui->checkBoxonlyminwindows->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/show-only-minimized boolean -t bool" + " -s " + param  + " --create";
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxwindowsallmonitors_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxwindowsallmonitors->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/" + plugintasklist + "/include-all-monitors -t bool" + " -s " + param  + " --create";
    system(cmd.toUtf8());
}
