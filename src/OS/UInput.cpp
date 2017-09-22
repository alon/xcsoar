/*
 * Copyright_License {
 * 
 *   Top Hat Soaring Glide Computer - http://www.tophatsoaring.org/
 *   Copyright (C) 2000-2017 The Top Hat Soaring Project
 *   A detailed list of copyright holders can be found in the file "AUTHORS".
 * 
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * }
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "/usr/include/linux/uinput.h"

#include "UInput.hpp"

const char *dev_filename = "/dev/uinput";

UInput::~UInput()
{
    if (fd == -1)
        return;
    ioctl(fd, UI_DEV_DESTROY);
}

UInput::UInput()
{
    fd = open(dev_filename, O_WRONLY | O_NONBLOCK);
    
    if (fd < 0)
    {
        // @todo log error & bail out here, don't just act as if everything is ok
        return;
    }

    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
    {
        // @todo log error & bail
        fd = -1;
        return;
    }
    
    for (int key = 0 ; key < KEY_MAX ; ++key)
    {
        if (ioctl(fd, UI_SET_KEYBIT, key) < 0)
        {
            // @todo - log error. continue as normal with the other successful keys
        }
    }
    
    struct uinput_user_dev uidev = {0};
    //uisetup.id.bustype = BUS_USB;
    //uisetup.id.vendor = 0x1234; // @todo - register something? use something reserved for unregistered devices
    //uisetup.id.product = 0x5678;
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "tophat-fakekeyboard");
    // UI_DEV_SETUP is too new for the TopHat default uinput.h headers/kernel?
    write(fd, &uidev, sizeof(uidev));
    if (ioctl(fd, UI_DEV_CREATE) < 0)
    {
        fd = -1;
        // @todo log error & bail out here, don't just act as if everything is ok
        return;
    }
}

static int emit(int fd, int type, int code, int val)
{
    struct input_event ev = {0};
    ev.code = code;
    ev.type = type;
    ev.value = val;
    // @todo - notify of error. don't bail.
    return write(fd, &ev, sizeof(ev));
}

void UInput::SendCode(unsigned code)
{
    if (fd < 0)
        // @todo remove this, we should bail out early if we fail to initialize
        return;
    emit(fd, EV_KEY, code, 1);
    emit(fd, EV_SYN, SYN_REPORT, 0);
    emit(fd, EV_KEY, code, 0);
    emit(fd, EV_SYN, SYN_REPORT, 0);
}
