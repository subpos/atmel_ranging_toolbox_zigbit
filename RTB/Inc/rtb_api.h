/**
 * @file rtb_api.h
 *
 * @brief RTB API for applications residing on MAC (IEEE 802.15.4-2006)
 *
 * $Id: rtb_api.h 34343 2013-02-22 11:45:08Z sschneid $
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
#ifndef RTB_API_H
#define RTB_API_H

/* === Includes ============================================================= */

#include "pal.h"
#include "rtb.h"
#include "rtb_pib.h"
#include "return_val.h"
#include "qmm.h"
#include "mac_api.h"

/* === Macros =============================================================== */


/* === Types ================================================================ */

/* Ranging API types ****************** */

/* RTB Range Request related types **** */
/** Structure creating the wpan_rtb_range_req() API function. */
typedef struct wpan_rtb_range_req_tag
{
    /**
     * The Initiator addressing mode for this primitive.
     * This value can take one of the following values:
     * 0x02 = 16 bit short address.
     * 0x03 = 64 bit extended address.
     * Other values are not allowed.
     */
    uint8_t InitiatorAddrMode;
#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
    /**
     * The 16 bit PAN identifier of the Initiator.
     */
    uint16_t InitiatorPANId;
    /**
     * The individual device address the Initiator.
     */
    uint64_t InitiatorAddr;
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */
    /**
     * The Reflector addressing mode for this primitive.
     * This value can take one of the following values:
     * 0x02 = 16 bit short address.
     * 0x03 = 64 bit extended address.
     * Other values are not allowed.
     */
    uint8_t ReflectorAddrMode;
    /**
     * The 16 bit PAN identifier of the Reflector.
     */
    uint16_t ReflectorPANId;
    /**
     * The individual device address the Reflector.
     */
    uint64_t ReflectorAddr;
#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
    /**
     * The Coordinator addressing mode for the received range request.
     * This value can take one of the following values:
     *
     * 0x00 = no Coordinator address; this is a regular range request
     *
     * 0x02 = 16 bit short address; 0x03 = 64 bit extended address;
     * This is a remote range request
     */
    uint8_t CoordinatorAddrMode;
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */
} wpan_rtb_range_req_t;


/* RTB Range Confirm related types **** */

/**
 * Structure implementing the ranging measurement pair.
 */
typedef struct measurement_pair_tag
{
    /** The measured ranging distance in cm. */
    uint32_t distance;
    /** The measured DQF in percent. */
    uint8_t dqf;
} measurement_pair_t;

/*
 * Structure containing the actual local ranging results within the usr_rtb_range_conf()
 * callback.
 */
typedef struct
{
    /** The status of the ranging measurement. */
    uint8_t status;
    /**
     * The final calculated distance of the ranging measurement in cm.
     */
    uint32_t distance;
    /**
     * The final calculated distance quality factor (DQF) of the ranging
     * measurement in percent.
     */
    uint8_t dqf;
#if !defined(RTB_WITHOUT_MAC) || defined(DOXYGEN)
    /* This is only allowed in case the MAC layer is included. */

    /**
     * The number of provided measurements pairs (consisting of distance and
     * DQF) based on the value of the PIB attribute ProvideAntennaDivResults
     * and the applied antenna diversity scheme on initator and Reflector.
     *
     * In case ProvideAntennaDivResults is zero, no further measurement pairs
     * are appended.
     *
     * In case ProvideAntennaDivResults is one, and neither the Initiator
     * nor the Reflector use antenna diversity, no further measurement pairs
     * are appended.
     *
     * In case ProvideAntennaDivResults is one, and either the Initiator or
     * the Reflector use antenna diversity, two further measurement pairs
     * are appended.
     *
     * In case ProvideAntennaDivResults is one, and both the Initiator and
     * the Reflector use antenna diversity, four further measurement pairs
     * are appended.
     *
     * This allows the application to calculate the final distance and/or
     * DQF using its own specific algorithm based on the various measurements.
     */
    uint8_t no_of_provided_meas_pairs;
    /**
     * The set of measurement pairs formed by distance and DQF indicated by
     * the value of no_of_provided_meas_pairs.
     */
    measurement_pair_t provided_meas_pairs[];
#endif  /* #if !defined(RTB_WITHOUT_MAC) || defined(DOXYGEN) */
} local_ranging_result_t;

/*
 * Structure containing the actual remote ranging results within the usr_rtb_range_conf()
 * callback.
 */
