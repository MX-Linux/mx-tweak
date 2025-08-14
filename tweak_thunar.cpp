#include <QFileInfo>
#include "ui_defaultlook.h"
#include "cmd.h"
#include "tweak_thunar.h"

using namespace Qt::Literals::StringLiterals;

TweakThunar::TweakThunar(Ui::defaultlook *ui, bool fluxbox, QObject *parent) noexcept
    : QObject{parent}, ui{ui}, fluxbox{fluxbox}
{
    setup();
    if (fluxbox) {
        ui->layoutFluxboxTab->replaceWidget(ui->widgetFluxboxThunar, ui->groupThunar);
        connect(ui->pushFluxboxApply, &QCheckBox::clicked, this, &TweakThunar::slotApplyClicked);
    } else {
        connect(ui->pushXfceApply, &QPushButton::clicked, this, &TweakThunar::slotApplyClicked);
    }
    connect(ui->checkThunarSingleClick, &QCheckBox::clicked, this, &TweakThunar::slotSettingChanged);
    connect(ui->checkThunarResetCustomActions, &QCheckBox::clicked, this, &TweakThunar::slotSettingChanged);
    connect(ui->checkThunarSplitView, &QCheckBox::clicked, this, &TweakThunar::slotSettingChanged);
    connect(ui->checkThunarSplitViewHorizontal, &QCheckBox::clicked, this, &TweakThunar::slotSettingChanged);
    setup();
}

bool TweakThunar::check() noexcept
{
    return QFileInfo::exists(u"/usr/bin/thunar"_s);
}
void TweakThunar::setup() noexcept
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

void TweakThunar::slotSettingChanged() noexcept
{
    if (fluxbox) {
        ui->pushFluxboxApply->setEnabled(true);
    } else {
        ui->pushXfceApply->setEnabled(true);
    }
}
void TweakThunar::slotApplyClicked() noexcept
{
    // Single-click
    runProc(u"xfconf-query"_s, {u"-c"_s, u"thunar"_s, u"-p"_s,
        u"/misc-single-click"_s, u"-t"_s, u"bool"_s, u"-s"_s,
        (ui->checkThunarSingleClick->isChecked() ? u"true"_s : u"false"_s), u"--create"_s});
    // Reset right-click custom actions
    if (ui->checkThunarResetCustomActions->isChecked()) {
        runCmd(u"cp /home/$USER/.config/Thunar/uca.xml /home/$USER/.config/Thunar/uca.xml.$(date +%Y%m%H%M%S)"_s);
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

    setup();
}
