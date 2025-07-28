/**********************************************************************
 *  base_manager.cpp
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

#include "base_manager.h"
#include <QDialog>

BaseManager::BaseManager(Ui::defaultlook *ui, bool verbose, QDialog *parent)
    : QObject(parent),
      ui(ui),
      verbose(verbose),
      parent(parent)
{
}

void BaseManager::setDesktopEnvironment(DesktopEnvironment env)
{
    desktop = env;
}

const DesktopEnvironment &BaseManager::getDesktopEnvironment() const
{
    return desktop;
}
