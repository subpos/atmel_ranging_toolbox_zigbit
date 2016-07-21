/**
 * @file tal_rx_frame_cb.c
 *
 * @brief This file contains user call back function for
 * tal_rx_frame_cb.
 *
 * $Id: tal_rx_frame_cb.c 21491 2010-04-13 07:49:11Z sschneid $
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

void tal_rx_frame_cb(frame_info_t *rx_frame)
{
    /* Keep compiler happy. */
    rx_frame = rx_frame;
}

/* EOF */
