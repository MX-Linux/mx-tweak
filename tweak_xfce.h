#ifndef TWEAK_XFCE_H
#define TWEAK_XFCE_H

#include <QObject>

namespace Ui {
class defaultlook;
}

class TweakXfce : public QObject
{
    Q_OBJECT
public:
    TweakXfce(Ui::defaultlook *ui, bool verbose, QObject *parent = nullptr);
    void setup();
    bool checkXfce() const;

private:
    Ui::defaultlook *ui;

    bool verbose;

    QString pluginTaskList;
    struct {
        bool hibernate;
    } flags = {};

    void slotSettingChanged();
    void pushXfceApply_clicked();
};

#endif // TWEAK_XFCE_H
