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
    explicit xfwm_compositor_settings(QWidget *parent = 0);
    ~xfwm_compositor_settings();

    void setup();

private:
    Ui::xfwm_compositor_settings *ui;

    void checkDockShadows_toggled(bool checked);
    void checkFrameShadows_toggled(bool checked);
    void checkPopupShadows_toggled(bool checked);
    void checkPreview_toggled(bool checked);
    void checkRedirect_toggled(bool checked);
    void checkVsync_toggled(bool checked);
    void sliderInactiveWindows_valueChanged(int value);
    void sliderPopup_valueChanged(int value);
    void sliderWindowDecorations_valueChanged(int value);
    void sliderWindowsMove_valueChanged(int value);
    void sliderWindowsResize_valueChanged(int value);
};

#endif // XFWM_COMPOSITOR_SETTINGS_H
