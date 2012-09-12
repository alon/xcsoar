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

#include "AircraftLook.hpp"
#include "Screen/Layout.hpp"

void
AircraftLook::Initialise(const ColorTheme &color_theme)
{
  aircraft_pen.Set(1, color_theme.dark_gray);
  aircraft_simple1_pen.Set(Layout::ScalePenWidth(1), COLOR_BLACK);
  aircraft_simple2_pen.Set(Layout::ScalePenWidth(3), COLOR_WHITE);

  canopy_pen.Set(1, DarkColor(color_theme.cyan));
  canopy_brush.Set(color_theme.cyan);
}
