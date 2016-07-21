/**
 * @file pal_trx_irq_spi.h
 *
 * @brief SPI based TRX IRQ API
 *
 * This header file declares prototypes of the SPI based TRX IRQ API.
 *
 * $Id: pal_trx_irq_spi.h 33806 2012-11-09 15:53:06Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2012, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef PAL_TRX_IRQ_SPI_H
#define PAL_TRX_IRQ_SPI_H

#if defined(PAL_USE_SPI_TRX) || defined(DOXYGEN)

/* === Includes ============================================================ */

#include <stdbool.h>
#include <stdint.h>
#include "pal_types.h"
#include "pal_config.h"

/* === Macros =============================================================== */


/* === Types =============================================================== */


/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief Initializes the transceiver main interrupt
     *
     * This function sets the microcontroller specific registers
     * responsible for handling the transceiver main interrupt
     *
     * @param trx_irq_cb Callback function for the transceiver main interrupt
     * @ingroup apiPalApi
     */
    void pal_trx_irq_init(FUNC_PTR(trx_irq_cb));

    /**
     * @brief Initializes the transceiver timestamp interrupt
     *
     * This function sets the microcontroller specific registers
     * responsible for handling the transceiver timestamp interrupt
     *
     * @param trx_irq_cb Callback function for the transceiver timestamp interrupt
     * @ingroup apiPalApi
     */
    void pal_trx_irq_init_tstamp(FUNC_PTR(trx_irq_cb));

    /**
     * @brief Enables the transceiver main interrupt
     *
     * This macro is only available for non-single chip transceivers, since
     * in single chip transceivers there is no separation between enabling
     * transceiver interrupts at the transceiver, and setting the IRQ mask
     * at the MCU. Therefore the transceiver interrupts in single chips are
     * enabled by setting the MCU IRQ mask.
     *
     * @ingroup apiPalApi
     */
#define pal_trx_irq_en()                ENABLE_TRX_IRQ()

    /**
     * @brief Enables the transceiver timestamp interrupt
     *
     * This macro is only available for non-single chip transceivers, since
     * in single chip transceivers there is no separation between enabling
     * transceiver interrupts at the transceiver, and setting the IRQ mask
     * at the MCU. Therefore the transceiver interrupts in single chips are
     * enabled by setting the MCU IRQ mask.
     *
     * @ingroup apiPalApi
     */
#define pal_trx_irq_en_tstamp()         ENABLE_TRX_IRQ_TSTAMP()

    /**
     * @brief Disables the transceiver main interrupt
     *
     * This macro is only available for non-single chip transceivers, since
     * in single chip transceivers there is no separation between disabling
     * transceiver interrupts at the transceiver, and clearing the IRQ mask
     * at the MCU. Therefore the transceiver interrupts in single chips are
     * disabled by clearing the MCU IRQ mask.
     *
     * @ingroup apiPalApi
     */
#define pal_trx_irq_dis()               DISABLE_TRX_IRQ()

    /**
     * @brief Disables the transceiver timestamp interrupt
     *
     * This macro is only available for non-single chip transceivers, since
     * in single chip transceivers there is no separation between disabling
     * transceiver interrupts at the transceiver, and clearing the IRQ mask
     * at the MCU. Therefore the transceiver interrupts in single chips are
     * disabled by clearing the MCU IRQ mask.
     *
     * @ingroup apiPalApi
     */
#define pal_trx_irq_dis_tstamp()        DISABLE_TRX_IRQ_TSTAMP()

    /**
     * @brief Clears the transceiver main interrupt
     *
     * @ingroup apiPalApi
     */
#define pal_trx_irq_flag_clr()          CLEAR_TRX_IRQ()

    /**
     * @brief Clears the transceiver timestamp interrupt
     *
     * @ingroup apiPalApi
     */
#define pal_trx_irq_flag_clr_tstamp()   CLEAR_TRX_IRQ_TSTAMP()


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #if defined(PAL_USE_SPI_TRX) || defined(DOXYGEN) */

#endif  /* PAL_TRX_IRQ_SPI_H */
/* EOF */
