/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2013 The XCSoar Project
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

#ifndef XCSOAR_DEVICE_DRIVER_LX_NANO_HPP
#define XCSOAR_DEVICE_DRIVER_LX_NANO_HPP

#include "Device/Port/Port.hpp"
#include "Device/Internal.hpp"

/**
 * Code specific to the LXNAV Nano.
 *
 * Source: Nano Dataport specification Version 2.1
 */
namespace Nano {
  /**
   * Request basic Nano information.
   */
  static bool
  RequestInfo(Port &port, OperationEnvironment &env)
  {
    return PortWriteNMEA(port, "PLXVC,INFO,R", env);
  }

  /**
   * Request basic Nano information through an LXNAV V7 (firmware >=
   * 2.01).
   */
  static bool
  RequestForwardedInfo(Port &port, OperationEnvironment &env)
  {
    return PortWriteNMEA(port, "PLXVC,GPSINFO,R", env);
  }
}

#endif