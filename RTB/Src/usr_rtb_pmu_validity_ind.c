/**
 * @file usr_rtb_pmu_validity_ind.c
 *
 * @brief This file contains user call back function for RTB-PMU-VALIDITY.indication.
 *
 * $Id: usr_rtb_pmu_validity_ind.c 33203 2012-08-23 13:59:03Z sschneid $
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

#ifndef RTB_WITHOUT_MAC
void usr_rtb_pmu_validity_ind(usr_rtb_pmu_validity_ind_t *urpv)
{
    /* Keep compiler happy. */
    urpv = urpv;
}
#endif  /* #ifndef RTB_WITHOUT_MAC */

#endif  /* #ifdef ENABLE_RTB */

/* EOF */
