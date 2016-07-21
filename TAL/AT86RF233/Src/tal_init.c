/**
 * @file tal_init.c
 *
 * @brief This file implements functions for initializing TAL.
 *
 * $Id: tal_init.c 33806 2012-11-09 15:53:06Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === INCLUDES ============================================================ */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "pal.h"
#include "return_val.h"
#include "tal.h"
#include "ieee_const.h"
#include "tal_pib.h"
#include "tal_irq_handler.h"
#include "stack_config.h"
#include "bmm.h"
#include "qmm.h"
#include "pal.h"
#include "tal.h"
#include "tal_internal.h"
#include "tal_constants.h"
#include "at86rf233.h"
#include "tal_config.h"
#ifdef BEACON_SUPPORT
#include "tal_slotted_csma.h"
#endif  /* BEACON_SUPPORT */
#ifdef ENABLE_TFA
#include "tfa.h"
#endif
#ifdef NON_BLOCKING_SPI
#include "tal_rx.h"
#endif
#include "mac_build_config.h"
#ifdef STB_ON_SAL
#include "sal_types.h"
#if (SAL_TYPE == AT86RF2xx)
#include "stb.h"
#endif
#endif

/* === MACROS ============================================================== */

/* Value in us used for delay between poll attempts for transceiver access. */
#define TRX_POLL_WAIT_TIME_US       (100)

/* Ratio between max time of TR1 / transceiver poll delay */
#define P_ON_TO_CLKM_ATTEMPTS       ((uint8_t) \
                                     (P_ON_TO_CLKM_AVAILABLE_MAX_US / TRX_POLL_WAIT_TIME_US))

/* Ratio between max time of TR2 / transceiver poll delay */
#define SLEEP_TO_TRX_OFF_ATTEMPTS   ((uint8_t) \
                                     (SLEEP_TO_TRX_OFF_MAX_US / TRX_POLL_WAIT_TIME_US))

/* Ratio between max time of TR15 / transceiver poll delay */
#define P_ON_TO_TRX_OFF_ATTEMPTS    ((uint8_t) \
                                     (P_ON_TO_TRX_OFF_AFTER_CLKM_AVAILABLE_MAX_US / \
                                      TRX_POLL_WAIT_TIME_US))

/* === GLOBALS ============================================================= */


/* === PROTOTYPES ========================================================== */

static retval_t trx_init(void);
static retval_t trx_reset(void);
static retval_t internal_tal_reset(bool set_default_pib);

/* === IMPLEMENTATION ====================================================== */


/**
 * @brief Initializes the TAL
 *
 * This function is called to initialize the TAL. The transceiver is
 * initialized, the TAL PIBs are set to their default values, and the TAL state
 * machine is set to TAL_IDLE state.
 *
 * @return MAC_SUCCESS  if the transceiver state is changed to TRX_OFF and the
 *                 current device part number and version number are correct;
 *         FAILURE otherwise
 */
