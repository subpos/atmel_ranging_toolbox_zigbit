/**
 * @file mac.c
 *
 * @brief This module runs the MAC scheduler.
 *
 * $Id: mac.c 32538 2012-07-03 17:10:47Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === Includes ============================================================= */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "pal.h"
#include "return_val.h"
#include "bmm.h"
#include "qmm.h"
#include "tal.h"
#include "tal_internal.h"
#include "ieee_const.h"
#include "mac_msg_const.h"
#include "mac_api.h"
#include "mac_msg_types.h"
#include "mac_data_structures.h"
#include "stack_config.h"
#include "mac_internal.h"
#include "mac.h"
#include "mac_build_config.h"
#ifdef MAC_SECURITY_ZIP
#include "mac_security.h"
#endif  /* MAC_SECURITY_ZIP */

/* === Macros =============================================================== */


/* === Globals ============================================================== */

/**
 * Current state of the MAC state machine
 */
mac_state_t mac_state;

/**
 * Current state of scanning process.
 */
mac_scan_state_t mac_scan_state;

/**
 * Current state of syncronization with parent
 * (either coordinator or PAN coordinator).
 */
mac_sync_state_t mac_sync_state;

/**
 * Current state of MAC poll state machine,
 * e.g. polling for data, polling for Association Response, etc.
 */
mac_poll_state_t mac_poll_state;

/**
 * Radio sleep state
 */
mac_radio_sleep_state_t mac_radio_sleep_state;

#ifdef BEACON_SUPPORT
/**
 * Final Cap Slot of current Superframe
 */
uint8_t mac_final_cap_slot;

/**
 * Flag stating that the last received beacon frame from the parent
 * indicated pending broadcast data to be received.
 */
bool mac_bc_data_indicated;
#endif  /* BEACON_SUPPORT */

/**
 * Global parse data
 */
parse_t mac_parse_data;

/*
 * Flag indicating that RxEnable is still active.
 */
bool mac_rx_enabled;

/*
 * Variables for duplicate detection.
 * In order to detect duplicated frames, the DSN and Source Address of the
 * last received data frame need to be stored.
 */
uint8_t mac_last_dsn;
uint64_t mac_last_src_addr;


#if (MAC_START_REQUEST_CONFIRM == 1)
/**
 * Holds the contents of the beacon payload.
 */
uint8_t mac_beacon_payload[aMaxBeaconPayloadLength];
#endif  /* (MAC_START_REQUEST_CONFIRM == 1) */

#ifdef MAC_SECURITY_ZIP
/**
 * Holds the values of all security related PIB attributes.
 */
mac_sec_pib_t mac_sec_pib;
#endif  /* MAC_SECURITY_ZIP */

/**
 * Holds the mlme request buffer pointer, used to give the respective
 * confirmation in scan, poll and association.
 */
uint8_t *mac_conf_buf_ptr;

#if (MAC_SCAN_SUPPORT == 1)
/**
 * Stores the original channel before start of scanning.
 */
uint8_t mac_scan_orig_channel;

/**
 * Stores the original channel page before start of scanning.
 */
uint8_t mac_scan_orig_page;

#if ((MAC_SCAN_ACTIVE_REQUEST_CONFIRM == 1) || (MAC_SCAN_PASSIVE_REQUEST_CONFIRM == 1))
/**
 * Stores the original PAN-Id before start of scanning.
 */
uint16_t mac_scan_orig_panid;
#endif /* ((MAC_SCAN_ACTIVE_REQUEST_CONFIRM == 1) || (MAC_SCAN_PASSIVE_REQUEST_CONFIRM == 1)) */

/**
 * Holds the buffer pointer which is used to send scan command.
 */
uint8_t *mac_scan_cmd_buf_ptr;
#endif /* (MAC_SCAN_SUPPORT == 1) */

/**
 * MAC busy state, indicates whether MAC can process any
 * request from NHLE.
 */
bool mac_busy;

/**
 * NHLE to MAC queue in which NHLE pushes all the requests to the MAC layer
 */
queue_t nhle_mac_q;

/**
 * Queue used by MAC for its internal operation. TAL pushes the incoming frames
 * in this queue.
 */
queue_t tal_mac_q;

#if (MAC_START_REQUEST_CONFIRM == 1)
#ifdef BEACON_SUPPORT
/**
 * Queue used by MAC layer in beacon-enabled network to put in broadcast data.
 * Any broadcast data given by NHLE at a Coordinator or PAN Coordinator
 * in a beacon-enabled network is placed here by MAC.
 */
queue_t broadcast_q;
#endif  /* BEACON_SUPPORT */
#endif /* (MAC_START_REQUEST_CONFIRM == 1) */


#if (MAC_INDIRECT_DATA_FFD == 1)
/**
 * Queue used by MAC layer to put in indirect data. Any indirect data given by
 * NHLE is placed here by MAC, until the device polls for the data.
 */
queue_t indirect_data_q;
#endif /* (MAC_INDIRECT_DATA_FFD == 1) */

extern volatile bool timer_trigger;

mac_pib_t mac_pib;

/* === Prototypes =========================================================== */


/* === Implementation ======================================================= */

/**
 * @brief Runs the MAC scheduler
 *
 * This function runs the MAC scheduler.
 *
 * MLME and MCPS queues are removed alternately, starting with MLME queue.
 *
 * @return true if event is dispatched, false if no event to dispatch.
 */
bool mac_task(void)
{
    uint8_t *event = NULL;
    bool processed_event = false;

    if (!mac_busy)
    {
        /* Check whether queue is empty */
        if (nhle_mac_q.size != 0)
        {
            event = (uint8_t *)qmm_queue_remove(&nhle_mac_q, NULL);

            /* If an event has been detected, handle it. */
            if (NULL != event)
            {
                /* Process event due to NHLE requests */
                dispatch_event(event);
                processed_event = true;
            }
        }
    }

    /*
     * Internal event queue should be dispatched
     * irrespective of the dispatcher state.
     */
    /* Check whether queue is empty */
    if (tal_mac_q.size != 0)
    {
        event = (uint8_t *)qmm_queue_remove(&tal_mac_q, NULL);

        /* If an event has been detected, handle it. */
        if (NULL != event)
        {
            dispatch_event(event);
            processed_event = true;
        }
    }

    return processed_event;
}

/**
 * @brief Checks if the mac stack is idle
 */
bool mac_ready_to_sleep(void)
{
    bool idle;

    if (mac_busy ||
        (mac_nhle_q.size != 0) ||
        (nhle_mac_q.size != 0) ||
        (tal_mac_q.size != 0) ||
#ifdef ENABLE_RTB
        (tal_rtb_q.size != 0) ||
#endif  /* #ifdef ENABLE_RTB */
        (tal_incoming_frame_queue.size != 0)
#if (PAL_GENERIC_TYPE == MEGA_RF)
        || timer_trigger
#ifdef NO_32KHZ_CRYSTAL
        || (tal_trx_status != TRX_OFF)
#endif
#else
        || (tal_trx_status != TRX_SLEEP)
#endif
       )
    {
        idle = false;
    }
    else
    {
        idle = true;
    }

    return idle;
}
/* EOF */
