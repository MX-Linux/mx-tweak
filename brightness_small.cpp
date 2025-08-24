#include "QDebug"
#include "QDialog"
#include "QDir"
#include "QKeyEvent"
#include "QMenu"
#include "QScreen"
#include "QSystemTrayIcon"

#include "brightness_small.h"
#include "ui_brightness_small.h"
import command;

using namespace Qt::Literals::StringLiterals;

brightness_small::brightness_small(QWidget *parent, const QStringList &args) noexcept :
    QMainWindow(parent),
    ui(new Ui::brightness_small)
{
    //check to see if running, if so, exit quick
    QString check = runCmd(u"ps -aux |grep -E 'mx-tweak.*--tray'|grep -v grep|wc -l"_s).output;

    if ( check.toInt() >= 2) {
        qDebug() << "check is " << check;
        exit(EXIT_FAILURE);
    }
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint); // for the close, min and max buttons
    QIcon save;
    save = QIcon::fromTheme(u"gtk-save"_s);
    ui->pushSave->setIcon(save);
    setupDisplay();
    QIcon icon;
    icon = QIcon::fromTheme(u"brightness-systray"_s);
    setWindowIcon(icon);
    setWindowTitle(tr("MX-Tweak"));
    expand = false;
    QString home_path = QDir::homePath();
    QString config_file_path = home_path + "/.config/MX-Linux/MX-Tweak/expand"_L1;
    if (args.contains("--full"_L1) || QFileInfo::exists(config_file_path)) {
        expand = true;
    }
    ui->labelHardwareBacklight->setVisible(expand);
    ui->sliderHardwareBacklight->setVisible(expand);
    ui->labelBacklight->setVisible(expand);
    trayicon = new QSystemTrayIcon;
    trayicon->setIcon(icon);
    trayicon->show();

    this->adjustSize();

    menu = new QMenu(this);

    if (runCmd(u"echo $XDG_CURRENT_DESKTOP | grep -q XFCE"_s).exitCode == 0) {
        full = new QAction(QIcon::fromTheme(u"video-display"_s), tr("Display"), this);
        connect(full, &QAction::triggered, this, &brightness_small::launchfulldisplaydialog);
        menu->addAction(full);
        menu->addSeparator();
    }
    quitAction = new QAction(QIcon::fromTheme(u"gtk-quit"_s), tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QGuiApplication::quit);
    menu->addAction(quitAction);

    connect(trayicon, &QSystemTrayIcon::activated, this, &brightness_small::iconActivated);
    trayicon->setContextMenu(menu);

    connect(ui->pushSave, &QPushButton::clicked, this, &brightness_small::pushSave_clicked);
    connect(ui->comboDisplay, &QComboBox::currentIndexChanged, this, &brightness_small::comboDisplay_currentIndexChanged);
    connect(ui->sliderBrightness, &QSlider::valueChanged, this, &brightness_small::sliderBrightness_valueChanged);
    connect(ui->sliderHardwareBacklight, &QSlider::actionTriggered, this, &brightness_small::setbacklight);
    connect(ui->pushExpandBacklight, &QToolButton::clicked, this, &brightness_small::pushExpandBacklight_clicked);
}

void brightness_small::iconActivated(QSystemTrayIcon::ActivationReason reason) noexcept
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        setupBrightness();
        setupGamma();
        setupbacklight();
        this->adjustSize();
        setPosition();
        this->show();
        break;
    default:
        ;
    }
}

void brightness_small::setPosition() noexcept
{
    QPoint pos = QCursor::pos();
    QScreen *screen = QGuiApplication::screenAt(pos);
    if (pos.y() + this->size().height() > screen->availableVirtualGeometry().height())
        pos.setY(screen->availableVirtualGeometry().height() - this->size().height());
    if (pos.x() + this->size().width() > screen->availableVirtualGeometry().width())
        pos.setX(screen->availableVirtualGeometry().width() - this->size().width());
    this->move(pos);
}

brightness_small::~brightness_small() noexcept
{
    delete ui;
}

