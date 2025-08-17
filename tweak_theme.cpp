#include <cassert>
#include <QMessageBox>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include "ui_tweak.h"
#include "cmd.h"
#include "theming_to_tweak.h"
#include "remove_user_theme_set.h"
#include "tweak_xfce_panel.h"
#include "tweak_theme.h"

using namespace Qt::Literals::StringLiterals;

TweakTheme::TweakTheme(Ui::Tweak *ui, bool verbose, Desktop desktop, QObject *parent) noexcept
    : QObject{parent}, ui{ui}, verbose{verbose}, desktop{desktop}
{
    ui->checkThemeFluxboxLegacyStyles->setVisible(desktop == Fluxbox);
    setup();
    setupComboTheme();
    connect(ui->comboTheme, &QComboBox::currentIndexChanged, this, &TweakTheme::comboTheme_currentIndexChanged);
    connect(ui->pushThemeSaveSet, &QPushButton::clicked, this, &TweakTheme::pushThemeSaveSet_clicked);
    connect(ui->pushThemeRemoveSet, &QPushButton::clicked, this, &TweakTheme::pushThemeRemoveSet_clicked);
    connect(ui->pushThemeApply, &QPushButton::clicked, this, &TweakTheme::pushThemeApply_clicked);
    connect(ui->spinThemeCursorSize, &QSpinBox::valueChanged, this, &TweakTheme::spinThemeCursorSize_valueChanged);
    connect(ui->listThemeWidget, &QListWidget::currentTextChanged, this, &TweakTheme::listThemeWidget_currentTextChanged);
    connect(ui->checkThemeFluxboxLegacyStyles, &QCheckBox::toggled, this, &TweakTheme::checkThemeFluxboxLegacyStyles_toggled);
    connect(ui->listThemeWindow, &QListWidget::currentTextChanged, this, &TweakTheme::listThemeWindow_currentTextChanged);
    connect(ui->listThemeIcons, &QListWidget::currentTextChanged, this, &TweakTheme::listThemeIcons_currentTextChanged);
    connect(ui->listThemeCursors, &QListWidget::currentTextChanged, this, &TweakTheme::listThemeCursors_currentTextChanged);
}
TweakTheme::TweakTheme(Ui::Tweak *ui, bool verbose, TweakXfcePanel *tweak, QObject *parent) noexcept
    : TweakTheme(ui, verbose, Xfce, parent)
{
    tweakXfcePanel = tweak;
    assert(tweakXfcePanel != nullptr);
}

void TweakTheme::setup() noexcept
{
    ui->pushThemeApply->setEnabled(false);

    //reset all checkboxes to unchecked

    if (desktop==Xfce || desktop==Fluxbox){
        populateThemeLists(u"gtk-3.0"_s);
        populateThemeLists(u"icons"_s);
        populateThemeLists(u"cursors"_s);
        getCursorSize();
    }

    if (desktop==Xfce){
        populateThemeLists(u"xfwm4"_s);
    } else if (desktop==Fluxbox) {
        populateThemeLists(u"fluxbox"_s);
    }

    if (desktop==Plasma) {
        ui->labelThemeWidget->setText(tr("Plasma Widget Themes","theme style of the kde plasma widgets"));
        ui->labelThemeWindow->setText(tr("Color Schemes", "plasma widget color schemes"));
        ui->groupTheme->setTitle(tr("Plasma Look & Feel Global Themes", "plasma global themes"));
        populateThemeLists(u"plasma"_s);
        populateThemeLists(u"colorscheme"_s);
        populateThemeLists(u"kdecursors"_s);
        populateThemeLists(u"icons"_s);
        ui->pushThemeSaveSet->hide();
        ui->labelThemeCursorSize->hide();
        ui->spinThemeCursorSize->hide();
    }
}