typedef struct
{
    /**
     * The Initiator addressing mode for the previously requested
     * remote ranging procedure.
     * This value can take one of the following values:
     * 0x02 = 16 bit short address.
     * 0x03 = 64 bit extended address.
     * Other values are not allowed.
     */
    uint8_t InitiatorAddrMode;
    /**
     * The 16 bit PAN identifier of the Initiator for the previously requested
     * remote ranging procedure.
     */
    uint16_t InitiatorPANId;
    /**
     * The individual device address the Initiator for the previously requested
     * remote ranging procedure.
     */
    uint64_t InitiatorAddr;
    /**
     * The Reflector addressing mode for the previously requested
     * remote ranging procedure.
     * This value can take one of the following values:
     * 0x02 = 16 bit short address.
     * 0x03 = 64 bit extended address.
     * Other values are not allowed.
     */
    uint8_t ReflectorAddrMode;
    /**
     * The 16 bit PAN identifier of the Reflector for the previously requested
     * remote ranging procedure.
     */
    uint16_t ReflectorPANId;
    /**
     * The individual device address the Reflector for the previously requested
     * remote ranging procedure.
     */
    uint64_t ReflectorAddr;
    /** The status of the remote ranging measurement. */
    uint8_t status;
    /**
     * The final calculated distance of the remote ranging measurement in cm.
     */
    uint32_t distance;
    /**
     * The final calculated distance quality factor (DQF) of the remote ranging
     * measurement in percent.
     */
    uint8_t dqf;
    /**
     * The number of provided measurements pairs (consisting of distance and
     * DQF) based on the value of the PIB attribute ProvideAntennaDivResults
     * and the applied antenna diversity scheme on initator and Reflector.
     *
     * In case ProvideAntennaDivResults is zero, no further measurement pairs
     * are appended.
     *
     * In case ProvideAntennaDivResults is one, and neither the Initiator
     * nor the Reflector use antenna diversity, no further measurement pairs
     * are appended.
     *
     * In case ProvideAntennaDivResults is one, and either the Initiator or
     * the Reflector use antenna diversity, two further measurement pairs
     * are appended.
     *
     * In case ProvideAntennaDivResults is one, and both the Initiator and
     * the Reflector use antenna diversity, four further measurement pairs
     * are appended.
     *
     * This allows the application to calculate the final distance and/or
     * DQF using its own specific algorithm based on the various measurements.
     */
    uint8_t no_of_provided_meas_pairs;
    /**
     * The set of measurement pairs formed by distance and DQF indicated by
     * the value of no_of_provided_meas_pairs.
     */
    measurement_pair_t provided_meas_pairs[];
} remote_ranging_result_t;

/*
 * Union containing the various ranging results within the usr_rtb_range_conf()
 * callback based on the actual ranging type.
 */
typedef union
{
    local_ranging_result_t local;
    remote_ranging_result_t remote;
} range_conf_result_t;


/** Supported types of ranging within RTB. */
typedef enum ranging_type_tag
{
    RTB_LOCAL_RANGING                   = 0x00, /**< Local Ranging */
    RTB_REMOTE_RANGING                  = 0x01  /**< Remote Ranging */
} SHORTENUM ranging_type_t;


/** Structure creating the usr_rtb_range_conf() callback. */
typedef struct
{
    ranging_type_t ranging_type;
    range_conf_result_t results;
} usr_rtb_range_conf_t;


#ifndef RTB_WITHOUT_MAC
/* RTB Reset Confirm related types **** */
/** Structure creating the usr_rtb_reset_conf() callback. */
typedef struct usr_rtb_reset_conf_tag
{
    /** The result of the request to reset the RTB. */
    uint8_t status;
} usr_rtb_reset_conf_t;
#endif  /* #ifndef RTB_WITHOUT_MAC */


/* RTB Set Request related types **** */
/** Structure creating the wpan_rtb_set_req() API function. */
typedef struct wpan_rtb_set_req_tag
{
    /** The identifier of the RTB PIB attribute to set. */
    uint8_t PIBAttribute;
    /** The value to write to the indicated RTB PIB attribute. */
    pib_value_t PIBAttributeValue;
} wpan_rtb_set_req_t;


/* RTB Set Confirm related types **** */
/** Structure creating the usr_rtb_set_conf() callback. */
typedef struct usr_rtb_set_conf_tag
{
    /** The result of the request to write the RTB PIB attribute. */
    uint8_t status;
    /** The identifier of the RTB PIB attribute that was written. */
    uint8_t PIBAttribute;
} usr_rtb_set_conf_t;


