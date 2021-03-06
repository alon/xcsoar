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

#include "CirclingComputer.hpp"
#include "NMEA/MoreData.hpp"
#include "NMEA/Derived.hpp"
#include "ComputerSettings.hpp"
#include "Math/LowPassFilter.hpp"

static const fixed min_turn_rate(4);
static const fixed cruise_climb_switch(15);
static const fixed climb_cruise_switch(10);

void
CirclingComputer::TurnRate(CirclingInfo &circling_info,
                           const NMEAInfo &basic, const NMEAInfo &last_basic,
                           const DerivedInfo &calculated,
                           const DerivedInfo &last_calculated)
{
  if (!basic.time_available || !last_basic.time_available ||
      !calculated.flight.flying) {
    circling_info.turn_rate = fixed_zero;
    circling_info.turn_rate_heading = fixed_zero;
    return;
  }

  if (!basic.HasTimeAdvancedSince(last_basic))
    return;

  // Calculate time passed since last calculation
  const fixed dt = basic.time - last_basic.time;
  assert(positive(dt));

  circling_info.turn_rate =
    (basic.track - last_basic.track).AsDelta().Degrees() / dt;
  circling_info.turn_rate_heading =
    (calculated.heading - last_calculated.heading).AsDelta().Degrees() / dt;
}

void
CirclingComputer::Turning(CirclingInfo &circling_info,
                          const MoreData &basic, const MoreData &last_basic,
                          const DerivedInfo &calculated,
                          const DerivedInfo &last_calculated,
                          const ComputerSettings &settings_computer)
{
  // You can't be circling unless you're flying
  if (!calculated.flight.flying || !basic.HasTimeAdvancedSince(last_basic))
    return;

  // JMW limit rate to 50 deg per second otherwise a big spike
  // will cause spurious lock on circling for a long time
  fixed turn_rate = max(fixed(-50), min(fixed(50), calculated.turn_rate));

  // Make the turn rate more smooth using the LowPassFilter
  turn_rate = LowPassFilter(last_calculated.turn_rate_smoothed,
                            turn_rate, fixed(0.3));
  circling_info.turn_rate_smoothed = turn_rate;
  circling_info.turning = fabs(turn_rate) >= min_turn_rate;

  // Force cruise or climb mode if external device says so
  bool force_cruise = false;
  bool force_circling = false;
  if (settings_computer.external_trigger_cruise_enabled && !basic.gps.replay) {
    switch (basic.switch_state.flight_mode) {
    case SwitchInfo::FlightMode::UNKNOWN:
      force_circling = false;
      force_cruise = false;
      break;

    case SwitchInfo::FlightMode::CIRCLING:
      force_circling = true;
      force_cruise = false;
      break;

    case SwitchInfo::FlightMode::CRUISE:
      force_circling = false;
      force_cruise = true;
      break;
    }
  }

  switch (calculated.turn_mode) {
  case CirclingMode::CRUISE:
    // If (in cruise mode and beginning of circling detected)
    if (circling_info.turning || force_circling) {
      // Remember the start values of the turn
      circling_info.turn_start_time = basic.time;
      circling_info.turn_start_location = basic.location;
      circling_info.turn_start_altitude = basic.nav_altitude;
      circling_info.turn_start_energy_height = basic.energy_height;
      circling_info.turn_mode = CirclingMode::POSSIBLE_CLIMB;
    }
    if (!force_circling)
      break;

  case CirclingMode::POSSIBLE_CLIMB:
    if (force_cruise) {
      circling_info.turn_mode = CirclingMode::CRUISE;
      break;
    }
    if (circling_info.turning || force_circling) {
      if (((basic.time - calculated.turn_start_time) > cruise_climb_switch)
          || force_circling) {
        // yes, we are certain now that we are circling
        circling_info.circling = true;

        // JMW Transition to climb
        circling_info.turn_mode = CirclingMode::CLIMB;

        // Remember the start values of the climbing period
        circling_info.climb_start_location = circling_info.turn_start_location;
        circling_info.climb_start_altitude = circling_info.turn_start_altitude
            + circling_info.turn_start_energy_height;
        circling_info.climb_start_time = circling_info.turn_start_time;
      }
    } else {
      // nope, not turning, so go back to cruise
      circling_info.turn_mode = CirclingMode::CRUISE;
    }
    break;

  case CirclingMode::CLIMB:
    if (!circling_info.turning || force_cruise) {
      // Remember the end values of the turn
      circling_info.turn_start_time = basic.time;
      circling_info.turn_start_location = basic.location;
      circling_info.turn_start_altitude = basic.nav_altitude;
      circling_info.turn_start_energy_height = basic.energy_height;

      // JMW Transition to cruise, due to not properly turning
      circling_info.turn_mode = CirclingMode::POSSIBLE_CRUISE;
    }
    if (!force_cruise)
      break;

  case CirclingMode::POSSIBLE_CRUISE:
    if (force_circling) {
      circling_info.turn_mode = CirclingMode::CLIMB;
      break;
    }

    if (!circling_info.turning || force_cruise) {
      if (((basic.time - circling_info.turn_start_time) > climb_cruise_switch)
          || force_cruise) {
        // yes, we are certain now that we are cruising again
        circling_info.circling = false;

        // Transition to cruise
        circling_info.turn_mode = CirclingMode::CRUISE;
        circling_info.cruise_start_location = circling_info.turn_start_location;
        circling_info.cruise_start_altitude = circling_info.turn_start_altitude;
        circling_info.cruise_start_time = circling_info.turn_start_time;
      }
    } else {
      // nope, we are circling again
      // JMW Transition back to climb, because we are turning again
      circling_info.turn_mode = CirclingMode::CLIMB;
    }
    break;
  }
}