void TweakTheme::setupComboTheme() noexcept
{
    QStringList theme_list;
    ui->comboTheme->clear();
    if (desktop == Xfce) {
        //build theme list
        QString home_path = QDir::homePath();
        if (verbose) qDebug() << "home path is " << home_path;
        bool xsettings_gtk_theme_present = false;
        bool icontheme_present = false;
        bool xfwm4_theme_present = false;
        QStringList filter(u"*.tweak"_s);
        QDirIterator it(u"/usr/share/mx-tweak-data"_s, filter, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            xsettings_gtk_theme_present = false;
            icontheme_present = false;
            xfwm4_theme_present = false;
            QFileInfo file_info(it.next());
            QString filename = file_info.absoluteFilePath();
            QString name = runCmd("cat '"_L1 + filename + "'|grep ^Name="_L1).output.section('=',1,1);
            QString xsettings_gtk_theme = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xsettings_gtk_theme="_L1).output.section('=',1,1);
            if (verbose) qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
            QString xsettings_icon_theme = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xsettings_icon_theme="_L1).output.section('=',1,1);
            if (verbose) qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
            QString xfwm4_window_decorations = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xfwm4_window_decorations="_L1).output.section('=',1,1);
            if (verbose) qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;

            //check theme existence, only list if all 3 elements present
            QFileInfo xsettings_theme("/usr/share/themes/"_L1 + xsettings_gtk_theme);
            QFileInfo xfwm4_theme("/usr/share/themes/"_L1 + xfwm4_window_decorations);
            QFileInfo icon_theme("/usr/share/icons/"_L1 + xsettings_icon_theme);
            QFileInfo xsettings_theme_home(home_path + "/.themes/"_L1 + xsettings_gtk_theme);
            QFileInfo xfwm4_theme_home(home_path + "/.themes/"_L1 + xfwm4_window_decorations);
            QFileInfo icon_theme_home(home_path + "/.icons/"_L1 + xsettings_icon_theme);
            QFileInfo xsettings_theme_home_alt(home_path + "/.local/share/themes/"_L1 + xsettings_gtk_theme);
            QFileInfo xfwm4_theme_home_alt(home_path + "/.local/share/themes/"_L1 + xfwm4_window_decorations);
            if (verbose) qDebug() << "xsettings_theme_home path" << xsettings_theme_home.absoluteFilePath();

            if ( xsettings_theme.exists() || xsettings_theme_home.exists() || xsettings_theme_home_alt.exists()) {
                xsettings_gtk_theme_present = true;
                if (verbose) qDebug() << "xsettings_gtk_theme_present" << xsettings_gtk_theme_present;
            }

            if ( xfwm4_theme.exists() || xfwm4_theme_home.exists() || xfwm4_theme_home_alt.exists()) {
                xfwm4_theme_present = true;
            }

            if ( icon_theme.exists() || icon_theme_home.exists() || xfwm4_theme_home_alt.exists()) {
                icontheme_present = true;
            }

            if ( xsettings_gtk_theme_present && xfwm4_theme_present && icontheme_present ) {
                if (verbose) qDebug() << "filename is " << filename;
                if (verbose) qDebug()<< "theme combo name" << name;
                theme_list << name;
                theme_info.insert(name,filename);
                if (verbose) qDebug() << "theme info hash value is" << name << " " << theme_info[name];
            }
        }
        theme_list.sort();
        theme_list.insert(0, tr("Choose a theme set"));

        //add user entries in ~/.local/share/mx-tweak-data

        QDirIterator it2(home_path + "/.local/share/mx-tweak-data"_L1, filter, QDir::Files, QDirIterator::Subdirectories);
        while (it2.hasNext()) {
            xsettings_gtk_theme_present = false;
            icontheme_present = false;
            xfwm4_theme_present = false;
            QString home_path = QDir::homePath();
            QFileInfo file_info(it2.next());
            QString filename = file_info.absoluteFilePath();
            QString name = runCmd("cat '"_L1 + filename + "'|grep ^Name="_L1).output.section('=',1,1);

            QString xsettings_gtk_theme = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xsettings_gtk_theme="_L1).output.section('=',1,1);
            if (verbose) qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
            QString xsettings_icon_theme = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xsettings_icon_theme="_L1).output.section('=',1,1);
            if (verbose) qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
            QString xfwm4_window_decorations = runCmd("cat '"_L1 + file_info.absoluteFilePath() + "' |grep xfwm4_window_decorations="_L1).output.section('=',1,1);
            if (verbose) qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;

            //check theme existence, only list if all 3 elements present
            QFileInfo xsettings_theme("/usr/share/themes/"_L1 + xsettings_gtk_theme);
            QFileInfo xfwm4_theme("/usr/share/themes/"_L1 + xfwm4_window_decorations);
            QFileInfo icon_theme("/usr/share/icons/"_L1 + xsettings_icon_theme);
            QFileInfo xsettings_theme_home(home_path + "/.themes/"_L1 + xsettings_gtk_theme);
            QFileInfo xfwm4_theme_home(home_path + "/.themes/"_L1 + xfwm4_window_decorations);
            QFileInfo icon_theme_home(home_path + "/.icons/"_L1 + xsettings_icon_theme);
            QFileInfo xsettings_theme_home_alt(home_path + "/.local/share/themes/"_L1 + xsettings_gtk_theme);
            QFileInfo xfwm4_theme_home_alt(home_path + "/.local/share/themes/"_L1 + xfwm4_window_decorations);
            QFileInfo icon_theme_home_alt(home_path + "/.local/share/icons/"_L1 + xsettings_icon_theme);
            if (verbose) qDebug() << "xsettings_theme_home path" << xsettings_theme_home.absoluteFilePath();

            if (xsettings_theme.exists() || xsettings_theme_home.exists() || xsettings_theme_home_alt.exists()) {
                xsettings_gtk_theme_present = true;
            }

            if (xfwm4_theme.exists() || xfwm4_theme_home.exists() || xfwm4_theme_home_alt.exists()) {
                xfwm4_theme_present = true;
            }

            if (icon_theme.exists() || icon_theme_home.exists() || icon_theme_home_alt.exists()) {
                icontheme_present = true;
            }

            if (xsettings_gtk_theme_present && xfwm4_theme_present && icontheme_present) {
                if (verbose) qDebug() << "filename is " << filename;
                if (verbose) qDebug()<< "theme combo name" << name;
                theme_list << name;
                theme_info.insert(name,filename);
                if (verbose) qDebug() << "theme info hash value is" << name << " " << theme_info[name];
            }
        }
    }
    QString current;
    if (desktop == Plasma){
        QString themes = runCmd(u"plasma-apply-lookandfeel --list"_s).output;
        themes.append("\n");
        theme_list = themes.split(u"\n"_s);
        current = runCmd(u"grep LookAndFeelPackage $HOME/.config/kdeglobals"_s).output.section('=',1,1);
        if (verbose) qDebug() << "current is " << current;
    }

    ui->comboTheme->addItems(theme_list);
    if (current.isEmpty()){
        ui->comboTheme->setCurrentIndex(0);
    } else {
        ui->comboTheme->setCurrentText(current);
    }
}

