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

#include "InfoBoxes/Content/Weather.hpp"
#include "InfoBoxes/Panel/WindEdit.hpp"
#include "InfoBoxes/Panel/ForecastTemperature.hpp"
#include "InfoBoxes/Data.hpp"
#include "Interface.hpp"
#include "Dialogs/dlgInfoBoxAccess.hpp"
#include "Util/Macros.hpp"
#include "Units/Units.hpp"
#include "Language/Language.hpp"

#include <tchar.h>
#include <stdio.h>

void
InfoBoxContentHumidity::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = XCSoarInterface::Basic();
  if (!basic.humidity_available) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.UnsafeFormatValue( _T("%d"), (int)basic.humidity);
}

void
InfoBoxContentTemperature::Update(InfoBoxData &data)
{
  const NMEAInfo &basic = XCSoarInterface::Basic();
  if (!basic.temperature_available) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValue(_T("%2.1f"),
                    Units::ToUserTemperature(basic.temperature));

  data.SetValueUnit(Units::current.temperature_unit);
}

void
InfoBoxContentTemperatureForecast::Update(InfoBoxData &data)
{
  fixed temperature = CommonInterface::GetComputerSettings().forecast_temperature;
  data.SetValue(_T("%2.1f"),
                    Units::ToUserTemperature(temperature));

  data.SetValueUnit(Units::current.temperature_unit);
}


static gcc_constexpr_data InfoBoxContentTemperatureForecast::PanelContent panels_temp[] = {
  InfoBoxContentTemperatureForecast::PanelContent (
    N_("Edit"),
    LoadForecastTemperaturePanel),
};

const InfoBoxContentTemperatureForecast::DialogContent InfoBoxContentTemperatureForecast::dlgContent = {
  ARRAY_SIZE(panels_temp), &panels_temp[0], false,
};

const InfoBoxContentTemperatureForecast::DialogContent *
InfoBoxContentTemperatureForecast::GetDialogContent() {
  return &dlgContent;
}

/*
 * Subpart callback function pointers
 */

static gcc_constexpr_data InfoBoxContentWind::PanelContent Panels[] = {
InfoBoxContentWind::PanelContent (
  N_("Edit"),
  LoadWindEditPanel),
};

const InfoBoxContentWind::DialogContent InfoBoxContentWind::dlgContent = {
  ARRAY_SIZE(Panels), &Panels[0], false,
};

const InfoBoxContentWind::DialogContent *
InfoBoxContentWind::GetDialogContent() {
  return &dlgContent;
}


void
InfoBoxContentWindSpeed::Update(InfoBoxData &data)
{
  const DerivedInfo &info = CommonInterface::Calculated();
  if (!info.wind_available) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValue(_T("%2.0f"),
                    Units::ToUserWindSpeed(info.wind.norm));

  // Set Unit
  data.SetValueUnit(Units::current.wind_speed_unit);

  // Set Comment
  data.SetComment(info.wind.bearing);
}

void
InfoBoxContentWindBearing::Update(InfoBoxData &data)
{
  const DerivedInfo &info = CommonInterface::Calculated();
  if (!info.wind_available) {
    data.SetInvalid();
    return;
  }

  data.SetValue(info.wind.bearing);
}

void
InfoBoxContentHeadWind::Update(InfoBoxData &data)
{
  const DerivedInfo &info = CommonInterface::Calculated();
  if (!info.head_wind_available) {
    data.SetInvalid();
    return;
  }

  // Set Value
  data.SetValue(_T("%2.0f"),
                    Units::ToUserWindSpeed(info.head_wind));

  // Set Unit
  data.SetValueUnit(Units::current.wind_speed_unit);
}
