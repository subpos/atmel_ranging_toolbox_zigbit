/**
 * @file usr_mlme_beacon_notify_ind.c
 *
 * @brief This file contains user call back function for
 * MLME-BEACON-NOTIFY.indication.
 *
 * $Id: usr_mlme_beacon_notify_ind.c 21505 2010-04-13 08:12:56Z sschneid $
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

#include <stdint.h>
#include <stdbool.h>
#include "mac_api.h"

#if (MAC_BEACON_NOTIFY_INDICATION == 1)

/* === Macros ============================================================== */


/* === Globals ============================================================= */


/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

void usr_mlme_beacon_notify_ind(uint8_t BSN,
                                wpan_pandescriptor_t *PANDescriptor,
                                uint8_t PendAddrSpec,
                                uint8_t *AddrList,
                                uint8_t sduLength,
                                uint8_t *sdu)
{
    /* Keep compiler happy. */
    BSN = BSN;
    PANDescriptor = PANDescriptor;
    PendAddrSpec = PendAddrSpec;
    AddrList = AddrList;
    sduLength = sduLength;
    sdu = sdu;
}

#endif /* (MAC_BEACON_NOTIFY_INDICATION == 1) */

/* EOF */