void TweakTheme::getCursorSize() noexcept
{
    ui->spinThemeCursorSize->blockSignals(true);
    if (desktop == Fluxbox) {
        const QString &file = QDir::homePath() + "/.Xresources"_L1;
        if (QFile::exists(file)){
            const QString &size = runCmd("grep Xcursor.size "_L1
                + file).output.section(':',1,1).simplified();
            ui->spinThemeCursorSize->setValue(size.toInt());
        } else {
            ui->spinThemeCursorSize->setValue(0);
        }
    } else if (desktop == Xfce){
        const QString &size = runCmd(u"xfconf-query --channel xsettings"_s
            u" --property /Gtk/CursorThemeSize"_s).output.simplified();
        ui->spinThemeCursorSize->setValue(size.toInt());
    }
    ui->spinThemeCursorSize->blockSignals(false);
}

void TweakTheme::populateThemeLists(const QString &value) noexcept
{
    flags.theme = false;
    QString home_path = QDir::homePath();
    QString themes;
    QStringList themelist;
    QString current;

    if (value == "plasma"_L1){
        themes = runCmd(u"LANG=C plasma-apply-desktoptheme --list-themes |grep \"*\" |cut -d\"*\" -f2"_s).output;
        themes.append("\n");
    }
    if (value == "colorscheme"_L1){
        themes = runCmd(u"LANG=C plasma-apply-colorscheme --list-schemes |grep \"*\" |cut -d\"*\" -f2 "_s).output;
        themes.append("\n");
    }
    if (value == "kdecursors"_L1){
        themes = runCmd(u"LANG=C plasma-apply-cursortheme --list-themes | grep \"*\""_s).output;
        themes.append("\n");
    }
    if (value == "gtk-3.0"_L1 || value == "xfwm4"_L1) {
        themes = runCmd("find /usr/share/themes/*/"_L1 + value + " -maxdepth 0 2>/dev/null|cut -d\"/\" -f5"_L1).output;
        themes.append("\n");
        themes.append(runCmd("find $HOME/.themes/*/"_L1 + value + " -maxdepth 0 2>/dev/null|cut -d\"/\" -f5"_L1).output);
        themes.append("\n");
        themes.append(runCmd("find $HOME/.local/share/themes/*/"_L1 + value + " -maxdepth 0 2>/dev/null|cut -d\"/\" -f7"_L1).output);
    }

    if (value == "icons"_L1){
        themes = runCmd(u"find /usr/share/icons/*/index.theme -maxdepth 1 2>/dev/null|cut -d\"/\" -f5"_s).output;
        themes.append("\n");
        themes.append(runCmd(u"find $HOME/.icons/*/index.theme -maxdepth 1 2>/dev/null|cut -d\"/\" -f5"_s).output);
        themes.append("\n");
        themes.append(runCmd(u"find $HOME/.local/share/icons/*/index.theme -maxdepth 1 2>/dev/null|cut -d\"/\" -f7"_s).output);
    }

    if (value == "fluxbox"_L1) {
        themes = runCmd(u"find /usr/share/mxflux/styles/ ! -name *attribution* ! -iname *README* -maxdepth 1 2>/dev/null |cut -d\"/\" -f6"_s).output;
        themes.append("\n");
        themes.append(runCmd(u"find $HOME/.fluxbox/styles/ ! -name *attribution* ! -iname *README* -maxdepth 1 2>/dev/null |cut -d\"/\" -f6"_s).output);
        themes.append("\n");
        if (ui->checkThemeFluxboxLegacyStyles->isChecked()) {
            themes.append(runCmd(u"find /usr/share/fluxbox/styles/ ! -name *attribution* ! -iname *README* -maxdepth 1 2>/dev/null |cut -d\"/\" -f6"_s).output);
            themes.append("\n");
        }
    }

    if (value == "cursors"_L1){
        themes = runCmd(u"find /usr/share/icons/*/ -maxdepth 1 2>/dev/null |grep cursors |cut -d\"/\" -f5"_s).output;
        themes.append("\n");
        themes.append(runCmd(u"find $HOME/.icons/*/ -maxdepth 1 2>/dev/null |grep cursors |cut -d\"/\" -f5"_s).output);
        themes.append("\n");
        themes.append(runCmd(u"find $HOME/.local/share/icons/*/ -maxdepth 1 2>/dev/null |grep cursors |cut -d\"/\" -f7"_s).output);
        themes.append("\n");
        themes.append("default");
    }



    themelist = themes.split(u"\n"_s, Qt::SkipEmptyParts);
    themelist.removeDuplicates();
    themelist.sort(Qt::CaseInsensitive);

    if (value == "plasma"_L1){
        ui->listThemeWidget->clear();
        QRegularExpression regex(u".*current.*"_s, QRegularExpression::CaseInsensitiveOption);
        int index = themelist.indexOf(regex);
        themelist[index] = themelist[index].section('(',0,0);
        //index of theme in list
        if (verbose) qDebug() << "index is " << index << themelist[index];
        ui->listThemeWidget->addItems(themelist);
        ui->listThemeWidget->setCurrentRow(index);
    }
    if (value == "colorscheme"_L1){
        ui->listThemeWindow->clear();
        QRegularExpression regex(u".*current.*"_s, QRegularExpression::CaseInsensitiveOption);
        int index = themelist.indexOf(regex);
        themelist[index] = themelist[index].section('(',0,0);
        //index of theme in list
        if (verbose) qDebug() << "index is " << index << themelist[index];
        ui->listThemeWindow->addItems(themelist);
        ui->listThemeWindow->setCurrentRow(index);
    }
    if (value == "kdecursors"_L1){
        ui->listThemeCursors->clear();
        QRegularExpression regex(u".*current.*"_s, QRegularExpression::CaseInsensitiveOption);
        int index = themelist.indexOf(regex);
        for (int i = 0; i < themelist.size(); ++i ){
            themelist[i] = themelist[i].section('[',1,1).section(']',0,0);
        }
        //index of theme in list
        if (verbose) qDebug() << "index is " << index << themelist[index];
        ui->listThemeCursors->addItems(themelist);
        ui->listThemeCursors->setCurrentRow(index);
    }

    if ( value == "gtk-3.0"_L1 ) {
        ui->listThemeWidget->clear();
        ui->listThemeWidget->addItems(themelist);
        //set current
        if (desktop == Xfce) {
            current = runCmd(u"xfconf-query -c xsettings -p /Net/ThemeName"_s).output;
        } else if (desktop == Fluxbox) {
            current = runCmd(u"grep gtk-theme-name ~/.config/gtk-3.0/settings.ini | cut -d\"=\" -f2"_s).output;
        }
        //index of theme in list
        ui->listThemeWidget->setCurrentRow(themelist.indexOf(current));
    }
    if ( value == "xfwm4"_L1) {

        ui->listThemeWindow->clear();
        ui->listThemeWindow->addItems(themelist);
        current = runCmd(u"xfconf-query -c xfwm4 -p /general/theme"_s).output;
        ui->listThemeWindow->setCurrentRow(themelist.indexOf(current));
    }

    if ( value == "fluxbox"_L1) {

        ui->listThemeWindow->clear();
        ui->listThemeWindow->addItems(themelist);
        current = runCmd(u"basename $(grep styleFile $HOME/.fluxbox/init |grep -v ^# |grep -oE '/[^ ]+')"_s).output;
        qDebug() << "current style is " << current;
        ui->listThemeWindow->setCurrentRow(themelist.indexOf(current));
    }

    if ( value == "cursors"_L1){
        ui->listThemeCursors->clear();
        ui->listThemeCursors->addItems(themelist);
        if (desktop == Xfce) {
            current = runCmd(u"xfconf-query -c xsettings -p /Gtk/CursorThemeName"_s).output;
            if (current.isEmpty()) current = "default"_L1;
        } else if (desktop == Fluxbox) {
            if (QFile::exists(home_path + "/.icons/default/index.theme"_L1)) {
                current = runCmd(u"grep Inherits $HOME/.icons/default/index.theme |cut -d= -f2"_s).output;
            } else {
                current = "default"_L1;
            }
        }
        ui->listThemeCursors->setCurrentRow(themelist.indexOf(current));
    }

    if ( value == "icons"_L1) {
        if (verbose) qDebug() << "themelist" << themelist;
        QStringList iconthemelist = themelist;
        for (const QString &item : iconthemelist) {
            const QString& icontheme = item;
            if (verbose) qDebug() << "icontheme" << icontheme;
            QString test = runCmd("find /usr/share/icons/"_L1 + icontheme + " -maxdepth 1 -mindepth 1 -type d |cut -d\"/\" -f6"_L1).output;
            if ( test == "cursors"_L1 ) {
                themelist.removeAll(icontheme);
            }
        }
        themelist.removeAll(u"default.kde4"_s);
        themelist.removeAll(u"default"_s);
        themelist.removeAll(u"hicolor"_s);
        ui->listThemeIcons->clear();
        ui->listThemeIcons->addItems(themelist);
        //current icon set
        if (desktop == Xfce) {
            current = runCmd(u"xfconf-query -c xsettings -p /Net/IconThemeName"_s).output;
        } else if (desktop == Fluxbox) {
            current = runCmd(u"grep gtk-icon-theme-name $HOME/.config/gtk-3.0/settings.ini |grep -v ^# | cut -d\"=\" -f2"_s).output;
        } else if (desktop == Plasma) {
            current = QIcon::themeName();
        }
        ui->listThemeIcons->setCurrentRow(themelist.indexOf(current));
    }

    flags.theme = true;
}

