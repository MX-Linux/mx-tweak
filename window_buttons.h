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
    explicit window_buttons(QWidget *parent = 0) noexcept;
    ~window_buttons() noexcept;
    void setup() noexcept;
    QString plugintasklist;

private:
    Ui::window_buttons *ui;

    void changePluginBool(const QLatin1StringView setting, bool value) const noexcept;
    void changePluginInt(const QLatin1StringView setting, int value) const noexcept;

    void checkButtonLabels_toggled(bool checked) noexcept;
    void checkDrawFrames_toggled(bool checked) noexcept;
    void checkOnlyMinWindows_toggled(bool checked) noexcept;
    void checkRestoreMinWindows_toggled(bool checked) noexcept;
    void checkShowFlatButtons_toggled(bool checked) noexcept;
    void checkShowHandle_toggled(bool checked) noexcept;
    void checkSwitchWindowsMouseWheel_toggled(bool checked) noexcept;
    void checkWindowsAllMonitors_toggled(bool checked) noexcept;
    void checkWindowsAllWorkspaces_toggled(bool checked) noexcept;
    void comboMiddleClickAction_currentIndexChanged(int index) const noexcept;
    void comboSortingOrder_currentIndexChanged(int index) const noexcept;
    void comboWindowGrouping_currentIndexChanged(int index) const noexcept;
};

#endif // WINDOW_BUTTONS_H
