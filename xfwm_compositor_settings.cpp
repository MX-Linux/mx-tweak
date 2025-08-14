#include "cmd.h"
#include "ui_xfwm_compositor_settings.h"
#include "xfwm_compositor_settings.h"

using namespace Qt::Literals::StringLiterals;

xfwm_compositor_settings::xfwm_compositor_settings(QWidget *parent) noexcept :
    QDialog(parent),
    ui(new Ui::xfwm_compositor_settings)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window); // for the close, min and max buttons
    connect(ui->pushClose, &QPushButton::clicked, this, &xfwm_compositor_settings::close);
    setup();
}

xfwm_compositor_settings::~xfwm_compositor_settings() noexcept
{
    delete ui;
}

//setup some initial values
void xfwm_compositor_settings::setup() noexcept
{
    this->setWindowTitle(tr("Xfwm Compositor Settings"));

    //initial settings

    QString value;
    value = runCmd(u"xfconf-query -c xfwm4 -p /general/unredirect_overlays"_s).output;
    ui->checkRedirect->setChecked(value == "true"_L1);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/sync_to_vblank"_s).output;
    ui->checkVsync->setChecked(value == "true"_L1);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/cycle_preview"_s).output;
    ui->checkPreview->setChecked(value == "true"_L1);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/show_popup_shadow"_s).output;
    ui->checkPopupShadows->setChecked(value == "true"_L1);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/show_dock_shadow"_s).output;
    ui->checkDockShadows->setChecked(value == "true"_L1);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/show_frame_shadow"_s).output;
    ui->checkFrameShadows->setChecked(value == "true"_L1);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/frame_opacity"_s).output;
    ui->sliderWindowDecorations->setValue(value.toInt());
    ui->sliderWindowDecorations->setToolTip(value);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/inactive_opacity"_s).output;
    ui->sliderInactiveWindows->setValue(value.toInt());
    ui->sliderInactiveWindows->setToolTip(value);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/move_opacity"_s).output;
    ui->sliderWindowsMove->setValue(value.toInt());
    ui->sliderWindowsMove->setToolTip(value);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/resize_opacity"_s).output;
    ui->sliderWindowsResize->setValue(value.toInt());
    ui->sliderWindowsResize->setToolTip(value);

    value = runCmd(u"xfconf-query -c xfwm4 -p /general/popup_opacity"_s).output;
    ui->sliderPopup->setValue(value.toInt());
    ui->sliderPopup->setToolTip(value);

    connect(ui->checkRedirect, &QCheckBox::toggled, this, &xfwm_compositor_settings::checkRedirect_toggled);
    connect(ui->checkVsync, &QCheckBox::toggled, this, &xfwm_compositor_settings::checkVsync_toggled);
    connect(ui->checkPreview, &QCheckBox::toggled, this, &xfwm_compositor_settings::checkPreview_toggled);
    connect(ui->checkPopupShadows, &QCheckBox::toggled, this, &xfwm_compositor_settings::checkPopupShadows_toggled);
    connect(ui->checkDockShadows, &QCheckBox::toggled, this, &xfwm_compositor_settings::checkDockShadows_toggled);
    connect(ui->checkFrameShadows, &QCheckBox::toggled, this, &xfwm_compositor_settings::checkFrameShadows_toggled);
    connect(ui->sliderWindowDecorations, &QSlider::valueChanged, this, &xfwm_compositor_settings::sliderWindowDecorations_valueChanged);
    connect(ui->sliderInactiveWindows, &QSlider::valueChanged, this, &xfwm_compositor_settings::sliderInactiveWindows_valueChanged);
    connect(ui->sliderWindowsMove, &QSlider::valueChanged, this, &xfwm_compositor_settings::sliderWindowsMove_valueChanged);
    connect(ui->sliderWindowsResize, &QSlider::valueChanged, this, &xfwm_compositor_settings::sliderWindowsResize_valueChanged);
    connect(ui->sliderPopup, &QSlider::valueChanged, this, &xfwm_compositor_settings::sliderPopup_valueChanged);
}

void xfwm_compositor_settings::checkRedirect_toggled(bool checked) noexcept
{
    runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s,
        u"/general/unredirect_overlays"_s, u"-s"_s, (checked ? u"true"_s : u"false"_s)});
}

void xfwm_compositor_settings::checkVsync_toggled(bool checked) noexcept
{
    runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s,
        u"/general/sync_to_vblank"_s, u"-s"_s, (checked ? u"true"_s : u"false"_s)});
}

void xfwm_compositor_settings::checkPreview_toggled(bool checked) noexcept
{
    runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s,
        u"/general/cycle_preview"_s, u"-s"_s, (checked ? u"true"_s : u"false"_s)});
}

void xfwm_compositor_settings::checkPopupShadows_toggled(bool checked) noexcept
{
    runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s,
        u"/general/show_popup_shadow"_s, u"-s"_s, (checked ? u"true"_s : u"false"_s)});
}

void xfwm_compositor_settings::checkDockShadows_toggled(bool checked) noexcept
{
    runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s,
        u"/general/show_dock_shadow"_s, u"-s"_s, (checked ? u"true"_s : u"false"_s)});
}

void xfwm_compositor_settings::checkFrameShadows_toggled(bool checked) noexcept
{
    runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s,
        u"/general/show_frame_shadow"_s, u"-s"_s, (checked ? u"true"_s : u"false"_s)});
}

void xfwm_compositor_settings::sliderWindowDecorations_valueChanged(int value) noexcept
{
    const QString &param = QString::number(value);
    ui->sliderWindowDecorations->setToolTip(param);
    runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s, u"/general/frame_opacity"_s, u"-s"_s, param});
}

void xfwm_compositor_settings::sliderInactiveWindows_valueChanged(int value) noexcept
{
    const QString &param = QString::number(value);
    ui->sliderInactiveWindows->setToolTip(param);
    runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s, u"/general/inactive_opacity"_s, u"-s"_s, param});
}

void xfwm_compositor_settings::sliderWindowsMove_valueChanged(int value) noexcept
{
    const QString &param = QString::number(value);
    ui->sliderWindowsMove->setToolTip(param);
    runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s, u"/general/move_opacity"_s, u"-s"_s, param});
}

void xfwm_compositor_settings::sliderWindowsResize_valueChanged(int value) noexcept
{
    const QString &param = QString::number(value);
    ui->sliderWindowsResize->setToolTip(param);
    runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s, u"/general/resize_opacity"_s, u"-s"_s, param});
}

void xfwm_compositor_settings::sliderPopup_valueChanged(int value) noexcept
{
    QString param = QString::number(value);
    ui->sliderWindowDecorations->setToolTip(param);
    runProc(u"xfconf-query"_s, {u"-c"_s, u"xfwm4"_s, u"-p"_s, u"/general/popup_opacity"_s, u"-s"_s, param});
}