void TweakTheme::setTheme(const QString &type, const QString &theme) const noexcept
{   //set new theme
    QString cmd;
    QString cmd1;
    QString cmd2;

    if (desktop == Xfce) {
        if ( type == "gtk-3.0"_L1 ) {
            cmd = "xfconf-query -c xsettings -p /Net/ThemeName -s \""_L1 + theme + '"';
            cmd1 ="gsettings set org.gnome.desktop.interface gtk-theme \""_L1 + theme + '"';
            if (theme.contains("dark"_L1, Qt::CaseInsensitive) || theme.contains("Blackbird"_L1)){ //blackbird special case
                cmd2="gsettings set org.gnome.desktop.interface color-scheme prefer-dark"_L1;
            } else {
                cmd2="gsettings set org.gnome.desktop.interface color-scheme default"_L1;
            }
        }
        if ( type == "xfwm4"_L1 ) {
            cmd = "xfconf-query -c xfwm4 -p /general/theme -s \""_L1 + theme + '"';
        }

        if ( type == "icons"_L1 ) {
            cmd = "xfconf-query -c xsettings -p /Net/IconThemeName -s \""_L1 + theme + '"';
        }

        if (type == "cursor"_L1) {
            cmd = "xfconf-query -c xsettings -p /Gtk/CursorThemeName -s \""_L1 + theme + '"';
        }
        runCmd(cmd);

    } else if (desktop == Plasma) {
        if ( type == "plasma"_L1 ) {
            cmd = "LANG=C plasma-apply-desktoptheme "_L1 + theme;
        }
        if ( type == "colorscheme"_L1 ) {
            cmd = "LANG=C plasma-apply-colorscheme "_L1 + theme;
        }

        if ( type == "icons"_L1 ) {
            cmd = "LANG=C /usr/lib/x86_64-linux-gnu/libexec/plasma-changeicons "_L1 + theme;
        }

        if (type == "kdecursor"_L1) {
            cmd = "LANG=C plasma-apply-cursortheme "_L1 + theme;
        }
        runCmd(cmd);

    } else if (desktop == Fluxbox) {
        QString home_path = QDir::homePath();
        if ( type == "gtk-3.0"_L1 ) {
            if (runCmd(u"grep gtk-theme-name $HOME/.config/gtk-3.0/settings.ini"_s).exitCode == 0) {
                cmd = "sed -i 's/gtk-theme-name=.*/gtk-theme-name="_L1 + theme + "/' $HOME/.config/gtk-3.0/settings.ini"_L1;
            } else {
                cmd = "echo gtk-theme-name="_L1 + theme + "\" >> $HOME/.config/gtk-3.0/settings.ini"_L1;
            }
            runCmd(cmd);

            if (runCmd(u"grep gtk-theme-name $HOME/.gtkrc-2.0"_s).exitCode == 0) {
                cmd = "sed -i 's/gtk-theme-name=.*/gtk-theme-name=\""_L1 + theme + "\"/' $HOME/.gtkrc-2.0"_L1;
            } else {
                cmd = "echo gtk-theme-name=\""_L1 + theme + "\" >> $HOME/.gtkrc-2.0"_L1;
            }
            runCmd(cmd);

            cmd1 ="gsettings set org.gnome.desktop.interface gtk-theme \""_L1 + theme + '"';
            if (theme.contains("dark"_L1, Qt::CaseInsensitive) || theme.contains("Blackbird"_L1)){ //blackbird special case
                cmd2="gsettings set org.gnome.desktop.interface color-scheme prefer-dark"_L1;
                if (runCmd(u"grep gtk-application-prefer-dark-theme $HOME/.config/gtk-3.0/settings.ini"_s).exitCode == 0) {
                    runCmd(u"sed -i 's/gtk-application-prefer-dark-theme=.*/gtk-application-prefer-dark-theme=true/' $HOME/.config/gtk-3.0/settings.ini"_s);
                } else {
                    runCmd(u"echo gtk-application-prefer-dark-theme=true/' >> $HOME/.config/gtk-3.0/settings.ini"_s);
                }
            } else {
                cmd2="gsettings set org.gnome.desktop.interface color-scheme default"_L1;
                if (runCmd(u"grep gtk-application-prefer-dark-theme $HOME/.config/gtk-3.0/settings.ini"_s).exitCode == 0) {
                    runCmd(u"sed -i 's/gtk-application-prefer-dark-theme=.*/gtk-application-prefer-dark-theme=false/' $HOME/.config/gtk-3.0/settings.ini"_s);
                } else {
                    runCmd(u"echo gtk-application-prefer-dark-theme=false/' >> $HOME/.config/gtk-3.0/settings.ini"_s);
                }
            }

            if (QFile::exists(u"/usr/bin/preview-mx"_s)){
                runCmd(u"preview-mx &"_s);
            }
        }
        if ( type == "fluxbox"_L1 ) {
            //always take home folder version, then mx-fluxbox version, then fluxbox version if conflicts arise
            QString filepath = home_path + "/.fluxbox/styles/"_L1 + theme;
            if (QFile(filepath).exists()){
                home_path.replace('/', "\\/"_L1);
                cmd = "sed -i 's/session.styleFile:.*/session.styleFile: "_L1 + home_path + "\\/.fluxbox\\/styles\\/"_L1 + theme + "/' $HOME/.fluxbox/init && fluxbox-remote reconfigure && fluxbox-remote reloadstyle"_L1;
            } else {
                if (QFile::exists("/usr/share/fluxbox/styles/"_L1 + theme)){
                    cmd = "sed -i 's/session.styleFile:.*/session.styleFile: \\/usr\\/share\\/fluxbox\\/styles\\/"_L1 + theme + "/' $HOME/.fluxbox/init && fluxbox-remote reconfigure && fluxbox-remote reloadstyle"_L1;
                }
                if (QFile::exists("/usr/share/mxflux/styles/"_L1 + theme)){
                    cmd = "sed -i 's/session.styleFile:.*/session.styleFile: \\/usr\\/share\\/mxflux\\/styles\\/"_L1 + theme + "/' $HOME/.fluxbox/init && fluxbox-remote reconfigure && fluxbox-remote reloadstyle"_L1;
                }
            }
            runCmd(cmd);
        }
        //for fluxbox, edit ~/.config/gtk-3.0/settings.ini and ~/.gtkrc-2.0 has quotes
        if ( type == "icons"_L1 ) {

            if (runCmd(u"grep gtk-icon-theme-name $HOME/.config/gtk-3.0/settings.ini"_s).exitCode == 0) {
                cmd = "sed -i 's/gtk-icon-theme-name=.*/gtk-icon-theme-name="_L1 + theme + "/' $HOME/.config/gtk-3.0/settings.ini"_L1;
            } else {
                cmd = "echo gtk-icon-theme-name="_L1 + theme + "\" >> $HOME/.config/gtk-3.0/settings.ini"_L1;
            }
            runCmd(cmd);
            if (runCmd(u"grep gtk-icon-theme-name $HOME/.gtkrc-2.0"_s).exitCode == 0) {
                cmd = "sed -i 's/gtk-icon-theme-name=.*/gtk-icon-theme-name=\""_L1 + theme + "\"/' $HOME/.gtkrc-2.0"_L1;
            } else {
                cmd = "echo gtk-icon-theme-name=\""_L1 + theme + "\" >> $HOME/.gtkrc-2.0"_L1;
            }
            runCmd(cmd);

            if (QFile::exists(u"/usr/bin/preview-mx"_s)){
                runCmd(u"preview-mx &"_s);
            }
        }

        //for fluxbox, edit ~/.config/gtk-3.0/settings.ini, ~/.gtkrc-2.0 has quotes, and .icons/default/index.theme (create if it doesn't exist)
        if ( type == "cursor"_L1 ) {

            if (runCmd(u"grep gtk-cursor-theme-name $HOME/.config/gtk-3.0/settings.ini"_s).exitCode == 0) {
                cmd = "sed -i 's/gtk-cursor-theme-name=.*/gtk-cursor-theme-name="_L1 + theme + "/' $HOME/.config/gtk-3.0/settings.ini"_L1;
            } else {
                cmd = "echo gtk-cursor-theme-name="_L1 + theme + "\" >> $HOME/.config/gtk-3.0/settings.ini"_L1;
            }
            runCmd(cmd);
            if (runCmd(u"grep gtk-cursor-theme-name $HOME/.gtkrc-2.0"_s).exitCode == 0) {
                cmd = "sed -i 's/gtk-cursor-theme-name=.*/gtk-cursor-theme-name=\""_L1 + theme + "\"/' $HOME/.gtkrc-2.0"_L1;
            } else {
                cmd = "echo gtk-cursor-theme-name=\""_L1 + theme + "\" >> $HOME/.gtkrc-2.0"_L1;
            }
            runCmd(cmd);
            if ( theme == "default"_L1){
                runCmd(u"rm -R $HOME/.icons/default"_s);
            } else {
                if (!QDir(home_path + "/.icons/default"_L1).exists() ){
                    runCmd(u"mkdir -p $HOME/.icons/default"_s);
                }
                runCmd(u"echo [Icon Theme] > $HOME/.icons/default/index.theme"_s);
                runCmd(u"echo Name=Default >> $HOME/.icons/default/index.theme"_s);
                runCmd(u"echo Comment=Default Cursor Theme >> $HOME/.icons/default/index.theme"_s);
                runCmd(u"echo Comment=Default Cursor Theme >> $HOME/.icons/default/index.theme"_s);
                runCmd("echo Inherits="_L1 + theme + " >> $HOME/.icons/default/index.theme"_L1);
            }
            runCmd(u"fluxbox-remote restart"_s);
        }
    }
    if (!cmd1.isEmpty()){
        runCmd(cmd1);
    }
    if (!cmd2.isEmpty()){
        runCmd(cmd2);
    }
}

