/**
 * @file rtb_dispatcher.c
 *
 * @brief Dispatches the RTB events by decoding the message type
 *
 * $Id: rtb_dispatcher.c 33203 2012-08-23 13:59:03Z sschneid $
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

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "pal.h"
#include "return_val.h"
#include "bmm.h"
#include "qmm.h"
#include "tal.h"
#include "ieee_const.h"
#include "stack_config.h"
#include "rtb_internal.h"
#include "rtb_msg_const.h"
#include "rtb_api.h"
#include "rtb.h"
#include "rtb_msg_types.h"

#ifdef ENABLE_RTB

/* === Macros ============================================================== */


/* === Globals ============================================================= */

/* RTB Dispatcher table */
static FLASH_DECLARE(handler_rtb_t dispatch_rtb_table[LAST_RTB_MESSAGE - FIRST_RTB_MESSAGE + 1]) =
{
    /* Internal message */
    [RTB_DATA_INDICATION - FIRST_RTB_MESSAGE]           = rtb_process_data_ind,

    [RTB_RANGE_REQUEST - FIRST_RTB_MESSAGE]             = rtb_range_request,
#ifndef RTB_WITHOUT_MAC
    [RTB_RESET_REQUEST - FIRST_RTB_MESSAGE]             = rtb_reset_request,
#endif  /* #ifndef RTB_WITHOUT_MAC */
    [RTB_SET_REQUEST - FIRST_RTB_MESSAGE]               = rtb_set_request,

    [RTB_RANGE_CONFIRM - FIRST_RTB_MESSAGE]             = rtb_range_conf,
#ifndef RTB_WITHOUT_MAC
    [RTB_RESET_CONFIRM - FIRST_RTB_MESSAGE]             = rtb_reset_conf,
#endif  /* #ifndef RTB_WITHOUT_MAC */
    [RTB_SET_CONFIRM - FIRST_RTB_MESSAGE]               = rtb_set_conf
#ifndef RTB_WITHOUT_MAC
    ,
    [RTB_PMU_VALIDITY_INDICATION - FIRST_RTB_MESSAGE]   = rtb_pmu_validitiy_ind
#endif  /* #ifndef RTB_WITHOUT_MAC */
};

/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

/**
 * @brief Obtains the message type from the buffer and calls the respective handler
 *
 * This function decodes all events/messages and calls the appropriate handler.
 *
 * @param event Pointer to the buffer header whose body part holds the message
 * type and message elemnets
 */
void dispatch_rtb_event(uint8_t *event)
{
    /*
     * A pointer to the body of the buffer is obtained from the pointer to the
     * received header.
     */
    uint8_t *buffer_body = BMM_BUFFER_POINTER((buffer_t *)event);

    /* Check is done to see if the message type is valid */
    /* Please note:
     * The macro PGM_READ_WORD is only relevant for AVR-GCC builds and
     * reads a DWord from flash, which is the start address of the function.
     *
     * How does this work for builds that are larger than 128K?
     *
     * For IAR builds this statement is fine, since PGM_READ_WORD just
     * turns to "*". The size of the function pointer is automatically
     * 3 byte for MCUs which have more than 128K flash. The the address
     * of the callback is properly derived from this location.
     *
     * AVR-GCC currently does not support function pointers larger than
     * 16 bit. This implies that functions residing in the upper 128K
     * cannot be addressed properly. Therefore this code does not work
     * in these cases.
     * In regular cases, where code is not larger than 128K, the size
     * of a function pointer is 16 bit and properly read via PGM_READ_WORD.
     */

    /* Check first whether we have a request for RTB. */
    if ((buffer_body[CMD_ID_OCTET] <= LAST_RTB_MESSAGE) &&
        (buffer_body[CMD_ID_OCTET] >= FIRST_RTB_MESSAGE)
       )
    {
        /*
         * The following statement reads the address from the RTB dispatch table
         * of the function to be called by utilizing function pointers.
         */
        handler_rtb_t handler =
            (handler_rtb_t)PGM_READ_WORD(&dispatch_rtb_table[buffer_body[CMD_ID_OCTET] - FIRST_RTB_MESSAGE]);

        if (handler != NULL)
        {
            handler(event);
        }
        else
        {
            bmm_buffer_free((buffer_t *)event);
#if (DEBUG > 0)
            ASSERT("RTB Dispatch handler unavailable" == 0);
#endif
        }
    }
}

#endif  /* #ifdef ENABLE_RTB */

/* EOF */

