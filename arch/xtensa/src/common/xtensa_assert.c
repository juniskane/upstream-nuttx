/****************************************************************************
 * arch/xtensa/src/common/xtensa_assert.c
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

#include <stdint.h>
#include <debug.h>
#include <assert.h>

#include <nuttx/irq.h>
#include <nuttx/board.h>
#include <nuttx/syslog/syslog.h>

#include <arch/board/board.h>

#include "sched/sched.h"
#include "xtensa.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: xtensa_panic
 *
 * Description:
 *   PANIC if an unhandled exception is received:
 *
 *   - NMI exception
 *   - Debug exception
 *   - Double exception
 *   - Kernel exception
 *   - Co-processor exception
 *   - High priority level2-6 Exception.
 *
 * Input Parameters:
 *   xcptcode - Identifies the unhandled exception (see include/esp32/irq.h)
 *   regs - The register save are at the time of the interrupt.
 *
 * Returned Value:
 *   Does not return
 *
 ****************************************************************************/

void xtensa_panic(int xptcode, uint32_t *regs)
{
  struct tcb_s **running_task = &g_running_tasks[this_cpu()];

  (*running_task)->xcp.regs = regs;

  up_set_interrupt_context(true);

  /* We get here when a un-dispatch-able, irrecoverable exception occurs */

  board_autoled_on(LED_ASSERTION);

  /* Flush any buffered SYSLOG data (from prior to the panic) */

  syslog_flush();

  _alert("Unhandled Exception %d task: %s\n", xptcode,
         get_task_name(running_task()));

  PANIC_WITH_REGS("panic", regs);  /* Should not return */
  for (; ; );
}

/****************************************************************************
 * Name: xtensa_user
 *
 * Description:
 *   PANIC if certain User Exceptions are received.  All values for EXCCAUSE
 *   are listed below (not all generate PANICs):
 *
 *   0  IllegalInstructionCause
 *      Illegal instruction
 *   1  SyscallCause
 *      SYSCALL instruction
 *   2  InstructionFetchErrorCause
 *      Processor internal physical address or data error during instruction
 *      fetch.
 *   3  LoadStoreErrorCause
 *      Processor internal physical address or data error during load or
 *      store.
 *   4  Level1InterruptCause
 *      Level-1 interrupt as indicated by set level-1 bits in the INTERRUPT
 *      register.
 *   5  AllocaCause
 *      MOVSP instruction, if caller's registers are not in the register
 *      file.
 *   6  IntegerDivideByZeroCause
 *      QUOS, QUOU, REMS, or REMU divisor operand is zero.
 *   7  PCValueErrorCause Next PC Value Illegal
 *   8  PrivilegedCause
 *      Attempt to execute a privileged operation when CRING != 0
 *   9  LoadStoreAlignmentCause
 *      Load or store to an unaligned address.
 *   10..11 Reserved for Cadence
 *   12 InstrPIFDataErrorCause
 *      PIF data error during instruction fetch.
 *   13 LoadStorePIFDataErrorCause
 *      Synchronous PIF data error during LoadStore access.
 *   14 InstrPIFAddrErrorCause
 *      PIF address error during instruction fetch.
 *   15 LoadStorePIFAddrErrorCause
 *      Synchronous PIF address error during LoadStore access.
 *   16 InstTLBMissCause
 *      Error during Instruction TLB refill
 *   17 InstTLBMultiHitCause
 *      Multiple instruction TLB entries matched
 *   18 InstFetchPrivilegeCause
 *      An instruction fetch referenced a virtual address at a ring level
 *      less than CRING.
 *   19 Reserved for Cadence
 *   20 InstFetchProhibitedCause
 *       An instruction fetch referenced a page mapped with an attribute
 *      that does not permit instruction fetch.
 *   21..23 Reserved for Cadence
 *   24 LoadStoreTLBMissCause
 *      Error during TLB refill for a load or store.
 *   25 LoadStoreTLBMultiHitCause
 *      Multiple TLB entries matched for a load or store.
 *   26 LoadStorePrivilegeCause
 *      A load or store referenced a virtual address at a ring level less
 *      than CRING.
 *   27 Reserved for Cadence
 *   28 LoadProhibitedCause
 *      A load referenced a page mapped with an attribute that does not
 *      permit loads.
 *   29 StoreProhibitedCause
 *     A store referenced a page mapped with an attribute that does not
 *     permit stores.
 *   30..31 Reserved for Cadence
 *   32..39 CoprocessornDisabled
 *     Coprocessor n instruction when cpn disabled. n varies 0..7 as the
 *     cause varies 32..39.
 *   40..63 Reserved
 *
 * Input Parameters:
 *   exccause - Identifies the EXCCAUSE of the user exception
 *   regs - The register save are at the time of the interrupt.
 *
 * Returned Value:
 *   Does not return
 *
 ****************************************************************************/

void xtensa_user_panic(int exccause, uint32_t *regs)
{
  struct tcb_s **running_task = &g_running_tasks[this_cpu()];

  (*running_task)->xcp.regs = regs;

  up_set_interrupt_context(true);

  /* We get here when a un-dispatch-able, irrecoverable exception occurs */

  board_autoled_on(LED_ASSERTION);

  /* Flush any buffered SYSLOG data (from prior to the error) */

  syslog_flush();

  _alert("User Exception: EXCCAUSE=%04x task: %s\n",
         exccause, get_task_name(running_task()));

  PANIC_WITH_REGS("user panic", regs); /* Should not return */
  for (; ; );
}
