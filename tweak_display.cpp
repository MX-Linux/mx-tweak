#include <QFileInfo>
#include <QDir>
#include "ui_tweak.h"
#include "tweak_display.h"
import command;

using namespace Qt::Literals::StringLiterals;

TweakDisplay::TweakDisplay(Ui::Tweak *ui, bool verbose, QObject *parent) noexcept
    : QObject{parent}, ui{ui}, verbose{verbose}
{
    setup();
    connect(ui->pushDisplayApplyScaling, &QPushButton::clicked, this, &TweakDisplay::setScale);
    connect(ui->pushDisplayGTKScaling, &QPushButton::clicked, this, &TweakDisplay::setGTKScaling);
    connect(ui->sliderDisplayHardwareBacklight, &QSlider::actionTriggered, this, &TweakDisplay::setBacklight);
    connect(ui->pushDisplaySaveBrightness, &QPushButton::clicked, this, &TweakDisplay::saveBrightness);
    connect(ui->pushDisplayApplyResolution, &QPushButton::clicked, this, &TweakDisplay::setResolution);
    connect(ui->comboDisplay, &QComboBox::currentIndexChanged, this, &TweakDisplay::comboDisplay_currentIndexChanged);
    connect(ui->sliderDisplayBrightness, &QSlider::valueChanged, this, &TweakDisplay::sliderDisplayBrightness_valueChanged);
}

void TweakDisplay::setup() noexcept
{
    //populate combobox
    QString displaydata = runCmd(u"LANG=C xrandr |grep -w connected | cut -d' ' -f1"_s).output;
    QStringList displaylist = displaydata.split(u"\n"_s);
    ui->comboDisplay->clear();
    ui->comboDisplay->addItems(displaylist);
    setupBrightness();
    setupGamma();
    setupScale();
    setupBacklight();
    setupResolutions();
    flags.brightness = true;

    //get gtk scaling value
    QString GTKScale = runCmd(u"LANG=C xfconf-query --channel xsettings -p /Gdk/WindowScalingFactor"_s).output;
    ui->spinDisplayGTKScaling->setValue(GTKScale.toInt());
    //disable resolution stuff
}

void TweakDisplay::setMissingXfconfVariables(const QString &activeProfile, const QString &resolution) noexcept
{
    //set display name
    runCmd("xfconf-query --channel displays -p /"_L1 + activeProfile + '/' + ui->comboDisplay->currentText() + " -t string -s "_L1 + ui->comboDisplay->currentText() + " --create"_L1);

    //set resolution
    runCmd("xfconf-query --channel displays -p /"_L1 + activeProfile + '/' + ui->comboDisplay->currentText() + "/Resolution -t string -s "_L1 + resolution.simplified() + " --create"_L1);

    //set active profile
    runCmd("xfconf-query --channel displays -p /"_L1 + activeProfile + '/' + ui->comboDisplay->currentText() + "/Active -t bool -s true --create"_L1);
}

void TweakDisplay::setupResolutions() noexcept
{
    const QString &display = ui->comboDisplay->currentText();
    ui->comboDisplayResolutions->clear();
    const QString &cmd = "LANG=C /usr/lib/mx-tweak/mx-tweak-lib-randr.sh "_L1 + display + " resolutions"_L1;
    if (verbose) qDebug() << "get resolution command is :" << cmd;
    const QString &resolutions = runCmd(cmd).output;
    if (verbose) qDebug() << "resolutions are :" << resolutions;
    ui->comboDisplayResolutions->addItems(resolutions.split(u'\n'));
    //set current resolution as default
    const QString &resolution = runCmd("xrandr |grep "_L1 + display + " |cut -d+ -f1 |grep -oE '[^ ]+$'"_L1).output;
    if (verbose) qDebug() << "resolution is : " << resolution;
    ui->comboDisplayResolutions->setCurrentText(resolution);
}
void TweakDisplay::setResolution() noexcept
{
    const QString &activeProfile = runCmd(u"LANG=C xfconf-query --channel displays -p /ActiveProfile"_s).output;
    const QString &display = ui->comboDisplay->currentText();
    const QString &resolution = ui->comboDisplayResolutions->currentText();
    const QString &cmd = "xrandr --output "_L1 + display + " --mode "_L1 + resolution;
    if (verbose) qDebug() << "resolution change command is " << cmd;
    runCmd(cmd);
    setMissingXfconfVariables(activeProfile, resolution);
    setRefreshRate(display, resolution, activeProfile);
}

