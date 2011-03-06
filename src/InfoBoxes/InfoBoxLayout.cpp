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

#include "InfoBoxes/InfoBoxLayout.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "InfoBoxes/InfoBoxWindow.hpp"
#include "Profile/Profile.hpp"
#include "Sizes.h"

#include <stdio.h>


/*
Screen
640x480 landscape

480/6 = 80 control height

2/3 of width is map = 420
leaving 220 = 110 control width
*/

namespace InfoBoxLayout
{
  Layouts InfoBoxGeometry = ibTop4Bottom4;
  unsigned ControlWidth;
  unsigned ControlHeight;
  unsigned TitleHeight;
  bool fullscreen = false;

  unsigned numInfoWindows = 8;
  RECT positions[InfoBoxPanelConfig::MAX_INFOBOXES];
  RECT remaining;
}

void
InfoBoxLayout::Init(RECT rc)
{
  LoadGeometry(rc.right - rc.left, rc.bottom - rc.top);
  CalcInfoBoxSizes(rc);
  CalcInfoBoxPositions(rc);
}

static int
MakeTopRow(RECT *p, unsigned n, int left, int top)
{
  RECT *const end = p + n;
  const int bottom = top + InfoBoxLayout::ControlHeight;
  while (p < end) {
    p->left = left;
    left += InfoBoxLayout::ControlWidth;
    p->right = left;
    p->top = top;
    p->bottom = bottom;

    ++p;
  }

  return bottom;
}

static int
MakeBottomRow(RECT *p, unsigned n, int left, int bottom)
{
  int top = bottom - InfoBoxLayout::ControlHeight;
  MakeTopRow(p, n, left, top);
  return top;
}

static int
MakeLeftColumn(RECT *p, unsigned n, int left, int top)
{
  RECT *const end = p + n;
  const int right = left + InfoBoxLayout::ControlWidth;
  while (p < end) {
    p->left = left;
    p->right = right;
    p->top = top;
    top += InfoBoxLayout::ControlHeight;
    p->bottom = top;

    ++p;
  }

  return right;
}

static int
MakeRightColumn(RECT *p, unsigned n, int right, int top)
{
  int left = right - InfoBoxLayout::ControlWidth;
  MakeLeftColumn(p, n, left, top);
  return left;
}

void
InfoBoxLayout::CalcInfoBoxPositions(RECT rc)
{
  switch (InfoBoxGeometry) {
  case ibTop4Bottom4:
    assert(numInfoWindows == 8);

    rc.top = MakeTopRow(positions, 4, rc.left, rc.top);
    rc.bottom = MakeBottomRow(positions + 4, 4, rc.left, rc.bottom);
    break;

  case ibBottom8:
    assert(numInfoWindows == 8);

    rc.bottom = MakeBottomRow(positions + 4, 4, rc.left, rc.bottom);
    rc.bottom = MakeBottomRow(positions, 4, rc.left, rc.bottom);
    break;

  case ibTop8:
    assert(numInfoWindows == 8);

    rc.top = MakeTopRow(positions, 4, rc.left, rc.top);
    rc.top = MakeTopRow(positions + 4, 4, rc.left, rc.top);
    break;

  case ibLeft4Right4:
    assert(numInfoWindows == 8);

    rc.left = MakeLeftColumn(positions, 4, rc.left, rc.top);
    rc.right = MakeRightColumn(positions + 4, 4, rc.right, rc.top);
    break;

  case ibLeft8:
    assert(numInfoWindows == 8);

    rc.left = MakeLeftColumn(positions, 4, rc.left, rc.top);
    rc.left = MakeLeftColumn(positions + 4, 4, rc.left, rc.top);
    break;

  case ibRight8:
    assert(numInfoWindows == 8);

    rc.right = MakeRightColumn(positions + 4, 4, rc.right, rc.top);
    rc.right = MakeRightColumn(positions, 4, rc.right, rc.top);
    break;

  case ibRight12:
    assert(numInfoWindows == 12);

    rc.right = MakeRightColumn(positions + 6, 6, rc.right, rc.top);
    rc.right = MakeRightColumn(positions, 6, rc.right, rc.top);
    break;

  case ibGNav:
    assert(numInfoWindows == 9);

    rc.right = MakeRightColumn(positions + 6, 3,
                               rc.right, rc.top + 3 * ControlHeight);
    rc.right = MakeRightColumn(positions, 6, rc.right, rc.top);
    break;

  case ibSquare:
    assert(numInfoWindows == 5);

    rc.right = MakeRightColumn(positions, 5, rc.right, rc.top);
    break;
  };

  remaining = rc;
}

void
InfoBoxLayout::LoadGeometry(unsigned width, unsigned height)
{
  unsigned tmp;
  if (Profile::Get(szProfileInfoBoxGeometry, tmp))
    InfoBoxGeometry = (Layouts)tmp;

  if (width > height) {
    /* landscape */
    if (InfoBoxGeometry < ibLeft4Right4)
      InfoBoxGeometry = ibGNav;
  } else if (width == height) {
    /* square */
    InfoBoxGeometry = ibSquare;
  } else {
    /* portrait */
    if (InfoBoxGeometry >= ibLeft4Right4)
      InfoBoxGeometry = ibTop4Bottom4;
  }

  Profile::Set(szProfileInfoBoxGeometry, (unsigned)InfoBoxGeometry);

  if (InfoBoxGeometry == ibGNav)
    numInfoWindows = 9;
  else if (InfoBoxGeometry == ibRight12)
    numInfoWindows = 12;
  else if (InfoBoxGeometry == ibSquare)
    numInfoWindows = 5;
  else
    numInfoWindows = 8;

  assert(numInfoWindows <= InfoBoxPanelConfig::MAX_INFOBOXES);
}

void
InfoBoxLayout::CalcInfoBoxSizes(RECT rc)
{
  switch (InfoBoxGeometry) {
  case ibTop4Bottom4:
  case ibBottom8:
  case ibTop8:
    // calculate control dimensions
    ControlWidth = 2 * (rc.right - rc.left) / numInfoWindows;
    ControlHeight = (rc.bottom - rc.top) / CONTROLHEIGHTRATIO;
    break;

  case ibLeft4Right4:
  case ibLeft8:
  case ibRight8:
    // calculate control dimensions
    ControlWidth = (rc.right - rc.left) / CONTROLHEIGHTRATIO * 1.3;
    ControlHeight = 2 * (rc.bottom - rc.top) / numInfoWindows;
    break;

  case ibGNav:
  case ibRight12:
    // calculate control dimensions
    ControlHeight = (rc.bottom - rc.top) / 6;
    ControlWidth = ControlHeight * 1.44; // preserve relative shape
    break;

  case ibSquare:
    // calculate control dimensions
    ControlWidth = (rc.right - rc.left) * 0.2;
    ControlHeight = (rc.bottom - rc.top) / 5;
    break;
  }

  TitleHeight = ControlHeight / 3.1;
}
