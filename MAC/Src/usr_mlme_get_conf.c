/**
 * @file usr_mlme_get_conf.c
 *
 * @brief This file contains user call back function for MLME-GET.confirm.
 *
 * $Id: usr_mlme_get_conf.c 26610 2011-05-11 08:47:45Z sschneid $
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

#if (MAC_GET_SUPPORT == 1)

/* === Macros ============================================================== */

/* === Globals ============================================================= */

/* === Prototypes ========================================================== */

/* === Implementation ====================================================== */

void usr_mlme_get_conf(uint8_t status,
                       uint8_t PIBAttribute,
#ifdef MAC_SECURITY_ZIP
                       uint8_t PIBAttributeIndex,
#endif  /* MAC_SECURITY_ZIP */
                       void *PIBAttributeValue)
{
    /* Keep compiler happy. */
    status = status;
    PIBAttribute = PIBAttribute;
#ifdef MAC_SECURITY_ZIP
    PIBAttributeIndex = PIBAttributeIndex;
#endif  /* MAC_SECURITY_ZIP */
    PIBAttributeValue = PIBAttributeValue;
}
#endif  /* (MAC_GET_SUPPORT == 1) */

/* EOF */
