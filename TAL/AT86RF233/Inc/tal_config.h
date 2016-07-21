/**
 * @file tal_config.h
 *
 * @brief File contains TAL configuration parameters.
 *
 * $Id: tal_config.h 33649 2012-10-11 08:57:12Z kschwieg $
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
#ifndef TAL_CONFIG_H
#define TAL_CONFIG_H

/* === INCLUDES ============================================================ */

#include "at86rf233.h"

/* === EXTERNALS =========================================================== */

/* === MACROS ============================================================== */

#define TAL_RADIO_WAKEUP_TIME_SYM       (TAL_CONVERT_US_TO_SYMBOLS(SLEEP_TO_TRX_OFF_TYP_US))
#define TAL_FIRST_TIMER_ID              (0)

#ifndef ANTENNA_DEFAULT
#define ANTENNA_DEFAULT                 (ANT_CTRL_1)
#endif

#ifdef ENABLE_FTN_PLL_CALIBRATION
/*
 * PLL calibration and filter tuning timer timeout in minutes
 */
#define TAL_CALIBRATION_TIMEOUT_MIN         (5UL)

/*
 * PLL calibration and filter tuning timer timeout in us,
 */
#define TAL_CALIBRATION_TIMEOUT_US          ((TAL_CALIBRATION_TIMEOUT_MIN) * (60UL) * (1000UL) * (1000UL))
#endif  /* ENABLE_FTN_PLL_CALIBRATION */

/* === TYPES =============================================================== */

/* Timer ID's used by TAL */
/* TAL_TFA is a timer used in TFA, it is defined here (and not in TFA) for
 * simplicity
 */
#ifdef BEACON_SUPPORT
// Beacon Support
#ifdef ENABLE_FTN_PLL_CALIBRATION
#ifdef ENABLE_TFA
typedef enum tal_timer_id_tag
{
    TAL_CSMA_CCA                    = (TAL_FIRST_TIMER_ID),
    TAL_CSMA_BEACON_LOSS_TIMER      = (TAL_FIRST_TIMER_ID + 1),
    TAL_CALIBRATION                 = (TAL_FIRST_TIMER_ID + 2),
    TAL_TFA                         = (TAL_FIRST_TIMER_ID + 3)
} SHORTENUM tal_timer_id_t;

#define NUMBER_OF_TAL_TIMERS        (4)
#else /* No ENABLE_TFA */
typedef enum tal_timer_id_tag
{
    TAL_CSMA_CCA                    = (TAL_FIRST_TIMER_ID),
    TAL_CSMA_BEACON_LOSS_TIMER      = (TAL_FIRST_TIMER_ID + 1),
    TAL_CALIBRATION                 = (TAL_FIRST_TIMER_ID + 2)
} SHORTENUM tal_timer_id_t;

#define NUMBER_OF_TAL_TIMERS        (3)
#endif /* ENABLE_TFA */
#else
#ifdef ENABLE_TFA
typedef enum tal_timer_id_tag
{
    TAL_CSMA_CCA                    = (TAL_FIRST_TIMER_ID),
    TAL_CSMA_BEACON_LOSS_TIMER      = (TAL_FIRST_TIMER_ID + 1),
    TAL_TFA                         = (TAL_FIRST_TIMER_ID + 2)
} SHORTENUM tal_timer_id_t;

#define NUMBER_OF_TAL_TIMERS        (3)
#else /* No ENABLE_TFA */
typedef enum tal_timer_id_tag
{
    TAL_CSMA_CCA                    = (TAL_FIRST_TIMER_ID),
    TAL_CSMA_BEACON_LOSS_TIMER      = (TAL_FIRST_TIMER_ID + 1)
} SHORTENUM tal_timer_id_t;

#define NUMBER_OF_TAL_TIMERS        (2)
#endif /* ENABLE_TFA */
#endif  /* ENABLE_FTN_PLL_CALIBRATION */
#else /* No BEACON_SUPPORT */
#ifdef ENABLE_FTN_PLL_CALIBRATION
#ifdef ENABLE_TFA
typedef enum tal_timer_id_tag
{
    TAL_CALIBRATION                 = (TAL_FIRST_TIMER_ID),
    TAL_TFA                         = (TAL_FIRST_TIMER_ID + 1)
} SHORTENUM tal_timer_id_t;

#define NUMBER_OF_TAL_TIMERS        (2)
#else /* No ENABLE_TFA */
typedef enum tal_timer_id_tag
{
    TAL_CALIBRATION                 = (TAL_FIRST_TIMER_ID)
} SHORTENUM tal_timer_id_t;

#define NUMBER_OF_TAL_TIMERS        (1)
#endif /* ENABLE_TFA */
#else
#ifdef ENABLE_TFA
typedef enum tal_timer_id_tag
{
    TAL_TFA                         = (TAL_FIRST_TIMER_ID)
} SHORTENUM tal_timer_id_t;

#define NUMBER_OF_TAL_TIMERS        (1)
#else /* No ENABLE_TFA */
#define NUMBER_OF_TAL_TIMERS        (0)
#endif /* ENABLE_TFA */
#endif  /* ENABLE_FTN_PLL_CALIBRATION */
#endif  /* BEACON_SUPPORT */

#if (NUMBER_OF_TAL_TIMERS > 0)
#define TAL_LAST_TIMER_ID    (TAL_FIRST_TIMER_ID + NUMBER_OF_TAL_TIMERS - 1) // -1: timer id starts with 0
#else
#define TAL_LAST_TIMER_ID    (TAL_FIRST_TIMER_ID)
#endif

#ifdef ENABLE_QUEUE_CAPACITY
#define TAL_INCOMING_FRAME_QUEUE_CAPACITY   (255)
#endif  /* ENABLE_QUEUE_CAPACITY */

/* === PROTOTYPES ========================================================== */


#endif /* TAL_CONFIG_H */
