#ifndef TWEAK_DISPLAY_H
#define TWEAK_DISPLAY_H

#include <QObject>

namespace Ui {
class defaultlook;
}

class TweakDisplay : public QObject
{
    Q_OBJECT
public:
    TweakDisplay(Ui::defaultlook *ui, bool verbose, QObject *parent = nullptr);
    void setup();

private:
    Ui::defaultlook *ui;

    bool verbose;

    QString strGamma1;
    QString strGamma2;
    QString strGamma3;

    struct {
        bool brightness;
    } flags = {};

    void setMissingXfconfVariables(const QString &activeProfile, const QString &resolution);
    void setupResolutions();
    void setResolution();
    void setupScale();
    void setScale();
    void setRefreshRate(const QString &display, const QString &resolution, const QString &activeProfile) const;
    void setupBacklight();
    void setBacklight();
    void setGTKScaling();
    void setupBrightness();
    void setBrightness();
    void saveBrightness();
    void setupGamma();

    void comboDisplay_currentIndexChanged(int  /*index*/);
    void sliderDisplayBrightness_valueChanged(int value);
};

#endif // TWEAK_DISPLAY_H
