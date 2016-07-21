/**
 * @file pal.h
 *
 * @brief PAL related APIs
 *
 * This header file declares prototypes of PAL APIs, enumerations
 * used by TAL and MAC.
 *
 * $Id: pal.h 34174 2013-02-01 08:48:35Z uwalter $
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
#ifndef PAL_H
#define PAL_H

/*
 * NOTE:- Include 'return_val.h' before this file, as return_val.h has the
 *        all return value enums.
 */

/* === Includes ============================================================ */

#include <stdbool.h>
#include <stdint.h>
#include "pal_types.h"
#include "pal_config.h"
#include "app_config.h"

/* Inclusion of feature-based other PAL header files. */
#ifdef PAL_SPI_BLOCK_MODE
#   include "pal_trx_spi_block_mode.h"
#else
#   ifdef PAL_USE_SPI_TRX
#       include "pal_trx_irq_spi.h"
#       include "pal_trx_access_spi.h"
#   else
#       include "pal_trx_irq_non_spi.h"
#       include "pal_mcu_generic.h"
#   endif  /* PAL_USE_SPI_TRX */
#endif /* PAL_SPI_BLOCK_MODE */

#ifdef ENABLE_PAL_TWI
#   include "pal_twi.h"
#endif  /* #ifdef ENABLE_PAL_TWI */

#ifdef GENERAL_PURPOSE_IRQ
#   include "pal_gp_irq.h"
#endif  /* #ifdef GENERAL_PURPOSE_IRQ */

#ifdef SNIFFER
#   include "pal_sniffer.h"
#endif   /* #ifdef SNIFFER */

/* === Macros =============================================================== */

/**
 * Adds two time values
 */
#define ADD_TIME(a, b)                  ((a) + (b))

/**
 * Subtracts two time values
 */
#define SUB_TIME(a, b)                  ((a) - (b))

/**
 * This macro is used for handling endianness among the different CPUs.
 */
#if (PAL_GENERIC_TYPE == AVR32)
#define U16_TO_TARGET(x) ((((x) << 8) & 0xFF00) | (((x) >> 8) & 0x00FF))
#else
#define U16_TO_TARGET(x) (x)
#endif

/* === Types =============================================================== */

/**
 * Pin State
 */
/* Please dont change this type. */
typedef enum pin_state_tag
{
    /** Pin state low */
    LOW,
    /** Pin state high */
    HIGH
} SHORTENUM pin_state_t;


#if defined (SIO_HUB) || defined(DOXYGEN)
/**
 * Stream I/O to serial mapping enumeration
 */
typedef enum sio_serial_tag
{
    /* Serial unit used by user */
    /** UART 0 */
    SIO_0,
    /** UART 1 */
    SIO_1,
    /** USB 0 */
    SIO_2,
    /** UART 2 */
    SIO_3,
    /** UART 3 */
    SIO_4
} SHORTENUM sio_serial_t;
#endif


/**
 * Timeout type
 */
typedef enum
timeout_type_tag
{
    /** The timeout is relative to the current time. */
    TIMEOUT_RELATIVE,
    /** The timeout is an absolute value. */
    TIMEOUT_ABSOLUTE
} SHORTENUM timeout_type_t;


/**
 * Timer clock source type
 */
typedef enum source_type_tag
{
    /** Identifier for timer clock source during sleep */
    TMR_CLK_SRC_DURING_TRX_SLEEP,
    /** Identifier for timer clock source while being awake */
    TMR_CLK_SRC_DURING_TRX_AWAKE
} SHORTENUM source_type_t;

/**
 * @brief IDs for persistence storage access
 */
typedef enum ps_id_tag
{
    PS_IEEE_ADDR,
    PS_XTAL_TRIM
} SHORTENUM ps_id_t;


/**
 * @brief IDs for persistence storage type
 */
typedef enum ps_type_tag
{
    /** MCU Internal EEPROM */
    INTERN_EEPROM,
    /** External EEPROM */
    EXTERN_EEPROM
} SHORTENUM ps_type_t;


/**
 * @brief LED action
 */
typedef enum led_action_tag
{
    /** Switch LED on. */
    LED_ON,
    /** Switch LED off. */
    LED_OFF,
    /** Toggle LED. */
    LED_TOGGLE
} SHORTENUM led_action_t;


/**
 * @brief Button action
 */
typedef enum button_state_tag
{
    /** Button is pressed. */
    BUTTON_PRESSED,
    /** Button is released. */
    BUTTON_OFF
} SHORTENUM button_state_t;