/* RTB PMU Validity Indication related types **** */
/** Structure creating the usr_rtb_pmu_validity_ind() callback. */
typedef struct usr_rtb_pmu_validity_ind_tag
{
    /**
     * The number of the antenna measurement value for this PMU measurement
     * depending on the used antenna diversity scheme.
     */
    uint8_t PMUAntennaMeasurementValue;
    /**
     * The number of PMU validity values.
     * Each value is indicated by one bit, i.e. each bit corresponds whether
     * the PMU value for one particular frequency is valid.
     * The 1st bits within the 1st octet corresponds to the PMU start frequency.
     * The maximum number of validity values is 160.
     */
    uint8_t PMUValidityValueNo;
    /**
     * An array of octets containing the PMU Validity Values.
     *
     * Note: Each PMU validity value corresponds to one bit in the
     * PMUValidityValues array of octets. I.e. the actual length of
     * the PMUValidityValues array is derived from PMUValidityValueNo
     * (number of included bits used as PMU validity values).
     *
     * Example: In case PMUValidityValueNo = 80, PMUValidityValues contains
     * exactly 8 octets.
     */
    uint8_t PMUValidityValues[];
} usr_rtb_pmu_validity_ind_t;


/* Other types ************************ */

/** Type definition for access to PMU averaged values */
/* DO NOT CHANGE THIS */
typedef struct pmu_avg_data_t_tag
{
    /**
     * General offset for pointers to averaged PMU value arrays
     * for various antenna measurement pairs.
     */
    uint16_t ant_meas_ptr_offset;

    /**
     * Actual number of utilized antenna measurement pairs during the
     * PMU measurement.
     * This variable defines how many of the pointers
     * p_pmu_avg_init/p_pmu_avg_refl are actually applicable.
     */
    uint8_t no_of_ant_meas;

    /**
     * Actual number of frequencies used during PMU measurement.
     * This variable defines the length of valid averaged PMU
     * values defined at p_pmu_avg.
     */
    uint8_t no_of_freq;

    /**
     * Pointers to start of averaged PMU value array for the
     * Initiator/Reflector for the FIRST per antenna measurement pair.
     *
     * Note 1):
     * If a pointer p_pmu_avg_init/p_pmu_avg_refl is NULL,
     * no valid averaged PMU values are available.
     * Any other value defines valid averaged PMU values starting at
     * p_pmu_avg_init/p_pmu_avg_refl.
     *
     * Note 2):
     * These pointers are only valid at the Initiator node, but never at the
     * Reflector node.
     *
     * Note 3):
     * In case any node (Initiator or Reflector) uses antenna diversity,
     * and thus more than one (i.e. 2 or 4) antenna measurement pairs are
     * available, the pointers to the averaged PMU value arrays corresponding
     * to the antenna measurement pairs beyond the first antenna measurement
     * pair are accessed according to the following scheme:
     *
     * a) no_of_ant_meas contains the number of valid antenna measurement pairs
     * b) The starting pointer to the next array of averaged PMU values is derived
     *    by adding ant_meas_ptr_offset to the previous pointer.
     *
     * Example:
     * no_of_ant_meas = 4 (Both nodes use antenna diversity resulting in 4 antenna
     * measurement pairs)
     * p_pmu_avg_init points to address 0x1000
     * (start address of array to averaged PMU values of Initiator for antenna pair 1)
     * p_pmu_avg_init points to address 0x2000
     * (start address of array to averaged PMU values of Reflector for antenna pair 1)
     *
     * Start address of array of averaged PMU values of Initiator for
     * antenna pair 2: 0x1000 + ant_meas_ptr_offset
     * antenna pair 3: 0x1000 + 2 * ant_meas_ptr_offset
     * antenna pair 4: 0x1000 + 3 * ant_meas_ptr_offset
     *
     * Start address of array of averaged PMU values of Reflector for
     * antenna pair 2: 0x2000 + ant_meas_ptr_offset
     * antenna pair 3: 0x2000 + 2 * ant_meas_ptr_offset
     * antenna pair 4: 0x2000 + 3 * ant_meas_ptr_offset
     *
     * Each array of averaged PMU values contains "no_of_freq" values.
     */
    uint8_t *p_pmu_avg_init;
    uint8_t *p_pmu_avg_refl;
} pmu_avg_data_t;

/* === Externals ============================================================ */

#ifdef RTB_WITHOUT_MAC
/**
 * Queue used by RTB for communication to next higher layer in case the RTB
 * is the highest stack layer (i.e. MAC layer is not available).
 */
