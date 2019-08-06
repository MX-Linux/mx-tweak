#ifndef THEMING_TO_TWEAK_H
#define THEMING_TO_TWEAK_H

#include <QDialog>
#include "ui_theming_to_tweak.h"

namespace Ui {
class theming_to_tweak;
}

class theming_to_tweak : public QDialog
{
    Q_OBJECT

public:
    explicit theming_to_tweak(QWidget *parent = 0);
    ~theming_to_tweak();
    QLineEdit* nameEditor();
private:
    Ui::theming_to_tweak *ui;
};

#endif // THEMING_TO_TWEAK_H
