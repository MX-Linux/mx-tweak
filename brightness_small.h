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
    void saveBrightness();
    void setupDisplay();
    bool brightnessflag = false;
    static void launchfulldisplaydialog();
    QString g1;
    QString g2;
    QString g3;
    bool expand;

private slots:
    void changeEvent(QEvent *event);
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void keyPressEvent(QKeyEvent *event);
    void messageClicked();
    void on_buttonSave_clicked();
    void on_comboBoxDisplay_currentIndexChanged(int index);
    void on_horizontalSliderBrightness_valueChanged(int value);
    void on_horizsliderhardwarebacklight_actionTriggered(int action);
    void on_toolButtonExpandBacklight_clicked();
    void setPosition();

private:
    Ui::brightness_small *ui;
    QSystemTrayIcon *trayicon;
    QAction *quitAction;
    QAction *full;
    QMenu *menu;
};

#endif // BRIGHTNESS_SMALL_H
