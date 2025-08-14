#ifndef TWEAK_SUPERKEY_H
#define TWEAK_SUPERKEY_H

#include <QObject>

namespace Ui {
class defaultlook;
}

class TweakSuperKey : public QObject
{
    Q_OBJECT
public:
    TweakSuperKey() = delete;
    TweakSuperKey(Ui::defaultlook *ui, bool verbose, QObject *parent = nullptr) noexcept;
    void setup() noexcept;
    static bool checkSuperKey() noexcept;

private:
    Ui::defaultlook *ui;
    bool verbose;

    void textSuperKeyCommand_textChanged(const QString &) noexcept;
    void pushSuperKeyBrowseAppFile_clicked() noexcept;
    void pushSuperKeyApply_clicked() noexcept;
};

#endif // TWEAK_SUPERKEY_H
