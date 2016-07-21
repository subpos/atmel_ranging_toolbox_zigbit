/**
 * @file rtb_config.h
 *
 * @brief File contains RTB configuration parameters.
 *
 * $Id: rtb_config.h 34339 2013-02-22 10:11:19Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2011, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef RTB_CONFIG_H
#define RTB_CONFIG_H

/* === INCLUDES ============================================================ */

#include "tal_config.h"
#include "rtb_types.h"

/* === EXTERNALS =========================================================== */


/* === MACROS ============================================================== */

/* Check for compile switch dependencies that need to be obeyed. */

#if (RTB_TYPE == RTB_PMU_233R)
/*
 * Ranging is done using SPI access to transceiver
 * and requires SRAM access functionality.
 */
#   define ENABLE_TRX_SRAM
#endif


#if (RTB_TYPE == RTB_PMU_RFR2)
/*
 * Enable Extended IRQ API for redirecting IRQ-handler.
 */
#   define PAL_XTD_IRQ_API

/*
 * Enable the PMU timer.
 */
#   define ENABLE_PMU_TIMER
#endif


#if defined(ENABLE_RP)
/* In case RP is specifically enabled, the highest stack layer must be RTB. */
#   if (HIGHEST_STACK_LAYER != RTB)
#       error ("HIGHEST_STACK_LAYER must be RTB if ENABLE_RP is defined")
#   endif  /* #if (HIGHEST_STACK_LAYER != RTB) */
#endif

#if (HIGHEST_STACK_LAYER == RTB)
/* No MAC layer included; only limited resource allocation allowed */

/*
 * MAC queues and task functions are not available, so a variety of
 * services for the application need to be provided by the RTB additionally
 * itself (such as application queues, stack initialization functions, etc.).
 * These services are enabled by the following compile switch.
 */
#   define RTB_WITHOUT_MAC

/* Check whether RTB Remote Ranging is illegally enabled. */
#   ifdef ENABLE_RTB_REMOTE
#       error ("ENABLE_RTB_REMOTE must NOT be defined if HIGHEST_STACK_LAYER == RTB")
#   endif

#endif  /* (HIGHEST_STACK_LAYER == RTB) */

#ifdef ENABLE_RTB_REMOTE
/*
 * In case remote ranging shall be supported, the highest stack layer
 * must be MAC or larger, i.e. the highest stack layer must NOT be RTB.
 */
#   if (HIGHEST_STACK_LAYER == RTB)
#       error ("HIGHEST_STACK_LAYER must NOT be RTB if ENABLE_RTB_REMOTE is defined")
#   endif   /* (HIGHEST_STACK_LAYER == RTB) */
#endif  /* #ifdef ENABLE_RTB_REMOTE */

#ifdef ENABLE_RH
/*
 * In case RTB PIB attribute handling only shall be supported (i.e. as
 * Ranging Host), the RTB type must be RTB_FOR_RH.
 */
#   ifdef (RTB_TYPE != RTB_FOR_RH)
#       error ("RTB_TYPE must be RTB_FOR_RH if ENABLE_RH is defined")
#   endif   /* (RTB_TYPE != RTB_FOR_RH) */

#endif  /* #ifdef ENABLE_RH */

/* === TYPES =============================================================== */

#if (NUMBER_OF_TAL_TIMERS == 0)
#   define RTB_FIRST_TIMER_ID           (0)
#else
#   define RTB_FIRST_TIMER_ID           (TAL_LAST_TIMER_ID + 1)
#endif

/* Timer ID's used by RTB */
typedef enum rtb_timer_id_tag
{
    T_RTB_Wait_Time                 = (RTB_FIRST_TIMER_ID)
} rtb_timer_id_t;

#define NUMBER_OF_RTB_TIMERS        (1)

#if (NUMBER_OF_RTB_TIMERS > 0)
#   define RTB_LAST_TIMER_ID    (RTB_FIRST_TIMER_ID + NUMBER_OF_RTB_TIMERS - 1) // -1: timer id starts with 0
#else
#   define RTB_LAST_TIMER_ID    (RTB_FIRST_TIMER_ID)
#endif

/* === PROTOTYPES ========================================================== */


#endif /* RTB_CONFIG_H */
