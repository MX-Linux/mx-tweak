#include "remove_user_theme_set.h"
#include "ui_remove_user_theme_set.h"

#include "defaultlook.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

remove_user_theme_set::remove_user_theme_set(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::remove_user_theme_set)
{
    ui->setupUi(this);
    setupThemeSelector();

}

remove_user_theme_set::~remove_user_theme_set()
{
    delete ui;
}

QComboBox *remove_user_theme_set::themeSelector()
{
    return ui->comboBoxThemes;
}

void remove_user_theme_set::setupThemeSelector()
{
    //build theme list
    ui->comboBoxThemes->clear();
    QStringList theme_list;
    QStringList filter(QStringLiteral("*.tweak"));
    bool xsettings_gtk_theme_present = false;
    bool icontheme_present = false;
    bool xfwm4_theme_present = false;

    //add user entries in ~/.local/share/mx-tweak-data



    QString home_path = QDir::homePath();
    QDirIterator it2(home_path + "/.local/share/mx-tweak-data", filter, QDir::Files, QDirIterator::Subdirectories);
    while (it2.hasNext()) {
        QFileInfo file_info(it2.next());
        QString filename = file_info.absoluteFilePath();
        QString name = runCmd("cat '" + filename + "'|grep Name=").output.section(QStringLiteral("="),1,1);

        QString xsettings_gtk_theme = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xsettings_gtk_theme=").output.section(QStringLiteral("="),1,1);
        qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
        QString xsettings_icon_theme = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xsettings_icon_theme=").output.section(QStringLiteral("="),1,1);
        qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
        QString xfwm4_window_decorations = runCmd("cat '" + file_info.absoluteFilePath() + "' |grep xfwm4_window_decorations=").output.section(QStringLiteral("="),1,1);
        qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;

        //check theme existence, only list if all 3 elements present
        QFileInfo xsettings_theme("/usr/share/themes/" + xsettings_gtk_theme);
        QFileInfo xfwm4_theme("/usr/share/themes/" + xfwm4_window_decorations);
        QFileInfo icon_theme("/usr/share/icons/" + xsettings_icon_theme);
        QFileInfo xsettings_theme_home(home_path + "/.themes/" + xsettings_gtk_theme);
        QFileInfo xfwm4_theme_home("" + home_path + "/.themes/" + xfwm4_window_decorations);
        QFileInfo icon_theme_home("" + home_path + "/.icons/" + xsettings_icon_theme);
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

    theme_list.insert(0, QStringLiteral("Select User Theme Set to Remove"));
    ui->comboBoxThemes->addItems(theme_list);
    ui->comboBoxThemes->setCurrentIndex(0);

}

ExecResult remove_user_theme_set::runCmd(const QString &cmd)
{
    QEventLoop loop;
    auto *proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::MergedChannels);
    connect(proc, SIGNAL(finished(int)), &loop, SLOT(quit()));
    proc->start(QStringLiteral("/bin/bash"), QStringList() << QStringLiteral("-c") << cmd);
    loop.exec();
    disconnect(proc, nullptr, nullptr, nullptr);
    ExecResult result;
    result.exitCode = proc->exitCode();
    result.output = proc->readAll().trimmed();
    delete proc;
    return result;
}

QString remove_user_theme_set::getFilename(const QString &name)
{
    return theme_info.value(name);
}
