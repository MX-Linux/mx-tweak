#ifndef XFWM_COMPOSITOR_SETTINGS_H
#define XFWM_COMPOSITOR_SETTINGS_H
#include "defaultlook.h"
#include <QDialog>

namespace Ui {
class xfwm_compositor_settings;
}

struct result2 {
    int exitCode;
    QString output;
};

class xfwm_compositor_settings : public QDialog
{
    Q_OBJECT

protected:
    QProcess *proc;
    QTimer *timer;

public:
    explicit xfwm_compositor_settings(QWidget *parent = 0);
    ~xfwm_compositor_settings();

    void setup();
    result2 runCmd(QString cmd);

private slots:
    void on_checkBoxRedirect_clicked();

    void on_checkBoxVsync_clicked();

    void on_checkBoxPreview_clicked();

    void on_checkBoxPopupShadows_clicked();

    void on_checkBoxDockShadows_clicked();

    void on_checkBoxFrameShadows_clicked();

    void on_horizontalSliderWIndowDecorations_valueChanged(int value);

    void on_horizontalSliderInactiveWindows_valueChanged(int value);

    void on_horizontalSliderWindowsMove_valueChanged(int value);

    void on_horizontalSliderWindowsResize_valueChanged(int value);

    void on_horizontalSliderPopup_valueChanged(int value);

    void on_ButtonCloseXfwmSettings_clicked();

private:
    Ui::xfwm_compositor_settings *ui;
};

#endif // XFWM_COMPOSITOR_SETTINGS_H
