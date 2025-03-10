/****************************************************************************
 * drivers/timers/mcp794xx.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <debug.h>

#include <nuttx/signal.h>
#include <nuttx/arch.h>
#include <nuttx/i2c/i2c_master.h>
#include <nuttx/timers/mcp794xx.h>

#include "mcp794xx.h"

#ifdef CONFIG_RTC_MCP794XX

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define MCP794XX_OSCRUN_READ_RETRY  5  /* How many time to read OSCRUN status */

/* Configuration ************************************************************/

/* This RTC implementation supports only date/time RTC hardware */

#ifndef CONFIG_RTC_DATETIME
#  error CONFIG_RTC_DATETIME must be set to use this driver
#endif

#ifdef CONFIG_RTC_HIRES
#  error CONFIG_RTC_HIRES must NOT be set with this driver
#endif

#ifndef CONFIG_MCP794XX_I2C_FREQUENCY
#  error CONFIG_MCP794XX_I2C_FREQUENCY is not configured
#  define CONFIG_MCP794XX_I2C_FREQUENCY 400000
#endif

#if CONFIG_MCP794XX_I2C_FREQUENCY > 400000
#  error CONFIG_MCP794XX_I2C_FREQUENCY is out of range
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This structure describes the state of the MCP794XX chip.
 * Only a single RTC is supported.
 */

