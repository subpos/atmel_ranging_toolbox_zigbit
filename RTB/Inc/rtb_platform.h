/**
 * @file rtb_platform.h
 *
 * @brief Header file for generic platform dependent functionality of RTB
 *
 * $Id: rtb_platform.h 34198 2013-02-05 12:36:47Z sschneid $
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
#ifndef RTB_PLATFORM_H
#define RTB_PLATFORM_H

/* === Includes ============================================================= */

#include "rtb_types.h"
#if (RTB_TYPE == RTB_PMU_233R)
#   include "rtb_hw_233r_xmega.h"
#elif (RTB_TYPE == RTB_PMU_RFR2)
#   include "rtb_hw_rfr2.h"
#elif (RTB_TYPE == RTB_FOR_RH)
/* Do nothing. */
#else
#   error Unsupported RTB type
#endif

/* === Macros =============================================================== */


/* === Types ================================================================ */


/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTB_PLATFORM_H */
/* EOF */
