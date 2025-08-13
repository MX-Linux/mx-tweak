#ifndef XFWM_COMPOSITOR_SETTINGS_H
#define XFWM_COMPOSITOR_SETTINGS_H

#include <QDialog>

namespace Ui {
class xfwm_compositor_settings;
}

class xfwm_compositor_settings : public QDialog
{
    Q_OBJECT

public:
    explicit xfwm_compositor_settings(QWidget *parent = 0) noexcept;
    ~xfwm_compositor_settings() noexcept;

    void setup() noexcept;

private:
    Ui::xfwm_compositor_settings *ui;

    void checkDockShadows_toggled(bool checked) noexcept;
    void checkFrameShadows_toggled(bool checked) noexcept;
    void checkPopupShadows_toggled(bool checked) noexcept;
    void checkPreview_toggled(bool checked) noexcept;
    void checkRedirect_toggled(bool checked) noexcept;
    void checkVsync_toggled(bool checked) noexcept;
    void sliderInactiveWindows_valueChanged(int value) noexcept;
    void sliderPopup_valueChanged(int value) noexcept;
    void sliderWindowDecorations_valueChanged(int value) noexcept;
    void sliderWindowsMove_valueChanged(int value) noexcept;
    void sliderWindowsResize_valueChanged(int value) noexcept;
};

#endif // XFWM_COMPOSITOR_SETTINGS_H
