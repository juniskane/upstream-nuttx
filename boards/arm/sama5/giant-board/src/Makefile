############################################################################
# boards/arm/sama5/giant-board/src/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
#  Licensed to the Apache Software Foundation (ASF) under one or more
#  contributor license agreements.  See the NOTICE file distributed with
#  this work for additional information regarding copyright ownership.  The
#  ASF licenses this file to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance with the
#  License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
#  License for the specific language governing permissions and limitations
#  under the License.
#
############################################################################/

include $(TOPDIR)/Make.defs

CSRCS = sam_boot.c

ifeq ($(CONFIG_SAMA5_SPI0),y)
CSRCS += sam_spi.c
else
ifeq ($(CONFIG_SAMA5_SPI1),y)
CSRCS += sam_spi.c
endif
endif

ifeq ($(CONFIG_SAMA5_DDRCS),y)
CSRCS += sam_sdram.c
endif

ifeq ($(CONFIG_SAMA5_SDMMC0),y)
CSRCS += sam_sdmmc.c
else
ifeq ($(CONFIG_SAMA5_SDMMC1),y)
CSRCS += sam_sdmmc.c
endif

endif
ifeq ($(CONFIG_SAMA5_HSMCI0),y)
CSRCS += sam_hsmci.c
else
ifeq ($(CONFIG_SAMA5_HSMCI1),y)
CSRCS += sam_hsmci.c
endif
endif

ifeq ($(CONFIG_SAMA5_UHPHS),y)
CSRCS += sam_usb.c
else
ifeq ($(CONFIG_SAMA5_UDPHS),y)
CSRCS += sam_usb.c
endif
endif

ifeq ($(CONFIG_BOARDCTL),y)
CSRCS += sam_appinit.c
CSRCS += sam_bringup.c
endif

ifeq ($(CONFIG_ADC),y)
CSRCS += sam_adc.c
ifeq ($(CONFIG_INPUT_AJOYSTICK),y)
CSRCS += sam_ajoystick.c
endif
endif

ifeq ($(CONFIG_PWM),y)
CSRCS += sam_pwm.c
endif

ifeq ($(CONFIG_AUDIO_I2SCHAR),y)
ifeq ($(CONFIG_SAMA5_SSC0),y)
CSRCS += sam_i2schar.c
else
ifeq ($(CONFIG_SAMA5_SSC1),y)
CSRCS += sam_i2schar.c
endif
endif
endif

ifeq ($(CONFIG_USBMSC),y)
CSRCS += sam_usbmsc.c
endif

ifeq ($(CONFIG_ARCH_LEDS),y)
CSRCS += sam_autoleds.c
else
CSRCS += sam_userleds.c
endif

ifeq ($(CONFIG_BOARDCTL_RESET),y)
CSRCS += sam_reset.c
endif

include $(TOPDIR)/boards/Board.mk
