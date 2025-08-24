#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

#include "remove_user_theme_set.h"
#include "ui_remove_user_theme_set.h"
import command;

using namespace Qt::Literals::StringLiterals;

remove_user_theme_set::remove_user_theme_set(QWidget *parent) noexcept
    : QDialog(parent), ui(new Ui::remove_user_theme_set)
{
    ui->setupUi(this);
    setupThemeSelector();
}

remove_user_theme_set::~remove_user_theme_set() noexcept
{
    delete ui;
}

QComboBox *remove_user_theme_set::themeSelector() const noexcept
{
    return ui->comboBoxThemes;
}

void remove_user_theme_set::setupThemeSelector() noexcept
{
    //build theme list
    ui->comboBoxThemes->clear();
    QStringList theme_list;
    QStringList filter(u"*.tweak"_s);
    bool xsettings_gtk_theme_present = false;
    bool icontheme_present = false;
    bool xfwm4_theme_present = false;

    //add user entries in ~/.local/share/mx-tweak-data

    QString home_path = QDir::homePath();
    QDirIterator it2(home_path + "/.local/share/mx-tweak-data"_L1, filter, QDir::Files, QDirIterator::Subdirectories);
    while (it2.hasNext()) {
        QFileInfo file_info(it2.next());
        QString filename = file_info.absoluteFilePath();
        QString name = runCmd("cat '"_L1 + filename + "'|grep Name="_L1).output.section('=',1,1);

        QString xsettings_gtk_theme = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xsettings_gtk_theme="_L1).output.section('=',1,1);
        qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
        QString xsettings_icon_theme = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xsettings_icon_theme="_L1).output.section('=',1,1);
        qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
        QString xfwm4_window_decorations = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xfwm4_window_decorations="_L1).output.section('=',1,1);
        qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;

        //check theme existence, only list if all 3 elements present
        QFileInfo xsettings_theme("/usr/share/themes/"_L1 + xsettings_gtk_theme);
        QFileInfo xfwm4_theme("/usr/share/themes/"_L1 + xfwm4_window_decorations);
        QFileInfo icon_theme("/usr/share/icons/"_L1 + xsettings_icon_theme);
        QFileInfo xsettings_theme_home(home_path + "/.themes/"_L1 + xsettings_gtk_theme);
        QFileInfo xfwm4_theme_home(home_path + "/.themes/"_L1 + xfwm4_window_decorations);
        QFileInfo icon_theme_home(home_path + "/.icons/"_L1 + xsettings_icon_theme);
        qDebug() << "xsettings_theme_home path" << xsettings_theme_home.absoluteFilePath();

        if (xsettings_theme.exists() || xsettings_theme_home.exists() ) {
            xsettings_gtk_theme_present = true;
        }

        if (xfwm4_theme.exists() || xfwm4_theme_home.exists()) {
            xfwm4_theme_present = true;
        }

        if (icon_theme.exists() || icon_theme_home.exists()) {
            icontheme_present = true;
        }

        if (xsettings_gtk_theme_present && xfwm4_theme_present && icontheme_present) {
            qDebug() << "filename is " << filename;
            qDebug()<< "theme combo name" << name;
            theme_list << name;
            theme_info.insert(name,filename);
            qDebug() << "theme info hash value is" << name << " " << theme_info[name];
        }
    }

    theme_list.insert(0, u"Select User Theme Set to Remove"_s);
    ui->comboBoxThemes->addItems(theme_list);
    ui->comboBoxThemes->setCurrentIndex(0);

}

QString remove_user_theme_set::getFilename(const QString &name) const noexcept
{
    return theme_info.value(name);
}
