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
    TweakTheme() = delete;
    TweakTheme(Ui::defaultlook *ui, bool verbose, Desktop desktop, QObject *parent = nullptr) noexcept;
    TweakTheme(Ui::defaultlook *ui, bool verbose, class TweakXfce *tweak, QObject *parent = nullptr) noexcept;
    void setup() noexcept;

private:
    Ui::defaultlook *ui;

    bool verbose;
    Desktop desktop;
    class TweakXfce *tweakXfce = nullptr;

    struct {
        bool theme;
    } flags = {};
    QHash<QString, QString> theme_info;

    void setupComboTheme() noexcept;
    void getCursorSize() noexcept;
    void populateThemeLists(const QString &value) noexcept;
    void setTheme(const QString &type, const QString &theme) const noexcept;

    void comboTheme_currentIndexChanged(int index) noexcept;
    void pushThemeSaveSet_clicked() noexcept;
    void pushThemeRemoveSet_clicked() noexcept;
    void pushThemeApply_clicked() noexcept;
    void spinThemeCursorSize_valueChanged(int value) noexcept;
    void listThemeWidget_currentTextChanged(const QString &currentText) noexcept;
    void checkThemeFluxboxLegacyStyles_toggled(bool) noexcept;
    void listThemeWindow_currentTextChanged(const QString &currentText) const noexcept;
    void listThemeIcons_currentTextChanged(const QString &currentText) const noexcept;
    void listThemeCursors_currentTextChanged(const QString &currentText) noexcept;
};

#endif // TWEAK_THEME_H
