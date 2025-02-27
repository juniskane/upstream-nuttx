/****************************************************************************
 * boards/arm/xmc4/xmc4700-relax/src/xmc4700-relax.h
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

#ifndef __BOARDS_ARM_XMC4_XMC4700_RELAX_SRC_XMC4700_RELAX_H
#define __BOARDS_ARM_XMC4_XMC4700_RELAX_SRC_XMC4700_RELAX_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include "xmc4_gpio.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* LEDs
 *
 * The XMC4700 Relax board has two LEDs:
 *
 * LED1 P5.9, Pad type A1+, High output illuminates
 * LED2 P5.8, Pad type A1+ High output illuminates
 */

#define GPIO_LED1 (GPIO_OUTPUT | GPIO_OUTPUT_PUSHPULL |             \
                   GPIO_PADA1P_STRONGSOFT | GPIO_PINCTRL_SOFTWARE | \
                   GPIO_OUTPUT_CLEAR | GPIO_PORT5 | GPIO_PIN9)
#define GPIO_LED2 (GPIO_OUTPUT | GPIO_OUTPUT_PUSHPULL |             \
                   GPIO_PADA1P_STRONGSOFT | GPIO_PINCTRL_SOFTWARE | \
                   GPIO_OUTPUT_CLEAR | GPIO_PORT5 | GPIO_PIN8)

/* BUTTONS
 *
 * The XMC4700 Relax board has two buttons:
 *
 * BUTTON1 P15.13, Pad type A2, Low input sensed when button pressed
 * BUTTON2 P15.12, Pad type A2, Low input sensed when button pressed
 */

#define GPIO_BUTTON1 (GPIO_INPUT | GPIO_PINCTRL_SOFTWARE | \
                      GPIO_PORT15 | GPIO_PIN13)
#define GPIO_BUTTON2 (GPIO_INPUT | GPIO_PINCTRL_SOFTWARE | \
                      GPIO_PORT15 | GPIO_PIN12)

/* SPIs Chip select */

#define GPIO_CS (GPIO_OUTPUT | GPIO_OUTPUT_PUSHPULL |             \
                 GPIO_PADA1P_STRONGSOFT | GPIO_PINCTRL_SOFTWARE | \
                 GPIO_OUTPUT_SET | GPIO_PORT3 | GPIO_PIN10)

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: xmc4_bringup
 *
 * Description:
 *   Bring up board features
 *
 ****************************************************************************/

int xmc4_bringup(void);

#endif /* __ASSEMBLY__ */
#endif /* __BOARDS_ARM_XMC4_XMC4700_RELAX_SRC_XMC4700_RELAX_H */
