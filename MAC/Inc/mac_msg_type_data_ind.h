/**
 * @file mac_msg_type_data_ind.h
 *
 * @brief This file defines all message structures for the MAC.
 *
 * $Id: mac_msg_type_data_ind.h 32634 2012-07-13 11:40:40Z agasthian.s $
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
#ifndef MAC_MSG_TYPE_DATA_IND_H
#define MAC_MSG_TYPE_DATA_IND_H

/* === Includes ============================================================= */

#include "mac_msg_const.h"

/* === Macros =============================================================== */


/* === Types ================================================================ */

/**
 * @brief This is the MCPS-DATA.indication message structure.
 */

/* In orer to support 32Bit MCUs the MCPS-DATA.indication structure
 * is packed, i.e.,for ARM(GCC & IAR)
 */
#if ((defined __ICCARM__) || (defined __GNUARM__))
#pragma pack(1)
#endif /* __ICCARM__, __GNUARM__*/
typedef struct mcps_data_ind_tag
{
    /**< This identifies the message as \ref MCPS_DATA_INDICATION */
    enum msg_code cmdcode;
    /**
     * The source addressing mode for this primitive corresponding to the
     * received MPDU. This value can take one of the following values:
     * 0x00 = no address (addressing fields omitted). 0 x 01 = reserved.
     * 0x02 = 16 bit short address. 0 x 03 = 64 bit extended address.
     */
    uint8_t SrcAddrMode;
    /**
     * The 16 bit PAN identifier of the entity from which the MSDU was received.
     */
    uint16_t SrcPANId;
    /**
     * The individual device address of the entity from which the
     * MSDU was received.
     */
    uint64_t SrcAddr;
    /**
     * The destination addressing mode for this primitive corresponding to the
     * received MPDU. This value can take one of the following values:
     * 0x00 = no address (addressing fields omitted). 0 x 01 = reserved.
     * 0x02 = 16 bit short device address. 0 x 03 = 64 bit extended
     * device address.
     */
    uint8_t DstAddrMode;
    /**
     * The 16 bit PAN identifier of the entity to which the MSDU is
     * being transferred.
     */
    uint16_t DstPANId;
    /**
     * The individual device address of the entity to which the MSDU is
     * being transferred.
     */
    uint64_t DstAddr;
    /**
     * LQI value measured during reception of the MPDU. Lower values
     * represent lower LQI (see 6.7.8).
     */
    uint8_t mpduLinkQuality;
    /**
     * The DSN of the received data frame.
     */
    uint8_t DSN;
#ifdef ENABLE_TSTAMP
    /**
     * Optional. The time, in symbols, at which the data were received
     * (see 7.5.4.1).
     * The symbol boundary is described by macSyncSymbolOffset (see Table 86
     * in 7.4.1).
     * This is a 24-bit value, and the precision of this value shall be a
     * minimum of 20 bits, with the lowest 4 bits being the least significant.
     */
    uint32_t Timestamp;
#endif  /* ENABLE_TSTAMP */
#ifdef MAC_SECURITY_ZIP
    /**
     * The security level purportedly used by the received data frame.
     */
    uint8_t SecurityLevel;
    /**
     * The mode used to identify the key purportedly used by
     * the originator of the received frame.
     */
    uint8_t KeyIdMode;
    /**
     * The index of the key purportedly used by the originator
     * of the received frame.
     */
    uint8_t KeyIndex;
#endif  /* MAC_SECURITY_ZIP */
    /**
     * The number of octets contained in the MSDU being indicated by the
     * MAC sublayer entity.
     */
    uint8_t msduLength;
    /**
     * The set of octets forming the MSDU being indicated by the
     * MAC sublayer entity.
     */
    uint8_t *msdu;
} mcps_data_ind_t;
/*End packing for ARM(GCC & IAR) */
#if ((defined __ICCARM__) || (defined __GNUARM__))
#pragma pack()
#endif /* __ICCARM__, __GNUARM__ */

/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MAC_MSG_TYPE_DATA_IND_H */
/* EOF */
