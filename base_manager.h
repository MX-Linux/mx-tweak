/**********************************************************************
 *  base_manager.h
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

#include <QObject>
#include <memory>

class QDialog;
class defaultlook;

namespace Ui
{
class defaultlook;
}

struct DesktopEnvironment {
    bool isXfce {false};
    bool isFluxbox {false};
    bool isKDE {false};
    bool isLightdm {false};
    bool isSuperkey {false};
};

class BaseManager : public QObject
{
    Q_OBJECT

public:
    explicit BaseManager(Ui::defaultlook *ui, bool verbose, QDialog *parent = nullptr);
    virtual ~BaseManager() = default;

    virtual void setup() = 0;

protected:
    Ui::defaultlook *ui;
    bool verbose;
    QDialog *parent;
    DesktopEnvironment desktop;

    void setDesktopEnvironment(DesktopEnvironment env);
    const DesktopEnvironment &getDesktopEnvironment() const;

signals:
    void setupCompleted();
    void errorOccurred(const QString &error);
};

