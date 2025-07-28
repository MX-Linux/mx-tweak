/**********************************************************************
 *  panel_manager.h
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

#pragma once

#include "base_manager.h"
#include <QStringList>

class PanelManager : public BaseManager
{
    Q_OBJECT

public:
    explicit PanelManager(Ui::defaultlook *ui, bool verbose, QDialog *parent = nullptr);

    void setup() override;
    void backupPanel();
    void restoreBackup();
    static void restoreDefaultPanel();
    void migratePanel(const QString &date) const;
    int validateArchive(const QString &path) const;

    void whichPanel();
    void flipToHorizontal();
    void flipToVertical();
    void topOrBottom();
    void leftOrRight();

    QString getTasklistId();
    void tasklistChange();

public slots:
    void onApplyButtonClicked();
    void onCheckHorzClicked();
    void onCheckVertClicked();
    void onRadioDefaultPanelClicked();
    void onRadioBackupPanelClicked();
    void onRadioRestoreBackupClicked();
    void onRadioButtonTasklistClicked();
    void onRadioButtonSetPanelPluginScalesClicked();
    void onComboboxHorzPositionChanged(int index);
    void onComboboxVertPositionChanged(int index);
    void onComboBoxTasklistPluginChanged(int index);
    void onLineEditBackupNameReturnPressed();

private:
    QStringList panelIDs;
    QString panel;
    QString pluginIdSystray;
    QString pluginTasklist;
    bool panelFlag {false};
    bool tasklistFlag {false};
    bool validateFlag {false};

    void setupPanelBackupRestore();
    void setupPanelOrientation();
    void setupTasklistPlugin();
};