/**
 * @brief Power modes
 */
typedef enum pwr_mode_tag
{
    SYSTEM_SLEEP
} pwr_mode_t;


/**
 * @brief Transceiver ISR handler
 */
typedef void (*irq_handler_t)(void);


#ifdef TEST_HARNESS
#if (DEBUG > 0)
/**
 * @brief This is the ALERT.indication message structure.
 */
typedef struct assert_tag
{
    /**< The total length of this message. */
    uint8_t size            ALIGN8BIT;
    /**< This identifies the message as ALERT_INDICATION */
    uint8_t assert_cmdcode  ALIGN8BIT;
    /**< Bytes to read to frame, data[0] gives the length */
    uint8_t data[1]         ALIGN8BIT;
} assert_t;
#endif  /* (DEBUG > 0) */
#endif  /* TEST_HARNESS */

/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @brief Initialization of PAL
     *
     * This function initializes the PAL. The RC Oscillator is calibrated.
     *
     * @return MAC_SUCCESS  if PAL initialization is successful, FAILURE otherwise
     * @ingroup apiPalApi
     */
    retval_t pal_init(void);


    /**
     * @brief Services timer and sio handler
     *
     * This function calls sio & timer handling functions.
     */
    void pal_task(void);

    /**
     * @brief Enables the global interrupt
     * @ingroup apiPalApi
     */
    static inline void pal_global_irq_enable(void)
    {
        ENABLE_GLOBAL_IRQ();
    }


    /**
     * @brief Disables the global interrupt
     * @ingroup apiPalApi
     */
    static inline void pal_global_irq_disable(void)
    {
        DISABLE_GLOBAL_IRQ();
    }

#if defined(ENABLE_HIGH_PRIO_TMR) || defined(DOXYGEN)
    /*
     * Internal prototypes for pal_timer module
     */
    /**
     * @brief Starts high priority timer
     *
     * This function starts a high priority timer for the specified timeout.
     *
     * @param timer_id Timer identifier
     * @param timer_count Timeout in microseconds
     * @param timer_cb Callback handler invoked upon timer expiry
     * @param param_cb Argument for the callback handler
     *
     * @return
     * - @ref PAL_TMR_INVALID_ID if the identifier is undefined,
     * - @ref MAC_INVALID_PARAMETER if the callback function for this timer is NULL,
     * - @ref PAL_TMR_ALREADY_RUNNING if the timer is already running, or
     * - @ref MAC_SUCCESS if timer is started successfully.
     * @ingroup apiPalApi
     */
    retval_t pal_start_high_priority_timer(uint8_t timer_id,
                                           uint16_t timer_count,
                                           FUNC_PTR(timer_cb),
                                           void *param_cb);

    /**
     * @brief Stops a high priority timer
     *
     * This function stops a high priority timer.
     *
     * @param timer_id Timer identifier
     *
     * @return
     * - @ref PAL_TMR_NOT_RUNNING if the timer id does not match with the high priority
     * timer register, or
     * - @ref MAC_SUCCESS otherwise.
     * @ingroup apiPalApi
     */
    retval_t pal_stop_high_priority_timer(uint8_t timer_id);
#endif

    /**
     * @brief Generates blocking delay
     *
     * This functions generates a blocking delay of specified time.
     *
     * @param delay in microseconds
     * @ingroup apiPalApi
     */
    void pal_timer_delay(uint16_t delay);

    /**
     * @brief Selects timer clock source
     *
     * This function selects the clock source of the timer.
     *
     * @param source
     * - @ref TMR_CLK_SRC_DURING_TRX_SLEEP if clock source during sleep is to be selected, and
     * - @ref TMR_CLK_SRC_DURING_TRX_AWAKE if clock source while being awake is selected.
     * @ingroup apiPalApi
     */
    void pal_timer_source_select(source_type_t source);

#if (TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN)
    /**
     * @brief Checks if the timer of requested timer identifier is running
     *
     * This function checks if the timer of requested timer identifier is running.
     *
     * @param timer_id Timer identifier
     *
     * @return
     * - true if timer with requested timer id is running,
     * - false otherwise.
     * @ingroup apiPalApi
     */
    bool pal_is_timer_running(uint8_t timer_id);
#endif

    /** @cond DOXYGEN_PAL_DEBUG */
#if (DEBUG > 0)
    bool pal_are_all_timers_stopped(void);
