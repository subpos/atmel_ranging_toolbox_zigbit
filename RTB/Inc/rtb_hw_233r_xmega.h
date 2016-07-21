/**
 * @file rtb_hw_233r_xmega.h
 *
 * @brief Header file for AT86RF233R on Xmega platforms dependent functionality of RTB
 *
 * $Id: rtb_hw_233r_xmega.h 34343 2013-02-22 11:45:08Z sschneid $
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
#ifndef RTB_HW_233R_XMEGA_H
#define RTB_HW_233R_XMEGA_H

/* === Includes ============================================================= */

#include "rtb_types.h"
#if (RTB_TYPE == RTB_PMU_233R)

/* === Macros =============================================================== */

#define T_INC_VAL           (8)

/** Time synchronization offset for Initiator. */
#define TIMER_OFFSET_DEV_A  (0 + 3)
/** Time synchronization offset for Reflector. */
#define TIMER_OFFSET_DEV_B  (9 + 3)

/** Start the synchronicity timer. */
#define start_timer(toffs) do {swOcrValue += toffs;} while(0)

/** Wait for synchronicity of timers. */
#define wait_timer() do {while(TCC1.CNTL != swOcrValue);} while(0)

/** Default Ranging Transmit Power is set to -17dBm. */
#define RTB_TRANSMIT_POWER_DEFAULT      (0xAF)

/* === Types ================================================================ */


/* === Externals ============================================================ */

/* The software timer output compare value. */
extern uint8_t swOcrValue;

/* Do NOT change this. */
extern SPI_t *rtb_trx_spi;
extern PORT_t *rtb_trx_spi_port;

/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

    void set_slp_trx_high(void);
    void set_slp_trx_low(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTB_PMU_233R */

#endif /* RTB_HW_233R_XMEGA_H */

/* EOF */
