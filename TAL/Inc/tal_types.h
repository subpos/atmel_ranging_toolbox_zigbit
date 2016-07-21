/**
 * @file tal_types.h
 *
 * @brief This file contains defines for TAL types.
 *
 * $Id: tal_types.h 33714 2012-10-18 11:44:00Z jwunsch $
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
#ifndef TAL_TYPES_H
#define TAL_TYPES_H

/* === INCLUDES ============================================================ */

/* TAL types: */
#define AT86RF230A              (0x01)
#define AT86RF230B              (0x02)
#define AT86RF231               (0x11)
#define AT86RF212               (0x21)
#define AT86RF212B              (0x22)
#define AT86RF232               (0x12)
#define AT86RF233               (0x13)
/* TAL Type for Mega RF single chips, e.g. ATMEGA128RFA1 */
#define ATMEGARFA1          (0x51)
#define ATMEGARFR2              (0x53)

/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TAL_TYPES_H */
/* EOF */
