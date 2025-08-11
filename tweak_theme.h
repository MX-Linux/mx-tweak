#ifndef TWEAK_THEME_H
#define TWEAK_THEME_H

#include <QObject>
#include <QHash>

namespace Ui {
class defaultlook;
}

class TweakTheme : public QObject
{
    Q_OBJECT
public:
    enum Desktop {
        Xfce,
        Plasma,
        Fluxbox
    };
    class TweakXfce *tweakXfce = nullptr;
    TweakTheme(Ui::defaultlook *ui, bool verbose, Desktop desktop, QObject *parent = nullptr);
    void setup();

private:
    Ui::defaultlook *ui;

    bool verbose;
    Desktop desktop;

    struct {
        bool theme;
    } flags = {};
    QHash<QString, QString>  theme_info;

    void setupComboTheme();
    void getCursorSize();
    void populateThemeLists(const QString &value);
    void setTheme(const QString &type, const QString &theme) const;

    void comboTheme_currentIndexChanged(int index);
    void pushThemeSaveSet_clicked();
    void pushThemeRemoveSet_clicked();
    void pushThemeApply_clicked();
    void spinThemeCursorSize_valueChanged(int value);
    void listThemeWidget_currentTextChanged(const QString &currentText);
    void checkThemeFluxboxLegacyStyles_toggled(bool);
    void listThemeWindow_currentTextChanged(const QString &currentText) const;
    void listThemeIcons_currentTextChanged(const QString &currentText) const;
    void listThemeCursors_currentTextChanged(const QString &currentText);
};

#endif // TWEAK_THEME_H
