#pragma once

#include <QDialog>
#include <QLineEdit>

namespace Ui {
class theming_to_tweak;
}

class theming_to_tweak : public QDialog
{
    Q_OBJECT

public:
    explicit theming_to_tweak(QWidget *parent = nullptr) noexcept;
    ~theming_to_tweak() noexcept;

    QLineEdit *nameEditor() noexcept;
private:
    Ui::theming_to_tweak *ui;
};
