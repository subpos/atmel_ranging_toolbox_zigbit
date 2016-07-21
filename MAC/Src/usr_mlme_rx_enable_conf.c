/**
 * @file usr_mlme_rx_enable_conf.c
 *
 * @brief This file contains user call back function for MLME-RX-ENABLE.confirm.
 *
 * $Id: usr_mlme_rx_enable_conf.c 20399 2010-02-18 08:10:29Z sschneid $
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

#if (MAC_RX_ENABLE_SUPPORT == 1)

/* === Macros ============================================================== */

/* === Globals ============================================================= */

/* === Prototypes ========================================================== */

/* === Implementation ====================================================== */

void usr_mlme_rx_enable_conf(uint8_t status)
{
    status = status;    /* Keep compiler happy. */
}

#endif /* (MAC_RX_ENABLE_SUPPORT == 1) */

/* EOF */
