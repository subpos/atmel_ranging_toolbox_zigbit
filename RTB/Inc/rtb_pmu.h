/**
 * @file rtb_pmu.h
 *
 * @brief Header file for PMU dependent functionality of RTB
 *
 * $Id: rtb_pmu.h 34343 2013-02-22 11:45:08Z sschneid $
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
#ifndef RTB_PMU_H
#define RTB_PMU_H

/* === Includes ============================================================= */

#include "rtb_types.h"

/* === Macros =============================================================== */

#if (ANTENNA_DIVERSITY == 1) || defined(DOXYGEN)
/**
 * The maximum number of antennas used in case both Initiator and
 * Reflector use both antennas.
 */
#define PMU_MAX_NO_ANTENNAS             (4)
#else
/**
 * The maximum number of antennas used in case antenna diversity is not
 * enabled for this build/configuration. The other party may have
 * enabled antenna diversity.
 */
#define PMU_MAX_NO_ANTENNAS             (2)
#endif  /* (ANTENNA_DIVERSITY == 1) */


/* Capabilities conveyed in Range Request frame between Initiator and Reflector. */
/** Bit position of Initiator antenna bit in capability field of Range Request frame. */
#define BIT_POS_INITIATOR_ANT           (0)
/** Bit position of Reflector antenna bit in capability field of Range Request frame. */
#define BIT_POS_REFLECTOR_ANT           (1)

/** Capability field bit mask for Initiator antenna diversity */
#define PMU_CAP_INITIATOR_ANT           (_BV(BIT_POS_INITIATOR_ANT))
/** Capability field bit mask for Reflector antenna diversity */
#define PMU_CAP_REFLECTOR_ANT           (_BV(BIT_POS_REFLECTOR_ANT))


#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
/*
 * Remote range capabilities conveyed in Remote Range Request frame between
 * Coordinator and Initiator.
 */
/**
 * Bit position of provisioning of antenna diversity results bit
 * in capability field of Range Request frame.
 */
#define BIT_POS_PROV_ANT_DIV_RES        (0)
/**
 * Bit position of applying minimum threshold during
 * weighted distance calculation bit
 * in capability field of Range Request frame.
 */
#define BIT_POS_APPLY_MIN_DIST_THRSHLD  (1)

/** Remote capability field bit mask for provisioning of antenna diversity results  */
#define PMU_REM_CAP_PROV_ANT_DIV_RES        (_BV(BIT_POS_PROV_ANT_DIV_RES))
/**
 * Remote capability field bit mask for
 * applying minimum threshold during weighted distance calculation
 */
#define PMU_REM_CAP_APPLY_MIN_DIST_THRSHLD  (_BV(BIT_POS_APPLY_MIN_DIST_THRSHLD))
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */

/** The Range Result Confirm frame must not exceed the MAC frame length. */
#define MAX_RESULT_VALUES_PER_FRAME     (aMaxMACSafePayloadSize - CMD_RESULT_CONF_LEN)

/* === Types ================================================================ */

/**
 * Type for Result Data IE within Range Result Request/Confirm frame.
 */
typedef enum result_frame_ie_tag
{
    RESULT_IE_PMU_VALUES    = 0x00, /**< Information Element for PMU results values */
} SHORTENUM result_frame_ie_t;

/**
 * Type for Additional Result IE within Remote Range Confirm frame.
 */
typedef enum additional_result_ie_tag
{
    NO_ADDITIONAL_RESULTS       = 0x00, /**< No aditional results included */
    ANT_DIV_MEAS_RESULTS        = 0x01  /**< Antenna diversity measurement results included */
} SHORTENUM additional_result_ie_t;

/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

    void pmu_configure_antenna(void);
    void pmu_configure_ranging(void);
#if (RTB_TYPE == RTB_PMU_233R)
    void pmu_disable_fec_measurement(void);
    void pmu_enable_fec_measurement(void);
#endif  /* (RTB_TYPE == RTB_PMU_233R) */
    void pmu_math_pmu_2_dist(void);
    void pmu_perform_pmu_measurement(void);
    void pmu_prepare_result_exchange(result_frame_ie_t next_result_data);
    void pmu_tx_pmu_time_sync_frame(void);
#if defined(SIO_HUB) && defined(ENABLE_RTB_PRINT)
    void pmu_range_pmu_result_dump(void);
#endif  /* if defined(SIO_HUB) && defined(ENABLE_RTB_PRINT) */
#ifndef RTB_WITHOUT_MAC
    void pmu_validity_indication(uint8_t antenna_value);
#endif  /* #ifndef RTB_WITHOUT_MAC */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTB_PMU_H */
/* EOF */