void TweakTheme::comboTheme_currentIndexChanged(int index) noexcept
{
    qDebug() << "combo box activated";
    if (desktop == Xfce) {
        if (index != 0) {
            ui->pushThemeApply->setEnabled(true);
        }
    } else if (desktop == Plasma) {
        ui->pushThemeApply->setEnabled(true);
    }
}
void TweakTheme::pushThemeSaveSet_clicked() noexcept
{
    if (desktop == Fluxbox) {
        if (QFile::exists(u"/usr/bin/mxfb-look"_s)) {
            ui->tabWidget->setEnabled(false);
            runCmd(u"/usr/bin/mxfb-look"_s);
            setup();
            ui->tabWidget->setEnabled(true);
            return;
        }
    }
    theming_to_tweak dialog(ui->tabWidget);
    if (dialog.exec() == QDialog::Rejected) {
        return;
    }
    const QString &fileName = dialog.nameEditor()->text();

    //declared locally to prevent an issues with other code
    auto pathAppend = [](const QString& path1, const QString& path2) {
        return QDir::cleanPath(path1 + QDir::separator() + path2);
    };

    QString panel;
    QString data = runCmd(u"xfconf-query -c xfce4-panel -p /panels --list"_s).output;
    int panelNum = 0;
    for (panelNum = 1;; panelNum++) {
        if (data.contains("panel-"_L1 + QString::number(panelNum)))
            break;
    }
    panel = "panel-"_L1 + QString::number(panelNum);

    int backgroundStyle = 0;
    data = runCmd("xfconf-query -c xfce4-panel -p /panels/"_L1 + panel + "/background-style"_L1).output;
    backgroundStyle = data.toInt(); //there may be newlines in output but qt ignores it

    QVector<double> backgroundColor;
    QString backgroundImage;
    backgroundColor.reserve(4);
    if (backgroundStyle == 1) {
        QStringList lines = runCmd("LANG=C xfconf-query -c xfce4-panel -p /panels/"_L1 + panel + "/background-rgba"_L1).output.split('\n');
        lines.removeAt(0);
        lines.removeAt(0);
        for (int i = 0; i < 4; i++)
        {
            backgroundColor << lines.at(i).toDouble();
        }
    } else if (backgroundStyle == 2) {
        backgroundImage = runCmd("xfconf-query -c /panels/"_L1 + panel + "/background-image"_L1).output;
    }

    QString iconThemeName = runCmd(u"xfconf-query -c xsettings -p /Net/IconThemeName"_s).output;
    QString themeName = runCmd(u"xfconf-query -c xsettings -p /Net/ThemeName"_s).output;
    QString windowDecorationsTheme = runCmd(u"xfconf-query -c xfwm4 -p /general/theme"_s).output;
    QString cursorthemename = runCmd(u"xfconf-query -c xsettings -p /Gtk/CursorThemeName"_s).output;

    QString whiskerThemeFileName = pathAppend(QDir::homePath(), u".config/gtk-3.0/whisker-tweak.css"_s);
    QFile whiskerThemeFile(whiskerThemeFileName);
    if (!whiskerThemeFile.open(QFile::ReadOnly | QFile::Text)) {
        if (verbose) qDebug() << "Failed to fetch whisker theming from: "_L1 + whiskerThemeFileName;
    }
    QTextStream whiskerThemeFileStream(&whiskerThemeFile);
    QString whiskerThemeData = whiskerThemeFileStream.readAll();
    whiskerThemeFile.close();

    QStringList fileLines;
    fileLines << "Name="_L1 + fileName;
    fileLines << "background-style="_L1 + QString::number(backgroundStyle);
    if (backgroundStyle == 1) {
        QString line;
        for (double num : std::as_const(backgroundColor)) {
            line.append(QString::number(num) + ',');
        }
        if (line.endsWith(',')) line.chop(1);
        fileLines << "background-rgba="_L1 + line;
    } else {
        fileLines << u"background-rgba=none"_s;
    }
    if (backgroundStyle == 2) {
        fileLines << "background-image="_L1 + backgroundImage;
    } else {
        fileLines << u"background-image=none"_s;
    }
    fileLines << "xsettings_gtk_theme="_L1 + themeName;
    fileLines << "xsettings_icon_theme="_L1 + iconThemeName;
    fileLines << "xfwm4_window_decorations="_L1 + windowDecorationsTheme;
    fileLines << "CursorThemeName="_L1 + cursorthemename;
    fileLines << u"<begin_gtk_whisker_theme_code>"_s;

    {
        const QStringList &themeSplit = whiskerThemeData.split('\n');
        for (const QString &line : themeSplit) {
            fileLines << line;
        }
    }
    fileLines << u"<end_gtk_whisker_theme_code>"_s;
    QFile file(pathAppend(QDir::homePath(), ".local/share/mx-tweak-data/"_L1 + fileName + ".tweak"_L1));
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        if (verbose) qDebug() << "Failed to open file for reading: "_L1 + fileName;
        return;
    }
    QTextStream fileStream(&file);
    for (const QString &line : std::as_const(fileLines)) {
        fileStream << line + '\n';
    }
    file.close();
    //Refresh
    setupComboTheme();
}

