/**********************************************************************
 *  defaultlook.h
 **********************************************************************
 * Copyright (C) 2015 MX Authors
 *
 * Authors: Dolphin Oracle
 *          MX Linux <http://mxlinux.org>
 *
 * This file is part of mx-tweak.
 *
 * mx-tweak is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mx-tweak is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mx-tweak.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef DEFAULTLOOK_H
#define DEFAULTLOOK_H

#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QHash>

namespace Ui {
class defaultlook;
}

struct Result {
    int exitCode;
    QString output;
};

class defaultlook : public QMainWindow
{
    Q_OBJECT

protected:
    QProcess *proc;
    QTimer *timer;

public:
    explicit defaultlook(QWidget *parent = 0);
    ~defaultlook();
    Result runCmd(QString cmd);
    QString getVersion(QString name);

    QString version;
    QString output;
    QStringList panelIDs;
    QString panel;
    bool message_flag;
    QHash<QString, QString>  theme_info;


    void setup();
    void setupuiselections();
    void setuppanel();
    void setuptheme();
    void setupComboTheme();
    void fliptohorizontal();
    void fliptovertical();
    void whichpanel();
    void message();
    void checkXFCE();

    void backupPanel();
    void restoreDefaultPanel();
    void restoreBackup();

    void top_or_bottom();
    void message2();


public slots:




private slots:
    void on_buttonApply_clicked();

    void on_buttonCancel_clicked();

    void on_buttonAbout_clicked();

    void on_buttonHelp_clicked();

    void on_radioDefaultPanel_clicked();

    void on_radioBackupPanel_clicked();

    void on_radioRestoreBackup_clicked();

    void on_checkHorz_clicked();

    void on_checkVert_clicked();

    void on_checkFirefox_clicked();

    void on_checkHexchat_clicked();

    void on_toolButtonXFCEpanelSettings_clicked();

    void on_toolButtonXFCEAppearance_clicked();

    void on_toolButtonXFCEWMsettings_clicked();

    void on_comboboxHorzPostition_currentIndexChanged(const QString &arg1);

    void on_buttonThemeApply_clicked();

    void on_comboTheme_activated(const QString &arg1);

private:
    Ui::defaultlook *ui;
};

#endif // DEFAULTLOOK_H
