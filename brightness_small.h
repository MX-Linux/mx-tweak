#ifndef BRIGHTNESS_SMALL_H
#define BRIGHTNESS_SMALL_H

#include <QMainWindow>
#include <QProcess>
#include <QFile>
#include <QApplication>
#include <unistd.h>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QKeyEvent>

namespace Ui {
class brightness_small;
}

class brightness_small : public QMainWindow
{
    Q_OBJECT

public:
    explicit brightness_small(QWidget *parent = 0, const QStringList &args = QStringList());
    ~brightness_small();
    void setmissingxfconfvariables(const QString &activeprofile, const QString &resolution);
    void setupbacklight();
    void setbacklight();
    void setupBrightness();
    void setupGamma();
    void setBrightness();
    void setupDisplay();
    bool brightnessflag = false;
    static void launchfulldisplaydialog();
    QString g1;
    QString g2;
    QString g3;
    bool expand;

private:
    Ui::brightness_small *ui;
    QSystemTrayIcon *trayicon;
    QAction *quitAction;
    QAction *full;
    QMenu *menu;
    void setPosition();

    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void pushSave_clicked();
    void comboDisplay_currentIndexChanged(int index);
    void sliderBrightness_valueChanged(int value);
    void sliderHardwareBacklight_actionTriggered(int action);
    void pushExpandBacklight_clicked();
};

#endif // BRIGHTNESS_SMALL_H
