/*

Copyright_License {

  Top Hat Soaring Glide Computer - http://www.tophatsoaring.org/
  Copyright (C) 2000-2016 The Top Hat Soaring Project
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

#include "Device/Simulator.hpp"
#include "NMEA/Info.hpp"
#include "../Simulator.hpp"
#include "Geo/Math.hpp"
#include "GlideSolvers/GlidePolar.hpp"
#include "Components.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Thread/Guard.hpp"
#include "Interface.hpp"

#include <stdio.h>

void
Simulator::Init(NMEAInfo &basic)
{
  /* just in case DeviceBlackboard::SetStartupLocation never gets
     called, set some dummy values that are better than uninitialised
     values */

  basic.location = GeoPoint::Zero();
  basic.track = Angle::Zero();
  basic.ground_speed = fixed(0);
  basic.gps_altitude = fixed(0);
}

void
Simulator::Touch(NMEAInfo &basic)
{
  assert(is_simulator());

  basic.UpdateClock();
  basic.alive.Update(basic.clock);
  basic.gps.simulator = true;
  basic.gps.real = false;

  basic.location_available.Update(basic.clock);
  basic.track_available.Update(basic.clock);
  basic.ground_speed_available.Update(basic.clock);
  basic.gps_altitude_available.Update(basic.clock);

  basic.time_available.Update(basic.clock);
  basic.time += fixed(1);
  basic.date_time_utc = basic.date_time_utc + 1;
}

void
Simulator::Process(NMEAInfo &basic)
{
  assert(is_simulator());

  Touch(basic);

  basic.location = FindLatitudeLongitude(basic.location, basic.track,
                                         basic.ground_speed);
  {
    assert(protected_task_manager != nullptr);
    ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
    const GlidePolar &polar = task_manager->GetGlidePolar();
    fixed airspeed =  CommonInterface::Basic().true_airspeed;

    if (!positive(airspeed))
        airspeed = basic.ground_speed;
    if (airspeed > polar.GetSMin()  / 2)
      basic.gps_altitude = basic.gps_altitude - polar.SinkRate(airspeed);

    basic.ProvideNettoVario(fixed(0));
    last_airspeed = airspeed;
  }
}
