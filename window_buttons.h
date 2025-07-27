#ifndef WINDOW_BUTTONS_H
#define WINDOW_BUTTONS_H

#include <QDialog>

namespace Ui {
class window_buttons;
}

class window_buttons : public QDialog
{
    Q_OBJECT

public:
    explicit window_buttons(QWidget *parent = 0);
    ~window_buttons();
    void setup();
    QString plugintasklist;

private:
    Ui::window_buttons *ui;

    void checkButtonLabels_toggled(bool checked);
    void checkDrawFrames_toggled(bool checked);
    void checkOnlyMinWindows_toggled(bool checked);
    void checkRestoreMinWindows_toggled(bool checked);
    void checkShowFlatButtons_toggled(bool checked);
    void checkShowHandle_toggled(bool checked);
    void checkSwitchWindowsMouseWheel_toggled(bool checked);
    void checkWindowsAllMonitors_toggled(bool checked);
    void checkWindowsAllWorkspaces_toggled(bool checked);
    void comboMiddleClickAction_currentIndexChanged(int index) const;
    void comboSortingOrder_currentIndexChanged(int index) const;
    void comboWindowGrouping_currentIndexChanged(int index) const;
};

#endif // WINDOW_BUTTONS_H
