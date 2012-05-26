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

#include "Base.hpp"
#include "Form/Button.hpp"
#include "Form/Frame.hpp"
#include "Form/Form.hpp"
#include "Interface.hpp"
#include "Units/Units.hpp"
#include "Units/Group.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Screen/Fonts.hpp"
#include "Language/Language.hpp"
#include "Screen/Layout.hpp"
#include "InfoBoxes/InfoBoxManager.hpp"
#include "MainWindow.hpp"

enum ControlIndex {
  SetUp = 1000,
};

gcc_const
static WindowStyle
GetDialogStyle()
{
  WindowStyle style;
  style.Hide();
  style.ControlParent();
  return style;
}

BaseAccessPanel::BaseAccessPanel(unsigned _id)
  :WndForm(UIGlobals::GetMainWindow(), UIGlobals::GetDialogLook(),
           UIGlobals::GetMainWindow().GetClientRect(),
           _T(""), GetDialogStyle()),

    id(_id) {}

gcc_pure PixelScalar
BaseAccessPanel::GetHeaderHeight() {
  return Layout::Scale(20);
}

void
BaseAccessPanel::Close()
{
  SetModalResult(mrOK);
}

void
BaseAccessPanel::OnPaint(Canvas &canvas)
{
  WndForm::OnPaint(canvas);
}

void
BaseAccessPanel::Hide()
{
  Close();
}

void
BaseAccessPanel::OnAction(int action_id)
{
  if (action_id == SetUp) {
    InfoBoxManager::ShowInfoBoxPicker(id);
    Close();
  }
}
void
BaseAccessPanel::Show(const PixelRect &rc)
{
  WndForm::ShowModeless();
}

void
BaseAccessPanel::Unprepare()
{
  delete header_text;
  delete setup_button;
  NullWidget::Unprepare();
}

void
BaseAccessPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{

  NullWidget::Prepare(parent, rc);
  WndForm::Move(rc);

  const DialogLook &look = UIGlobals::GetDialogLook();

  PixelRect base_rc;
  base_rc.left = base_rc.top = 0;
  base_rc.right = rc.right - rc.left;
  base_rc.bottom = rc.bottom - rc.top;

  PixelScalar setup_button_width = 0.2 * (rc.right - rc.left);
  PixelRect setup_button_rc = base_rc;
  setup_button_rc.left = setup_button_rc.right - setup_button_width;
  setup_button_rc.bottom = setup_button_rc.top + GetHeaderHeight();

  ButtonWindowStyle button_style;
  button_style.TabStop();
  button_style.multiline();
  setup_button = new WndButton(GetClientAreaWindow(), look,
                               _("Setup"), setup_button_rc,
                               button_style,
                               this, SetUp);

  WindowStyle style_frame;
  PixelRect frame_rc = base_rc;
  frame_rc.right = setup_button_rc.left - 1;
  frame_rc.bottom = setup_button_rc.bottom;
  header_text = new WndFrame(GetClientAreaWindow(), look,
                             frame_rc.left, frame_rc.top,
                             frame_rc.right - frame_rc.left,
                             frame_rc.bottom - frame_rc.top,
                             style_frame);
  header_text->SetVAlignCenter();
  header_text->SetFont(Fonts::infobox_small);

  StaticString<32> buffer;
  buffer = gettext(InfoBoxFactory::GetName(
      InfoBoxManager::GetType(id, InfoBoxManager::GetCurrentPanel())));
  header_text->SetText(buffer);

  content_rc = base_rc;
  content_rc.top += GetHeaderHeight();
}
