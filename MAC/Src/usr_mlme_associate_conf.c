/**
 * @file usr_mlme_associate_conf.c
 *
 * @brief This file contains user call back function for MLME-ASSOCIATE.confirm.
 *
 * $Id: usr_mlme_associate_conf.c 20399 2010-02-18 08:10:29Z sschneid $
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

#if (MAC_ASSOCIATION_REQUEST_CONFIRM == 1)

/* === Macros ============================================================== */


/* === Globals ============================================================= */


/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

void usr_mlme_associate_conf(uint16_t AssocShortAddress,
                             uint8_t status)
{
    /* Keep compiler happy. */
    AssocShortAddress = AssocShortAddress;
    status = status;
}

#endif /* (MAC_ASSOCIATION_REQUEST_CONFIRM == 1) */

/* EOF */
