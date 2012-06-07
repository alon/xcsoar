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

#ifndef XCSOAR_INFO_BOX_MANAGER_HPP
#define XCSOAR_INFO_BOX_MANAGER_HPP

#include "Screen/PaintWindow.hpp"
#include "InfoBoxes/Content/Base.hpp"
#include "InfoBoxes/Content/Factory.hpp"
#include "Profile/InfoBoxConfig.hpp"

struct InfoBoxSettings;

struct InfoBoxLook;
struct UnitsLook;
struct ButtonLook;
class InfoBoxWindow;

namespace InfoBoxLayout {
  struct Layout;
};

namespace InfoBoxManager
{
  enum PanelSelection {
    PANEL_CIRCLING,
    PANEL_CRUISE,
    PANEL_FINAL_GLIDE,
    PANEL_AUXILIARY,
  };

  extern InfoBoxLayout::Layout layout;

  void Event_Select(int i);
  void Event_Change(int i);

  /**
   * ProcessQuickAccess takes the id of the InfoBox where to pass the
   * value Value. It doesn't expect the target InfoBox to be focussed.
   * @param id
   * @param Value
   */
  void ProcessQuickAccess(const int id, const TCHAR *Value);

  /**
   * GetQuickAccess takes the id of the InfoBox and returns the value
   * determined by the InfoBox. It doesn't expect the target InfoBox to be focussed.
   * @param id
   * @return value set by InfoBoxContent or 0 if no content exists.
   */
  gcc_pure unsigned GetQuickAccess(const int id);

  bool Click(InfoBoxWindow &ib);

  void ProcessTimer();
  void SetDirty();

  void Create(PixelRect rc, const InfoBoxLayout::Layout &layout,
              const InfoBoxLook &look, const UnitsLook &units_look,
              const ButtonLook &button_look);
  void Destroy();
  void Show();
  void Hide();

  const InfoBoxContent::DialogContent *GetDialogContent(const int id);

  unsigned GetCurrentPanel();
  const TCHAR* GetCurrentPanelName();
  const TCHAR* GetPanelName(unsigned panel);

  /**
   * returns ID InfoBox access dialog currently being shown
   * or -1 if none is currently shown
   */
  int GetVisibleAccessPopupID();

  /**
   * sets the ID of the InfoBox access dialog that will be
   * displayed, or -1 if none shall be displayed
   */
  void SetVisibleAccessPopupID(int id);

  InfoBoxFactory::t_InfoBox GetType(unsigned box, unsigned panel);
  const TCHAR* GetTitle(unsigned box);

  bool IsEmpty(unsigned panel);

  bool HasFocus();

  void ShowDlgInfoBox(const int id);

  /**
   * Opens a dialog to select the InfoBox contents for
   * the InfoBox indicated by id, or the focused InfoBox.
   * @param id The id of the InfoBox to configure.  If negative,
   * then it configures the focused InfoBox if there is one.
   */
  void ShowInfoBoxPicker(const int id = -1);
};

#endif
