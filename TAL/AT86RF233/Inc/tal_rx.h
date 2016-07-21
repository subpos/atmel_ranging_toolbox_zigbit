/**
 * @file tal_rx.h
 *
 * @brief File contains macros and modules used while processing
 * a received frame.
 *
 * $Id: tal_rx.h 28417 2011-09-02 14:15:09Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef TAL_RX_H
#define TAL_RX_H

/* === INCLUDES ============================================================ */


/* === EXTERNALS =========================================================== */


/* === TYPES =============================================================== */


/* === MACROS ============================================================== */


/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

    void handle_received_frame_irq(void);

    void process_incoming_frame(buffer_t *buf);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TAL_RX_H */
