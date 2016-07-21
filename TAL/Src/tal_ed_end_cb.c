/**
 * @file tal_ed_end_cb.c
 *
 * @brief This file contains user call back function for
 * tal_ed_end_cb.
 *
 * $Id: tal_ed_end_cb.c 16323 2009-06-23 16:38:09Z sschneid $
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
#include "tal.h"

/* === Macros ============================================================== */

/* === Globals ============================================================= */

/* === Prototypes ========================================================== */

/* === Implementation ====================================================== */

void tal_ed_end_cb(uint8_t energy_level)
{
    energy_level = energy_level;    /* Keep compiler happy. */
}
/* EOF */