void TweakDisplay::setupScale() noexcept
{
    //setup scale for currently shown display and in the active profile
    QString xscale = u"1"_s;
    QString yscale = u"1"_s;
    double scale = 1;

    //get active profile
    const QString &activeProfile = runCmd(u"LANG=C xfconf-query --channel displays -p /ActiveProfile"_s).output;
    const QString &cmdquery = "LANG=C xfconf-query --channel displays -p /"_L1
        + activeProfile + '/' + ui->comboDisplay->currentText();
    //get scales for display show in combobox
    Result res = runCmd(cmdquery + "/Scale/X"_L1);
    if (res.exitCode == 0) {
        xscale = res.output;
    }
    res = runCmd(cmdquery + "/Scale/Y"_L1);
    if (res.exitCode == 0) {
        yscale = res.output;
    }

    // since we want scales equal, set the scale spin box to xscale.  invert so that 2 = .5
    scale = 1 / xscale.toDouble();
    if (verbose) {
        qDebug() << "active profile is: " << activeProfile
            << " xscale is " << xscale << " yscale is " << yscale << " scale is: " << scale;
    }
    ui->spinDisplayScale->setValue(scale);

    //hide scale setup if X and Y don't match
    if ( xscale != yscale) {
        ui->spinDisplayScale->hide();
        ui->labelDisplayScale->hide();
        ui->pushDisplayApplyScaling->hide();
    }
}
void TweakDisplay::setScale() noexcept
{
    //get active profile and desired scale for given resolution
    double scale = 1 / ui->spinDisplayScale->value();
    const QString &resolution = runCmd("xrandr |grep "_L1
        + ui->comboDisplay->currentText() + " |cut -d' ' -f3 |cut -d'+' -f1"_L1).output;
    if (verbose) qDebug() << "resolution is : " << resolution;
    const QString &scaleString = QString::number(scale, 'G', 5);
    const QString &activeProfile = runCmd(u"LANG=C xfconf-query --channel displays -p /ActiveProfile"_s).output;

    //set missing variables
    setMissingXfconfVariables(activeProfile, resolution);

    //set scale value
    const QStringList args1{u"--channel"_s, u"displays"_s, u"-p"_s,
        '/' + activeProfile + '/' + ui->comboDisplay->currentText()};
    const QStringList args2{u"-t"_s, u"double"_s, u"-s"_s, scaleString, u"--create"_s};
    runProc(u"xfconf-query"_s, QStringList() << args1 << u"/Scale/Y"_s << args2);
    runProc(u"xfconf-query"_s, QStringList() << args1 << u"/Scale/X"_s << args2);

    //set initial scale with xrandr
    runProc(u"xrandr"_s, {u"--output"_s, ui->comboDisplay->currentText(),
        u"--scale"_s, scaleString + 'x' + scaleString});
}

void TweakDisplay::setRefreshRate(const QString &display, const QString &resolution, const QString &activeProfile) const noexcept
{
    const QString &rate = runCmd("/usr/lib/mx-tweak/mx-tweak-lib-randr.sh "_L1 + display + " refreshrate"_L1).output;
    QStringList rates = rate.split(QRegularExpression(u"\\s"_s), Qt::SkipEmptyParts);
    rates.removeAll(resolution);
    if (verbose) {
        qDebug() << "defualt refreshreate list is :" << rates.at(0).section('*',0,0);
    }
    runProc(u"xfconf-query"_s, {u"--channel"_s, u"displays"_s, u"-p"_s,
        (u'/' + activeProfile + u'/' + display + "/RefreshRate"_L1),
        u"-t"_s, u"double"_s, u"-s"_s, rates.at(0).section('*',0,0), u" --create"_s});
}

void TweakDisplay::setupBacklight() noexcept
{
    //check for backlights
    const QString &test = runCmd(u"ls /sys/class/backlight"_s).output;
    if (!test.isEmpty()) {
        //get backlight value for currently
        const QString &backlight = runCmd(u"sudo /usr/lib/mx-tweak/backlight-brightness -g"_s).output;
        ui->sliderDisplayHardwareBacklight->setValue(backlight.toInt());
        ui->sliderDisplayHardwareBacklight->setToolTip(backlight);
        ui->labelCurrentBacklight->setText(backlight);
        if (verbose) qDebug() << "backlight string is " << backlight;
    } else {
        ui->sliderDisplayHardwareBacklight->hide();
        ui->labelCurrentBacklight->hide();
        ui->labelHardwareBacklight->hide();
    }
}
void TweakDisplay::setBacklight() noexcept
{
    const QString &backlight = QString::number(ui->sliderDisplayHardwareBacklight->value());
    runProc(u"sudo"_s, {u"/usr/lib/mx-tweak/backlight-brightness"_s, u"-s"_s, backlight});
    ui->labelCurrentBacklight->setText(backlight);
}

