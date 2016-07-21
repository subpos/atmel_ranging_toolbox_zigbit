/**
 * @file pal.c
 *
 * @brief General PAL functions for AVR ATxmega MCUs
 *
 * This file implements generic PAL function for AVR ATxmega MCUs.
 *
 * $Id: pal.c 33951 2012-11-30 14:11:45Z jwunsch $
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
#include <stdbool.h>
#include "pal.h"
#include "pal_config.h"
#include "pal_timer.h"
#include "pal_internal.h"

/* === Globals ============================================================= */

/*
 * This section defines all global variables for the PAL
 */

/*
 * This is the most significant part of the system time. The least one is
 * timer1 (TCNT1) itself.  Exported since input capture units might want
 * to know about this value as well.
 */
volatile uint16_t sys_time;

/* === Prototypes ========================================================== */

static uint8_t eeprom_read_byte(uint8_t *addr);
static void eeprom_write_byte(uint8_t addr, uint8_t value);
static inline void eeprom_wait_for_NVM(void);
static void eeprom_flush_buffer(void);
#ifndef __ICCAVR__
static inline void NVM_EXEC();
#endif

/* === Implementation ====================================================== */

/**
 * @brief Initialization of PAL
 *
 * This function initializes the PAL.
 *
 * @return MAC_SUCCESS  if PAL initialization is successful, FAILURE otherwise
  */
retval_t pal_init(void)
{
    clock_init();
#if defined(ENABLE_RP) || defined(ENABLE_RH)
    gpio_init(true);
#else
    gpio_init();
#endif
    trx_interface_init();
#ifdef CW_SUPPORTED
    TST_INIT();
#endif
    timer_init();
    event_system_init();
    interrupt_system_init();
#ifdef WATCHDOG
    wdt_init();
#else
    PROTECTED_WRITE(WDT.CTRL, WDT_CEN_bm);
#endif
#ifdef SLEEPING_TIMER
    sleeping_timer_init();
#else
#ifdef WATCHDOG
    wdt_parallel_timer_init();
#endif
#endif

    return MAC_SUCCESS;
}


#ifdef ENABLE_RP
/**
 * @brief todo
 *
 */
void pal_basic_init(void)
{
    clock_init();
    gpio_init(false);
#ifdef CW_SUPPORTED
    TST_INIT();
#endif
    timer_init();
    event_system_init();
    interrupt_system_init();
#ifdef WATCHDOG
    wdt_init();
#else
    PROTECTED_WRITE(WDT.CTRL, WDT_CEN_bm);
#endif
#ifdef SLEEPING_TIMER
    sleeping_timer_init();
#else
#ifdef WATCHDOG
    wdt_parallel_timer_init();
#endif
#endif
}
#endif  /* ENABLE_RP */


/**
 * @brief Services timer and sio handler
 *
 * This function calls sio & timer handling functions.
 */
void pal_task(void)
{
#if (TOTAL_NUMBER_OF_TIMERS > 0)
    timer_service();
#endif
}



/**
 * @brief Non-Volatile Memory Execute Command
 *
 * This macro set the CCP register before setting the CMDEX bit in the
 * NVM.CTRLA register.
 * Implementation used from application note AVR1315.
 *
 * @note The CMDEX bit must be set within 4 clock cycles after setting the
 *       protection byte in the CCP register.
 */
#ifdef __ICCAVR__
#define NVM_EXEC()  asm("push r30"      "\n\t"  \
                        "push r31"      "\n\t"  \
                        "push r16"      "\n\t"  \
                        "push r18"      "\n\t"  \
                        "ldi r30, 0xCB" "\n\t"  \
                        "ldi r31, 0x01" "\n\t"  \
                        "ldi r16, 0xD8" "\n\t"  \
                        "ldi r18, 0x01" "\n\t"  \
                        "out 0x34, r16" "\n\t"  \
                        "st Z, r18"     "\n\t"  \
                        "pop r18"       "\n\t"  \
                        "pop r16"       "\n\t"  \
                        "pop r31"       "\n\t"  \
                        "pop r30"       "\n\t"  \
                       )
#else
static inline void NVM_EXEC()
{
    void *z = (void *)&NVM_CTRLA;

    __asm__ volatile("out %[ccp], %[ioreg]"  "\n\t"
                     "st z, %[cmdex]"
                     :
                     : [ccp] "I" (_SFR_IO_ADDR(CCP)),
                     [ioreg] "d" (CCP_IOREG_gc),
                     [cmdex] "r" (NVM_CMDEX_bm),
                     [z] "z" (z)
                    );
}
#endif


/**
 * @brief Wait for any NVM access to finish, including EEPROM.
 *
 * This function is blcoking and waits for any NVM access to finish,
 * including EEPROM. Use this function before any EEPROM accesses,
 * if you are not certain that any previous operations are finished yet,
 * like an EEPROM write.
 * Implementation used from application note AVR1315.
 */
static inline void eeprom_wait_for_NVM(void)
{
    do
    {
        /* Block execution while waiting for the NVM to be ready. */
    }
    while ((NVM.STATUS & NVM_NVMBUSY_bm) == NVM_NVMBUSY_bm);
}



/**
 * @brief Read one byte from EEPROM address
 *
 * Implementation used from application note AVR1315.

 * @param addr Pointer to EEPROM address
 * @return Value at EEPROM address
 */