void TweakTheme::pushThemeRemoveSet_clicked() noexcept
{
    remove_user_theme_set dialog;
    int result = dialog.exec();
    if (result != QDialog::Accepted)
        return;
    QString theme = dialog.themeSelector()->currentText();
    if (theme == "Select User Theme Set to Remove"_L1)
        return;
    result = QMessageBox::warning(ui->tabWidget, u"Remove User Theme Set"_s,
        "Are you sure you want to remove "_L1 + theme + " theme set?"_L1,
        QMessageBox::Ok | QMessageBox::Cancel);
    if (result != QMessageBox::Ok)
        return;
    QString file = dialog.getFilename(theme);
    file.replace(' ', "\\ "_L1);
    auto cmd = runCmd("rm "_L1 + file);
    if (cmd.exitCode != 0) {
        if (verbose) qDebug() << "Removing theme set failed: exitCode: " << cmd.exitCode << " | output: " << cmd.output;
        return;
    }
    //refresh
    setupComboTheme();
}

void TweakTheme::pushThemeApply_clicked() noexcept
{
    flags.theme = false;
    if (desktop == Xfce){
        ui->pushThemeApply->setEnabled(false);
        QString themename = theme_info.value(ui->comboTheme->currentText());
        QFileInfo fileinfo(themename);
        //initialize variables
        QString backgroundColor = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep background-rgba="_L1).output.section('=' , 1,1);
        if (verbose) qDebug() << "backgroundColor = " << backgroundColor;
        QString color1 = backgroundColor.section(',',0,0);
        QString color2 = backgroundColor.section(',', 1, 1);
        QString color3 = backgroundColor.section(',',2,2);
        QString color4 = backgroundColor.section(',',3,3);
        if (verbose) qDebug() << "sep colors" << color1 << color2 << color3 << color4;
        QString background_image = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep background-image="_L1).output.section('=',1,1);
        if (verbose) qDebug() << "backgroundImage = " << background_image;
        QString background_style = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep background-style="_L1).output.section('=',1,1);
        if (verbose) qDebug() << "backgroundstyle = " << background_style;
        QString xsettings_gtk_theme = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep xsettings_gtk_theme="_L1).output.section('=',1,1);
        if (verbose) qDebug() << "xsettings_gtk_theme = " << xsettings_gtk_theme;
        QString xsettings_icon_theme = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep xsettings_icon_theme="_L1).output.section('=',1,1);
        if (verbose) qDebug() << "xsettings_icon_theme = " << xsettings_icon_theme;
        QString xfwm4_window_decorations = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep xfwm4_window_decorations="_L1).output.section('=',1,1);
        if (verbose) qDebug() << "xfwm4_window_decorations = " << xfwm4_window_decorations;
        QString cursorthemename = runCmd("cat '"_L1 + fileinfo.absoluteFilePath() + "' |grep CursorThemeName="_L1).output.section('=',1,1);
        if (verbose) qDebug() << "CursorThemeName = " << cursorthemename;
        //  use xfconf system to change values

        //set gtk theme
        runCmd("xfconf-query -c xsettings -p /Net/ThemeName -s "_L1 + xsettings_gtk_theme);
        runCmd(u"sleep .5"_s);
        runCmd("gsettings set org.gnome.desktop.interface gtk-theme \""_L1 + xsettings_gtk_theme + '"');
        if (xsettings_gtk_theme.contains("dark"_L1, Qt::CaseInsensitive)){
            runCmd(u"gsettings set org.gnome.desktop.interface color-scheme prefer-dark"_s);
        } else {
            runCmd(u"gsettings set org.gnome.desktop.interface color-scheme default"_s);
        }

        //set window decorations theme
        runCmd("xfconf-query -c xfwm4 -p /general/theme -s "_L1 + xfwm4_window_decorations);
        runCmd(u"sleep .5"_s);

        //set icon theme
        runCmd("xfconf-query -c xsettings -p /Net/IconThemeName -s "_L1 + xsettings_icon_theme);
        runCmd(u"sleep .5"_s);

        //set cursor theme if exists
        if ( ! cursorthemename.isEmpty()){
            runCmd("xfconf-query -c xsettings -p /Gtk/CursorThemeName -s "_L1 + cursorthemename);
        }

        //deal with panel customizations for each panel
        assert(tweakXfcePanel != nullptr);
        QStringListIterator changeIterator(tweakXfcePanel->panelIDs);
        while (changeIterator.hasNext()) {
            QString value = changeIterator.next();

            //set panel background mode
            if (background_style == "1"_L1 || background_style == "2"_L1 || background_style == "0"_L1) {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + value + "/background-style -t int -s "_L1 + background_style + " --create"_L1);
            } else {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + value + "/background-style -t int -s 0 --create"_L1);
            }

            //set panel background image
            QFileInfo image(background_image);
            if (image.exists()) {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + value + "/background-image -t string -s "_L1 + background_image + " --create"_L1);
            } else {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + value + "/background-image --reset"_L1);
            }

            //set panel color
            if (!backgroundColor.isEmpty()) {
                runCmd("xfconf-query -c xfce4-panel -p /panels/panel-"_L1 + value + "/background-rgba -t double -t double -t double -t double -s "_L1 + color1 + " -s "_L1 + color2 + " -s "_L1 + color3 + " -s "_L1 + color4 + " --create"_L1);
            }
        }

        //set whisker themeing
        QString home_path = QDir::homePath();
        QFileInfo whisker_check(home_path + "/.config/gtk-3.0/gtk.css"_L1);
        if (whisker_check.exists()) {
            if (verbose) qDebug() << "existing gtk.css found";
            QString cmd = "cat "_L1 + home_path + "/.config/gtk-3.0/gtk.css |grep -q whisker-tweak.css"_L1;
            if (runCmd(cmd).exitCode == 0 ) {
                if (verbose) qDebug() << "include statement found";
            } else {
                if (verbose) qDebug() << "adding include statement";
                runCmd("echo '@import url(\"whisker-tweak.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1);
            }
        } else {
            if (verbose) qDebug() << "creating simple gtk.css file";
            runCmd("echo '@import url(\"whisker-tweak.css\");' >> "_L1 + home_path + "/.config/gtk-3.0/gtk.css"_L1);
        }

        //add whisker info
        runCmd("awk '/<begin_gtk_whisker_theme_code>/{flag=1;next}/<end_gtk_whisker_theme_code>/{flag=0}flag' \""_L1 +fileinfo.absoluteFilePath() +"\" > "_L1 + home_path + "/.config/gtk-3.0/whisker-tweak.css"_L1);

        //restart xfce4-panel
        runProc(u"xfce4-panel"_s, {u"--restart"_s});
    } else if (desktop == Plasma) {
        runCmd("plasma-apply-lookandfeel --apply "_L1 + ui->comboTheme->currentText());
    }
    setup();
}

