#include "cmd.h"
#include "defaultlook.h"
#include "ui_xfwm_compositor_settings.h"
#include "xfwm_compositor_settings.h"

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
    value = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/unredirect_overlays")).output;
    ui->checkBoxRedirect->setChecked(value == QLatin1String("true"));

    value = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/sync_to_vblank")).output;
    ui->checkBoxVsync->setChecked(value == QLatin1String("true"));

    value = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/cycle_preview")).output;
    ui->checkBoxPreview->setChecked(value == QLatin1String("true"));

    value = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/show_popup_shadow")).output;
    ui->checkBoxPopupShadows->setChecked(value == QLatin1String("true"));

    value = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/show_dock_shadow")).output;
    ui->checkBoxDockShadows->setChecked(value == QLatin1String("true"));

    value = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/show_frame_shadow")).output;
    ui->checkBoxFrameShadows->setChecked(value == QLatin1String("true"));

    value = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/frame_opacity")).output;
    ui->horizontalSliderWIndowDecorations->setValue(value.toInt());
    ui->horizontalSliderWIndowDecorations->setToolTip(value);

    value = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/inactive_opacity")).output;
    ui->horizontalSliderInactiveWindows->setValue(value.toInt());
    ui->horizontalSliderInactiveWindows->setToolTip(value);

    value = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/move_opacity")).output;
    ui->horizontalSliderWindowsMove->setValue(value.toInt());
    ui->horizontalSliderWindowsMove->setToolTip(value);

    value = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/resize_opacity")).output;
    ui->horizontalSliderWindowsResize->setValue(value.toInt());
    ui->horizontalSliderWindowsResize->setToolTip(value);

    value = runCmd(QStringLiteral("xfconf-query -c xfwm4 -p /general/popup_opacity")).output;
    ui->horizontalSliderPopup->setValue(value.toInt());
    ui->horizontalSliderPopup->setToolTip(value);
}

void xfwm_compositor_settings::on_checkBoxRedirect_clicked()
{
    QString cmd;
    if (ui->checkBoxRedirect->isChecked()) {
        cmd = QStringLiteral("xfconf-query -c xfwm4 -p /general/unredirect_overlays -s true");

    } else {
        cmd = QStringLiteral("xfconf-query -c xfwm4 -p /general/unredirect_overlays -s false");
    }
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_checkBoxVsync_clicked()
{
    QString cmd;
    if (ui->checkBoxVsync->isChecked()) {
        cmd = QStringLiteral("xfconf-query -c xfwm4 -p /general/sync_to_vblank -s true");
    } else {
        cmd = QStringLiteral("xfconf-query -c xfwm4 -p /general/sync_to_vblank -s false");
    }
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_checkBoxPreview_clicked()
{
    QString cmd;
    if (ui->checkBoxPreview->isChecked()) {
        cmd = QStringLiteral("xfconf-query -c xfwm4 -p /general/cycle_preview -s true");
    } else {
        cmd = QStringLiteral("xfconf-query -c xfwm4 -p /general/cycle_preview -s false");
    }
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_checkBoxPopupShadows_clicked()
{
    QString cmd;
    if (ui->checkBoxPopupShadows->isChecked()) {
        cmd = QStringLiteral("xfconf-query -c xfwm4 -p /general/show_popup_shadow -s true");
    } else {
        cmd = QStringLiteral("xfconf-query -c xfwm4 -p /general/show_popup_shadow -s false");
    }
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_checkBoxDockShadows_clicked()
{
    QString cmd;
    if (ui->checkBoxDockShadows->isChecked()) {
        cmd = QStringLiteral("xfconf-query -c xfwm4 -p /general/show_dock_shadow -s true");
    } else {
        cmd = QStringLiteral("xfconf-query -c xfwm4 -p /general/show_dock_shadow -s false");
    }
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_checkBoxFrameShadows_clicked()
{
    QString cmd;
    if (ui->checkBoxFrameShadows->isChecked()) {
        cmd = QStringLiteral("xfconf-query -c xfwm4 -p /general/show_frame_shadow -s true");
    } else {
        cmd = QStringLiteral("xfconf-query -c xfwm4 -p /general/show_frame_shadow -s false");
    }
}

void xfwm_compositor_settings::on_horizontalSliderWIndowDecorations_valueChanged(int value)
{
    QString param = QString::number(value);
    QString cmd;
    ui->horizontalSliderWIndowDecorations->setToolTip(param);

    cmd = "xfconf-query -c xfwm4 -p /general/frame_opacity -s " + param;
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_horizontalSliderInactiveWindows_valueChanged(int value)
{
    QString param = QString::number(value);
    QString cmd;
    ui->horizontalSliderInactiveWindows->setToolTip(param);
    cmd = "xfconf-query -c xfwm4 -p /general/inactive_opacity -s " + param;
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_horizontalSliderWindowsMove_valueChanged(int value)
{
    QString param = QString::number(value);
    QString cmd;
    ui->horizontalSliderWindowsMove->setToolTip(param);
    cmd = "xfconf-query -c xfwm4 -p /general/move_opacity -s " + param;
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_horizontalSliderWindowsResize_valueChanged(int value)
{
    QString param = QString::number(value);
    QString cmd;
    ui->horizontalSliderWindowsResize->setToolTip(param);
    cmd = "xfconf-query -c xfwm4 -p /general/resize_opacity -s " + param;
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_horizontalSliderPopup_valueChanged(int value)
{
    QString param = QString::number(value);
    QString cmd;
    ui->horizontalSliderWIndowDecorations->setToolTip(param);
    cmd = "xfconf-query -c xfwm4 -p /general/popup_opacity -s " + param;
    system(cmd.toUtf8());
}

void xfwm_compositor_settings::on_ButtonCloseXfwmSettings_clicked()
{
    close();
}
