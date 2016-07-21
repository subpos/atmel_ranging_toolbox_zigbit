/**
 * @file rtb.h
 *
 * @brief Provides access RTB Layer functionality.
 *
 * $Id: rtb.h 34198 2013-02-05 12:36:47Z sschneid $
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
#ifndef RTB_H
#define RTB_H

/* === Includes ============================================================= */

#include "tal.h"
#include "rtb_types.h"
#include "rtb_pmu.h"
#ifdef RTB_WITHOUT_MAC
#include "qmm.h"
#endif  /* #ifdef RTB_WITHOUT_MAC */

/* === Macros =============================================================== */

/** Defines an invalid ranging distance. */
#define INVALID_DISTANCE                (0xFFFFFFFF)
/** Defines an invalid DQF. */
#define DQF_ZERO                        (0)

/* === Types ================================================================ */


/* === Externals ============================================================ */

#ifdef RTB_WITHOUT_MAC
/**
 * NHLE to RTB queue in which NHLE pushes all the requests to the RTB layer
 * in case the RTB is the highest stack layer
 * (i.e. MAC layer is not available).
 */
extern queue_t nhle_rtb_q;
#endif  /* #ifdef RTB_WITHOUT_MAC */

/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

    void dispatch_rtb_event(uint8_t *event);

    retval_t rtb_init(void);

    void rtb_range_request(uint8_t *msg);

#ifndef RTB_WITHOUT_MAC
    void rtb_reset_request(uint8_t *msg);
#endif  /* #ifndef RTB_WITHOUT_MAC */

    void rtb_process_data_ind(uint8_t *msg);

#ifdef ENABLE_RTB_REMOTE
    void rtb_remote_range_conf(uint8_t *msg);
#endif  /* ENABLE_RTB_REMOTE */

    void rtb_range_conf(uint8_t *msg);

#ifndef RTB_WITHOUT_MAC
    void rtb_reset_conf(uint8_t *msg);
#endif  /* #ifndef RTB_WITHOUT_MAC */

    void rtb_task(void);

    void rtb_tx_frame_done_cb(retval_t status, frame_info_t *frame);

    void rtb_set_request(uint8_t *msg);
    void rtb_set_conf(uint8_t *msg);
    retval_t rtb_set(uint8_t attribute, pib_value_t *attribute_value, bool set_trx_to_sleep);

#ifndef RTB_WITHOUT_MAC
    void rtb_pmu_validitiy_ind(uint8_t *msg);
#endif  /* #ifndef RTB_WITHOUT_MAC */

#if (RTB_TYPE == RTB_PMU_233R)
    /* Called form transceiver main ISR */
    void rtb_update_fec(void);
#endif  /* (RTB_TYPE == RTB_PMU_233R) */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTB_H */
/* EOF */