static uint8_t eeprom_read_byte(uint8_t *addr)
{
    uint16_t addrcpy;

    addrcpy = (uint16_t)addr;

    eeprom_wait_for_NVM();

    NVM.ADDR2 = 0;
    NVM.ADDR1 = (addrcpy >> 8) & 0xFF;
    NVM.ADDR0 = (addrcpy) & 0xFF;
    NVM.CMD =  NVM_CMD_READ_EEPROM_gc;

    /*
     * The CMDEX bit must be set within 4 clock cycles after setting the
     * protection byte in the CCP register.
     */
    NVM_EXEC();
    return NVM.DATA0;
}



/**
 * @brief Flush temporary EEPROM page buffer.
 *
 * This function flushes the EEPROM page buffers. This function will cancel
 * any ongoing EEPROM page buffer loading operations, if any.
 * This function also works for memory mapped EEPROM access.
 * Implementation used from application note AVR1315.
 *
 * @note The EEPROM write operations will automatically flush the buffer for you.
 */
static void eeprom_flush_buffer(void)
{
    /* Wait until NVM is not busy. */
    eeprom_wait_for_NVM();

    /* Flush EEPROM page buffer if necessary. */
    if ((NVM.STATUS & NVM_EELOAD_bm) != 0)
    {
        NVM.CMD = NVM_CMD_ERASE_EEPROM_BUFFER_gc;
        NVM_EXEC();
    }
}


/**
 * @brief Write one byte to EEPROM address
 *
 * Implementation used from application note AVR1315.
 *
 * @param addr EEPROM address
 * @param Value for EEPROM address
 */
static void eeprom_write_byte(uint8_t addr, uint8_t value)
{
    uint16_t addrcpy;

    addrcpy = (uint16_t)addr;

    /*  Flush buffer to make sure no unintetional data is written and load
     *  the "Page Load" command into the command register.
     */
    eeprom_flush_buffer();
    NVM.CMD = NVM_CMD_LOAD_EEPROM_BUFFER_gc;

    /* Set address to write to. */
    NVM.ADDR0 = addrcpy & 0xFF;
    NVM.ADDR1 = (addrcpy >> 8) & 0x1F;
    NVM.ADDR2 = 0x00;

    /* Load data to write, which triggers the loading of EEPROM page buffer. */
    NVM.DATA0 = value;

    /*  Issue EEPROM Atomic Write (Erase&Write) command. Load command, write
     *  the protection signature and execute command.
     */
    NVM.CMD = NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc;
    NVM_EXEC();
}


/**
 * @brief Get data from persistence storage
 *
 * @param[in]  ps_type Persistence storage type
 * @param[in]  start_addr Start offset within EEPROM
 * @param[in]  length Number of bytes to read from EEPROM
 * @param[out] value Data from persistence storage
 *
 * @return MAC_SUCCESS  if everything went OK else FAILURE
 */
retval_t pal_ps_get(ps_type_t ps_type, uint16_t start_addr, uint16_t length, void *value)
{
#if (EXTERN_EEPROM_AVAILABLE == 1)
    if (ps_type == EXTERN_EEPROM)
    {
        return extern_eeprom_get(start_addr, length, value);
    }
    else
#endif

        if (ps_type == INTERN_EEPROM)
        {
            uint16_t index;
            uint8_t *data_ptr;

            if ((start_addr + length) > (E2END + 1))
            {
                return FAILURE;
            }

            data_ptr = (uint8_t *)(value);
            for (index = 0; index < length; index++)
            {
                *data_ptr = eeprom_read_byte((uint8_t *)(index + start_addr));
                data_ptr++;
            }
        }
        else    // no internal eeprom and no external eeprom available
        {
            return MAC_INVALID_PARAMETER;
        }

    return MAC_SUCCESS;
}


/**
 * @brief Write data to persistence storage
 *
 * @param[in]  start_addr Start address offset within EEPROM
 * @param[in]  length Number of bytes to be written to EEPROM
 * @param[in]  value Data to persistence storage
 *
 * @return MAC_SUCCESS  if everything went OK else FAILURE
 */
retval_t pal_ps_set(uint16_t start_addr, uint16_t length, void *value)
{
    uint8_t *data_ptr;
    uint16_t i;
    uint8_t read_data;

    if ((start_addr + length) > (E2END + 1))
    {
        return FAILURE;
    }

    data_ptr = (uint8_t *)(value);
    for (i = 0; i < length; i++)
    {
        read_data = eeprom_read_byte((uint8_t *)(start_addr));
        if (read_data != *data_ptr)
        {
            eeprom_write_byte(start_addr, *data_ptr);
        }
        data_ptr++;
        start_addr++;
    }

    return MAC_SUCCESS;
}


/*
 * @brief Alert indication
 *
 * This Function can be used by any application to indicate an error condition.
 * The function is blocking and does never return.
 */
void pal_alert(void)
{
#if (DEBUG > 0)
    bool debug_flag = false;
#endif
    ALERT_INIT();

    while (1)
    {
        pal_timer_delay(0xFFFF);
        ALERT_INDICATE();

#if (DEBUG > 0)
        /* Used for debugging purposes only */
        if (debug_flag == true)
        {
            break;
        }
#endif
    }
}


/* EOF */
