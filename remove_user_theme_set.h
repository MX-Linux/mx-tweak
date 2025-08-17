#ifndef REMOVE_USER_THEME_SET_H
#define REMOVE_USER_THEME_SET_H

#include <QDialog>

namespace Ui {
class remove_user_theme_set;
}

class QComboBox;

class remove_user_theme_set : public QDialog
{
    Q_OBJECT

public:
    explicit remove_user_theme_set(QWidget *parent = 0) noexcept;
    ~remove_user_theme_set() noexcept;

    QComboBox *themeSelector() const noexcept;
    QString getFilename(const QString &name) const noexcept;
    void setupThemeSelector() noexcept;

private:
    Ui::remove_user_theme_set *ui;
    QHash<QString, QString> theme_info;
};

#endif // REMOVE_USER_THEME_SET_H