void TweakDisplay::setGTKScaling() noexcept
{
    runProc(u"xfconf-query"_s, {u"--channel"_s, u"xsettings"_s, u"-p"_s,
        u"/Gdk/WindowScalingFactor"_s, u"-t"_s, u"int"_s, u"-s"_s,
        QString::number(ui->spinDisplayGTKScaling->value())});
    runProc(u"xfce4-panel"_s, {u"--restart"_s});
}

void TweakDisplay::setupBrightness() noexcept
{
    //get brightness value for currently shown display
    const QString &brightness = runCmd("LANG=C xrandr --verbose | awk '/"_L1 + ui->comboDisplay->currentText()
        + "/{flag=1;next}/Clones/{flag=0}flag'|grep Brightness|cut -d' ' -f2"_L1).output;
    const int value = static_cast<int>(brightness.toFloat() * 100);
    ui->sliderDisplayBrightness->setValue(value);
    // Tool top and label to reflect the actual value of the slider.
    const QString &strValue = QString::number(ui->sliderDisplayBrightness->value());
    if (verbose) qDebug() << "brightness string" << brightness;
    if (verbose) qDebug() << "brightness slider" << value << strValue;
    ui->sliderDisplayBrightness->setToolTip(strValue);
    ui->labelDisplayBrightness->setText(strValue);
}
void TweakDisplay::setBrightness() noexcept
{
    double num = ui->sliderDisplayBrightness->value() / 100.0;
    if (verbose) qDebug() << "num is :" << num;
    QString brightness = QString::number(num, 'G', 5);
    if (verbose) qDebug() << "changed brightness is :" << brightness;
    runProc(u"xrandr"_s, {u"--output"_s, ui->comboDisplay->currentText(),
        u"--brightness"_s, brightness,
        u"--gamma"_s, strGamma1+':'+strGamma2+':'+strGamma3});
}
void TweakDisplay::saveBrightness() noexcept
{
    //save cmd used in user's home file under .config
    //make directory when its not present
    const double num = ui->sliderDisplayBrightness->value() / 100.0;
    if (verbose) qDebug() << "num is :" << num;
    const QString &configPath = QDir::homePath() + "/.config/MX-Linux/MX-Tweak/brightness"_L1;
    if (!QFileInfo::exists(configPath)) {
        runCmd("mkdir -p "_L1 + configPath);
    }
    //save config in file named after the display
    runCmd("echo 'xrandr --output "_L1 + ui->comboDisplay->currentText()
        + " --brightness "_L1 + QString::number(num, 'G', 5)
        + " --gamma "_L1 + strGamma1 + ':' + strGamma2 + ':' + strGamma3
        + "'>"_L1 + configPath + '/' + ui->comboDisplay->currentText());
}

void TweakDisplay::setupGamma() noexcept
{
    QString gamma = runCmd("/usr/lib/mx-tweak/mx-tweak-lib-randr.sh "_L1
                           + ui->comboDisplay->currentText() + " gamma"_L1).output;
    gamma = gamma.simplified();
    gamma = gamma.section(u':',1,3).simplified();
    double gamma1 = 1.0 / gamma.section(u':',0,0).toDouble();
    double gamma2 = 1.0 / gamma.section(u':',1,1).toDouble();
    double gamma3 = 1.0 / gamma.section(u':',2,2).toDouble();
    strGamma1 = QString::number(gamma1,'G', 3);
    strGamma2 = QString::number(gamma2,'G', 3);
    strGamma3 = QString::number(gamma3,'G', 3);
    if (verbose) qDebug() << "gamma is" << strGamma1 << strGamma2 << strGamma3;
}

void TweakDisplay::comboDisplay_currentIndexChanged(int  /*index*/) noexcept
{
    setupBrightness();
    setupScale();
    setupResolutions();
    setupGamma();
}

void TweakDisplay::sliderDisplayBrightness_valueChanged(int value) noexcept
{
    const QString &strValue = QString::number(value);
    ui->sliderDisplayBrightness->setToolTip(strValue);
    ui->labelDisplayBrightness->setText(strValue);
    if (flags.brightness) {
        setBrightness();
    }
}
