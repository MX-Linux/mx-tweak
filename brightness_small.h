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
    explicit brightness_small(QWidget *parent = 0, const QStringList &args = QStringList()) noexcept;
    ~brightness_small() noexcept;
    void setmissingxfconfvariables(const QString &activeprofile, const QString &resolution) noexcept;
    void setupbacklight() noexcept;
    void setbacklight() noexcept;
    void setupBrightness() noexcept;
    void setupGamma() noexcept;
    void setBrightness() noexcept;
    void setupDisplay() noexcept;
    bool brightnessflag = false;
    static void launchfulldisplaydialog() noexcept;
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
    void setPosition() noexcept;

    void changeEvent(QEvent *event) noexcept;
    void keyPressEvent(QKeyEvent *event) noexcept;
    void iconActivated(QSystemTrayIcon::ActivationReason reason) noexcept;
    void pushSave_clicked() noexcept;
    void comboDisplay_currentIndexChanged(int index) noexcept;
    void sliderBrightness_valueChanged(int value) noexcept;
    void pushExpandBacklight_clicked() noexcept;
};

#endif // BRIGHTNESS_SMALL_H
