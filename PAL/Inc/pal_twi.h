/**
 * @file pal_twi.h
 *
 * @brief TWI related APIs
 *
 * This header file declares prototypes of the TWI API.
 *
 * $Id: pal_twi.h 33806 2012-11-09 15:53:06Z uwalter $
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
#ifndef PAL_TWI_H
#define PAL_TWI_H

#ifdef ENABLE_PAL_TWI

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
     * Public prototypes for TWI module
     */
    void pal_twim_init(void);
    void pal_twis_init(void);
    void pal_twis_smart_mode(bool mode);
    uint8_t pal_twis_get_status(void);
    bool pal_twis_get_direction(void);
    uint8_t pal_twis_read_data(void);
    bool pal_twis_get_nack(void);
    void pal_twis_setnack(void);
    void pal_twis_transfer(uint8_t *data);
    bool pal_twim_wait(void);
    void pal_twim_stop(void);
    void pal_twim_idle(void);
    bool pal_twim_write(uint8_t saddr,
                        uint8_t *wdata,
                        uint8_t wcount);
    uint8_t pal_twim_read(uint8_t saddr, uint8_t *rdata, uint8_t read_length);

    /**
     * @brief Initializes TWI interrupt
     *
     * This function sets callback function for the TWI interrupt.
     *
     * @param twi_irq_cb Callback function for the general purpose interrupt
     * @ingroup apiPalApi
     */
    void pal_twi_irq_init(FUNC_PTR(twi_irq_cb));

#define pal_twi_addr_irq_en()                 ENABLE_TWI_ADDR_IRQ()
#define pal_twi_data_irq_en()                 ENABLE_TWI_DATA_IRQ()
#define pal_twi_stop_irq_en()                 ENABLE_TWI_STOP_IRQ()
#define pal_twi_addr_irq_dis()                DISABLE_TWI_ADDR_IRQ()
#define pal_twi_data_irq_dis()                DISABLE_TWI_DATA_IRQ()
#define pal_twi_stop_irq_dis()                DISABLE_TWI_STOP_IRQ()

#define pal_twi_irq_clr()                     CLEAR_TWI_IRQ()

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifdef ENABLE_PAL_TWI */

#endif  /* PAL_TWI_H */
/* EOF */
