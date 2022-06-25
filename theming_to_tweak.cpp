#include "theming_to_tweak.h"
#include "ui_theming_to_tweak.h"

theming_to_tweak::theming_to_tweak(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::theming_to_tweak)
{
    ui->setupUi(this);
    connect(ui->lineEdit_Name, &QLineEdit::textChanged, this, [=](){
        QString text = ui->lineEdit_Name->text();
        text.replace('\'', QString());
        ui->lineEdit_Name->setText(text);
    });
}

theming_to_tweak::~theming_to_tweak()
{
    delete ui;
}

QLineEdit *theming_to_tweak::nameEditor()
{
    return ui->lineEdit_Name;
}
