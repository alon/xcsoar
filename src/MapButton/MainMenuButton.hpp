/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
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

#ifndef XCSOAR_MAIN_MENU_BUTTON_HPP
#define XCSOAR_MAIN_MENU_BUTTON_HPP

#include "MapButton/MapButton.hpp"
#include "Screen/Point.hpp"
#include "PeriodClock.hpp"
#include "Util/StaticString.hpp"

class Canvas;

/**
 * Customizes MapButton with functionality in InputEvents
 * Renders the "M" icon on the map that is clickable
 * and triggers the MainMenu display.
 * Also shows the Help overlay in the middle of the screen
 * until the menu is clicked or 30 seconds passes
 */
class MainMenuButton : public MapButton {
private:

  PeriodClock clock;
  StaticString <40> help_line1;
  StaticString <40> help_line2;
  StaticString <4> m_text;

public:
  MainMenuButton();

  /**
   * draws the "M" menu button
   * Also sets width and height of button size
   * @param canvas
   * @param rc
   */
  virtual void Draw(Canvas &canvas, const PixelRect &rc);

  /** draws menu help message for first 30 seconds or until shown once
   * Looks at InputEvents to see if menu has never been shown
   * @param canvas
   * @param rc
   */
  void DrawMenuButtonHelp(Canvas &canvas, const PixelRect &rc) const;
};

#endif
