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

#include "VarioConfigPanel.hpp"
#include "Profile/ProfileKeys.hpp"
#include "Language/Language.hpp"
#include "Interface.hpp"
#include "Form/Form.hpp"
#include "Form/RowFormWidget.hpp"
#include "UIGlobals.hpp"
#include "Audio/Features.hpp"

enum ControlIndex {
  AppGaugeVarioSpeedToFly,
  AppGaugeVarioAvgText,
  AppGaugeVarioMc,
  AppGaugeVarioBugs,
  AppGaugeVarioBallast,
  AppGaugeVarioGross,
  AppAveNeedle,
#ifdef HAVE_PCM_PLAYER
  AudioVario,
  Volume,
#endif
};


class VarioConfigPanel : public RowFormWidget {
public:
  VarioConfigPanel()
    :RowFormWidget(UIGlobals::GetDialogLook()) {}

public:
  virtual void Prepare(ContainerWindow &parent, const PixelRect &rc);
  virtual bool Save(bool &changed, bool &require_restart);
};

void
VarioConfigPanel::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  const VarioSettings &settings = CommonInterface::GetUISettings().vario;

  RowFormWidget::Prepare(parent, rc);

  AddBoolean(_("Speed arrows"),
             _("Whether to show speed command arrows on the vario gauge.  When shown, in cruise mode, "
                 "arrows point up to command slow down; arrows point down to command speed up."),
             settings.show_speed_to_fly);
  SetExpertRow(AppGaugeVarioSpeedToFly);

  AddBoolean(_("Show average"),
             _("Whether to show the average climb rate.  In cruise mode, this switches to showing the "
                 "average netto airmass rate."),
             settings.show_average);
  SetExpertRow(AppGaugeVarioAvgText);

  AddBoolean(_("Show MacReady"), _("Whether to show the MacCready setting."), settings.show_mc);
  SetExpertRow(AppGaugeVarioMc);

  AddBoolean(_("Show bugs"), _("Whether to show the bugs percentage."), settings.show_bugs);
  SetExpertRow(AppGaugeVarioBugs);

  AddBoolean(_("Show ballast"), _("Whether to show the ballast percentage."), settings.show_ballast);
  SetExpertRow(AppGaugeVarioBallast);

  AddBoolean(_("Show gross"), _("Whether to show the gross climb rate."), settings.show_gross);
  SetExpertRow(AppGaugeVarioGross);

  AddBoolean(_("Averager needle"),
             _("If true, the vario gauge will display a hollow averager needle.  During cruise, this "
                 "needle displays the average netto value.  During circling, this needle displays the "
                 "average gross value."),
             settings.show_average_needle);
  SetExpertRow(AppAveNeedle);

#ifdef HAVE_PCM_PLAYER
  const SoundSettings &sound = CommonInterface::GetComputerSettings().sound;

  AddBoolean(_("Audio vario"),
             _("Emulate the sound of an electronic vario."),
             sound.sound_vario_enabled);

  AddInteger(_("Volume"), NULL, _T("%u %%"), _T("%u"),
             0, 100, 1, sound.sound_volume);
#endif
}

bool
VarioConfigPanel::Save(bool &_changed, bool &_require_restart)
{
  bool changed = false, require_restart = false;

  VarioSettings &settings = CommonInterface::SetUISettings().vario;

  changed |= SaveValue(AppGaugeVarioSpeedToFly, szProfileAppGaugeVarioSpeedToFly, settings.show_speed_to_fly);

  changed |= SaveValue(AppGaugeVarioAvgText, szProfileAppGaugeVarioAvgText, settings.show_average);

  changed |= SaveValue(AppGaugeVarioMc, szProfileAppGaugeVarioMc, settings.show_mc);

  changed |= SaveValue(AppGaugeVarioBugs, szProfileAppGaugeVarioBugs, settings.show_bugs);

  changed |= SaveValue(AppGaugeVarioBallast, szProfileAppGaugeVarioBallast, settings.show_ballast);

  changed |= SaveValue(AppGaugeVarioGross, szProfileAppGaugeVarioGross, settings.show_gross);

  changed |= SaveValue(AppAveNeedle, szProfileAppAveNeedle, settings.show_average_needle);

#ifdef HAVE_PCM_PLAYER
  SoundSettings &sound = CommonInterface::SetComputerSettings().sound;
  changed |= SaveValue(AudioVario, szProfileSoundAudioVario,
                       sound.sound_vario_enabled);
  unsigned volume = sound.sound_volume;
  if (SaveValue(Volume, szProfileSoundVolume, volume)) {
    sound.sound_volume = volume;
    changed = true;
  }
#endif

  _changed |= changed;
  _require_restart |= require_restart;

  return true;
}

Widget *
CreateVarioConfigPanel()
{
  return new VarioConfigPanel();
}
