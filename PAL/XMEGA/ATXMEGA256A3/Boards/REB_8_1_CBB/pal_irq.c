/**
 * @file pal_irq.c
 *
 * @brief PAL IRQ functionality
 *
 * This file contains functions to initialize, enable, disable and install
 * handler for the transceiver interrupts.
 *
 * $Id: pal_irq.c 33806 2012-11-09 15:53:06Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === Includes ============================================================ */

#include <stdint.h>
#include "pal.h"
#include "pal_boardtypes.h"

#if (BOARD_TYPE == REB_8_1_CBB)

/* === Types ============================================================== */


/* === Globals ============================================================= */

/*
 * Function pointers to store the callback function of
 * the transceiver interrupt
 */
static irq_handler_t irq_hdl_trx;
#if ((defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)) && (DISABLE_TSTAMP_IRQ == 0)
static irq_handler_t irq_hdl_trx_tstamp;
#endif

/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

/**
 * @brief Initializes the transceiver main interrupt
 *
 * This function sets the microcontroller specific registers
 * responsible for handling the transceiver main interrupt
 *
 * @param trx_irq_cb Callback function for the transceiver main
 * interrupt
 */
void pal_trx_irq_init(FUNC_PTR(trx_irq_cb))
{
    /*
     * Set the handler function.
     * The handler is set before enabling the interrupt to prepare for spurious
     * interrupts, that can pop up the moment they are enabled
     */
    irq_hdl_trx = (irq_handler_t)trx_irq_cb;

    /* Rising edge on IRQ pin used to trigger IRQ */
    PORTC.PIN2CTRL = PORT_ISC0_bm;

    /* Set pin 2 as source for port interrupt 0 */
    PORTC.INT0MASK = PIN2_bm;

    /* Clear pending interrupts */
    PORTC.INTFLAGS = PORT_INT0IF_bm;
}


#if ((defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)) && (DISABLE_TSTAMP_IRQ == 0)
/**
 * @brief Initializes the transceiver timestamp interrupt
 *
 * This function sets the microcontroller specific registers
 * responsible for handling the transceiver timestamp interrupt
 *
 * @param trx_irq_cb Callback function for the transceiver timestamp
 * interrupt
 */
void pal_trx_irq_init_tstamp(FUNC_PTR(trx_irq_cb))
{
    /*
     * Set the handler function.
     * The handler is set before enabling the interrupt to prepare for spurious
     * interrupts, that can pop up the moment they are enabled
     */
    irq_hdl_trx_tstamp = (irq_handler_t)trx_irq_cb;

    /* Rising edge on DIG2 pin used to trigger IRQ */
    PORTC.PIN1CTRL = PORT_ISC0_bm;

    /* Set pin 1 as source for port interrupt 1 */
    PORTC.INT1MASK = PIN1_bm;

    /* Clear pending interrupts */
    PORTC.INTFLAGS = PORT_INT1IF_bm;
}
#endif


/**
 * @brief ISR for transceiver's main interrupt
 */
ISR(TRX_MAIN_ISR_VECTOR)
{
    irq_hdl_trx();
}


#if ((defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)) && (DISABLE_TSTAMP_IRQ == 0)
/**
 * @brief ISR for transceiver's timestamp interrupt
 */
ISR(TRX_TSTAMP_ISR_VECTOR)
{
#if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)
    /* Clear capture interrupt. */
    TCC1_INTFLAGS = TC1_CCAIF_bm;

    irq_hdl_trx_tstamp();
#endif
}
#endif

#endif /* REB_8_1_CBB */

/* EOF */
