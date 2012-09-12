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

#include "TaskCalculatorPanel.hpp"
#include "Internal.hpp"
#include "Dialogs/Task.hpp"
#include "Interface.hpp"
#include "Units/Units.hpp"
#include "Formatter/UserUnits.hpp"
#include "Form/Draw.hpp"
#include "Form/Button.hpp"
#include "Form/DataField/Float.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Components.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Icon.hpp"
#include "UIGlobals.hpp"
#include "Look/DialogLook.hpp"
#include "Look/MapLook.hpp"
#include "Language/Language.hpp"

enum Controls {
  WARNING,
  AAT_TIME,
  AAT_ESTIMATED,
  DISTANCE,
  MC,
  RANGE,
  SPEED_REMAINING,
  EFFECTIVE_MC,
  SPEED_ACHIEVED,
  CRUISE_EFFICIENCY,
};

class WndButton;

/** XXX this hack is needed because the form callbacks don't get a
    context pointer - please refactor! */
static TaskCalculatorPanel *instance;

void
TaskCalculatorPanel::GetCruiseEfficiency()
{
  cruise_efficiency = XCSoarInterface::Calculated().task_stats.cruise_efficiency;
}

void
TaskCalculatorPanel::Refresh()
{
  const CommonStats &common_stats = CommonInterface::Calculated().common_stats;
  const TaskStats &task_stats = CommonInterface::Calculated().task_stats;

  if (target_button != NULL)
    target_button->SetVisible(common_stats.ordered_has_targets);

  LoadValue(AAT_ESTIMATED, (common_stats.task_time_remaining
                            + common_stats.task_time_elapsed) / 60);

  SetRowVisible(AAT_TIME, task_stats.has_targets);
  if (task_stats.has_targets)
    LoadValue(AAT_TIME,
              protected_task_manager->GetOrderedTaskBehaviour().aat_min_time / 60);

  fixed rPlanned = task_stats.total.solution_planned.IsDefined()
    ? task_stats.total.solution_planned.vector.distance
    : fixed_zero;

  if (positive(rPlanned))
    LoadValue(DISTANCE, rPlanned, UnitGroup::DISTANCE);
  else
    ClearValue(DISTANCE);

  LoadValue(MC, CommonInterface::GetComputerSettings().polar.glide_polar_task.GetMC(),
            UnitGroup::VERTICAL_SPEED);
  LoadValue(EFFECTIVE_MC, emc, UnitGroup::VERTICAL_SPEED);

  if (positive(rPlanned)) {
    fixed rMax = task_stats.distance_max;
    fixed rMin = task_stats.distance_min;

    if (rMin < rMax) {
      fixed range = Double((rPlanned - rMin) / (rMax - rMin)) - fixed_one;
      LoadValue(RANGE, range * 100);
    } else
      ClearValue(RANGE);
  } else
    ClearValue(RANGE);

  if (task_stats.total.remaining_effective.IsDefined())
    LoadValue(SPEED_REMAINING, task_stats.total.remaining_effective.GetSpeed(),
              UnitGroup::TASK_SPEED);
  else
    ClearValue(SPEED_REMAINING);

  if (task_stats.total.travelled.IsDefined())
    LoadValue(SPEED_ACHIEVED, task_stats.total.travelled.GetSpeed(),
              UnitGroup::TASK_SPEED);
  else
    ClearValue(SPEED_ACHIEVED);

  LoadValue(CRUISE_EFFICIENCY, task_stats.cruise_efficiency * 100);
}

static void
OnTargetClicked(gcc_unused WndButton &Sender)
{
  dlgTargetShowModal();
}

void
TaskCalculatorPanel::OnModified(DataField &df)
{
  if (IsDataField(MC, df)) {
    const DataFieldFloat &dff = (const DataFieldFloat &)df;
    fixed mc = Units::ToSysVSpeed(dff.GetAsFixed());
    ActionInterface::SetManualMacCready(mc);
    Refresh();
  } else if (IsDataField(CRUISE_EFFICIENCY, df)) {
    const DataFieldFloat &dff = (const DataFieldFloat &)df;
    SetCruiseEfficiency(dff.GetAsFixed() / 100);
  }
}

void
TaskCalculatorPanel::OnSpecial(DataField &df)
{
  if (IsDataField(MC, df)) {
    const DerivedInfo &calculated = CommonInterface::Calculated();
    if (positive(calculated.time_climb)) {
      fixed mc = calculated.total_height_gain / calculated.time_climb;
      DataFieldFloat &dff = (DataFieldFloat &)df;
      dff.Set(Units::ToUserVSpeed(mc));
      ActionInterface::SetManualMacCready(mc);
      Refresh();
    }
  } else if (IsDataField(CRUISE_EFFICIENCY, df)) {
    GetCruiseEfficiency();
  }
}