retval_t tal_init(void)
{
    /* Init the PAL and by this means also the transceiver interface */
#ifdef ENABLE_RP
    /*
     * The ranging processor (RP) only performs a minimalistic
     * initialization here.
     */
    pal_basic_init();
#else  /* !ENABLE_RP */
    if (pal_init() != MAC_SUCCESS)
    {
        return FAILURE;
    }

    if (trx_init() != MAC_SUCCESS)
    {
        return FAILURE;
    }

#if (EXTERN_EEPROM_AVAILABLE == 1)
    pal_ps_get(EXTERN_EEPROM, EE_IEEE_ADDR, 8, &tal_pib.IeeeAddress);
#else
    pal_ps_get(INTERN_EEPROM, EE_IEEE_ADDR, 8, &tal_pib.IeeeAddress);
#endif

    /*
     * Do the reset stuff.
     * Set the default PIBs.
     * Generate random seed.
     */
    if (internal_tal_reset(true) != MAC_SUCCESS)
    {
        return FAILURE;
    }

#ifndef DISABLE_IEEE_ADDR_CHECK
    /* Check if a valid IEEE address is available. */
    /*
     * This while loop is on purpose, since just in the
     * rare case that such an address is randomly
     * generated again, we must repeat this.
     */
    while ((tal_pib.IeeeAddress == 0x0000000000000000) ||
           (tal_pib.IeeeAddress == 0xFFFFFFFFFFFFFFFF))
    {
        /*
         * In case no valid IEEE address is available, a random
         * IEEE address will be generated to be able to run the
         * applications for demonstration purposes.
         * In production code this can be omitted.
         */
        /*
         * The proper seed for function rand() has already been generated
         * in function tal_generate_rand_seed().
         */
        uint8_t *ptr_pib = (uint8_t *)&tal_pib.IeeeAddress;

        for (uint8_t i = 0; i < 8; i++)
        {
            *ptr_pib++ = (uint8_t)rand();
            /*
             * Note:
             * Even if casting the 16 bit rand value back to 8 bit,
             * and running the loop 8 timers (instead of only 4 times)
             * may look cumbersome, it turns out that the code gets
             * smaller using 8-bit here.
             * And timing is not an issue at this place...
             */
        }
    }
#endif  /* #ifndef DISABLE_IEEE_ADDR_CHECK */
#endif  /* ENABLE_RP */

    /*
     * Configure interrupt handling.
     * Install a handler for the transceiver interrupt.
     */
    pal_trx_irq_init(trx_irq_handler_cb);
#ifndef ENABLE_RP
    pal_trx_irq_en();   /* Enable transceiver main interrupt. */
#endif

#if ((defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)) && (DISABLE_TSTAMP_IRQ == 0)
    /* Configure time stamp interrupt. */
    pal_trx_irq_init_tstamp(trx_irq_timestamp_handler_cb);
#ifndef ENABLE_RP
    pal_trx_irq_en_tstamp();    /* Enable timestamp interrupt. */
#endif
#endif

    /* Initialize the buffer management module and get a buffer to store reveived frames. */
    bmm_buffer_init();
    tal_rx_buffer = bmm_buffer_alloc(LARGE_BUFFER_SIZE);
#if DEBUG > 0
    if (tal_rx_buffer == NULL)
    {
        return FAILURE;
    }
#endif

    /* Init incoming frame queue */
#ifdef ENABLE_QUEUE_CAPACITY
    qmm_queue_init(&tal_incoming_frame_queue, TAL_INCOMING_FRAME_QUEUE_CAPACITY);
#else
    qmm_queue_init(&tal_incoming_frame_queue);
#endif  /* ENABLE_QUEUE_CAPACITY */

#ifdef ENABLE_TFA
    tfa_init();
#endif

    return MAC_SUCCESS;
} /* tal_init() */


#if defined(ENABLE_RP) || defined(ENABLE_RH)
/*
 * Ranging host (RH) and ranging processor (RP) routines to handover
 * control of the transceiver to each other.
 */

/**
 * @brief todo
 *
 */
retval_t tal_rampup(void)
{
    pal_trx_init();

    if (trx_init() != MAC_SUCCESS)
    {
        return FAILURE;
    }

    /*
     * Do the reset stuff.
     * Set the default PIBs.
     * Generate random seed.
     */
    if (internal_tal_reset(true) != MAC_SUCCESS)
    {
        return FAILURE;
    }

    /*
     * Activate interrupt handling.
     */
    pal_trx_irq_en();   /* Enable transceiver main interrupt. */

#if ((defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)) && (DISABLE_TSTAMP_IRQ == 0)
    /* Configure time stamp interrupt. */
    pal_trx_irq_en_tstamp();    /* Enable timestamp interrupt. */
#endif

    return MAC_SUCCESS;
} /* tal_rampup() */

/**
 * @brief todo
 *
 */
void tal_rampdown(void)
{
    /*
     * Deactivate interrupt handling.
     */
    pal_trx_irq_dis();   /* Enable transceiver main interrupt. */

#if ((defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)) && (DISABLE_TSTAMP_IRQ == 0)
    /* Configure time stamp interrupt. */
    pal_trx_irq_dis_tstamp();    /* Enable timestamp interrupt. */
#endif

    pal_trx_uninit(false);      /* release SPI interface */

} /* tal_rampdown() */
#endif  /* defined(ENABLE_RP) || defined(ENABLE_RH) */

/**
 * @brief Initializes the transceiver
 *
 * This function is called to initialize the transceiver.
 *
 * @return MAC_SUCCESS  if the transceiver state is changed to TRX_OFF and the
 *                 current device part number and version number are correct;
 *         FAILURE otherwise
 */
