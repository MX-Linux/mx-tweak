#include "brightness_small.h"
#include "ui_brightness_small.h"
#include "QDebug"
#include "QDir"
#include "QSystemTrayIcon"
#include "QMenu"
#include "QDialog"
#include "QKeyEvent"

brightness_small::brightness_small(QWidget *parent, QStringList args) :
    QMainWindow(parent),
    ui(new Ui::brightness_small)
{
    //check to see if running, if so, exit quick
    QString check = runCmd("ps -aux |grep -E 'mx-tweak.*--tray'|grep -v grep|wc -l").output;

    if ( check.toInt() >= 2) {
        qDebug() << "check is " << check;
        exit(1);
    }
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint); // for the close, min and max buttons
    QIcon save;
    save = QIcon::fromTheme("gtk-save");
    ui->buttonSave->setIcon(save);
    setupDisplay();
    QIcon icon;
    icon = QIcon::fromTheme("display-brightness");
    setWindowIcon(icon);
    setWindowTitle(tr("MX-Tweak"));
    expand = false;
    QString home_path = QDir::homePath();
    QString config_file_path = home_path + "/.config/MX-Linux/MX-Tweak/expand";
    if (args.contains("--full") || QFileInfo::exists(config_file_path)) {
        expand = true;
    }
    if (expand){
        ui->label_xbacklight->show();
        ui->horizsliderhardwarebacklight->show();
        ui->backlight_label->show();
    } else {
        ui->label_xbacklight->hide();
        ui->horizsliderhardwarebacklight->hide();
        ui->backlight_label->hide();
    }
    trayicon = new QSystemTrayIcon;
    trayicon->setIcon(icon);
    trayicon->show();


    this->adjustSize();

    menu = new QMenu(this);

    if (QFileInfo::exists("/usr/bin/mx-tweak")) {
            full = new QAction(QIcon::fromTheme("video-display"), tr("Display"), this);
            connect(full, &QAction::triggered, this, &brightness_small::launchfulldisplaydialog);
             menu->addAction(full);
             menu->addSeparator();
        }
        quitAction = new QAction(QIcon::fromTheme("gtk-quit"), tr("&Quit"), this);
        connect(quitAction, &QAction::triggered, qApp, &QGuiApplication::quit);
        menu->addAction(quitAction);

    connect(trayicon, &QSystemTrayIcon::messageClicked, this, &brightness_small::messageClicked);
    connect(trayicon, &QSystemTrayIcon::activated, this, &brightness_small::iconActivated);

    trayicon->setContextMenu(menu);
}

void brightness_small::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        setupBrightness();
        setupGamma();
        setupbacklight();
        this->move(QCursor::pos());
        this->adjustSize();
        this->show();
        break;
    default:
        ;
    }
}


// Util function for getting bash command output and error code
Result3 brightness_small::runCmd(QString cmd)
{
    QEventLoop loop;
    proc = new QProcess(this);
    proc->setReadChannelMode(QProcess::MergedChannels);
    connect(proc, SIGNAL(finished(int)), &loop, SLOT(quit()));
    proc->start("/bin/bash", QStringList() << "-c" << cmd);
    loop.exec();
    disconnect(proc, 0, 0, 0);
    Result3 result = {proc->exitCode(), proc->readAll().trimmed()};
    delete proc;
    return result;
}

void brightness_small::messageClicked()
{
    this->show();
}

brightness_small::~brightness_small()
{
    delete ui;
}