struct mcp794xx_dev_s
{
  FAR struct i2c_master_s *i2c;  /* Contained reference to the I2C bus driver. */
  uint8_t addr;                  /* The I2C device address. */
  bool coarse_trim;              /* Coarse trim mode */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* g_rtc_enabled is set true after the RTC has successfully initialized */

volatile bool g_rtc_enabled = false;

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* The state of the MCP794XX chip.  Only a single RTC is supported */

static struct mcp794xx_dev_s g_mcp794xx;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: rtc_dumptime
 *
 * Description:
 *   Show the broken out time.
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

#ifdef CONFIG_DEBUG_RTC_INFO
static void rtc_dumptime(FAR struct tm *tp, FAR const char *msg)
{
  rtcinfo("%s:\n", msg);
  rtcinfo("   tm_sec: %08x\n", tp->tm_sec);
  rtcinfo("   tm_min: %08x\n", tp->tm_min);
  rtcinfo("  tm_hour: %08x\n", tp->tm_hour);
  rtcinfo("  tm_mday: %08x\n", tp->tm_mday);
  rtcinfo("   tm_mon: %08x\n", tp->tm_mon);
  rtcinfo("  tm_year: %08x\n", tp->tm_year);
  rtcinfo("  tm_wday: %08x\n", tp->tm_wday);
  rtcinfo("  tm_yday: %08x\n", tp->tm_yday);
  rtcinfo(" tm_isdst: %08x\n", tp->tm_isdst);
}
#else
#  define rtc_dumptime(tp, msg)
#endif

/****************************************************************************
 * Name: rtc_bin2bcd
 *
 * Description:
 *   Converts a 2 digit binary to BCD format
 *
 * Input Parameters:
 *   value - The byte to be converted.
 *
 * Returned Value:
 *   The value in BCD representation
 *
 ****************************************************************************/

static uint8_t rtc_bin2bcd(int value)
{
  uint8_t msbcd = 0;

  while (value >= 10)
    {
      msbcd++;
      value -= 10;
    }

  return (msbcd << 4) | value;
}

/****************************************************************************
 * Name: rtc_bcd2bin
 *
 * Description:
 *   Convert from 2 digit BCD to binary.
 *
 * Input Parameters:
 *   value - The BCD value to be converted.
 *
 * Returned Value:
 *   The value in binary representation
 *
 ****************************************************************************/

static int rtc_bcd2bin(uint8_t value)
{
  int tens = ((int)value >> 4) * 10;
  return tens + (value & 0x0f);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: mcp794xx_rtc_set_trim
 *
 * Description:
 *   Sets the digital trimming to correct for inaccuracies of clock source.
 *   Digital trimming consists of the MCP794XX periodically adding or
 *   subtracting clock cycles, resulting in small adjustments in the internal
 *   timing.
 *
 * Input Parameters:
 *   trim_val    - Calculated trimming value, refer to MCP794XX reference
 *                 manual.
 *   rtc_slow    - True indicates RTC is behind real clock, false otherwise.
 *                 This has to be set to ensure correct trimming direction.
 *   coarse_mode - MCP794XX allows coarse mode that trims every second
 *                 instead of every minute.
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno on failure
 *
 ****************************************************************************/

int mcp794xx_rtc_set_trim(uint8_t trim_val, bool rtc_slow, bool coarse_mode)
{
  struct i2c_msg_s msg[2];
  uint8_t buffer[2];
  uint8_t address;
  uint8_t ctrl;
  int ret;

  if (g_mcp794xx.coarse_trim != coarse_mode)
    {
      address = MCP794XX_REG_CONTROL;
      msg[0].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
      msg[0].addr      = g_mcp794xx.addr;
      msg[0].flags     = I2C_M_NOSTOP;
      msg[0].buffer    = &address;
      msg[0].length    = 1;

      msg[1].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
      msg[1].addr      = g_mcp794xx.addr;
      msg[1].flags     = I2C_M_READ;
      msg[1].buffer    = &ctrl;
      msg[1].length    = 1;

      ret = I2C_TRANSFER(g_mcp794xx.i2c, msg, 2);
      if (ret < 0)
        {
          rtcerr("ERROR: I2C_TRANSFER failed: %d\n", ret);
          return ret;
        }

      ctrl &= ~MCP794XX_CONTROL_CRSTRIM;
      if (coarse_mode)
        {
          ctrl |= MCP794XX_CONTROL_CRSTRIM;
        }

      buffer[0] = MCP794XX_REG_CONTROL;
      buffer[1] = ctrl;
      msg[0].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
      msg[0].addr      = g_mcp794xx.addr;
      msg[0].flags     = 0;
      msg[0].buffer    = buffer;
      msg[0].length    = 2;

      ret = I2C_TRANSFER(g_mcp794xx.i2c, msg, 1);
      if (ret < 0)
        {
          rtcerr("ERROR: I2C_TRANSFER failed: %d\n", ret);
          return ret;
        }

      g_mcp794xx.coarse_trim = coarse_mode;
    }

  buffer[0] = MCP794XX_REG_OSCTRIM;
  buffer[1] = trim_val & 0x7;
  if (rtc_slow)
    {
      buffer[1] |= MCP794XX_OSCTRIM_SIGN;
    }

  msg[0].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
  msg[0].addr      = g_mcp794xx.addr;
  msg[0].flags     = 0;
  msg[0].buffer    = buffer;
  msg[0].length    = 2;

  ret = I2C_TRANSFER(g_mcp794xx.i2c, msg, 1);
  if (ret < 0)
    {
      rtcerr("ERROR: I2C_TRANSFER failed: %d\n", ret);
      return ret;
    }

  return OK;
}

/****************************************************************************
 * Name: mcp794xx_rtc_initialize
 *
 * Description:
 *   Initialize the hardware RTC per the selected configuration.  This
 *   function is called once during the OS initialization sequence by board-
 *   specific logic.
 *
 *   After mcp794xx_rtc_initialize() is called, the OS function
 *   clock_synchronize() should also be called to synchronize the system
 *   timer to a hardware RTC.  That operation is normally performed
 *   automatically by the system during clock initialization.  However, when
 *   an external RTC is used, the board logic will need to explicitly re-
 *   synchronize the system timer to the RTC when the RTC becomes available.
 *
 * Input Parameters:
 *   i2c  - An instance of the I2C interface used to access the MCP794XX
 *          device
 *   addr - The (7-bit) I2C address of the MCP794XX device
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno on failure
 *
 ****************************************************************************/

int mcp794xx_rtc_initialize(FAR struct i2c_master_s *i2c, uint8_t addr)
{
  /* Remember the i2c device and claim that the RTC is enabled */

  g_mcp794xx.i2c         = i2c;
  g_mcp794xx.addr        = addr;
  g_mcp794xx.coarse_trim = false;
  g_rtc_enabled          = true;
  return OK;
}

/****************************************************************************
 * Name: up_rtc_getdatetime
 *
 * Description:
 *   Get the current date and time from the date/time RTC.  This interface
 *   is only supported by the date/time RTC hardware implementation.
 *   It is used to replace the system timer.  It is only used by the RTOS
 *   during initialization to set up the system time when CONFIG_RTC and
 *   CONFIG_RTC_DATETIME are selected (and CONFIG_RTC_HIRES is not).
 *
 *   NOTE: Some date/time RTC hardware is capability of sub-second accuracy.
 *   That sub-second accuracy is lost in this interface.  However, since the
 *   system time is reinitialized on each power-up/reset, there will be no
 *   timing inaccuracy in the long run.
 *
 * Input Parameters:
 *   tp - The location to return the high resolution time value.
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno on failure
 *
 ****************************************************************************/

int up_rtc_getdatetime(FAR struct tm *tp)
{
  struct i2c_msg_s msg[4];
  uint8_t secaddr;
  uint8_t buffer[7];
  uint8_t seconds;
  int ret;

  /* If this function is called before the RTC has been initialized (and it
   * will be), then just return the data/time of the epoch, 12:00 am, Jan 1,
   * 1970.
   */

  if (!g_rtc_enabled)
    {
      tp->tm_sec  = 0;
      tp->tm_min  = 0;
      tp->tm_hour = 0;

      /* Jan 1, 1970 was a Thursday */

      tp->tm_wday = 4;
      tp->tm_mday = 1;
      tp->tm_mon  = 0;
      tp->tm_year = 70;
      return -EAGAIN;
    }

  /* The start address of the read is the seconds address (0x00)
   * The chip increments the address to read from after each read.
   */

  secaddr = MCP794XX_REG_RTCSEC;

  msg[0].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
  msg[0].addr      = g_mcp794xx.addr;
  msg[0].flags     = I2C_M_NOSTOP;
  msg[0].buffer    = &secaddr;
  msg[0].length    = 1;

  /* Setup the read. Seven (7) registers will be read.
   * (Seconds, minutes, hours, wday, date, month and year)
   */

  msg[1].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
  msg[1].addr      = g_mcp794xx.addr;
  msg[1].flags     = I2C_M_READ;
  msg[1].buffer    = buffer;
  msg[1].length    = 7;

  /* Read the seconds register again */

  msg[2].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
  msg[2].addr      = g_mcp794xx.addr;
  msg[2].flags     = I2C_M_NOSTOP;
  msg[2].buffer    = &secaddr;
  msg[2].length    = 1;

  msg[3].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
  msg[3].addr      = g_mcp794xx.addr;
  msg[3].flags     = I2C_M_READ;
  msg[3].buffer    = &seconds;
  msg[3].length    = 1;

  /* Perform the transfer. The transfer may be performed repeatedly of the
   * seconds values decreases, meaning that was a rollover in the seconds.
   */

  do
    {
      ret = I2C_TRANSFER(g_mcp794xx.i2c, msg, 4);
      if (ret < 0)
        {
          rtcerr("ERROR: I2C_TRANSFER failed: %d\n", ret);
          return ret;
        }
    }
  while ((buffer[0] & MCP794XX_RTCSEC_BCDMASK) >
         (seconds & MCP794XX_RTCSEC_BCDMASK));

  /* Format the return time */

  /* Return seconds (0-59) */

  tp->tm_sec = rtc_bcd2bin(buffer[0] & MCP794XX_RTCSEC_BCDMASK);

  /* Return minutes (0-59) */

  tp->tm_min = rtc_bcd2bin(buffer[1] & MCP794XX_RTCMIN_BCDMASK);

  /* Return hour (0-23).  This assumes 24-hour time was set. */

  tp->tm_hour = rtc_bcd2bin(buffer[2] & MCP794XX_RTCHOUR_BCDMASK);

  /* Return the day of the week (0-6) */

  tp->tm_wday = (rtc_bcd2bin(buffer[3]) & MCP794XX_RTCWKDAY_BCDMASK) - 1;

  /* Return the day of the month (1-31) */

  tp->tm_mday = rtc_bcd2bin(buffer[4] & MCP794XX_RTCDATE_BCDMASK);

  /* Return the month (0-11) */

  tp->tm_mon = rtc_bcd2bin(buffer[5] & MCP794XX_RTCMTH_BCDMASK) - 1;

  /* Return the years since 1900 */

  tp->tm_year = rtc_bcd2bin(buffer[6] & MCP794XX_RTCYEAR_BCDMASK);

  /* The Year is stored in the RTC starting from 2001. We need to convert it
   * to POSIX format that expects the year starting from 1900.
   */

  tp->tm_year += 101;

  rtc_dumptime(tp, "Returning");
  return OK;
}

/****************************************************************************
 * Name: up_rtc_settime
 *
 * Description:
 *   Set the RTC to the provided time.  All RTC implementations must be able
 *   to set their time based on a standard timespec.
 *
 * Input Parameters:
 *   tp - the time to use
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno on failure
 *
 ****************************************************************************/

int up_rtc_settime(FAR const struct timespec *tp)
{
  struct i2c_msg_s msg[3];
  struct tm newtm;
  time_t newtime;
  uint8_t buffer[8];
  uint8_t wkdayaddr;
  uint8_t wkday;
  int ret;
  int retries = MCP794XX_OSCRUN_READ_RETRY;

  /* If this function is called before the RTC has been initialized then
   * just return an error.
   */

  if (!g_rtc_enabled)
    {
      return -EAGAIN;
    }

  rtcinfo("Setting time tp=(%d,%d)\n", (int)tp->tv_sec, (int)tp->tv_nsec);

  /* Get the broken out time */

  newtime = (time_t)tp->tv_sec;
  if (tp->tv_nsec >= 500000000)
    {
      /* Round up */

      newtime++;
    }

#ifndef CONFIG_MCP794XX_DATETIME_UTC
  /* Save datetime in local time. */

  if (localtime_r(&newtime, &newtm) == NULL)
    {
      rtcerr("ERROR: localtime_r failed\n");
      return -EINVAL;
    }
#else
  /* Save datetime in UTC time. */

  if (gmtime_r(&newtime, &newtm) == NULL)
    {
      rtcerr("ERROR: gmtime_r failed\n");
      return -EINVAL;
    }
#endif

  rtc_dumptime(&newtm, "New time");

  /* Stop the oscillator first. */

  buffer[0]        = MCP794XX_REG_RTCSEC;
  buffer[1]        = 0;

  msg[0].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
  msg[0].addr      = g_mcp794xx.addr;
  msg[0].flags     = 0;
  msg[0].buffer    = buffer;
  msg[0].length    = 2;

  ret = I2C_TRANSFER(g_mcp794xx.i2c, msg, 1);
  if (ret < 0)
    {
      rtcerr("ERROR: I2C_TRANSFER failed: %d\n", ret);
      return ret;
    }

  /* Verify that the oscillator is not running. */

  wkdayaddr = MCP794XX_REG_RTCWKDAY;

  msg[0].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
  msg[0].addr      = g_mcp794xx.addr;
  msg[0].flags     = I2C_M_NOSTOP;
  msg[0].buffer    = &wkdayaddr;
  msg[0].length    = 1;

  msg[1].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
  msg[1].addr      = g_mcp794xx.addr;
  msg[1].flags     = I2C_M_READ;
  msg[1].buffer    = &wkday;
  msg[1].length    = 1;

  retries = MCP794XX_OSCRUN_READ_RETRY;

  do
    {
      /* Give time to oscillator to change its status */

      nxsig_usleep(10000);

      ret = I2C_TRANSFER(g_mcp794xx.i2c, msg, 2);
      if (ret < 0)
        {
          rtcerr("ERROR: I2C_TRANSFER failed: %d\n", ret);
          return ret;
        }

      retries--;
    }
  while ((wkday & MCP794XX_RTCWKDAY_OSCRUN) != 0 && retries > 0);

  /* Construct the message */

  /* Write starting with the seconds register */

  buffer[0] = MCP794XX_REG_RTCSEC;

  /* Save seconds (0-59) converted to BCD. And keep ST cleared. */

  buffer[1] = rtc_bin2bcd(newtm.tm_sec);

  /* Save minutes (0-59) converted to BCD */

  buffer[2] = rtc_bin2bcd(newtm.tm_min);

  /* Save hour (0-23) with 24-hour time indication */

  buffer[3] = rtc_bin2bcd(newtm.tm_hour);

  /* Save the day of the week (1-7) and enable VBAT. */

  buffer[4] = rtc_bin2bcd(newtm.tm_wday + 1) | MCP794XX_RTCWKDAY_VBATEN;

  /* Save the day of the month (1-31) */

  buffer[5] = rtc_bin2bcd(newtm.tm_mday);

  /* Save the month (1-12) */

  buffer[6] = rtc_bin2bcd(newtm.tm_mon + 1);

  /* Save the year (00-99) */

  /* First we need to convert "tm_year" to value starting from 2001.
   * The "tm_year" in POSIX is relative to 1900, so 2019 is 119,
   * so you just need to subtract 101: year = (1900 + value) - 2001
   */

  buffer[7]        = rtc_bin2bcd(newtm.tm_year - 101);

  msg[0].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
  msg[0].addr      = g_mcp794xx.addr;
  msg[0].flags     = 0;
  msg[0].buffer    = buffer;
  msg[0].length    = 8;

  ret = I2C_TRANSFER(g_mcp794xx.i2c, msg, 1);
  if (ret < 0)
    {
      rtcerr("ERROR: I2C_TRANSFER failed: %d\n", ret);
      return ret;
    }

  /* Start the oscillator. */

  buffer[1]       |= MCP794XX_RTCSEC_ST;

  msg[0].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
  msg[0].addr      = g_mcp794xx.addr;
  msg[0].flags     = 0;
  msg[0].buffer    = buffer;
  msg[0].length    = 2;

  ret = I2C_TRANSFER(g_mcp794xx.i2c, msg, 1);
  if (ret < 0)
    {
      rtcerr("ERROR: I2C_TRANSFER failed: %d\n", ret);
      return ret;
    }

  /* Verify that the oscillator is running. */

  wkdayaddr = MCP794XX_REG_RTCWKDAY;

  msg[0].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
  msg[0].addr      = g_mcp794xx.addr;
  msg[0].flags     = I2C_M_NOSTOP;
  msg[0].buffer    = &wkdayaddr;
  msg[0].length    = 1;

  msg[1].frequency = CONFIG_MCP794XX_I2C_FREQUENCY;
  msg[1].addr      = g_mcp794xx.addr;
  msg[1].flags     = I2C_M_READ;
  msg[1].buffer    = &wkday;
  msg[1].length    = 1;

  retries = MCP794XX_OSCRUN_READ_RETRY;

  do
    {
      /* Give time to oscillator to change its status */

      nxsig_usleep(10000);

      ret = I2C_TRANSFER(g_mcp794xx.i2c, msg, 2);
      if (ret < 0)
        {
          rtcerr("ERROR: I2C_TRANSFER failed: %d\n", ret);
          return ret;
        }

      retries--;
    }
  while ((wkday & MCP794XX_RTCWKDAY_OSCRUN) == 0 && retries > 0);

  return OK;
}

#endif /* CONFIG_RTC_MCP794XX */