void TweakTheme::listThemeWidget_currentTextChanged(const QString &currentText) noexcept
{
    if (flags.theme) {
        if (desktop == Xfce) {
            setTheme(u"gtk-3.0"_s, currentText);
        } else if (desktop == Plasma) {
            setTheme(u"plasma"_s, currentText);
        } else if (desktop == Fluxbox){
            setTheme(u"gtk-3.0"_s, currentText);
        }
    }
}

void TweakTheme::checkThemeFluxboxLegacyStyles_toggled(bool) noexcept
{
    populateThemeLists(u"fluxbox"_s);
}
void TweakTheme::listThemeWindow_currentTextChanged(const QString &currentText) const noexcept
{
    if (flags.theme) {
        if (desktop == Xfce) {
            setTheme(u"xfwm4"_s, currentText);
        } else if (desktop == Plasma) {
            setTheme(u"colorscheme"_s, currentText);
        } else if (desktop == Fluxbox){
            setTheme(u"fluxbox"_s, currentText);
        }
    }
}

void TweakTheme::listThemeIcons_currentTextChanged(const QString &currentText) const noexcept
{
    if (flags.theme) {
        if (desktop == Xfce) {
            setTheme(u"icons"_s, currentText);
        } else if (desktop == Plasma) {
            setTheme(u"icons"_s, currentText);
        } else if (desktop == Fluxbox){
            setTheme(u"icons"_s, currentText);
        }
    }
}

