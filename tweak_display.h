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
    TweakDisplay() = delete;
    TweakDisplay(Ui::defaultlook *ui, bool verbose, QObject *parent = nullptr) noexcept;
    void setup() noexcept;

private:
    Ui::defaultlook *ui;

    bool verbose;

    QString strGamma1;
    QString strGamma2;
    QString strGamma3;

    struct {
        bool brightness;
    } flags = {};

    void setMissingXfconfVariables(const QString &activeProfile, const QString &resolution) noexcept;
    void setupResolutions() noexcept;
    void setResolution() noexcept;
    void setupScale() noexcept;
    void setScale() noexcept;
    void setRefreshRate(const QString &display, const QString &resolution, const QString &activeProfile) const noexcept;
    void setupBacklight() noexcept;
    void setBacklight() noexcept;
    void setGTKScaling() noexcept;
    void setupBrightness() noexcept;
    void setBrightness() noexcept;
    void saveBrightness() noexcept;
    void setupGamma() noexcept;

    void comboDisplay_currentIndexChanged(int  /*index*/) noexcept;
    void sliderDisplayBrightness_valueChanged(int value) noexcept;
};

#endif // TWEAK_DISPLAY_H
