/**
 * @file mac_msg_const.h
 *
 * @brief This file defines all message constants.
 *
 * $Id: mac_msg_const.h 26610 2011-05-11 08:47:45Z sschneid $
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
#ifndef MAC_MSG_CONST_H
#define MAC_MSG_CONST_H

/* === Includes ============================================================= */


/* === Macros =============================================================== */

/**
 * This type contains the service primitives of the PHY-, MAC- and Network-layer
 * as well the command-interface primitives
 */
enum msg_code
{
    TAL_DATA_INDICATION                 = (0x00),

    MLME_ASSOCIATE_REQUEST              = (0x01),
    MLME_ASSOCIATE_RESPONSE             = (0x02),

    MCPS_DATA_REQUEST                   = (0x03),
    MCPS_PURGE_REQUEST                  = (0x04),

    MLME_DISASSOCIATE_REQUEST           = (0x05),
#if (HIGHEST_STACK_LAYER == MAC)
    MLME_SET_REQUEST                    = (0x06),
#endif /* (HIGHEST_STACK_LAYER == MAC) */
    MLME_ORPHAN_RESPONSE                = (0x07),
    MLME_GET_REQUEST                    = (0x08),
    MLME_RESET_REQUEST                  = (0x09),
    MLME_RX_ENABLE_REQUEST              = (0x0A),
    MLME_SCAN_REQUEST                   = (0x0B),
    MLME_START_REQUEST                  = (0x0D),
    MLME_POLL_REQUEST                   = (0x0E),
    MLME_SYNC_REQUEST                   = (0x0F),

    MCPS_DATA_CONFIRM                   = (0x10),
    MCPS_DATA_INDICATION                = (0x11),
    MCPS_PURGE_CONFIRM                  = (0x12),

    MLME_ASSOCIATE_INDICATION           = (0x13),
    MLME_ASSOCIATE_CONFIRM              = (0x14),
    MLME_DISASSOCIATE_INDICATION        = (0x15),
    MLME_DISASSOCIATE_CONFIRM           = (0x16),
    MLME_BEACON_NOTIFY_INDICATION       = (0x17),
    MLME_ORPHAN_INDICATION              = (0x1A),
    MLME_SCAN_CONFIRM                   = (0x1B),
    MLME_COMM_STATUS_INDICATION         = (0x1C),
    MLME_SYNC_LOSS_INDICATION           = (0x1D),
    MLME_GET_CONFIRM                    = (0x1E),
#if (HIGHEST_STACK_LAYER == MAC)
    MLME_SET_CONFIRM                    = (0x1F),
#endif /* (HIGHEST_STACK_LAYER == MAC) */
    MLME_RESET_CONFIRM                  = (0x20),
    MLME_RX_ENABLE_CONFIRM              = (0x21),
    MLME_START_CONFIRM                  = (0x22),
    MLME_POLL_CONFIRM                   = (0x23)
} SHORTENUM;

/**
 * Bump this when extending the list!
 */
#define LAST_MESSAGE                    MLME_POLL_CONFIRM

/** The following defines describe the minimum length of a primitive message. */
#ifdef ENABLE_TSTAMP
#define MCPS_DATA_CON_LEN               (7)
#define MCPS_DATA_IND_LEN               (31)
#else   /* ENABLE_TSTAMP not enabled */
#define MCPS_DATA_CON_LEN               (7 - 4)
#define MCPS_DATA_IND_LEN               (31 - 4)
#endif  /* ENABLE_TSTAMP */
#define MCPS_PURGE_CONF_LEN             (3)

#define MLME_ASSOCIATE_CONF_LEN         (4)
#define MLME_ASSOCIATE_IND_LEN          (10)
#define MLME_BEACON_NOTIFY_IND_LEN      (2)
#define MLME_COMM_STATUS_IND_LEN        (22)
#define MLME_DISASSOCIATE_CONF_LEN      (13)
#define MLME_DISASSOCIATE_IND_LEN       (10)
#ifdef MAC_SECURITY_ZIP
#define MLME_GET_CONF_LEN               (5)
#else
#define MLME_GET_CONF_LEN               (4)
#endif  /* MAC_SECURITY_ZIP */
#define MLME_ORPHAN_IND_LEN             (9)
#define MLME_POLL_CONF_LEN              (2)
#define MLME_RESET_CONF_LEN             (2)
#define MLME_RX_ENABLE_CONF_LEN         (2)
#define MLME_SCAN_CONF_LEN              (10)
#ifdef MAC_SECURITY_ZIP
#define MLME_SET_CONF_LEN               (4)
#else
#define MLME_SET_CONF_LEN               (3)
#endif  /* MAC_SECURITY_ZIP */
#define MLME_START_CONF_LEN             (2)
#define MLME_SYNC_LOSS_IND_LEN          (6)

#ifdef ENABLE_TSTAMP
#define WPAN_DESCRIPTOR_LEN             (21)
#else   /* ENABLE_TSTAMP not enabled */
#define WPAN_DESCRIPTOR_LEN             (21 - 4)
#endif  /* ENABLE_TSTAMP */

/* === Types ================================================================ */


/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MAC_MSG_CONST_H */
/* EOF */
