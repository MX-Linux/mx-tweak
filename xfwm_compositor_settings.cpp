#include "cmd.h"
#include "ui_xfwm_compositor_settings.h"
#include "xfwm_compositor_settings.h"

using namespace Qt::Literals::StringLiterals;

xfwm_compositor_settings::xfwm_compositor_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::xfwm_compositor_settings)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    setup();
}

xfwm_compositor_settings::~xfwm_compositor_settings()
{
    delete ui;
}

//setup some initial values
void xfwm_compositor_settings::setup()
{
    this->setWindowTitle(tr("Xfwm Compositor Settings"));

    //initial settings

    QString value;
    value = runCmd(u"xfconf-query -c xfwm4 -p /general/unredirect_overlays"_s).output;
    ui->checkBoxRedirect->setChecked(value == "true"_L1);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/sync_to_vblank"_s).output;
    ui->checkBoxVsync->setChecked(value == "true"_L1);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/cycle_preview"_s).output;
    ui->checkBoxPreview->setChecked(value == "true"_L1);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/show_popup_shadow"_s).output;
    ui->checkBoxPopupShadows->setChecked(value == "true"_L1);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/show_dock_shadow"_s).output;
    ui->checkBoxDockShadows->setChecked(value == "true"_L1);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/show_frame_shadow"_s).output;
    ui->checkBoxFrameShadows->setChecked(value == "true"_L1);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/frame_opacity"_s).output;
    ui->horizontalSliderWIndowDecorations->setValue(value.toInt());
    ui->horizontalSliderWIndowDecorations->setToolTip(value);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/inactive_opacity"_s).output;
    ui->horizontalSliderInactiveWindows->setValue(value.toInt());
    ui->horizontalSliderInactiveWindows->setToolTip(value);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/move_opacity"_s).output;
    ui->horizontalSliderWindowsMove->setValue(value.toInt());
    ui->horizontalSliderWindowsMove->setToolTip(value);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/resize_opacity"_s).output;
    ui->horizontalSliderWindowsResize->setValue(value.toInt());
    ui->horizontalSliderWindowsResize->setToolTip(value);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/popup_opacity"_s).output;
    ui->horizontalSliderPopup->setValue(value.toInt());
    ui->horizontalSliderPopup->setToolTip(value);
}

void xfwm_compositor_settings::on_checkBoxRedirect_clicked()
{
    QString cmd;
    if (ui->checkBoxRedirect->isChecked()) {
        cmd = u"xfconf-query -c xfwm4 -p /general/unredirect_overlays -s true"_s;

    } else {
        cmd = u"xfconf-query -c xfwm4 -p /general/unredirect_overlays -s false"_s;
    }
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_checkBoxVsync_clicked()
{
    QString cmd;
    if (ui->checkBoxVsync->isChecked()) {
        cmd = u"xfconf-query -c xfwm4 -p /general/sync_to_vblank -s true"_s;
    } else {
        cmd = u"xfconf-query -c xfwm4 -p /general/sync_to_vblank -s false"_s;
    }
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_checkBoxPreview_clicked()
{
    QString cmd;
    if (ui->checkBoxPreview->isChecked()) {
        cmd = u"xfconf-query -c xfwm4 -p /general/cycle_preview -s true"_s;
    } else {
        cmd = u"xfconf-query -c xfwm4 -p /general/cycle_preview -s false"_s;
    }
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_checkBoxPopupShadows_clicked()
{
    QString cmd;
    if (ui->checkBoxPopupShadows->isChecked()) {
        cmd = u"xfconf-query -c xfwm4 -p /general/show_popup_shadow -s true"_s;
    } else {
        cmd = u"xfconf-query -c xfwm4 -p /general/show_popup_shadow -s false"_s;
    }
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_checkBoxDockShadows_clicked()
{
    QString cmd;
    if (ui->checkBoxDockShadows->isChecked()) {
        cmd = u"xfconf-query -c xfwm4 -p /general/show_dock_shadow -s true"_s;
    } else {
        cmd = u"xfconf-query -c xfwm4 -p /general/show_dock_shadow -s false"_s;
    }
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_checkBoxFrameShadows_clicked()
{
    QString cmd;
    if (ui->checkBoxFrameShadows->isChecked()) {
        cmd = u"xfconf-query -c xfwm4 -p /general/show_frame_shadow -s true"_s;
    } else {
        cmd = u"xfconf-query -c xfwm4 -p /general/show_frame_shadow -s false"_s;
    }
}

void xfwm_compositor_settings::on_horizontalSliderWIndowDecorations_valueChanged(int value)
{
    QString param = QString::number(value);
    QString cmd;
    ui->horizontalSliderWIndowDecorations->setToolTip(param);

    cmd = "xfconf-query -c xfwm4 -p /general/frame_opacity -s "_L1 + param;
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_horizontalSliderInactiveWindows_valueChanged(int value)
{
    QString param = QString::number(value);
    QString cmd;
    ui->horizontalSliderInactiveWindows->setToolTip(param);
    cmd = "xfconf-query -c xfwm4 -p /general/inactive_opacity -s "_L1 + param;
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_horizontalSliderWindowsMove_valueChanged(int value)
{
    QString param = QString::number(value);
    QString cmd;
    ui->horizontalSliderWindowsMove->setToolTip(param);
    cmd = "xfconf-query -c xfwm4 -p /general/move_opacity -s "_L1 + param;
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_horizontalSliderWindowsResize_valueChanged(int value)
{
    QString param = QString::number(value);
    QString cmd;
    ui->horizontalSliderWindowsResize->setToolTip(param);
    cmd = "xfconf-query -c xfwm4 -p /general/resize_opacity -s "_L1 + param;
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_horizontalSliderPopup_valueChanged(int value)
{
    QString param = QString::number(value);
    QString cmd;
    ui->horizontalSliderWIndowDecorations->setToolTip(param);
    cmd = "xfconf-query -c xfwm4 -p /general/popup_opacity -s "_L1 + param;
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_ButtonCloseXfwmSettings_clicked()
{
    close();
}
