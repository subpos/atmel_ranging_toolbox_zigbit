/**
 * @file mac_data_structures.h
 *
 * @brief This file contains MAC related data structures, types and enums.
 *
 * $Id: mac_data_structures.h 21776 2010-04-23 08:47:59Z sschneid $
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
#ifndef MAC_DATA_STRUCTURES_H
#define MAC_DATA_STRUCTURES_H

/* === Includes ============================================================= */

#include "mac.h"

/* === Macros =============================================================== */


/* === Types ================================================================ */

/**
 * Beacon Payload type
 */
typedef struct mac_beacon_payload_tag
{
    uint16_t superframe_spec;
    uint8_t gts_spec;
    uint8_t pending_addr_spec;
    uint8_t *pending_addr_list;
    uint8_t beacon_payload_len;
    uint8_t *beacon_payload;
} mac_beacon_payload_t;



/**
 * Data Payload type
 */
typedef struct mac_data_payload_tag
{
    uint8_t *payload;
} mac_data_payload_t;



/**
 * Association Request type
 */
typedef struct mac_assoc_req_tag
{
    uint8_t capability_info;
} mac_assoc_req_t;

/**
 * Association Response type
 */
typedef struct mac_assoc_response_tag
{
    uint16_t short_addr;
    uint8_t assoc_status;
} mac_assoc_response_t;



/**
 * Disassociation Request type
 */
typedef struct mac_disassoc_req_tag
{
    uint8_t disassoc_reason;
} mac_disassoc_req_t;

/**
 * Coordinator Realignment type
 */
typedef struct mac_coord_realign_tag
{
    uint16_t pan_id;
    uint16_t coord_short_addr;
    uint8_t logical_channel;
    uint16_t short_addr;
    uint8_t channel_page;
} mac_coord_realign_t;



/**
 * General Command frame payload type
 */
typedef union
{
    mac_beacon_payload_t beacon_data;
    mac_data_payload_t data;
    mac_assoc_req_t assoc_req_data;
    mac_assoc_response_t assoc_response_data;
    mac_disassoc_req_t disassoc_req_data;
    mac_coord_realign_t coord_realign_data;
} frame_payload_t;


/**
 * Structure containing auxiliary security header information
 */
typedef struct sec_ctrl_tag
{
    uint8_t sec_level : 3;
    uint8_t key_id_mode : 2;
    uint8_t /* reserved */ : 3;
} sec_ctrl_t;


typedef struct parse_tag
{
    uint16_t fcf;
    uint8_t frame_type;
    uint8_t mpdu_length;
    uint8_t sequence_number;
    uint8_t dest_addr_mode;
    uint16_t dest_panid;
    address_field_t dest_addr;
    uint8_t src_addr_mode;
    uint16_t src_panid;
    address_field_t src_addr;
#ifdef MAC_SECURITY_ZIP
    sec_ctrl_t sec_ctrl;
    uint8_t key_id_len;
    uint32_t frame_cnt;
    uint8_t key_id[MAX_KEY_ID_FIELD_LEN];
#endif  /* MAC_SECURITY_ZIP */
    uint8_t mac_command;
    uint8_t ppdu_link_quality;
#if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)
    /*
     * The timestamping is only required for beaconing networks
     * or if timestamping is explicitly enabled.
     */
    uint32_t time_stamp;
#endif  /* #if (defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP) */
    uint8_t mac_payload_length;     /* Length of the MAC payload without FCS */
    frame_payload_t mac_payload_data;
} parse_t;

/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MAC_DATA_STRUCTURES_H */
/* EOF */