void TweakTheme::listThemeCursors_currentTextChanged(const QString &currentText) noexcept
{
    if (flags.theme) {
        if (desktop == Xfce) {
            setTheme(u"cursor"_s, currentText);
        } else if (desktop == Plasma) {
            setTheme(u"kdecursor"_s, currentText);
        } else if (desktop == Fluxbox){
            setTheme(u"cursor"_s, currentText);
        }
    }
}

void TweakTheme::spinThemeCursorSize_valueChanged(int value) noexcept
{
    const QString &size = QString::number(value);
    QString cmd;

    if (desktop == Fluxbox) {
        QString file = QDir::homePath() + "/.Xresources"_L1;
        if (runCmd("grep Xcursor.size "_L1 + file).exitCode == 0) {
            cmd = "sed -i 's/Xcursor.size:.*/Xcursor.size: "_L1 + size + "/' "_L1 + file;
        } else {
            cmd = "echo Xcursor.size: "_L1 + size + " >"_L1 + file;
        }
    } else if (desktop == Xfce) {
        cmd = ("xfconf-query --channel xsettings --property /Gtk/CursorThemeSize -t int -s "_L1 + size + " --create"_L1);
    }

    runCmd(cmd);
    //restart fluxbox after set
    if (desktop == Fluxbox) {
        runCmd(u"xrdb -merge $HOME/.Xresources && fluxbox-remote restart"_s);
    }
}
