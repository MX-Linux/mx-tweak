#ifndef WINDOW_BUTTONS_H
#define WINDOW_BUTTONS_H
#include "defaultlook.h"
#include <QDialog>

namespace Ui {
class window_buttons;
}

struct result3 {
    int exitCode;
    QString output;
};



class window_buttons : public QDialog
{
    Q_OBJECT

protected:
    QProcess *proc{};
    QTimer *timer{};

public:
    explicit window_buttons(QWidget *parent = 0);
    ~window_buttons();
    void setup();
    result3 runCmd(const QString &cmd);
    QString plugintasklist;

private slots:
    void on_pushButton_clicked();

    void on_checkBoxbuttonlabels_toggled(bool checked);

    void on_checkBoxshowflatbuttons_toggled(bool checked);

    void on_checkBoxshowhandle_toggled(bool checked);

    void on_comboBoxsortingorder_currentIndexChanged(int index) const;

    void on_comboBoxwindowgrouping_currentIndexChanged(int index) const;

    void on_comboBoxmiddleclickaction_currentIndexChanged(int index) const;

    void on_checkBoxrestoreminwindows_toggled(bool checked);

    void on_checkBoxdrawframes_toggled(bool checked);

    void on_checkBoxswitchwindowsmousewheel_toggled(bool checked);

    void on_checkBoxwindowsallworkspaces_toggled(bool checked);

    void on_checkBoxonlyminwindows_toggled(bool checked);

    void on_checkBoxwindowsallmonitors_toggled(bool checked);

private:
    Ui::window_buttons *ui;
};

#endif // WINDOW_BUTTONS_H