#endif  /* (DEBUG > 0) */
    /** @endcond */


    /**
     * @brief Adds two time values
     *
     * @param a Time value 1
     * @param b Time value 2
     *
     * @return Addition of a and b
     * @ingroup apiPalApi
     */
    static inline uint32_t pal_add_time_us(uint32_t a, uint32_t b)
    {
        return (ADD_TIME(a, b));
    }


    /**
     * @brief Subtracts two time values
     *
     * @param a Time value 1
     * @param b Time value 2
     *
     * @return Difference between a and b
     * @ingroup apiPalApi
     */
    static inline uint32_t pal_sub_time_us(uint32_t a, uint32_t b)
    {
        return (SUB_TIME(a, b));
    }


    /*
     * Macros for TRX GPIO pins access.
     */
    /** Macro to set Reset pin to high */
#define PAL_RST_HIGH()                  RST_HIGH()
    /** Macro to set Reset pin to low */
#define PAL_RST_LOW()                   RST_LOW()
    /** Macro to set SLP_TR pin to high */
#define PAL_SLP_TR_HIGH()               SLP_TR_HIGH()
    /** Macro to set SLP_TR pin to low */
#define PAL_SLP_TR_LOW()                SLP_TR_LOW()
    /** Macro to get the transceiver's main IRQ status */
#define PAL_TRX_IRQ_HIGH()              IRQ_PINGET()

    /**
     * @brief Provides timestamp of the last received frame
     *
     * This function provides the timestamp (in microseconds)
     * of the last received frame.
     *
     * @param[out] timestamp in microseconds
     * @ingroup apiPalApi
     */
    void pal_trx_read_timestamp(uint32_t *timestamp);

#if defined(SIO_HUB) || defined(DOXYGEN)
    /*
     * Prototypes for Stream I/O interface
     */
    /**
     * @brief Initializes the requested SIO unit
     *
     * This function initializes the requested SIO unit.
     *
     * @param sio_unit Specifies the SIO uint to be initialized
     *
     * @return MAC_SUCCESS  if SIO unit is initialized successfully, FAILURE otherwise
     * @ingroup apiPalApi
     */
    retval_t pal_sio_init(uint8_t sio_unit);

    /**
     * @brief Transmits data through selected SIO unit
     *
     * This function transmits data through the selected SIO unit.
     *
     * @param sio_unit Specifies the SIO unit
     * @param data Pointer to the data to be transmitted is present
     * @param length Number of bytes to be transmitted
     *
     * @return Actual number of transmitted bytes
     * @ingroup apiPalApi
     */
    uint8_t pal_sio_tx(uint8_t sio_unit, uint8_t *data, uint8_t length);

    /**
     * @brief Receives data from selected SIO unit
     *
     * This function receives data from the selected SIO unit.
     *
     * @param sio_unit Specifies SIO unit
     * @param[out] data Pointer to the buffer to store received data
     * @param[in] max_length Maximum number of bytes to be received
     *
     * @return Actual number of received bytes
     * @ingroup apiPalApi
     */
    uint8_t pal_sio_rx(uint8_t sio_unit, uint8_t *data, uint8_t max_length);
#endif  /* SIO_HUB */


    /**
     * @brief Calibrates the internal RC oscillator
     *
     * @return True if calibration is successful, false otherwise.
     * @ingroup apiPalApi
     */
    bool pal_calibrate_rc_osc(void);


    /*
     * Prototypes for persistence storage
     */
    /**
     * @brief Get data from persistence storage
     *
     * @param[in]  ps_type Persistence storage type
     * @param[in]  start_addr Start offset within EEPROM
     * @param[in]  length Number of bytes to read from EEPROM
     * @param[out] value Data from persistence storage
     *
     * @return MAC_SUCCESS  if everything went OK else FAILURE
     * @ingroup apiPalApi
     */
    retval_t pal_ps_get(ps_type_t ps_type, uint16_t start_addr, uint16_t length, void *value);

    /**
     * @brief Write data to persistence storage
     *
     * @param[in]  start_addr Start address offset within EEPROM
     * @param[in]  length Number of bytes to be written to EEPROM
     * @param[in]  value Data to persistence storage
     *
     * @return MAC_SUCCESS  if everything went OK else FAILURE
     * @ingroup apiPalApi
     */
    retval_t pal_ps_set(uint16_t start_addr, uint16_t length, void *value);


    /**
     * @brief Alert indication
     *
     * This Function can be used by any application to indicate an error condition.
     * The function is blocking and does never return.
     * @ingroup apiPalApi
     */
    void pal_alert(void);


    /** @cond DOXYGEN_PAL_DEBUG */
