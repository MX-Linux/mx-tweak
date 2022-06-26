#ifndef XFWM_COMPOSITOR_SETTINGS_H
#define XFWM_COMPOSITOR_SETTINGS_H

#include <QDialog>
#include "defaultlook.h"

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

private slots:
    void on_ButtonCloseXfwmSettings_clicked();
    void on_checkBoxDockShadows_clicked();
    void on_checkBoxFrameShadows_clicked();
    void on_checkBoxPopupShadows_clicked();
    void on_checkBoxPreview_clicked();
    void on_checkBoxRedirect_clicked();
    void on_checkBoxVsync_clicked();
    void on_horizontalSliderInactiveWindows_valueChanged(int value);
    void on_horizontalSliderPopup_valueChanged(int value);
    void on_horizontalSliderWIndowDecorations_valueChanged(int value);
    void on_horizontalSliderWindowsMove_valueChanged(int value);
    void on_horizontalSliderWindowsResize_valueChanged(int value);

private:
    Ui::xfwm_compositor_settings *ui;
};

#endif // XFWM_COMPOSITOR_SETTINGS_H
