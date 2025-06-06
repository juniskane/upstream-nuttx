/****************************************************************************
 * arch/arm/src/at32/at32_rtc.h
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

#ifndef __ARCH_ARM_SRC_AT32_AT32_RTC_H
#define __ARCH_ARM_SRC_AT32_AT32_RTC_H

#include <nuttx/config.h>

#include "chip.h"

#if defined(CONFIG_AT32_AT32F43XX)
#  include "hardware/at32_rtcc.h"
#endif

/* Alarm function differs from part to part */

#if defined(CONFIG_AT32_AT32F43XX)
#  include "at32f43xxx_alarm.h"
#else
#  include "at32_alarm.h"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define AT32_RTC_PRESCALER_SECOND        32767  /* Default prescaler to get a
                                                  * second base */
#define AT32_RTC_PRESCALER_MIN           1      /* Maximum speed of 16384 Hz */

#if !defined(CONFIG_AT32_RTC_MAGIC)
#  define CONFIG_AT32_RTC_MAGIC          (0xfacefeed)
#endif

#if !defined(CONFIG_AT32_RTC_MAGIC_TIME_SET)
#  define CONFIG_AT32_RTC_MAGIC_TIME_SET (0xf00dface)
#endif

#if !defined(CONFIG_AT32_RTC_MAGIC_REG)
#  define CONFIG_AT32_RTC_MAGIC_REG      (0)
#endif

#define RTC_MAGIC_REG                     AT32_RTC_BKR(CONFIG_AT32_RTC_MAGIC_REG)

#define RTC_MAGIC                         CONFIG_AT32_RTC_MAGIC
#define RTC_MAGIC_TIME_SET                CONFIG_AT32_RTC_MAGIC_TIME_SET

/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifndef __ASSEMBLY__

/****************************************************************************
 * Public Data
 ****************************************************************************/

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: at32_rtc_irqinitialize
 *
 * Description:
 *   Initialize IRQs for RTC, not possible during up_rtc_initialize because
 *   up_irqinitialize is called later.
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno on failure
 *
 ****************************************************************************/

int at32_rtc_irqinitialize(void);

/****************************************************************************
 * Name: at32_rtc_getdatetime_with_subseconds
 *
 * Description:
 *   Get the current date and time from the date/time RTC.  This interface
 *   is only supported by the date/time RTC hardware implementation.
 *   It is used to replace the system timer.  It is only used by the RTOS
 *   during initialization to set up the system time when CONFIG_RTC and
 *   CONFIG_RTC_DATETIME are selected (and CONFIG_RTC_HIRES is not).
 *
 *   NOTE: Some date/time RTC hardware is capability of sub-second accuracy.
 *   Thatsub-second accuracy is returned through 'nsec'.
 *
 * Input Parameters:
 *   tp - The location to return the high resolution time value.
 *   nsec - The location to return the subsecond time value.
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno on failure
 *
 ****************************************************************************/

#ifdef CONFIG_AT32_HAVE_RTC_SUBSECONDS
int at32_rtc_getdatetime_with_subseconds(struct tm *tp, long *nsec);
#endif

/****************************************************************************
 * Name: at32_rtc_setdatetime
 *
 * Description:
 *   Set the RTC to the provided time. RTC implementations which provide
 *   up_rtc_getdatetime() (CONFIG_RTC_DATETIME is selected) should provide
 *   this function.
 *
 * Input Parameters:
 *   tp - the time to use
 *
 * Returned Value:
 *   Zero (OK) on success; a negated errno on failure
 *
 ****************************************************************************/

#ifdef CONFIG_RTC_DATETIME
struct tm;
int at32_rtc_setdatetime(const struct tm *tp);
#endif

/****************************************************************************
 * Name: at32_rtc_lowerhalf
 *
 * Description:
 *   Instantiate the RTC lower half driver for the AT32.  General usage:
 *
 *     #include <nuttx/timers/rtc.h>
 *     #include "at32_rtc.h"
 *
 *     struct rtc_lowerhalf_s *lower;
 *     lower = at32_rtc_lowerhalf();
 *     rtc_initialize(0, lower);
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   On success, a non-NULL RTC lower interface is returned.  NULL is
 *   returned on any failure.
 *
 ****************************************************************************/

#ifdef CONFIG_RTC_DRIVER
struct rtc_lowerhalf_s;
struct rtc_lowerhalf_s *at32_rtc_lowerhalf(void);
#endif

#undef EXTERN
#if defined(__cplusplus)
}
#endif
#endif /* __ASSEMBLY__ */
#endif /* __ARCH_ARM_SRC_AT32_AT32_RTC_H */
