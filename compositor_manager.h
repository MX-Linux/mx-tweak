/**********************************************************************
 *  compositor_manager.h
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

class CompositorManager : public BaseManager
{
    Q_OBJECT

public:
    explicit CompositorManager(Ui::defaultlook *ui, bool verbose, QDialog *parent = nullptr);

    void setup() override;
    void checkComptonRunning();
    void checkAptNotifierRunning() const;

public slots:
    void onButtonCompositorApplyClicked();
    void onButtonConfigureComptonClicked();
    void onButtonConfigureXfwmClicked();
    void onButtonEditComptonConfClicked();
    void onComboBoxCompositorCurrentIndexChanged(int index);
    void onComboBoxVblankActivated(int index);

private:
    QString vblankInitial;
    bool vblankFlag {false};
    bool setupFlag {false};

    void setupVblankSettings();
    void setupCompositorButtons();
    void enableCompositorByIndex(int index);
};