#if (DEBUG > 0)
    void pal_assert(bool expression,
                    FLASH_STRING_T message,
                    int8_t *file,
                    uint16_t line);

    /* DEBUG only: Test for an assertion, similar to Std-C assert() */
#define ASSERT(expr)    pal_assert(expr, FLASH_STRING(#expr), (int8_t*)__FILE__, __LINE__)
#else
#define ASSERT(expr)
#endif /* (DEBUG > 0) */
    /** @endcond */


    /*
     * Public prototypes for pal_timer module
     */

#if (TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN)
    /**
     * @brief Start regular timer
     *
     * This function starts a regular timer and installs the corresponding
     * callback function handle the timeout event.
     *
     * @param timer_id Timer identifier
     * @param timer_count Timeout in microseconds
     * @param timeout_type @ref TIMEOUT_RELATIVE or @ref TIMEOUT_ABSOLUTE
     * @param timer_cb Callback handler invoked upon timer expiry
     * @param param_cb Argument for the callback handler
     *
     * @return
     *          - @ref PAL_TMR_INVALID_ID  if the timer identifier is undefined,
     *          - @ref MAC_INVALID_PARAMETER if the callback function for this timer
     *                 is NULL,
     *          - @ref PAL_TMR_ALREADY_RUNNING if the timer is already running.
     *          - @ref MAC_SUCCESS if timer is started or
     *          - @ref PAL_TMR_INVALID_TIMEOUT if timeout is not within timeout range.
     * @ingroup apiPalApi
     */
    retval_t pal_timer_start(uint8_t timer_id,
                             uint32_t timer_count,
                             timeout_type_t timeout_type,
                             FUNC_PTR(timer_cb),
                             void *param_cb);
#endif

#if (TOTAL_NUMBER_OF_TIMERS > 0) || defined(DOXYGEN)
    /**
     * @brief Stops a running timer
     *
     * This function stops a running timer with specified timer_id
     *
     * @param timer_id Timer identifier
     *
     * @return
     *          - @ref MAC_SUCCESS if timer stopped successfully,
     *          - @ref PAL_TMR_NOT_RUNNING if specified timer is not running,
     *          - @ref PAL_TMR_INVALID_ID if the specifed timer id is undefined.
     * @ingroup apiPalApi
     */
    retval_t pal_timer_stop(uint8_t timer_id);
#endif

    /**
     * @brief Gets current time
     *
     * This function returns the current time.
     *
     * @param[out] current_time Returns current system time
     * @ingroup apiPalApi
     */
    void pal_get_current_time(uint32_t *current_time);


    /**
     * @brief LED initialization
     */
    void pal_led_init(void);

    /**
     * @brief LED handling
     *
     * @param led_no LED id
     * @param led_setting LED action
     * @ingroup apiPalApi
     */
    void pal_led(led_id_t led_no, led_action_t led_setting);


    /**
     * @brief Button initialization
     */
    void pal_button_init(void);


    /**
     * @brief Button handling
     *
     * @param button_no Button id
     *
     * @return
     *          - @ref BUTTON_PRESSED if button is pressed,
     *          - @ref BUTTON_OFF if button is not pressed.
     * @ingroup apiPalApi
     */
    button_state_t pal_button_read(button_id_t button_no);


    /**
     * @brief Sleep/Power handling
     *
     * Handles sleep/power modes for entire system
     *
     * @param sleep_mode desired sleep/power mode, see sleep.h
     *
     * @ingroup apiPalApi
     */
    void pal_sleep_mode(uint8_t sleep_mode);


#if defined(NON_BLOCKING_SPI) || defined(DOXYGEN)
    /**
     * @brief SPI done callback initialization
     *
     * @param spi_done_cb Pointer to SPI done callback function
     * @ingroup apiPalApi
     */
    void pal_spi_done_cb_init(void *spi_done_cb);
#endif

    /**
     * @brief Generation of random seed for function srand() in case this
     *        is not supported by the transceiver (e.g. AT86RF230)
     *
     * @return uint16_t Random number to be used as seed for function srand()
     * @ingroup apiPalApi
     */
    uint16_t pal_generate_rand_seed(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* PAL_H */
/* EOF */
