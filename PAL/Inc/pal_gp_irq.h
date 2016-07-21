/**
 * @file pal_gp_irq.h
 *
 * @brief General Purpose IRQ API
 *
 * This header file declares prototypes of the General Purpose IRQ API.
 *
 * $Id: pal_gp_irq.h 33806 2012-11-09 15:53:06Z uwalter $
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
#ifndef PAL_GP_IRQ_H
#define PAL_GP_IRQ_H

#ifdef GENERAL_PURPOSE_IRQ

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

    /*
     * Public prototypes for General Purpose IQR
     */
    /**
     * @brief Initializes general purpose input interrupt
     *
     * This function sets callback function for the general
     * purpose input interrupt.
     *
     * @param gp_irq_cb Callback function for the general purpose interrupt
     * @ingroup apiPalApi
     */
    void pal_gp_irq_init(FUNC_PTR(gp_irq_cb));

    /**
     * @brief Enables general purpose input interrupt
     *
     * This macro is only available for non-single chip transceivers, since
     * in single chip transceivers there is no separation between enabling
     * transceiver interrupts at the transceiver, and setting the IRQ mask
     * at the MCU. Therefore the transceiver interrupts in single chips are
     * enabled by setting the MCU IRQ mask.
     *
     * @ingroup apiPalApi
     */
#define pal_gp_irq_en()                 ENABLE_GP_IRQ()

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifdef GENERAL_PURPOSE_IRQ */

#endif  /* PAL_GP_IRQ_H */
/* EOF */
