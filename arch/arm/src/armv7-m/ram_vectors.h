/****************************************************************************
 * arch/arm/src/armv7-m/ram_vectors.h
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

#ifndef __ARCH_ARM_SRC_ARMV7_M_RAM_VECTORS_H
#define __ARCH_ARM_SRC_ARMV7_M_RAM_VECTORS_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <arch/irq.h>

#include "arm_internal.h"
#include "chip.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef ARMV7M_PERIPHERAL_INTERRUPTS
#  error ARMV7M_PERIPHERAL_INTERRUPTS must be defined to the number of I/O interrupts to be supported
#endif

/* This is the size of the vector table (in 4-byte entries).  This size
 * includes the (1) the peripheral interrupts, (2) space for 15 Cortex-M
 * exceptions, and (3) IDLE stack pointer which lies at the beginning of the
 * table.
 */

#define ARMV7M_VECTAB_SIZE (ARMV7M_PERIPHERAL_INTERRUPTS + 16)

/* Vector Table Offset Register (VECTAB).  This mask seems to vary among
 * ARMv7-M implementations.  It may need to be redefined in some
 * architecture-specific header file. By default, the base address of the
 * new vector table must be aligned to the size of the vector table extended
 * to the next larger power of 2.
 */

#ifndef NVIC_VECTAB_TBLOFF_MASK
#  if ARMV7M_VECTAB_SIZE > 512
#    define NVIC_VECTAB_TBLOFF_MASK     (0xfffff000)
#  elif ARMV7M_VECTAB_SIZE > 256
#    define NVIC_VECTAB_TBLOFF_MASK     (0xfffff800)
#  elif ARMV7M_VECTAB_SIZE > 128
#    define NVIC_VECTAB_TBLOFF_MASK     (0xfffffc00)
#  elif ARMV7M_VECTAB_SIZE > 64
#    define NVIC_VECTAB_TBLOFF_MASK     (0xfffffe00)
#  elif ARMV7M_VECTAB_SIZE > 32
#    define NVIC_VECTAB_TBLOFF_MASK     (0xffffff00)
#  else
#    define NVIC_VECTAB_TBLOFF_MASK     (0xffffff80)
#  endif
#endif

/* Alignment ****************************************************************/

/* Per the ARMv7M Architecture reference manual, the NVIC vector table
 * requires 7-bit address alignment (i.e, bits 0-6 of the address of the
 * vector table must be zero).  In this case alignment to a 128 byte address
 * boundary is sufficient.
 *
 * Some parts, such as the LPC17xx/LPC40xx family, require alignment to a 256
 * byte address boundary.  Any other unusual alignment requirements for the
 * vector can be specified for a given architecture be redefining
 * NVIC_VECTAB_TBLOFF_MASK in the chip-specific chip.h header file for the
 * appropriate mask.
 */

#define VECTAB_ALIGN ((~NVIC_VECTAB_TBLOFF_MASK & 0xffff) + 1)

#ifdef CONFIG_ARCH_RAMVECTORS

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* If CONFIG_ARCH_RAMVECTORS is defined, then the ARM logic must provide
 * ARM-specific implementations of irq_initialize(), irq_attach(), and
 * irq_dispatch.  In this case, it is also assumed that the ARM vector
 * table resides in RAM, has the name g_ram_vectors, and has been
 * properly positioned and aligned in memory by the linker script.
 *
 * REVISIT: Can this alignment requirement vary from core-to-core?  Yes, it
 * depends on the number of vectors supported by the MCU. The safest thing
 * to do is to put the vector table at the beginning of RAM in order to force
 * the highest alignment possible.
 */

extern up_vector_t g_ram_vectors[ARMV7M_VECTAB_SIZE]
  locate_data(".ram_vectors") aligned_data(VECTAB_ALIGN);

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: arm_ramvec_initialize
 *
 * Description:
 *   Copy vectors to RAM an configure the NVIC to use the RAM vectors.
 *
 ****************************************************************************/

void arm_ramvec_initialize(void);

/****************************************************************************
 * Name: exception_common
 *
 * Description:
 *   This is the default, common vector handling entrypoint.
 *
 ****************************************************************************/

void exception_common(void);

/****************************************************************************
 * Name: arm_ramvec_attach
 *
 * Description:
 *   Configure the ram vector table so that IRQ number 'irq' will be
 *   dipatched by hardware to 'vector'
 *
 ****************************************************************************/

int arm_ramvec_attach(int irq, up_vector_t vector);

#endif /* CONFIG_ARCH_RAMVECTORS */
#endif /* __ARCH_ARM_SRC_ARMV7_M_RAM_VECTORS_H */