static retval_t trx_init(void)
{
    tal_trx_status_t trx_status;
    uint8_t poll_counter = 0;

    PAL_RST_HIGH();
    PAL_SLP_TR_LOW();

    /* Wait typical time of timer TR1. */
    pal_timer_delay(P_ON_TO_CLKM_AVAILABLE_TYP_US);

    /* Apply reset pulse */
    PAL_RST_LOW();
    pal_timer_delay(RST_PULSE_WIDTH_US);
    PAL_RST_HIGH();

#if !(defined FPGA_EMULATION)
    do
    {
        /* Wait not more than max. value of TR1. */
        if (poll_counter == P_ON_TO_CLKM_ATTEMPTS)
        {
            return FAILURE;
        }
        /* Wait a short time interval. */
        pal_timer_delay(TRX_POLL_WAIT_TIME_US);
        poll_counter++;
        /* Check if AT86RF233 is connected; omit manufacturer id check */
    }
    while (pal_trx_reg_read(RG_PART_NUM) != PART_NUM_AT86RF233);
#endif  /* !defined FPGA_EMULATION */

    /* Verify that TRX_OFF can be written */
    pal_trx_reg_write(RG_TRX_STATE, CMD_TRX_OFF);

    /* Verify that the trx has reached TRX_OFF. */
    poll_counter = 0;
    do
    {
        /* Wait a short time interval. */
        pal_timer_delay(TRX_POLL_WAIT_TIME_US);

        trx_status = (tal_trx_status_t)pal_trx_bit_read(SR_TRX_STATUS);

        /* Wait not more than max. value of TR15. */
        if (poll_counter == P_ON_TO_TRX_OFF_ATTEMPTS)
        {
#if (DEBUG > 0)
            pal_alert();
#endif
            return FAILURE;
        }
        poll_counter++;
    }
    while (trx_status != TRX_OFF);

    tal_trx_status = TRX_OFF;

    return MAC_SUCCESS;
}


/**
 * @brief Internal TAL reset function
 *
 * @param set_default_pib Defines whether PIB values need to be set
 *                        to its default values
 *
 * @return MAC_SUCCESS  if the transceiver state is changed to TRX_OFF and the
 *                 current device part number and version number are correct;
 *         FAILURE otherwise
 */
static retval_t internal_tal_reset(bool set_default_pib)
{
    if (trx_reset() != MAC_SUCCESS)
    {
        return FAILURE;
    }

    /*
     * Generate a seed for the random number generator in function rand().
     * This is required (for example) as seed for the CSMA-CA algorithm.
     */
    tal_generate_rand_seed();

    /* Configure the transceiver register values. */
    trx_config();

    if (set_default_pib)
    {
        /* Set the default PIB values */
        init_tal_pib(); /* implementation can be found in 'tal_pib.c' */
    }
    else
    {
        /* nothing to do - the current TAL PIB attribute values are used */
    }

    /*
     * Write all PIB values to the transceiver
     * that are needed by the transceiver itself.
     */
    write_all_tal_pib_to_trx(); /* implementation can be found in 'tal_pib.c' */

    /* Reset TAL variables. */
    tal_state = TAL_IDLE;

#ifdef BEACON_SUPPORT
    tal_csma_state = CSMA_IDLE;
#endif  /* BEACON_SUPPORT */

#if ((MAC_START_REQUEST_CONFIRM == 1) && (defined BEACON_SUPPORT))
    tal_beacon_transmission = false;
#endif /* ((MAC_START_REQUEST_CONFIRM == 1) && (defined BEACON_SUPPORT)) */

    tal_rx_on_required = false;

    return MAC_SUCCESS;
}


/**
 * @brief Configures the transceiver
 *
 * This function is called to configure the transceiver after reset.
 */
