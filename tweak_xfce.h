#ifndef TWEAK_XFCE_H
#define TWEAK_XFCE_H

#include <QObject>

namespace Ui {
class Tweak;
}

class TweakXfce : public QObject
{
    Q_OBJECT
public:
    TweakXfce() = delete;
    TweakXfce(Ui::Tweak *ui, bool verbose, QObject *parent = nullptr) noexcept;
    void setup() noexcept;
    bool checkXfce() const noexcept;

signals:
    void toolRun() noexcept;

private:
    Ui::Tweak *ui;

    bool verbose;

    QString pluginTaskList;
    struct {
        bool hibernate;
    } flags = {};

    void slotSettingChanged() noexcept;
    void pushXfceApply_clicked() noexcept;
    void pushXfceAppearance_clicked() noexcept;
    void pushXfceWindowManager_clicked() noexcept;
};

#endif // TWEAK_XFCE_H
