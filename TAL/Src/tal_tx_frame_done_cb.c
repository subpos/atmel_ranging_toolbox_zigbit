/**
 * @file tal_tx_frame_done_cb.c
 *
 * @brief This file contains user call back function for
 * tal_tx_frame_done_cb.
 *
 * $Id: tal_tx_frame_done_cb.c 18579 2009-10-15 16:25:01Z sschneid $
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

void tal_tx_frame_done_cb(retval_t status, frame_info_t *frame)
{
    /* Keep compiler happy. */
    status = status;
    frame = frame;
}

/* EOF */