void trx_config(void)
{
    /* Set pin driver strength */
    pal_trx_bit_write(SR_CLKM_SHA_SEL, CLKM_SHA_DISABLE);
    pal_trx_bit_write(SR_CLKM_CTRL, CLKM_1MHZ);

    /*
     * After we have initialized a proper seed for rand(),
     * the transceiver's CSMA seed can be initialized.
     * It needs to be assured that a seed for function rand()
     * had been generated before.
     */
    /*
     * Init the SEED value of the CSMA backoff algorithm.
     */
    uint16_t rand_value = (uint16_t)rand();
    pal_trx_reg_write(RG_CSMA_SEED_0, (uint8_t)rand_value);
    pal_trx_bit_write(SR_CSMA_SEED_1, (uint8_t)(rand_value >> 8));

    /*
     * Since the TAL is supporting 802.15.4-2006,
     * frames with version number 0 (compatible to 802.15.4-2003) and
     * with version number 1 (compatible to 802.15.4-2006) are acknowledged.
     */
    pal_trx_bit_write(SR_AACK_FVN_MODE, FRAME_VERSION_01);
    pal_trx_bit_write(SR_AACK_SET_PD, SET_PD); /* ACKs for data requests, indicate pending data */
    pal_trx_bit_write(SR_RX_SAFE_MODE, RX_SAFE_MODE_ENABLE);    /* Enable buffer protection mode */
    pal_trx_reg_write(RG_IRQ_MASK, TRX_IRQ_DEFAULT);    /* The TRX_END interrupt of the transceiver is enabled. */
    pal_trx_reg_write(RG_TRX_RPC, 0xFF); /* RPC feature configuration. */

#if (ANTENNA_DIVERSITY == 1)
    /* Use antenna diversity */
    pal_trx_bit_write(SR_ANT_CTRL, ANTENNA_DEFAULT);
    pal_trx_bit_write(SR_PDT_THRES, THRES_ANT_DIV_ENABLE);
    pal_trx_bit_write(SR_ANT_DIV_EN, ANT_DIV_ENABLE);
    pal_trx_bit_write(SR_ANT_EXT_SW_EN, ANT_EXT_SW_ENABLE);
#endif  /* ANTENNA_DIVERSITY */
#if (DISABLE_TSTAMP_IRQ == 0)
#if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)
    /* Enable Rx timestamping */
    pal_trx_bit_write(SR_IRQ_2_EXT_EN, RX_TIMESTAMPING_ENABLE);
    /* Enable Tx timestamping */
    pal_trx_bit_write(SR_ARET_TX_TS_EN, TX_ARET_TIMESTAMPING_ENABLE);
#endif  /* #if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP) */
#endif

#ifdef CCA_ED_THRESHOLD
    /*
     * Set CCA ED Threshold to other value than standard register due to
     * board specific loss (see pal_config.h). */
    pal_trx_bit_write(SR_CCA_ED_THRES, CCA_ED_THRESHOLD);
#endif

#ifdef EXT_RF_FRONT_END_CTRL
    /* Enable RF front end control */
    pal_trx_bit_write(SR_PA_EXT_EN, 1);
#endif
}



/**
 * @brief Reset transceiver
 *
 * @return MAC_SUCCESS  if the transceiver state is changed to TRX_OFF
 *         FAILURE otherwise
 */
static retval_t trx_reset(void)
{
    tal_trx_status_t trx_status;
    uint8_t poll_counter = 0;
#if (EXTERN_EEPROM_AVAILABLE == 1)
    uint8_t xtal_trim_value;
#endif

    /* Get trim value for 16 MHz xtal; needs to be done before reset due to board constraints. */
#if (EXTERN_EEPROM_AVAILABLE == 1)
    pal_ps_get(EXTERN_EEPROM, EE_XTAL_TRIM_ADDR, 1, &xtal_trim_value);
#endif

    /* trx might sleep, so wake it up */
    PAL_SLP_TR_LOW();
    pal_timer_delay(SLEEP_TO_TRX_OFF_TYP_US);

    /* Apply reset pulse */
    PAL_RST_LOW();
    pal_timer_delay(RST_PULSE_WIDTH_US);
    PAL_RST_HIGH();

    /* verify that trx has reached TRX_OFF */
    do
    {
        /* Wait a short time interval. */
        pal_timer_delay(TRX_POLL_WAIT_TIME_US);

        trx_status = (tal_trx_status_t)pal_trx_bit_read(SR_TRX_STATUS);

        /* Wait not more than max. value of TR2. */
        if (poll_counter == SLEEP_TO_TRX_OFF_ATTEMPTS)
        {
#if (DEBUG > 0)
            pal_alert();
#endif
            return FAILURE;
        }
        poll_counter++;
    }
    while (trx_status != TRX_OFF);

    tal_trx_status = TRX_OFF;

    /*
     * Write 16MHz xtal trim value to trx.
     * It's only necessary if it differs from the reset value.
     */
#if (EXTERN_EEPROM_AVAILABLE == 1)
    if (xtal_trim_value != 0x00)
    {
        pal_trx_bit_write(SR_XTAL_TRIM, xtal_trim_value);
    }
#endif

#ifdef STB_ON_SAL
#if (SAL_TYPE == AT86RF2xx)
    stb_restart();
#endif
#endif

    return MAC_SUCCESS;
}


/**
 * @brief Resets TAL state machine and sets the default PIB values if requested
 *
 * @param set_default_pib Defines whether PIB values need to be set
 *                        to its default values
 *
 * @return MAC_SUCCESS  if the transceiver state is changed to TRX_OFF
 *         FAILURE otherwise
 */
