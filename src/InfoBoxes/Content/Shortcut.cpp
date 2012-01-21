/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "InfoBoxes/Content/Shortcut.hpp"
#include "Input/InputEvents.hpp"
#include "Language/Language.hpp"

#include "InfoBoxes/Data.hpp"
#include "Interface.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Engine/Util/Gradient.hpp"
#include "Dialogs/Dialogs.h"
#include "MainWindow.hpp"

#include <stdio.h>
#include <tchar.h>

void
InfoBoxContentMenu::Update(InfoBoxData &data)
{
  data.SetTitle(_T(""));
  data.SetValue(_("Menu"));
}

void
InfoBoxContentMenu::OnClick()
{
  InputEvents::ShowMenu();
}
