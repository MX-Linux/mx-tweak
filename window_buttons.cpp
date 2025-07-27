#include "QDebug"

#include "cmd.h"
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
    connect(ui->pushClose, &QPushButton::clicked, this, &window_buttons::close);
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

    ui->checkButtonLabels->setChecked(test.contains("does not exist"_L1) || test == "true"_L1);

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/flat-buttons"_L1).output;
    qDebug() << "flatbuttons is: " << test;

    ui->checkShowFlatButtons->setChecked(test == "true"_L1 && !test.contains("does not exist"_L1));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-handle"_L1).output;
    qDebug() << "show handles is: " << test;

    ui->checkShowHandle->setChecked(test.contains("does not exist"_L1) || test == "true"_L1);

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/sort-order"_L1).output;
    qDebug() << "sort order is: " << test;

    if ( test.contains("does not exist"_L1)) {
        ui->comboSortingOrder->setCurrentIndex(1);
    } else {
        ui->comboSortingOrder->setCurrentIndex(test.toInt());
    }

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/grouping"_L1).output;
    qDebug() << "grouping is: " << test;

    if ( test.contains("does not exist"_L1)) {
        ui->comboWindowGrouping->setCurrentIndex(0);
    } else {
        ui->comboWindowGrouping->setCurrentIndex(test.toInt());
    }


    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/middle-click"_L1).output;
    qDebug() << "middle-click is: " << test;

    if ( test.contains("does not exist"_L1)) {
        ui->comboMiddleClickAction->setCurrentIndex(0);
    } else {
        ui->comboMiddleClickAction->setCurrentIndex(test.toInt());
    }

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/switch-workspace-on-unminimize"_L1).output;
    qDebug() << "restore minimize is: " << test;

    ui->checkRestoreMinWindows->setChecked(test == "true"_L1 && !test.contains("does not exist"_L1));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-wireframes"_L1).output;
    qDebug() << "draw window frames is: " << test;

    ui->checkDrawFrames->setChecked(test == "true"_L1 && !test.contains("does not exist"_L1));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/window-scrolling"_L1).output;
    qDebug() << "window scrolling is: " << test;
    ui->checkSwitchWindowsMouseWheel->setChecked(test.contains("does not exist"_L1) || test == "true"_L1);

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/include-all-workspaces"_L1).output;
    qDebug() << "include-all-workspaces is: " << test;

    ui->checkWindowsAllWorkspaces->setChecked(test == "true"_L1 && !test.contains("does not exist"_L1));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-only-minimized"_L1).output;
    qDebug() << "show-only-minimized is: " << test;

    ui->checkOnlyMinWindows->setChecked(test == "true"_L1 && !test.contains("does not exist"_L1));

    test = runCmd("LANG=C xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/include-all-monitors"_L1).output;
    qDebug() << "include-all-monitors is: " << test;

    ui->checkWindowsAllMonitors->setChecked(test.contains("does not exist"_L1) || test == "true"_L1);

    connect(ui->checkButtonLabels, &QCheckBox::toggled, this, &window_buttons::checkButtonLabels_toggled);
    connect(ui->checkShowFlatButtons, &QCheckBox::toggled, this, &window_buttons::checkShowFlatButtons_toggled);
    connect(ui->checkShowHandle, &QCheckBox::toggled, this, &window_buttons::checkShowHandle_toggled);
    connect(ui->comboSortingOrder, &QComboBox::currentIndexChanged, this, &window_buttons::comboSortingOrder_currentIndexChanged);
    connect(ui->comboWindowGrouping, &QComboBox::currentIndexChanged, this, &window_buttons::comboWindowGrouping_currentIndexChanged);
    connect(ui->comboMiddleClickAction, &QComboBox::currentIndexChanged, this, &window_buttons::comboMiddleClickAction_currentIndexChanged);
    connect(ui->checkRestoreMinWindows, &QCheckBox::toggled, this, &window_buttons::checkRestoreMinWindows_toggled);
    connect(ui->checkDrawFrames, &QCheckBox::toggled, this, &::window_buttons::checkDrawFrames_toggled);
    connect(ui->checkSwitchWindowsMouseWheel, &QCheckBox::toggled, this, &window_buttons::checkSwitchWindowsMouseWheel_toggled);
    connect(ui->checkWindowsAllWorkspaces, &QCheckBox::toggled, this, &window_buttons::checkWindowsAllWorkspaces_toggled);
    connect(ui->checkOnlyMinWindows, &QCheckBox::toggled, this, &window_buttons::checkOnlyMinWindows_toggled);
    connect(ui->checkWindowsAllMonitors, &QCheckBox::toggled, this, &window_buttons::checkWindowsAllMonitors_toggled);
}

void window_buttons::checkButtonLabels_toggled(bool checked)
{
    QString param = checked ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-labels -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::checkShowFlatButtons_toggled(bool checked)
{
    QString param = checked ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/flat-buttons -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}


void window_buttons::checkShowHandle_toggled(bool checked)
{
    QString param = checked ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-handle -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::comboSortingOrder_currentIndexChanged(int index) const
{
    QString param = QString::number(index);
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/sort-order -t int"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}


void window_buttons::comboWindowGrouping_currentIndexChanged(int index) const
{
    QString param = QString::number(index);
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/grouping -t int"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::comboMiddleClickAction_currentIndexChanged(int index) const
{
    QString param = QString::number(index);
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/middle-click -t int"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::checkRestoreMinWindows_toggled(bool checked)
{
    QString param = checked ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/switch-workspace-on-unminimize -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::checkDrawFrames_toggled(bool checked)
{
    QString param = checked ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-wireframes -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::checkSwitchWindowsMouseWheel_toggled(bool checked)
{
    QString param = checked ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/window-scrolling -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::checkWindowsAllWorkspaces_toggled(bool checked)
{
    QString param = checked ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/include-all-workspaces boolean -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::checkOnlyMinWindows_toggled(bool checked)
{
    QString param = checked ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/show-only-minimized boolean -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}

void window_buttons::checkWindowsAllMonitors_toggled(bool checked)
{
    QString param = checked ? u"true"_s : u"false"_s;
    QString cmd = "xfconf-query -c xfce4-panel -p /plugins/"_L1 + plugintasklist + "/include-all-monitors -t bool"_L1 + " -s "_L1 + param  + " --create"_L1;
    system(cmd.toUtf8());
}