retval_t tal_reset(bool set_default_pib)
{
    /*
     * Do the reset stuff.
     * Set the default PIBs depending on the given parameter set_default_pib.
     * Do NOT generate random seed again.
     */
    if (internal_tal_reset(set_default_pib) != MAC_SUCCESS)
    {
        return FAILURE;
    }

#if (NUMBER_OF_TAL_TIMERS > 0)
    /* Clear all running TAL timers. */
    {
        uint8_t timer_id;

        ENTER_CRITICAL_REGION();

        for (timer_id = TAL_FIRST_TIMER_ID; timer_id <= TAL_LAST_TIMER_ID;
             timer_id++)
        {
            pal_timer_stop(timer_id);
        }

        LEAVE_CRITICAL_REGION();
    }
#endif

    /* Clear TAL Incoming Frame queue and free used buffers. */
    while (tal_incoming_frame_queue.size > 0)
    {
        buffer_t *frame = qmm_queue_remove(&tal_incoming_frame_queue, NULL);
        if (NULL != frame)
        {
            bmm_buffer_free(frame);
        }
    }

#ifdef ENABLE_TFA
    tfa_reset(set_default_pib);
#endif

    /*
     * Configure interrupt handling.
     * Install a handler for the transceiver interrupt.
     */
    pal_trx_irq_init(trx_irq_handler_cb);
    /* The pending transceiver interrupts on the microcontroller are cleared. */
    pal_trx_irq_flag_clr();
    pal_trx_irq_en();   /* Enable transceiver main interrupt. */

#ifdef ENABLE_FTN_PLL_CALIBRATION
    {
        /* Handle PLL calibration and filter tuning. */
        retval_t timer_status;

        /* Calibration timer has already been stopped within this function. */

        /* Start periodic calibration timer. */
        timer_status = pal_timer_start(TAL_CALIBRATION,
                                       TAL_CALIBRATION_TIMEOUT_US,
                                       TIMEOUT_RELATIVE,
                                       (FUNC_PTR())calibration_timer_handler_cb,
                                       NULL);

        if (timer_status != MAC_SUCCESS)
        {
            ASSERT("PLL calibration timer start problem" == 0);
        }
    }
#endif  /* ENABLE_FTN_PLL_CALIBRATION */

    return MAC_SUCCESS;
}



/**
 * @brief Generates a 16-bit random number used as initial seed for srand()
 *
 * This function generates a 16-bit random number by means of using the
 * Random Number Generator from the transceiver.
 * The Random Number Generator generates 2-bit random values. These 2-bit
 * random values are concatenated to the required 16-bit random seed.
 *
 * The generated random 16-bit number is feed into function srand()
 * as initial seed.
 *
 * The transceiver state is initally set to RX_ON.
 * After the completion of the random seed generation, the
 * trancseiver is set to TRX_OFF.
 *
 * As a prerequisite the preamble detector must not be disabled.
 *
 * Also in case the function is called from a different state than TRX_OFF,
 * additional trx state handling is required, such as reading the original
 * value and restoring this state after finishing the sequence.
 * Since in our case the function is called from TRX_OFF, this is not required
 * here.
 */
void tal_generate_rand_seed(void)
{
    uint16_t seed = 0;
    uint8_t cur_random_val = 0;

    /* RPC could influence the randomness; therefore disable it here. */
    uint8_t previous_RPC_value = pal_trx_reg_read(RG_TRX_RPC);
    pal_trx_reg_write(RG_TRX_RPC, 0xC1);

    /*
     * We need to disable TRX IRQs while generating random values in RX_ON,
     * we do not want to receive frames at this point of time at all.
     */
    ENTER_TRX_REGION();

    /* Ensure that PLL has locked and receive mode is reached. */
    tal_trx_status_t trx_state;
    do
    {
        trx_state = set_trx_state(CMD_RX_ON);
    }
    while (trx_state != RX_ON);

    /* Ensure that register bit RX_PDT_DIS is set to 0. */
    pal_trx_bit_write(SR_RX_PDT_DIS, RX_ENABLE);

    /*
     * The 16-bit random value is generated from various 2-bit random values.
     */
    for (uint8_t i = 0; i < 8; i++)
    {
        /* Now we can safely read the 2-bit random number. */
        cur_random_val = pal_trx_bit_read(SR_RND_VALUE);
        seed = seed << 2;
        seed |= cur_random_val;
        PAL_WAIT_1_US();    // wait that the random value gets updated
    }

    set_trx_state(CMD_FORCE_TRX_OFF);

    /*
     * Now we need to clear potential pending TRX IRQs and
     * enable the TRX IRQs again.
     */
    pal_trx_reg_read(RG_IRQ_STATUS);
    pal_trx_irq_flag_clr();
    LEAVE_TRX_REGION();

    /* Set the seed for the random number generator. */
    srand(seed);

    /* Restore RPC settings. */
    pal_trx_reg_write(RG_TRX_RPC, previous_RPC_value);
}
/* EOF */