extern queue_t rtb_nhle_q;
#endif  /* #ifdef RTB_WITHOUT_MAC */
extern const uint8_t aRTBMaxPMUVerboseLevel;
/* DO NOT CHANGE THIS */
extern pmu_avg_data_t pmu_avg_data;

/* === Macros =============================================================== */

#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
/** No Coordinator requested, i.e. regular ranging */
#define NO_COORDINATOR                  (FCF_NO_ADDR)
/** Coordinator uses its short address for remote ranging */
#define COORDINATOR_SHORT_ADDR          (FCF_SHORT_ADDR)
/** Coordinator uses its long address for remote ranging */
#define COORDINATOR_LONG_ADDR           (FCF_LONG_ADDR)
#endif  /* ENABLE_RTB_REMOTE */

/* === Prototypes =========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

    /*--------------------------------------------------------------------*/

    /*
     * These functions have to be called from the application
     * in order to initiate an action in the communication
     * stack at the RTB API at MAC level
     */

    /**
     * Initiate RTB-RANGE.request service and have it placed in the RTB-SAP queue.
     *
     * @param wrrr  Pointer to wpan_rtb_range_req_t structure
     *
     * @return true - success; false - buffer not available or queue full.
     *
     * @ingroup apiRTB_API
     */
    bool wpan_rtb_range_req(wpan_rtb_range_req_t *wrrr);

#ifndef RTB_WITHOUT_MAC
    /**
     * Initiate RTB-RESET.request service and have it placed in RTB-SAP queue.
     *
     * @return true - success; false - buffer not available or queue full.
     *
     * @ingroup apiRTB_API
     */
    bool wpan_rtb_reset_req(void);
#endif  /* #ifndef RTB_WITHOUT_MAC */


    /**
     * Initiate RTB-SET.request service and have it placed in RTB-SAP queue.
     *
     * @param wrsr  Pointer to wpan_rtb_set_req_t structure
     *
     * @return true - success; false - buffer not available or queue full.
     *
     * @ingroup apiRTB_API
     */
    bool wpan_rtb_set_req(wpan_rtb_set_req_t *wrsr);


#ifndef RTB_WITHOUT_MAC
    /**
     * Callback function that must be implemented by the application (NHLE)
     * for the RTB service RTB-RESET.confirm.
     *
     * @param urrc  Pointer to usr_rtb_reset_conf_t result structure.
     *
     * @return void
     *
     * @ingroup apiRTB_API
     */
    void usr_rtb_reset_conf(usr_rtb_reset_conf_t *urrc);
#endif  /* #ifndef RTB_WITHOUT_MAC */


    /**
     * Callback function that must be implemented by the application (NHLE)
     * for the RTB service RTB-SET.confirm.
     *
     * @param ursc  Pointer to usr_rtb_set_conf_t result structure.
     *
     * @return void
     *
     * @ingroup apiRTB_API
     */
    void usr_rtb_set_conf(usr_rtb_set_conf_t *ursc);


    /**
     * Callback function that must be implemented by the application (NHLE)
     * for the RTB service RTB-RANGE.confirm.
     *
     * @param urrc Pointer to usr_rtb_range_conf_t result structure.
     *
     * @return void
     *
     * @ingroup apiRTB_API
     */
    void usr_rtb_range_conf(usr_rtb_range_conf_t *urrc);


    /**
     * Callback function that must be implemented by the application (NHLE)
     * for the RTB service RTB-PMU-VALIDITY.indication.
     *
     * @param urpv Pointer to usr_rtb_pmu_validity_ind_t parameter structure.
     *
     * @return void
     *
     * @ingroup apiRTB_API
     */
    void usr_rtb_pmu_validity_ind(usr_rtb_pmu_validity_ind_t *urpv);


#ifdef RTB_WITHOUT_MAC
    /**
     * @brief The stack initialization function in case RTB is highest stack layer.
     *
     * This function initializes all resources, which are used from the stack.
     * It has to be called before any other function of the stack is called.
     */
    retval_t wpan_init(void);


    /**
     * @brief The stack task function called by the application
     *        in case RTB is highest stack layer.
     *
     * This function should be called as frequently as possible by the application
     * in order to provide a permanent execution of the protocol stack.
     *
     * @return Boolean true if an event was processed otherwise false.
     */
    bool wpan_task(void);
#endif  /* #ifdef RTB_WITHOUT_MAC */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTB_API_H */
/* EOF */
