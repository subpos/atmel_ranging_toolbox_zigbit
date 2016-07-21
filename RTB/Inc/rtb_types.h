/**
 * @file rtb_types.h
 *
 * @brief This file contains defines for RTB types.
 *
 * $Id: rtb_types.h 32879 2012-07-30 09:23:28Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2011, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef RTB_TYPES_H
#define RTB_TYPES_H

/* === INCLUDES ============================================================ */

/* RTB types for Ranging: */
/** Dummy RTB type */
#define NO_RTB                          (0x00)
/** RTB based on PMU implementation in AT86RF233R */
#define RTB_PMU_233R                    (0x01)
/** RTB based on PMU implementation in ATmega256RFR2 */
#define RTB_PMU_RFR2                    (0x02)
/** RTB for Ranging host (HR) */
#define RTB_FOR_RH                      (0x03)

/* === PROTOTYPES ========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTB_TYPES_H */
/* EOF */
