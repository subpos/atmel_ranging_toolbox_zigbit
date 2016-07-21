/**
 * @file rtb_api.c
 *
 * @brief This file contains RTB API functions at MAC level.
 *
 * $Id: rtb_api.c 34322 2013-02-21 09:50:35Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2011, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

#ifdef ENABLE_RTB

/* === Includes ============================================================ */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pal.h"
#include "return_val.h"
#include "bmm.h"
#include "qmm.h"
#include "ieee_const.h"
#include "rtb_api.h"
#include "rtb_msg_const.h"
#include "rtb_msg_types.h"
#include "stack_config.h"
#include "rtb_internal.h"

/* === Types =============================================================== */


/* === Macros ============================================================== */


/* === Globals ============================================================= */

#ifdef RTB_WITHOUT_MAC
/**
 * Queue used by RTB for communication to next higher layer in case the RTB
 * is the highest stack layer (i.e. MAC layer is not available).
 */
queue_t rtb_nhle_q;
#endif  /* #ifdef RTB_WITHOUT_MAC */

/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

/* RTB API at MAC level */

#ifndef RTB_WITHOUT_MAC
bool wpan_rtb_reset_req(void)
{
    buffer_t *buffer_header;
    rtb_reset_req_t *rtb_reset_req;

    buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);

    /* Check for buffer availability */
    if (NULL == buffer_header)
    {
        return false;
    }

    /* Get the buffer body from buffer header */
    rtb_reset_req = (rtb_reset_req_t *)BMM_BUFFER_POINTER(buffer_header);

    /* Construct rtb_reset_req_t message */
    rtb_reset_req->cmdcode = RTB_RESET_REQUEST;

#ifdef RTB_WITHOUT_MAC
    /* Insert message into NHLE RTB queue */
    qmm_queue_append(&nhle_rtb_q, buffer_header);
#else   /* #ifdef RTB_WITHOUT_MAC */
    /* Insert message into NHLE MAC queue */
    qmm_queue_append(&nhle_mac_q, buffer_header);
#endif  /* #ifdef RTB_WITHOUT_MAC */

    return true;
}
#endif  /* #ifndef RTB_WITHOUT_MAC */



bool wpan_rtb_set_req(wpan_rtb_set_req_t *wrsr)
{
    buffer_t *buffer_header;
    rtb_set_req_t *rtb_set_req;
    uint8_t pib_attribute_octet_no;

    buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);

    /* Check for buffer availability */
    if (NULL == buffer_header)
    {
        return false;
    }

    /* Get size of PIB attribute to be set */
    pib_attribute_octet_no = rtb_get_pib_attribute_size(wrsr->PIBAttribute);

    /* Get the buffer body from buffer header */
    rtb_set_req = (rtb_set_req_t *)BMM_BUFFER_POINTER(buffer_header);

    /* Construct rtb_set_req_t message */
    rtb_set_req->cmdcode = RTB_SET_REQUEST;

    /* Attribute and attribute value length */
    rtb_set_req->set_req.PIBAttribute = wrsr->PIBAttribute;

    /* Attribute value */
    memcpy((void *) & (rtb_set_req->set_req.PIBAttributeValue),
           (void *) & (wrsr->PIBAttributeValue),
           (size_t)pib_attribute_octet_no);

#ifdef RTB_WITHOUT_MAC
    /* Insert message into NHLE RTB queue */
    qmm_queue_append(&nhle_rtb_q, buffer_header);
#else   /* #ifdef RTB_WITHOUT_MAC */
    /* Insert message into NHLE MAC queue */
    qmm_queue_append(&nhle_mac_q, buffer_header);
#endif  /* #ifdef RTB_WITHOUT_MAC */

    return true;
}



/**
 * Initiate RTB-RANGE.request service and have it placed in the RTB-SAP queue.
 *
 * @param InitiatorAddrSpec   Pointer to wpan_addr_spec_t structure for Initiator.
 * @param ReflectorAddrSpec   Pointer to wpan_addr_spec_t structure for Reflector.
 * @param CoordinatorAddrMode     Address mode of Coordinator in case remote ranging is enabled
 *                            0x00: Regular ranging (i.e. No Coordinator available)
 *                            0x02: Remote ranging, Coordinator uses its short address
 *                            0x03: Remote ranging, Coordinator uses its long address
 *
 * @return true - success; false - buffer not available or queue full.
 */
bool wpan_rtb_range_req(wpan_rtb_range_req_t *wrrr)
{
    buffer_t *buffer_header;
    rtb_range_req_t *rtb_range_req;

    /* Allocate a large buffer for rtb range request */
    buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);

    if (NULL == buffer_header)
    {
        /* Buffer is not available */
        return false;
    }

    /* Get the buffer body from buffer header */
    rtb_range_req = (rtb_range_req_t *)BMM_BUFFER_POINTER(buffer_header);

    /* Construct rtb_range_req_t message */
    rtb_range_req->cmdcode = RTB_RANGE_REQUEST;

    memcpy(&rtb_range_req->range_req, wrrr, sizeof(wpan_rtb_range_req_t));

#ifdef RTB_WITHOUT_MAC
    /* Insert message into NHLE RTB queue */
    qmm_queue_append(&nhle_rtb_q, buffer_header);
#else   /* #ifdef RTB_WITHOUT_MAC */
    /* Insert message into NHLE MAC queue */
    qmm_queue_append(&nhle_mac_q, buffer_header);
#endif  /* #ifdef RTB_WITHOUT_MAC */

    return true;
}



#ifdef RTB_WITHOUT_MAC
/*
 * MAC is not available, therefore the functions wpan_init and
 * wpan_task need to be re-written.
 */
retval_t wpan_init(void)
{
    /* Init queue used for RTB to next higher layer communication */
    qmm_queue_init(&rtb_nhle_q);

    /* Initialize TAL */
    if (tal_init() != MAC_SUCCESS)
    {
        return FAILURE;
    }

    /* Initialize RTB */
    if (rtb_init() != RTB_SUCCESS)
    {
        return FAILURE;
    }

    /* Calibrate MCU's RC oscillator */
    if (!pal_calibrate_rc_osc())
    {
        return FAILURE;
    }

    /* Initialize the NHLE to RTB queue */
    qmm_queue_init(&nhle_rtb_q);
    return MAC_SUCCESS;
}



bool wpan_task(void)
{
    bool event_processed;
    uint8_t *event = NULL;

    /*
     * RTB to NHLE event queue should be dispatched
     * irrespective of the dispatcher state.
     */
    event = (uint8_t *)qmm_queue_remove(&rtb_nhle_q, NULL);

    /* If an event has been detected, handle it. */
    if (NULL != event)
    {
        dispatch_rtb_event(event);
        event_processed = true;
    }

    rtb_task();
    tal_task();
    pal_task();

    return (event_processed);
}
#endif  /* #ifdef RTB_WITHOUT_MAC */

#endif /* ENABLE_RTB */

/* EOF */

