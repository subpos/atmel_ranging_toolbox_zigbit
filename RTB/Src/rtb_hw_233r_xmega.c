/**
 * @file rtb_hw_233r_xmega.c
 *
 * @brief Platform dependent functionality of RTB using AT86RF233R on Xmega
 *
 * This file implements platform dependent functionality within the RTB
 * using AT86RF233R on Xmega.
 *
 * $Id: rtb_hw_233r_xmega.c 34342 2013-02-22 10:44:18Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2011, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */
/* === Includes ============================================================ */

#include "rtb_types.h"
#include "pal.h"
#if ((RTB_TYPE == RTB_PMU_233R) && (PAL_GENERIC_TYPE == XMEGA))

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "rtb_internal.h"

/* === Macros ============================================================== */


/* === Globals ============================================================= */

uint8_t swOcrValue;
/*
 * Pointer to SPI structure to control SPI between TRX and MCU during
 * the actual Ranging procedure.
 */
/* Do NOT change or move this to another file. */
SPI_t *rtb_trx_spi = &TRX_SPI;
PORT_t *rtb_trx_spi_port = &TRX_SPI_PORT;

/* === Externals =========================================================== */


/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

void set_slp_trx_high(void)
{
    PAL_SLP_TR_HIGH();
}



void set_slp_trx_low(void)
{

    PAL_SLP_TR_LOW();
}



/**
 * Function initializing the Timestamp IRQ to get synchronized
 * as required for ranging.
 */
void rtb_tstamp_irq_init(void)
{
    pal_trx_irq_dis();

    pal_trx_reg_read(RG_IRQ_STATUS);
    pal_trx_bit_write(SR_ARET_TX_TS_EN, 0x01);
    pal_trx_bit_write(SR_IRQ_2_EXT_EN, 0x01);

    PORTC.INTFLAGS = PORT_INT1IF_bm;

    TCC1_CTRLB &= ~TC1_CCAEN_bm;

    TIMER_SRC_DURING_TRX_AWAKE();

    PORTC.INT1MASK = PIN1_bm;

    /* Reset register, until time out is triggered is 65535 ms */
    TCC1_CNT = 0;

    PORTC.PIN1CTRL = PORT_ISC1_bm;

    TCC1_INTFLAGS = TC1_CCAIF_bm;

    TCC1_INTCTRLB = TC_CCAINTLVL_HI_gc;

    TCC1_CTRLB |= TC1_CCAEN_bm;
}



/**
 * Function disabling the Timestamp IRQ as utilized for ranging.
 */
void rtb_tstamp_irq_exit(void)
{
    TCC1_INTCTRLB = TC_CCAINTLVL_OFF_gc;

    TCC1_CTRLB &= ~TC1_CCAEN_bm;

    /* Clear status register. */
    pal_trx_reg_read(RG_IRQ_STATUS);

    /* Enable main Trx IRQ. */
    pal_trx_irq_en();

    /* Re-enable all interrupts. */
    pal_global_irq_enable();
}



/**
 * Time stamp IRQ handler function.
 */
ISR(TCC1_CCA_vect)
{
    TCC1_CTRLB = 0;
    TCC1_INTCTRLB = TC_CCAINTLVL_OFF_gc;
    swOcrValue = TCC1.CNTL + 2;

    while (TCC1.CNTL != swOcrValue);

    if (RTB_ROLE_INITIATOR == rtb_role)
    {
        start_timer(TIMER_OFFSET_DEV_A);
    }
    else
    {
        start_timer(TIMER_OFFSET_DEV_B);
    }

    wait_timer();

    start_timer(64);

    timer_is_synced = true;
}

#endif  /* ((RTB_TYPE == RTB_PMU_233R) && (PAL_GENERIC_TYPE == XMEGA)) */

/* EOF */
