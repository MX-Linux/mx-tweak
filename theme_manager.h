/**********************************************************************
 *  theme_manager.h
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
#include <QHash>
#include <QStringList>

class ThemeManager : public BaseManager
{
    Q_OBJECT

public:
    explicit ThemeManager(Ui::defaultlook *ui, bool verbose, QDialog *parent = nullptr);

    void setup() override;
    void setupComboTheme();
    void populateThemeLists(const QString &value);
    static void setTheme(const QString &type, const QString &theme, const QString &desktop);

    void saveThemeUndo();
    void themeUndo();
    void getCursorSize();
    void setCursorSize();

public slots:
    void onButtonThemeApplyClicked();
    void onButtonThemeUndoClicked();
    void onComboThemeActivated(int index);
    void onListWidgetThemeCurrentTextChanged(const QString &currentText);
    void onListWidgetWMthemeCurrentTextChanged(const QString &currentText);
    void onListWidgetIconsCurrentTextChanged(const QString &currentText);
    void onListWidgetCursorThemesCurrentTextChanged(const QString &currentText);
    void onPushButtonPreviewClicked();
    void onPushButtonSettingsToThemeSetClicked();
    void onPushButtonRemoveUserThemeSetClicked();
    void onSpinBoxPointerSizeValueChanged(int value);

private:
    QHash<QString, QString> themeInfo;
    QStringList undoTheme;
    bool themeFlag {false};
    bool cursorSizeFlag {false};

    void setupThemeLists();
    void setupThemeButtons();
    void applyCurrentTheme();
};