static void
OnWarningPaint(gcc_unused WndOwnerDrawFrame *Sender, Canvas &canvas)
{
  const DialogLook &look = UIGlobals::GetDialogLook();

  if (instance->IsTaskModified()) {
    const UPixelScalar textheight = canvas.GetFontHeight();
    const TCHAR* message = _("Calculator excludes unsaved task changes!");
    canvas.Select(*look.small_font);

    const AirspaceLook &look = UIGlobals::GetMapLook().airspace;
    const MaskedIcon *bmp = &look.intercept_icon;
    const int offsetx = bmp->GetSize().cx;
    const int offsety = canvas.get_height() - bmp->GetSize().cy;
    canvas.Clear(color_theme->yellow);
    bmp->Draw(canvas, offsetx, offsety);

    canvas.SetBackgroundColor(color_theme->yellow);
    canvas.SetTextColor(COLOR_BLACK);
    canvas.text(offsetx * 2 + Layout::Scale(2),
                (int)(canvas.get_height() - textheight) / 2,
                message);
  }
  else {
    canvas.Clear(look.background_color);
  }
}

void
TaskCalculatorPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  assert(protected_task_manager != NULL);

  instance = this;

  if (target_button != NULL)
    target_button->SetOnClickNotify(OnTargetClicked);

  Add(new WndOwnerDrawFrame(*(ContainerWindow *)GetWindow(),
                            PixelRect{0, 0, 100, Layout::Scale(17)},
                            WindowStyle(), OnWarningPaint));

  AddReadOnly(_("Assigned task time"), NULL, _T("%.0f min"), fixed_zero);
  AddReadOnly(_("Estimated task time"), NULL, _T("%.0f min"), fixed_zero);
  AddReadOnly(_("Task distance"), NULL, _T("%.0f %s"),
              UnitGroup::DISTANCE, fixed_zero);

  AddFloat(_("Set MacCready"),
           _("Adjusts MC value used in the calculator.  "
             "Use this to determine the effect on estimated task time due to changes in conditions.  "
             "This value will not affect the main computer's setting if the dialog is exited with the Cancel button."),
           _T("%.1f %s"), _T("%.1f"),
           fixed_zero, Units::ToUserVSpeed(fixed(5)),
           GetUserVerticalSpeedStep(), false, fixed_zero,
           this);
  DataFieldFloat &mc_df = (DataFieldFloat &)GetDataField(MC);
  mc_df.SetFormat(GetUserVerticalSpeedFormat(false, false));

  AddReadOnly(_("AAT range"),
              /* xgettext:no-c-format */
              _("For AAT tasks, this value tells you how far based on the targets of your task you will fly relative to the minimum and maximum possible tasks. -100% indicates the minimum AAT distance.  0% is the nominal AAT distance.  +100% is maximum AAT distance."),
              _T("%.0f %%"), fixed_zero);

  AddReadOnly(_("Speed remaining"), NULL, _T("%.0f %s"),
              UnitGroup::TASK_SPEED, fixed_zero);

  AddReadOnly(_("Achieved MacCready"), NULL, _T("%.1f %s"),
              UnitGroup::VERTICAL_SPEED, fixed_zero);
  DataFieldFloat &emc_df = (DataFieldFloat &)GetDataField(EFFECTIVE_MC);
  emc_df.SetFormat(GetUserVerticalSpeedFormat(false, false));

  AddReadOnly(_("Achieved speed"), NULL, _T("%.0f %s"),
              UnitGroup::TASK_SPEED, fixed_zero);

  AddFloat(_("Cruise efficiency"),
           _("Efficiency of cruise.  100 indicates perfect MacCready performance, greater than 100 indicates better than MacCready performance is achieved through flying in streets.  Less than 100 is appropriate if you fly considerably off-track.  This value estimates your cruise efficiency according to the current flight history with the set MC value.  Calculation begins after task is started."),
           _T("%.0f %%"), _T("%.0f"),
           fixed_zero, fixed(100), fixed_one, false, fixed_zero,
           this);
}

void
TaskCalculatorPanel::Show(const PixelRect &rc)
{
  const GlidePolar &polar =
    CommonInterface::GetComputerSettings().polar.glide_polar_task;

  cruise_efficiency = polar.GetCruiseEfficiency();
  emc = XCSoarInterface::Calculated().task_stats.effective_mc;

  Refresh();

  CommonInterface::GetLiveBlackboard().AddListener(*this);

  RowFormWidget::Show(rc);
}

void
TaskCalculatorPanel::Hide()
{
  if (target_button != NULL)
    target_button->Hide();

  CommonInterface::GetLiveBlackboard().RemoveListener(*this);

  RowFormWidget::Hide();
}
