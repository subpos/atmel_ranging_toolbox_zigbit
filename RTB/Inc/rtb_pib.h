/**
 * @file rtb_pib.h
 *
 * @brief Header file for PIB attributes of RTB
 *
 * $Id: rtb_pib.h 34339 2013-02-22 10:11:19Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2012, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef RTB_PIB_H
#define RTB_PIB_H

/* === Includes ============================================================= */


/* === Macros =============================================================== */

/* Below the start index for RTB PIB attributes is defined. */
#if (defined RTB_WITHOUT_MAC) ||  (defined ENABLE_RH)
/*
 * RTB_WITHOUT_MAC:
 * No MAC or higher layer is inlcuded, so the native RTB PIB attributes starts
 * at a higher value (far behind the MAC standard PIB attributes),
 * since the required MAC standard PIB attributes have a to use the same
 * API.
 *
 * ENABLE_RH:
 * This is a build incorporating ONLY PIB attributes from RTB
 * (i.e. Ranging Host) (which implies MAC or higher layers are included).
 * The native RTB PIB attributes starts
 * at a higher value (far behind the MAC standard PIB attributes).
 */
#   define RTB_NATIVE_PIB_START         (200)
#else   /* RTB_WITHOUT_MAC / ENABLE_RH */
/**
 * If MAC or higher layers are included, the native RTB PIB attributes starts
 * at 0, since all MAC standard PIB attributes have a separate API.
 */
#   define RTB_NATIVE_PIB_START         (0)
#endif  /* RTB_WITHOUT_MAC / ENABLE_RH */

/** Default start frequency in the ISM band. */
#define PMU_START_FREQ_DEFAULT          (2403)
/** Minimum possible PMU measurement frequency (in the none ISM band) */
#define PMU_MIN_FREQ                    (2324)

/** Default stop frequency in the ISM band. */
#define PMU_STOP_FREQ_DEFAULT           (2443)
/** Maximum possible PMU measurement frequency */
#define PMU_MAX_FREQ                    (2527)

/** PMU frequency step size is 500kHz. */
#define PMU_STEP_FREQ_500kHz            (0)
/** PMU frequency step size is 1000kHz. */
#define PMU_STEP_FREQ_1MHz              (1)
/** PMU frequency step size is 2000kHz. */
#define PMU_STEP_FREQ_2MHz              (2)
/** PMU frequency step size is 4000kHz. */
#define PMU_STEP_FREQ_4MHz              (3)
/** Default PMU frequency step */
#define PMU_STEP_FREQ_DEFAULT           (PMU_STEP_FREQ_2MHz)
/** Maximum PMU frequency step in MHz (required for further calculation) */
#define PMU_STEP_FREQ_MAX_IN_MHZ        (4) /* == 4MHz */

/* === Types ================================================================ */

