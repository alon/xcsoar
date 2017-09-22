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

#include <cstdio>

#include "NMEA/Checksum.hpp"
#include "NMEA/Info.hpp"
#include "NMEA/InputLine.hpp"

#include "Device/Driver/FakeKeyboard.hpp"
#include "Device/Driver.hpp"
#include "OS/UInput.hpp"


class FakeKeyboardDevice : public AbstractDevice {
private:
    UInput uinput;
    bool ParseFAKBD(NMEAInputLine &line, NMEAInfo &info);
public:
    FakeKeyboardDevice();
    bool ParseNMEA(const char *String, NMEAInfo &info) override;
};

FakeKeyboardDevice::FakeKeyboardDevice() : uinput()
{
}

bool
FakeKeyboardDevice::ParseFAKBD(NMEAInputLine &line, NMEAInfo &info)
{
    unsigned code = line.ReadHex(0);
    uinput.SendCode(code);
    return true;
}

bool
FakeKeyboardDevice::ParseNMEA(const char *String, NMEAInfo &info)
{
  if (!VerifyNMEAChecksum(String))
    return false;

  NMEAInputLine line(String);
  char type[16];
  line.Read(type, 16);

  if (StringIsEqual(type, "$FAKBD"))
    return ParseFAKBD(line, info);
  else
    return false;
}


static Device *
FakeKeyboardCreateOnPort(const DeviceConfig &config, Port &com_port)
{
  printf("Hello from FakeKeyboardDevice\n");
  return new FakeKeyboardDevice();
}

const struct DeviceRegister fakekeyboard_driver = {
  _T("FakeKeyboard"),
  _T("FakeKeyboard"),
  0,
  FakeKeyboardCreateOnPort,
};