void brightness_small::setmissingxfconfvariables(QString activeprofile, QString resolution)
{
    //set resolution, set active, set scales, set display name

    //set display name
    runCmd("xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() + " -t string -s " + ui->comboBoxDisplay->currentText() + " --create");

    //set resolution
    runCmd("xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() + "/Resolution -t string -s " + resolution.simplified() + " --create");

    //set active profile
    runCmd("xfconf-query --channel displays -p /" + activeprofile + "/" + ui->comboBoxDisplay->currentText() + "/Active -t bool -s true --create");
}

void brightness_small::setupbacklight()
{
    //check for backlights
    QString test = runCmd("ls /sys/class/backlight").output;
    if ( ! test.isEmpty()) {
        //get backlight value for currently
        QString backlight=runCmd("sudo /usr/lib/mx-tweak/backlight-brightness -g").output;
        int backlight_slider_value = backlight.toInt();
        ui->horizsliderhardwarebacklight->setValue(backlight_slider_value);
        ui->horizsliderhardwarebacklight->setToolTip(backlight);
        ui->backlight_label->setText(backlight);
        qDebug() << "backlight string is " << backlight;
        qDebug() << " backlight_slider_value is " << backlight_slider_value;
    } else {
        ui->toolButtonExpandBacklight->hide();
        ui->horizsliderhardwarebacklight->hide();
        ui->backlight_label->hide();
        ui->label_xbacklight->hide();
    }
}

void brightness_small::setbacklight()
{
    QString backlight = QString::number(ui->horizsliderhardwarebacklight->value());
    QString cmd = "sudo /usr/lib/mx-tweak/backlight-brightness -s " + backlight;
    ui->backlight_label->setText(backlight);
    system(cmd.toUtf8());
}

void brightness_small::setupBrightness()
{
    //get brightness value for currently shown display
    QString brightness=runCmd("LANG=C xrandr --verbose | awk '/" + ui->comboBoxDisplay->currentText() +"/{flag=1;next}/CONNECTOR_ID/{flag=0}flag'|grep Brightness|cut -d' ' -f2").output;
    int brightness_slider_value = brightness.toFloat() * 100;
    ui->horizontalSliderBrightness->setValue(brightness_slider_value);
    qDebug() << "brightness string is " << brightness;
    qDebug() << " brightness_slider_value is " << brightness_slider_value;
    ui->horizontalSliderBrightness->setToolTip(QString::number(ui->horizontalSliderBrightness->value()));
    ui->label_brightness_slider->setText(QString::number(ui->horizontalSliderBrightness->value()));
}

void brightness_small::setupGamma()
{
    QString gamma = runCmd("/usr/lib/mx-tweak/mx-tweak-lib-randr.sh " + ui->comboBoxDisplay->currentText() + " gamma").output;
    gamma=gamma.simplified();
    gamma = gamma.section(":",1,3).simplified();
    double gamma1 = 1.0 / gamma.section(":",0,0).toDouble();
    double gamma2 = 1.0 / gamma.section(":",1,1).toDouble();
    double gamma3 = 1.0 / gamma.section(":",2,2).toDouble();
    g1 = QString::number(gamma1,'G', 3);
    g2 = QString::number(gamma2,'G', 3);
    g3 = QString::number(gamma3,'G', 3);
    qDebug() << "gamma is " << g1 << " " << g2 << " " << g3;
}

void brightness_small::on_horizontalSliderBrightness_valueChanged(int value)
{
    QString slider_value = QString::number(ui->horizontalSliderBrightness->value());
    ui->horizontalSliderBrightness->setToolTip(slider_value);
    ui->label_brightness_slider->setText(slider_value);
    if ( brightnessflag ) {
        //setupBrightness();
        //setupGamma();
        //setupbacklight();
        setBrightness();
    }
}

void brightness_small::setBrightness()
{
    QString cmd;
    double num = ui->horizontalSliderBrightness->value() / 100.0;
    qDebug() << "num is :" << num;
    QString brightness = QString::number(num, 'G', 5);
    qDebug() << "changed brightness is :" << brightness;
    cmd = "xrandr --output " + ui->comboBoxDisplay->currentText() + " --brightness " + brightness + " --gamma " + g1 + ":" + g2 + ":" +g3;
    system(cmd.toUtf8());
}

void brightness_small::saveBrightness()
{
    //save cmd used in user's home file under .config
    //make directory when its not present
    double num = ui->horizontalSliderBrightness->value() / 100.0;
    qDebug() << "num is :" << num;
    QString brightness = QString::number(num, 'G', 5);
    QString home_path = QDir::homePath();
    QString config_file_path = home_path + "/.config/MX-Linux/MX-Tweak/brightness";
    if ( ! QFileInfo::exists(config_file_path)) {
        runCmd("mkdir -p " + config_file_path);
    }
    //save config in file named after the display
    runCmd("echo 'xrandr --output " + ui->comboBoxDisplay->currentText() + " --brightness " + brightness + " --gamma " + g1 + ":" + g2 + ":" + g3 + "'>" + config_file_path + "/" + ui->comboBoxDisplay->currentText());

}

void brightness_small::on_comboBoxDisplay_currentIndexChanged(int index)
{
    if (brightnessflag) {
        setupBrightness();
        setupGamma();
    }
}

void brightness_small::setupDisplay()
{
    //populate combobox
    QString displaydata = runCmd("LANG=C xrandr |grep -w connected | cut -d' ' -f1").output;
    QStringList displaylist = displaydata.split("\n");
    ui->comboBoxDisplay->clear();
    ui->comboBoxDisplay->addItems(displaylist);
    brightnessflag = true;
}

void brightness_small::on_horizsliderhardwarebacklight_actionTriggered(int action)
{
    setbacklight();
}

// implement change event that closes app when window loses focus
void brightness_small::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        if(this->isActiveWindow()) {
            qDebug() << "focusinEvent";
        } else {
            qDebug() << "focusOutEvent";
            this->hide();
        }
    }
}

// process keystrokes
void brightness_small::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        this->hide();
    }
}

void brightness_small::launchfulldisplaydialog()
{
    QString cmd = "mx-tweak --display";
    system(cmd.toUtf8());
}

void brightness_small::on_buttonSave_clicked()
{
    saveBrightness();
}

void brightness_small::on_toolButtonExpandBacklight_clicked()
{
    QString home_path = QDir::homePath();
    QString config_file_path = home_path + "/.config/MX-Linux/MX-Tweak/expand";
    //expand toggle
    if ( ! expand ){
        expand = true;
    } else {
        expand = false;
    }

    if (! expand){

        QString cmd = "rm " + config_file_path;
        system(cmd.toUtf8());
        ui->label_xbacklight->hide();
        ui->horizsliderhardwarebacklight->hide();
        ui->backlight_label->hide();
    } else {
        QString cmd = "touch " + config_file_path;
        system(cmd.toUtf8());
        ui->label_xbacklight->show();
        ui->horizsliderhardwarebacklight->show();
        ui->backlight_label->show();
    }
}