/** Structure implementing the RTB related PIB attributes. */
typedef struct rtb_pib_tag
{
    /** Holds the status whether ranging is currently enabled. */
    bool RangingEnabled;

#if !defined(RTB_WITHOUT_MAC) || defined(DOXYGEN)
    /** Holds the ranging method currently utilized. */
    uint8_t RangingMethod;
#endif  /* #if !defined(RTB_WITHOUT_MAC) || defined(DOXYGEN) */

    /** Holds the PMU measurement start index in the frequency table. */
    uint16_t PMUFreqStart;

    /** Holds the PMU measurement step size in the frequency table. */
    uint8_t PMUFreqStep;

    /** Holds the PMU measurement stop index in the frequency table. */
    uint16_t PMUFreqStop;

#if !defined(RTB_WITHOUT_MAC) || defined(DOXYGEN)
    /** Holds the verbose level. */
    uint8_t PMUVerboseLevel;
#endif  /* #if !defined(RTB_WITHOUT_MAC) || defined(DOXYGEN) */
    /**
     * Holds the ranging procedure default antenna of the node:
     *
     * false: antenna 0 used as default
     *
     * true: antenna 1 used as default
     */
    bool DefaultAntenna;

    /**
     * Holds the default value for antenna diversity:
     * false: antenna diversity not used as default
     * true: antenna diversity used as default
     */
    bool EnableAntennaDiv;

#if !defined(RTB_WITHOUT_MAC) || defined(DOXYGEN)
    /**
     * Holds the default value for providing all measured
     * distances and DQFs based on the applied antenna diversity
     * scheme within the RTB-Range.confirm message if this node act as the
     * Initiator.
     *
     * false: Only the final calculated distance and DQF is provided
     *        within RTB-Range.confirm
     *
     * true: In addition to the final calculated distance and DQF also
     *       each separate measured distance and DQF is appended at the end
     *       of the RTB-Range.confirm message
     *
     * Note: Even if this node does not provide antenna diversity, the peer
     *       node may use antenna diversity, so the utilization of this feature
     *       is not only dependent from the status of EnableAntennaDiv from this
     *       node, but also from the status of EnableAntennaDiv from the peer
     *       node.
     */
    bool ProvideAntennaDivResults;
#endif  /* #if !defined(RTB_WITHOUT_MAC) || defined(DOXYGEN) */

    /**
     * Holds the current Ranging Transmit Power of the node to be explicitly
     * applied during ranging.
     */
    uint8_t RangingTransmitPower;

    /**
     * Holds the current value for sending the other ranging party the
     * Ranging Transmit Power to be applied during the actual ranging
     * measurement phase.
     *
     * If this PIB attribute is set, the following behaviour will occur.
     *
     * (1) This node is the Initiator of a Local Ranging procedure
     * and will sent the Reflector its own value of the PIB attribute
     * RangingTransmitPower to be applied at the Reflector during
     * ranging.
     *
     * (2) This node is the Coordinator during a Remote Ranging procedure
     * and will sent the Reflector its own value of the PIB attribute
     * RangingTransmitPower to be applied at both the Initiator and
     * the Reflector during ranging.
     *
     * If this PIB attribute is NOT set, this node will not provide
     * the other parties (as described above) with its own
     * Ranging Transmit Power.
     */
    bool ProvideRangingTransmitPower;

    /**
     * Holds the current value for applying a minimum threshold during
     * weighted distance calculation.
     *
     * false: The minimum threshold will be applied during weighted distance
     *        calculation
     *
     * true: The minimum threshold will not be applied during weighted distance
     *       calculation; a regular minimum search among all measured distance
     *       values will be applied
     */
    bool ApplyMinDistThreshold;
} rtb_pib_t;



/** RTB PIB attribute type */
typedef enum rtb_pib_id_tag
{
    RTB_PIB_RANGING_ENABLED             = RTB_NATIVE_PIB_START + 0x00,  /**< Defines if ranging is currently enabled. */
    RTB_PIB_RANGE_METHOD                = RTB_NATIVE_PIB_START + 0x01,  /**< Defines the current ranging method. */
    RTB_PIB_PMU_FREQ_START              = RTB_NATIVE_PIB_START + 0x02,  /**< Defines the PMU start frequency. */
    RTB_PIB_PMU_FREQ_STEP               = RTB_NATIVE_PIB_START + 0x03,  /**< Defines the PMU frequency step. */
    RTB_PIB_PMU_FREQ_STOP               = RTB_NATIVE_PIB_START + 0x04,  /**< Defines the PMU stop frequency. */
    RTB_PIB_PMU_VERBOSE_LEVEL           = RTB_NATIVE_PIB_START + 0x05,  /**< Defines the current verbosity level. */
    RTB_PIB_DEFAULT_ANTENNA             = RTB_NATIVE_PIB_START + 0x06,  /**< Defines the current default antenna (if antenna diversity is disabled). */
    RTB_PIB_ENABLE_ANTENNA_DIV          = RTB_NATIVE_PIB_START + 0x07,  /**< Defines if antenna diversity is enabled. */
    RTB_PIB_PROVIDE_ANTENNA_DIV_RESULTS = RTB_NATIVE_PIB_START + 0x08,  /**< Defines if provisioning of antenna diversity measurement is enabled. */
    RTB_PIB_RANGING_TX_POWER            = RTB_NATIVE_PIB_START + 0x09,  /**< Defines current own Ranging Transmit Power. */
    RTB_PIB_PROVIDE_RANGING_TX_POWER    = RTB_NATIVE_PIB_START + 0x0A,  /**< Defines if own Ranging Transmit Power is forced at other nodes. */
    RTB_PIB_APPLY_MIN_DIST_THRESHOLD    = RTB_NATIVE_PIB_START + 0x0B   /**< Defines if minimum threshold for weighted distance calc is applied. */
} SHORTENUM rtb_pib_id_t;

/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

    uint8_t rtb_get_pib_attribute_size(uint8_t pib_attribute_id);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTB_PIB_H */
/* EOF */
