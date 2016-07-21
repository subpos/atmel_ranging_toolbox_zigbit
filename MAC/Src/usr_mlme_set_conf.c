/**
 * @file usr_mlme_set_conf.c
 *
 * @brief This file contains user call back function for MLME-SET.confirm.
 *
 * $Id: usr_mlme_set_conf.c 26610 2011-05-11 08:47:45Z sschneid $
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

/* === Macros ============================================================== */

/* === Globals ============================================================= */

/* === Prototypes ========================================================== */

/* === Implementation ====================================================== */

#ifdef MAC_SECURITY_ZIP
void usr_mlme_set_conf(uint8_t status, uint8_t PIBAttribute, uint8_t PIBAttributeIndex)
#else
void usr_mlme_set_conf(uint8_t status, uint8_t PIBAttribute)
#endif  /* MAC_SECURITY_ZIP */
{
    /* Keep compiler happy. */
    status = status;
    PIBAttribute = PIBAttribute;
#ifdef MAC_SECURITY_ZIP
    PIBAttributeIndex = PIBAttributeIndex;
#endif  /* MAC_SECURITY_ZIP */
}
/* EOF */