//following function is not actually used by the tray application
void brightness_small::setmissingxfconfvariables(const QString &activeprofile, const QString &resolution) noexcept
{
    //set resolution, set active, set scales, set display name

    //set display name
    runCmd("xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + ui->comboDisplay->currentText() + " -t string -s "_L1 + ui->comboDisplay->currentText() + " --create"_L1);

    //set resolution
    runCmd("xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + ui->comboDisplay->currentText() + "/Resolution -t string -s "_L1 + resolution.simplified() + " --create"_L1);

    //set active profile
    runCmd("xfconf-query --channel displays -p /"_L1 + activeprofile + '/' + ui->comboDisplay->currentText() + "/Active -t bool -s true --create"_L1);
}

void brightness_small::setupbacklight() noexcept
{
    //check for backlights
    QString test = runCmd(u"ls /sys/class/backlight"_s).output;
    if ( ! test.isEmpty()) {
        //get backlight value for currently
        QString backlight=runCmd(u"sudo /usr/lib/mx-tweak/backlight-brightness -g"_s).output;
        int backlight_slider_value = backlight.toInt();
        ui->sliderHardwareBacklight->setValue(backlight_slider_value);
        ui->sliderHardwareBacklight->setToolTip(backlight);
        ui->labelBacklight->setText(backlight);
        qDebug() << "backlight string is " << backlight;
        qDebug() << " backlight_slider_value is " << backlight_slider_value;
    } else {
        ui->pushExpandBacklight->hide();
        ui->sliderHardwareBacklight->hide();
        ui->labelBacklight->hide();
        ui->labelHardwareBacklight->hide();
    }
}

void brightness_small::setbacklight() noexcept
{
    const QString &backlight = QString::number(ui->sliderHardwareBacklight->value());
    runProc(u"sudo"_s, {u"/usr/lib/mx-tweak/backlight-brightness"_s, u"-s"_s, backlight});
    ui->labelBacklight->setText(backlight);
}

void brightness_small::setupBrightness() noexcept
{
    //get brightness value for currently shown display
    QString brightness=runCmd("LANG=C xrandr --verbose | awk '/"_L1 + ui->comboDisplay->currentText()
        + "/{flag=1;next}/Clones/{flag=0}flag'|grep Brightness|cut -d' ' -f2"_L1).output;
    int brightness_slider_value = static_cast<int>(brightness.toFloat() * 100);
    ui->sliderBrightness->setValue(brightness_slider_value);
    qDebug() << "brightness string is " << brightness;
    qDebug() << " brightness_slider_value is " << brightness_slider_value;
    ui->sliderBrightness->setToolTip(QString::number(ui->sliderBrightness->value()));
    ui->labelBrightness->setText(QString::number(ui->sliderBrightness->value()));
}

void brightness_small::setupGamma() noexcept
{
    QString gamma = runCmd("/usr/lib/mx-tweak/mx-tweak-lib-randr.sh "_L1 + ui->comboDisplay->currentText() + " gamma"_L1).output;
    gamma=gamma.simplified();
    gamma = gamma.section(':',1,3).simplified();
    double gamma1 = 1.0 / gamma.section(':',0,0).toDouble();
    double gamma2 = 1.0 / gamma.section(':',1,1).toDouble();
    double gamma3 = 1.0 / gamma.section(':',2,2).toDouble();
    g1 = QString::number(gamma1,'G', 3);
    g2 = QString::number(gamma2,'G', 3);
    g3 = QString::number(gamma3,'G', 3);
    qDebug() << "gamma is " << g1 << " " << g2 << " " << g3;
}

void brightness_small::sliderBrightness_valueChanged(int value) noexcept
{
    QString slider_value = QString::number(value);
    ui->sliderBrightness->setToolTip(slider_value);
    ui->labelBrightness->setText(slider_value);
    if ( brightnessflag ) {
        //setupBrightness();
        //setupGamma();
        //setupbacklight();
        setBrightness();
    }
}

void brightness_small::setBrightness() noexcept
{
    double num = ui->sliderBrightness->value() / 100.0;
    qDebug() << "num is :" << num;
    QString brightness = QString::number(num, 'G', 5);
    qDebug() << "changed brightness is :" << brightness;
    runProc(u"xrandr"_s, {u"--output"_s, ui->comboDisplay->currentText(),
        u"--brightness"_s, brightness,
        u"--gamma"_s + g1+':'+g2+':'+g3});
}

void brightness_small::pushSave_clicked() noexcept
{
    //save cmd used in user's home file under .config
    //make directory when its not present
    double num = ui->sliderBrightness->value() / 100.0;
    qDebug() << "num is :" << num;
    QString brightness = QString::number(num, 'G', 5);
    QString home_path = QDir::homePath();
    QString config_file_path = home_path + "/.config/MX-Linux/MX-Tweak/brightness"_L1;
    if ( ! QFileInfo::exists(config_file_path)) {
        runCmd("mkdir -p "_L1 + config_file_path);
    }
    //save config in file named after the display
    runCmd("echo 'xrandr --output "_L1 + ui->comboDisplay->currentText()
        + " --brightness "_L1 + brightness + " --gamma "_L1 + g1 + ':' + g2 + ':' + g3
        + "'>"_L1 + config_file_path + '/' + ui->comboDisplay->currentText());
}

void brightness_small::comboDisplay_currentIndexChanged(int) noexcept
{
    if (brightnessflag) {
        setupBrightness();
        setupGamma();
    }
}

void brightness_small::setupDisplay() noexcept
{
    //populate combobox
    QString displaydata = runCmd(u"LANG=C xrandr |grep -w connected | cut -d' ' -f1"_s).output;
    QStringList displaylist = displaydata.split('\n');
    ui->comboDisplay->clear();
    ui->comboDisplay->addItems(displaylist);
    brightnessflag = true;
}

// implement change event that closes app when window loses focus
void brightness_small::changeEvent(QEvent *event) noexcept
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        if (this->isActiveWindow()) {
            qDebug() << "focusinEvent";
        } else {
            qDebug() << "focusOutEvent";
            this->hide();
        }
    }
}

// process keystrokes
void brightness_small::keyPressEvent(QKeyEvent *event) noexcept
{
    if (event->key() == Qt::Key_Escape) {
        this->hide();
    }
}

void brightness_small::launchfulldisplaydialog() noexcept
{
    runProc(u"mx-tweak"_s, {u"--display"_s});
}

void brightness_small::pushExpandBacklight_clicked() noexcept
{
    QString config_file_path = QDir::homePath() + "/.config/MX-Linux/MX-Tweak/expand"_L1;
    //expand toggle
    expand = !expand;
    if (! expand) {
        QFile::remove(config_file_path);
        ui->labelHardwareBacklight->hide();
        ui->sliderHardwareBacklight->hide();
        ui->labelBacklight->hide();
    } else {
        QFile file(config_file_path);
        file.open(QIODevice::NewOnly);
        ui->labelHardwareBacklight->show();
        ui->sliderHardwareBacklight->show();
        ui->labelBacklight->show();
    }
}
