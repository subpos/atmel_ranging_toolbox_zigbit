/**
 * @file pal_types.h
 *
 * @brief Definition of supported PAL types
 *
 * This header file contains the supported PAL types.
 *
 * $Id: pal_types.h 34062 2013-01-09 14:08:23Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef PAL_TYPES_H
#define PAL_TYPES_H

/* PAL_GENERIC_TYPE */
#define AVR                             (0x01)
#define MEGA_RF                         (0x02)
#define XMEGA                           (0x03)
#define AVR32                           (0x04)
#define ARM7                            (0x05)
#define SAM3                            (0x06)
#define SAM4                            (0x07)

#if (PAL_GENERIC_TYPE == AVR)
/* PAL_TYPE for AVR 8-bit MCUs */
#    define AT90USB1287                 (0x01)
#    define ATMEGA1281                  (0x02)
#    define ATMEGA1284P                 (0x03)
#    define ATMEGA2561                  (0x04)
#    define ATMEGA644P                  (0x05)

#elif (PAL_GENERIC_TYPE == XMEGA)
/* PAL_TYPE for XMEGA MCUs */
#    define ATXMEGA64A3                 (0x01)
#    define ATXMEGA128A1                (0x02)
#    define ATXMEGA128A3                (0x03)
#    define ATXMEGA256A3                (0x04)
#    define ATXMEGA256D3                (0x05)

#elif (PAL_GENERIC_TYPE == AVR32)
/* PAL_TYPE for AVR 32-bit MCUs */
#    define AT32UC3A3256               (0x01)
#    define AT32UC3L064                (0x02)
#    define AT32UC3A0512               (0x03)
#    define AT32UC3B1128               (0x04)
#    define AT32UC3B164                (0x05)
#elif (PAL_GENERIC_TYPE == ARM7)
/* PAL_TYPE for ARM7 MCUs */
#    define AT91SAM7S256               (0x01)
/* AT91SAM7X256 and AT91SAM7CX256:
 *
 * AT91SAM7CX256 is the same as AT91SAM7X256 except hardware AES engine.
 * If this boardtype is used, the file are actually taken from the
 * directory PAL/ARM7/AT91SAM7X256.
 * Only in case security shall be used, separate security files are selected
 * based on this PAL type.
 */
#    define AT91SAM7X256               (0x02)
#    define AT91SAM7XC256              (0x03)

#elif (PAL_GENERIC_TYPE == SAM3)

#    define AT91SAM3S4A                (0x01)
#    define AT91SAM3S4B                (0x02)
#    define AT91SAM3S4C                (0x03)

#elif (PAL_GENERIC_TYPE == SAM4)

#    define AT91SAM4S16C               (0x01)
#    define AT91SAM4S16B               (0x02)

#elif (PAL_GENERIC_TYPE == MEGA_RF)
/* PAL_TYPE for MEGA-RF single chips (MCU plus transceiver) */
#    define ATMEGA128RFA1              (0x01)
#    define ATMEGA256RFR2              (0x03)
/* ATmega2564RFR2 is the low-pincount version of ATmega256RFR2 */
#    define ATMEGA2564RFR2             ATMEGA256RFR2
#    define ATMEGA64RFR2               (0x05)
#    if (PAL_TYPE == ATMEGA128RFA1)
#        define __ATMEGA128RFA1__      (ATMEGA128RFA1)
#    endif
#    if (PAL_TYPE == ATMEGA256RFR2)
#        define __ATMEGA256RFR2__      (ATMEGA256RFR2)
#    endif
#    if (PAL_TYPE == ATMEGA64RFR2)
#        define __ATMEGA64RFR2__       (ATMEGA64RFR2)
#    endif
#else
#    error "Undefined PAL_GENERIC_TYPE"
#endif

/* Depending on the generic device type include platform-dependend types (IAR, GCC) */
#if ((PAL_GENERIC_TYPE == AVR) || (PAL_GENERIC_TYPE == XMEGA) || (PAL_GENERIC_TYPE == MEGA_RF))
#include "avrtypes.h"
#elif (PAL_GENERIC_TYPE == ARM7)
#include "armtypes.h"
#elif (PAL_GENERIC_TYPE == SAM3)
#include "core_cm3.h"
#include "armtypes.h"
#elif (PAL_GENERIC_TYPE == SAM4)
#include "armtypes.h"
#elif (PAL_GENERIC_TYPE == AVR32)
#include "avr32types.h"
#else
#error "Unknown PAL_GENERIC_TYPE"
#endif
#endif  /* PAL_TYPES_H */
/* EOF */
