/**
 * @file usr_rtb_range_conf.c
 *
 * @brief This file contains user call back function for RTB-RANGE.confirm.
 *
 * $Id: usr_rtb_range_conf.c 32879 2012-07-30 09:23:28Z sschneid $
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

/* === Includes ============================================================= */

#include <stdint.h>
#include <stdbool.h>
#include "rtb_api.h"

/* === Macros ============================================================== */


/* === Globals ============================================================= */


/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

void usr_rtb_range_conf(usr_rtb_range_conf_t *urrc)
{
    /* Keep compiler happy. */
    urrc = urrc;
}

#endif  /* #ifdef ENABLE_RTB */

/* EOF */
