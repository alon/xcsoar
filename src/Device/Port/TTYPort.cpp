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

#include "TTYPort.hpp"
#include "Asset.hpp"
#include "OS/LogError.hpp"
#include "OS/Sleep.h"

#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/poll.h>
#include <termios.h>

#include <assert.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

TTYPort::~TTYPort()
{
  if (fd < 0)
    return;

  StopRxThread();

  close(fd);
}

bool
TTYPort::IsValid() const
{
  return valid.Get();
}

bool
TTYPort::Drain()
{
#ifdef __BIONIC__
  /* bionic doesn't have tcdrain() */
  return fd >= 0;
#else
  return fd >= 0 && tcdrain(fd) == 0;
#endif
}

bool
TTYPort::Open(const TCHAR *path, unsigned _baud_rate)
{
  fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {
    LogErrno(_T("Failed to open port '%s'"), path);
    return false;
  }

  baud_rate = _baud_rate;
  if (!SetBaudrate(baud_rate))
    return false;

  valid.Set();
  return true;
}

const char *
TTYPort::OpenPseudo()
{
  fd = open("/dev/ptmx", O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0)
    return NULL;

  if (unlockpt(fd) < 0)
    return NULL;

  valid.Set();
  return ptsname(fd);
}

void
TTYPort::Flush()
{
  if (!valid.Get())
    return;

  tcflush(fd, TCIFLUSH);
}

void
TTYPort::Run()
{
  char buffer[1024];

  while (true) {
    /* wait for data to arrive on the port */
    switch (WaitRead(200)) {
    case WaitResult::READY:
      /* linger for a few more milliseconds so the device can send
         some more data; without this, we would be waking up for every
         single byte */
      if (WaitForStopped(10))
        return;

      break;

    case WaitResult::FAILED:
      if (errno != EAGAIN && errno != EINTR) {
        valid.Reset();
        return;
      }

      /* non-fatal error, fall through */

    case WaitResult::TIMEOUT:
    case WaitResult::CANCELLED:
      /* throttle */
      if (WaitForStopped(500))
        return;

      continue;
    }

    ssize_t nbytes = read(fd, buffer, sizeof(buffer));
    if (nbytes == 0 || (nbytes < 0 && errno != EAGAIN && errno != EINTR)) {
      valid.Reset();
      return;
    }

    if (nbytes > 0)
      handler.DataReceived(buffer, nbytes);
  }

  Flush();
}

Port::WaitResult
TTYPort::WaitWrite(unsigned timeout_ms)
{
  assert(fd >= 0);

  if (!valid.Get())
    return WaitResult::FAILED;

  struct pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLOUT;

  int ret = poll(&pfd, 1, timeout_ms);
  if (ret > 0)
    return WaitResult::READY;
  else if (ret == 0)
    return WaitResult::TIMEOUT;
  else
    return WaitResult::FAILED;
}

size_t
TTYPort::Write(const void *data, size_t length)
{
  assert(fd >= 0);

  if (!valid.Get())
    return 0;

  ssize_t nbytes = write(fd, data, length);
  if (nbytes < 0 && errno == EAGAIN) {
    /* the output fifo is full; wait until we can write (or until the
       timeout expires) */
    if (WaitWrite(5000) != Port::WaitResult::READY)
      return 0;

    nbytes = write(fd, data, length);
  }

  return nbytes < 0 ? 0 : nbytes;
}

bool
TTYPort::StopRxThread()
{
  // Make sure the thread isn't terminating itself
  assert(!Thread::IsInside());
  assert(fd >= 0);

  // If the thread is not running, cancel the rest of the function
  if (!Thread::IsDefined())
    return true;

  BeginStop();

  Thread::Join();

  return true;
}

bool
TTYPort::StartRxThread()
{
  assert(fd >= 0);

  if (!valid.Get())
    return false;

  if (Thread::IsDefined())
    /* already running */
    return true;

  // Start the receive thread
  StoppableThread::Start();
  return true;
}

static unsigned
speed_t_to_baud_rate(speed_t speed)
{
  switch (speed) {
  case B1200:
    return 1200;

  case B2400:
    return 2400;

  case B4800:
    return 4800;

  case B9600:
    return 9600;

  case B19200:
    return 19200;

  case B38400:
    return 38400;

  case B57600:
    return 57600;

  case B115200:
    return 115200;

  default:
    return 0;
  }
}

unsigned
TTYPort::GetBaudrate() const
{
  struct termios attr;
  if (tcgetattr(fd, &attr) < 0)
    return 0;

  return speed_t_to_baud_rate(cfgetispeed(&attr));
}

/**
 * Convert a numeric baud rate to a termios.h constant (B*).  Returns
 * B0 on error.
 */
static speed_t
baud_rate_to_speed_t(unsigned baud_rate)
{
  switch (baud_rate) {
  case 1200:
    return B1200;

  case 2400:
    return B2400;

  case 4800:
    return B4800;

  case 9600:
    return B9600;

  case 19200:
    return B19200;

  case 38400:
    return B38400;

  case 57600:
    return B57600;

  case 115200:
    return B115200;

  default:
    return B0;
  }
}

bool
TTYPort::SetBaudrate(unsigned BaudRate)
{
  assert(fd >= 0);

  speed_t speed = baud_rate_to_speed_t(BaudRate);
  if (speed == B0)
    /* not supported */
    return false;

  struct termios attr;
  if (tcgetattr(fd, &attr) < 0)
    return false;

  attr.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  attr.c_oflag &= ~OPOST;
  attr.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  attr.c_cflag &= ~(CSIZE | PARENB | CRTSCTS);
  attr.c_cflag |= (CS8 | CLOCAL);
  attr.c_cc[VMIN] = 0;
  attr.c_cc[VTIME] = 1;
  cfsetospeed(&attr, speed);
  cfsetispeed(&attr, speed);
  if (tcsetattr(fd, TCSANOW, &attr) < 0) {
    close(fd);
    return false;
  }

  baud_rate = BaudRate;
  return true;
}

int
TTYPort::Read(void *Buffer, size_t Size)
{
  assert(fd >= 0);

  if (!valid.Get())
    return -1;

  return read(fd, Buffer, Size);
}

Port::WaitResult
TTYPort::WaitRead(unsigned timeout_ms)
{
  assert(fd >= 0);

  if (!valid.Get())
    return WaitResult::FAILED;

  struct pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN;

  int ret = poll(&pfd, 1, timeout_ms);
  if (ret > 0)
    return WaitResult::READY;
  else if (ret == 0)
    return WaitResult::TIMEOUT;
  else
    return WaitResult::FAILED;
}
