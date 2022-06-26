#ifndef WINDOW_BUTTONS_H
#define WINDOW_BUTTONS_H

#include <QDialog>
#include "defaultlook.h"

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

private slots:
    void on_checkBoxbuttonlabels_toggled(bool checked);
    void on_checkBoxdrawframes_toggled(bool checked);
    void on_checkBoxonlyminwindows_toggled(bool checked);
    void on_checkBoxrestoreminwindows_toggled(bool checked);
    void on_checkBoxshowflatbuttons_toggled(bool checked);
    void on_checkBoxshowhandle_toggled(bool checked);
    void on_checkBoxswitchwindowsmousewheel_toggled(bool checked);
    void on_checkBoxwindowsallmonitors_toggled(bool checked);
    void on_checkBoxwindowsallworkspaces_toggled(bool checked);
    void on_comboBoxmiddleclickaction_currentIndexChanged(int index) const;
    void on_comboBoxsortingorder_currentIndexChanged(int index) const;
    void on_comboBoxwindowgrouping_currentIndexChanged(int index) const;
    void on_pushButton_clicked();

private:
    Ui::window_buttons *ui;
};

#endif // WINDOW_BUTTONS_H
