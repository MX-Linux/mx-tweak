#ifndef REMOVE_USER_THEME_SET_H
#define REMOVE_USER_THEME_SET_H

#include <QDialog>

namespace Ui {
class remove_user_theme_set;
}

class QComboBox;

struct ExecResult
{
    QString output;
    int exitCode;
};

class remove_user_theme_set : public QDialog
{
    Q_OBJECT

public:
    explicit remove_user_theme_set(QWidget *parent = 0);
    ~remove_user_theme_set();

    QComboBox *themeSelector();

    void setupThemeSelector();

    ExecResult runCmd(const QString &cmd);

    QString getFilename(const QString &name);

private:
    Ui::remove_user_theme_set *ui;
    QHash<QString, QString> theme_info;
};

#endif // REMOVE_USER_THEME_SET_H
