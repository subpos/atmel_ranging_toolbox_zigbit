/**
 * @file pal_internal.h
 *
 * @brief PAL internal functions prototypes for AVR ATxmega MCUs
 *
 * $Id: pal_internal.h 33383 2012-08-31 12:45:27Z sschneid $
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
#ifndef PAL_INTERNAL_H
#define PAL_INTERNAL_H

/* === Includes ============================================================= */

#include "pal.h"

/* === Types ================================================================ */


/* === Externals ============================================================ */


/* === Macros ================================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

    void clock_init(void);
    void event_system_init(void);
#if (EXTERN_EEPROM_AVAILABLE == 1)
    retval_t extern_eeprom_get(uint8_t start_offset, uint8_t length, void *value);
#endif
#if defined(ENABLE_RP) || defined(ENABLE_RH)
    void gpio_init(bool);
#else
    void gpio_init(void);
#endif
    void interrupt_system_init(void);
    void trx_interface_init(void);
#if defined(ENABLE_RP) || defined(ENABLE_RH)
    void trx_interface_uninit(void);
#endif

#if defined(WATCHDOG) || defined(SLEEPING_TIMER) || defined(DOXYGEN)
    /**
     * @brief Initialize the watchdog timer of the ATxmega1281
     */
    void wdt_init(void);

    /**
     * @brief Parallel Software timer initialization if the Sleeping timer (RTC) is
     * not enabled to reset the Watchdog timer periodically.
     */
    void wdt_parallel_timer_init(void);

    /**
     * @brief Initialization of Sleeping timer (RTC) of the Atxmega1281 when Watchdog Timer
     * is not enabled. The period for the Sleeping timer should be given by the user in this case.
     */
    void sleeping_timer_without_wdt(void);

    /**
     * @brief Initialization of Sleeping timer (RTC) of the Atxmega1281 when Watchdog Timer
     * is enabled. The period for the Sleeping timer should be given by the user in this case
     * and the period of the sleeping timer should be less than the time-out period of the
     * watchdog timer.
     * @note It is required that the period of the Sleeping Timer (RTC) is less than the Time-out period
     * of Watchdog Timer (If enabled) to avoid the unintentional reset of the device.
     */
    void sleeping_timer_with_wdt(void);

    /**
     * @brief Initialization of sleeping timer (RTC) of the Atxmega1281
     */
    void sleeping_timer_init(void);

    /**
     * @brief Resets the watchdog timer of the ATxmega1281 before it
     * reaches its time-out period
     */
    void wdt_clear(void);

    /** @brief Enable Watchdog and set prescaler.
     *
     *  This function enables the Watchdog and applies prescaler settings.
     *  The Watchdog will be reset automatically.
     *
     *  The function writes the correct signature to the Configuration
     *  Change Protection register before writing the CTRL register. Interrupts are
     *  automatically ignored during the change enable period. The function will
     *  wait for the watchdog to be synchronized to the other clock domains before
     *  proceeding
     *
     *  @param  period  Watchdog Timer timeout period
     */
    void pal_wdt_enable(WDT_PER_t period);
#endif /* (defined(WATCHDOG) || defined(SLEEPING_TIMER) || defined(DOXYGEN)) */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* PAL_INTERNAL_H */
/* EOF */
