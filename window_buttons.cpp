#include "QDebug"

#include "cmd.h"
#include "defaultlook.h"
#include "ui_window_buttons.h"
#include "window_buttons.h"

using namespace Qt::Literals::StringLiterals;

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
    plugintasklist = runCmd(uR"(cat ~/.config/xfce4/xfconf/xfce-perchannel-xml/xfce4-panel.xml | grep \"tasklist\"|cut -d '=' -f2 | cut -d '' -f1| cut -d '"' -f2)"_s).output;
    qDebug() << "tasklist is " << plugintasklist;

    //setup initial values

    // use Xfce defaults for "do not exist" values"

    QString test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-labels"_L1).output;
    qDebug() << "show button labels is: " << test;

    ui->checkBoxbuttonlabels->setChecked(test.contains("does not exist"_L1) || test == "true"_L1);

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/flat-buttons"_L1).output;
    qDebug() << "flatbuttons is: " << test;

    ui->checkBoxshowflatbuttons->setChecked(test == "true"_L1 && !test.contains("does not exist"_L1));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-handle"_L1).output;
    qDebug() << "show handles is: " << test;

    ui->checkBoxshowhandle->setChecked(test.contains("does not exist"_L1) || test == "true"_L1);

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/sort-order"_L1).output;
    qDebug() << "sort order is: " << test;

    if ( test.contains("does not exist"_L1)) {
        ui->comboBoxsortingorder->setCurrentIndex(1);
    } else {
        ui->comboBoxsortingorder->setCurrentIndex(test.toInt());
    }

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/grouping"_L1).output;
    qDebug() << "grouping is: " << test;

    if ( test.contains("does not exist"_L1)) {
        ui->comboBoxwindowgrouping->setCurrentIndex(0);
    } else {
        ui->comboBoxwindowgrouping->setCurrentIndex(test.toInt());
    }


    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/middle-click"_L1).output;
    qDebug() << "middle-click is: " << test;

    if ( test.contains("does not exist"_L1)) {
        ui->comboBoxmiddleclickaction->setCurrentIndex(0);
    } else {
        ui->comboBoxmiddleclickaction->setCurrentIndex(test.toInt());
    }

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/switch-workspace-on-unminimize"_L1).output;
    qDebug() << "restore minimize is: " << test;

    ui->checkBoxrestoreminwindows->setChecked(test == "true"_L1 && !test.contains("does not exist"_L1));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-wireframes"_L1).output;
    qDebug() << "draw window frames is: " << test;

    ui->checkBoxdrawframes->setChecked(test == "true"_L1 && !test.contains("does not exist"_L1));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/window-scrolling"_L1).output;
    qDebug() << "window scrolling is: " << test;
    ui->checkBoxswitchwindowsmousewheel->setChecked(test.contains("does not exist"_L1) || test == "true"_L1);

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/include-all-workspaces"_L1).output;
    qDebug() << "include-all-workspaces is: " << test;

    ui->checkBoxwindowsallworkspaces->setChecked(test == "true"_L1 && !test.contains("does not exist"_L1));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-only-minimized"_L1).output;
    qDebug() << "show-only-minimized is: " << test;

    ui->checkBoxonlyminwindows->setChecked(test == "true"_L1 && !test.contains("does not exist"_L1));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/include-all-monitors"_L1).output;
    qDebug() << "include-all-monitors is: " << test;

    ui->checkBoxwindowsallmonitors->setChecked(test.contains("does not exist"_L1) || test == "true"_L1);
}

void window_buttons::on_pushButton_clicked()
{
    close();
}

void window_buttons::on_checkBoxbuttonlabels_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxbuttonlabels->isChecked() ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-labels -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxshowflatbuttons_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxshowflatbuttons->isChecked() ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/flat-buttons -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}


void window_buttons::on_checkBoxshowhandle_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxshowhandle->isChecked() ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-handle -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::on_comboBoxsortingorder_currentIndexChanged(int index) const
{
    QString param = QString::number(index);
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/sort-order -t int"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}


void window_buttons::on_comboBoxwindowgrouping_currentIndexChanged(int index) const
{
    QString param = QString::number(index);
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/grouping -t int"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::on_comboBoxmiddleclickaction_currentIndexChanged(int index) const
{
    QString param = QString::number(index);
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/middle-click -t int"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxrestoreminwindows_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxrestoreminwindows->isChecked() ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/switch-workspace-on-unminimize -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxdrawframes_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxdrawframes->isChecked() ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-wireframes -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxswitchwindowsmousewheel_toggled(bool /*checked*/)
{
    QString param = ui->checkBoxswitchwindowsmousewheel->isChecked() ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/window-scrolling -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxwindowsallworkspaces_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxwindowsallworkspaces->isChecked() ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/include-all-workspaces boolean -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxonlyminwindows_toggled(bool  /*checked*/)
{
    QString param =ui->checkBoxonlyminwindows->isChecked() ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-only-minimized boolean -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::on_checkBoxwindowsallmonitors_toggled(bool  /*checked*/)
{
    QString param = ui->checkBoxwindowsallmonitors->isChecked() ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/include-all-monitors -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}
