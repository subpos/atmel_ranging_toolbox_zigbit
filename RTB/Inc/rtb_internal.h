/**
 * @file rtb_internal.h
 *
 * @brief RTB internal functions
 *
 * $Id: rtb_internal.h 34343 2013-02-22 11:45:08Z sschneid $
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
#ifndef RTB_INTERNAL_H
#define RTB_INTERNAL_H

/* === Includes ============================================================= */

#include "qmm.h"
#include "tal.h"
#ifdef RTB_WITHOUT_MAC
#include "mac_data_structures.h"
#else   /* #ifdef RTB_WITHOUT_MAC */
#include "mac_internal.h"
#endif  /* #ifdef RTB_WITHOUT_MAC */
#if defined(SIO_HUB) && defined(ENABLE_RTB_PRINT)
#include "sio_handler.h"
#endif  /* if defined(SIO_HUB) && defined(ENABLE_RTB_PRINT) */
#include "rtb_platform.h"
#include "rtb_pmu.h"
#include "rtb_api.h"

/* === Macros =============================================================== */

/**
 * Length of RTB frame identifier, which forms the first octets of any
 * ranging related frame (i.e. first octets of data frame payload).
 */
#define RTB_FRAME_ID_LEN                (3)
#define RTB_FRAME_ID_1                  ('R')   /**< RTB frame id octet 1 */
#define RTB_FRAME_ID_2                  ('T')   /**< RTB frame id octet 2 */
#define RTB_FRAME_ID_3                  ('B')   /**< RTB frame id octet 3 */

/**
 * RTB Protocol Version Id.
 * This is required to be able to differentiate between various versions of
 * the utilized Ranging Protocol and the corresponding capabilites,
 * limitations, etc.
 */
#define RTB_PROTOCOL_VERSION_01         (0x01)


/**
 * Data Payload length of the ranging command frames including RTB frame id
 * octets and RTB CMD-Id octet.
 */
#define CMD_RANGE_REQ_LEN               (RTB_FRAME_ID_LEN + 11)         /**< Length of Range Request Frame */
#define CMD_RANGE_ACPT_LEN              (RTB_FRAME_ID_LEN + 5)          /**< Length of Range Accept Frame */
#define CMD_PMU_SYNC_REQ_LEN            (RTB_FRAME_ID_LEN + 2)          /**< Length of PMU Time Sync Frame */
#define CMD_PMU_START_LEN               (RTB_FRAME_ID_LEN + 1)          /**< Length of PMU Start Frame */
#define CMD_RESULT_REQ_LEN              (RTB_FRAME_ID_LEN + 5)          /**< Length of Result Request Frame */
#define CMD_RESULT_CONF_LEN             (RTB_FRAME_ID_LEN + 5)          /**< Length of Result Confirm Frame */
#if defined(ENABLE_RTB_REMOTE) ||  defined(DOXYGEN)
#   define CMD_REMOTE_RANGE_REQ_LEN     (CMD_RANGE_REQ_LEN + 1 + 2 + 2) /**< Length of remote Range Request Frame */
// 1 octet Reflector AddrMode
// 2 octets Reflector PAN-Id
// 2 octets Reflector Address in case of short address
#   define CMD_REMOTE_RANGE_CONF_LEN    (RTB_FRAME_ID_LEN + 1 + 5 + 2 + 5 + 1)  /**< Length of Remote Range Confirm Frame */
// 1 octet RTB CMD-Id
// 5 octets Reflector Address Spec
//  (i.e. 1 octet Reflector AddrMode,
//        2 octets Reflector PAN-Id,
//        2 octets Reflector Address in case of short address)
// 2 octets Range Acceptance Status and Range Reject Reason
// 5 octets Ranging Distance and DQF
// 1 octet Additional Result IE
#endif  /* #if defined(ENABLE_RTB_REMOTE) ||  defined(DOXYGEN) */

#if (ANTENNA_DIVERSITY == 1)
/*
 * Always initially request antenna diversity from Reflector,
 * even if we currently are not using antenna diversity ourselves.
 */
#   define SET_INITIATOR_CAPS(x)    {x = \
                                             (rtb_pib.EnableAntennaDiv << BIT_POS_INITIATOR_ANT) | \
                                             (1 << BIT_POS_REFLECTOR_ANT); \
}
#else   /* ANTENNA_DIVERSITY */
#   define SET_INITIATOR_CAPS(x)
#endif  /* (ANTENNA_DIVERSITY == 1) */

/** Requested Ranging Transmit Power IE identifier */
#define REQ_RANGING_TX_POWER_IE         (0x01)

/** Waiting time for expected next RTB frame */
#define RTB_AWAIT_FRAME_TIME            (TAL_CONVERT_SYMBOLS_TO_US(macResponseWaitTime_def))

/* === Types ================================================================ */

/**
 * RTB role type.
 */
typedef enum rtb_role_tag
{
    RTB_ROLE_NONE = 0,      /**< Node is currently not involved in ranging. */
    RTB_ROLE_INITIATOR,     /**< Node is currently acting as Initiator in ranging. */
    RTB_ROLE_REFLECTOR      /**< Node is currently acting as Reflector in ranging. */
#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
    ,
    RTB_ROLE_COORDINATOR        /**< Node is currently acting as Coordinator in ranging. */
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */
} SHORTENUM rtb_role_t;



/**
 * RTB state
 */
typedef enum rtb_state_tag
{
    /** Node is not performing any ranging tasks. */
    RTB_IDLE,


    /* States for Initiator */

    /** Initiate transmission of Range Request frame. */
    RTB_INIT_RANGE_REQ_FRAME,

    /** Range Request frame transmitted. */
    RTB_RANGE_REQ_FRAME_DONE,

    /** Await reception of Range Accept frame. */
    RTB_AWAIT_RANGE_ACPT_FRAME,

    /** Initiate transmission of Time Sync Request frame. */
    RTB_INIT_TIME_SYNC_REQ_FRAME,

    /** Time Sync Request frame transmitted. */
    RTB_TIME_SYNC_REQ_FRAME_DONE,

    /** Await reception of PMU Measurement start frame. */
    RTB_AWAIT_PMU_START_FRAME,

    /** Initiate transmission of Result Request frame. */
    RTB_INIT_RESULT_REQ_FRAME,

    /** Range Result frame transmitted. */
    RTB_RESULT_REQ_FRAME_DONE,

    /** Await reception of Result Confirm frame. */
    RTB_AWAIT_RESULT_CONF_FRAME,

    /** Start result computation at Initiator. */
    RTB_RESULT_CALC,

#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
    /** Initiate transmission of Remote Range Confirm frame. */
    /* Only required if remote ranging is allowed. */
    RTB_INIT_REMOTE_RANGE_CONF_FRAME,

    /** Remote Range Confirm frame transmitted. */
    /* Only required if remote ranging is allowed. */
    RTB_REMOTE_RANGE_CONF_FRAME_DONE,
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */



    /* States for Reflector */

    /** Initiate transmission of Range Accept frame. */
    RTB_INIT_RANGE_ACPT_FRAME,

    /** Range Accept frame transmitted. */
    RTB_RANGE_ACPT_FRAME_DONE,

    /** Await reception of Time Sync Request frame. */
    RTB_AWAIT_TIME_SYNC_REQ_FRAME,

    /** Initiate transmission of PMU Measurement start frame. */
    RTB_INIT_PMU_START_FRAME,

    /** RTB Measurement start frame transmitted. */
    RTB_PMU_START_FRAME_DONE,

    /** Await reception of Result Request frame. */
    RTB_AWAIT_RESULT_REQ_FRAME,

    /** Initiate transmission of Range Confirm frame. */
    RTB_INIT_RESULT_CONF_FRAME,

    /** Range Result frame transmitted. */
    RTB_RESULT_CONF_FRAME_DONE,



    /* States for Initiator AND Reflector */

    /** Initialization of the PMU measurement procedure is executed.  */
    RTB_INITIALIZE_PMU,

    /** Actual PMU measurement is executed. */
    RTB_PMU_MEASUREMENT,

    /** Initial preparation for result exchange. */
    RTB_PREPARE_RESULT_EXCHANGE
#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
    ,

    /* States for Coordinator */

    /** Remote Range Request frame transmitted. */
    RTB_REMOTE_RANGE_REQ_FRAME_DONE
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */
} SHORTENUM rtb_state_t;



/* === Frame structures ===*/

/** Enumeration for RTB frames */
typedef enum range_cmd_tag
{
    /** Command code of Range Request frame */
    CMD_RANGE_REQ = 0x01,

    /** Command code of Range Accept frame */
    CMD_RANGE_ACPT = 0x02,

    /** Command code of Time Sync Request frame */
    CMD_PMU_TIME_SYNC_REQ = 0x11,

    /** Command code of PMU Start frame */
    CMD_PMU_START = 0x12,

    /** Command code of Result Request frame */
    CMD_RESULT_REQ = 0x21,

    /** Command code of Result Confirm frame */
    CMD_RESULT_CONF = 0x22

#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
                      ,

    /** Command code of Remote Range Request frame */
    CMD_REMOTE_RANGE_REQ = 0x31,

    /** Command code of Remote Range Confirm frame */
    CMD_REMOTE_RANGE_CONF = 0x32
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */
} SHORTENUM range_cmd_t;


/* === Frame format === */

/** Reflector address type */
typedef struct refl_addr_tag
{
    /** Ranging Reflector address */
    uint8_t refl_addr_mode;
    uint16_t refl_pan_id;
    address_field_t refl_addr;
} refl_addr_t;


#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
/**
 * Parameters for Provisioning of Antenna Diversity Results as
 * Additional Result Fields within the Remote Range Confirm Frame.
 */
typedef struct prov_antenna_div_results_tag
{
    uint8_t no_of_provided_meas_pairs;
    measurement_pair_t provided_meas_pairs[];
} prov_antenna_div_results_t;
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */

#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
typedef union
{
    prov_antenna_div_results_t prov_antenna_div_results;
} additional_result_fields_t;
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */

#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
/** Parameters for Remote Range Confirm to be returned to the Coordinator. */
typedef struct range_remote_answer_tag
{
    /** Status of remote range request */
    uint8_t status;
    /** Reject reason of range request in case ranging is rejected */
    uint8_t range_reject_reason;
    uint32_t distance_cm;
    uint8_t dqf;
    additional_result_ie_t additional_result_ie;
    additional_result_fields_t additional_result_fields;
} range_remote_answer_t;
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */



/** Range error values. */
typedef enum range_error_t
{
    RANGE_OK = 0,                                                       /**< No range error. */
    NO_SYNC,                                                            /**< Range node synchronization failed. */
    TMO_RTB_AWAIT_RANGE_ACPT_FRAME = RTB_AWAIT_RANGE_ACPT_FRAME,        /**< Range Accept frame not received. */
    TMO_RTB_AWAIT_TIME_SYNC_REQ_FRAME = RTB_AWAIT_TIME_SYNC_REQ_FRAME,  /**< Time Sync Request frame not received. */
    TMO_RTB_AWAIT_PMU_START_FRAME = RTB_AWAIT_PMU_START_FRAME,          /**< PMU Start frame not received. */
    TMO_RTB_INIT_PMU_START_FRAME = RTB_INIT_PMU_START_FRAME,            /**< PMU Measurement not finshed. */
    TMO_RTB_AWAIT_RESULT_CONF_FRAME = RTB_AWAIT_RESULT_CONF_FRAME,      /**< Range Result Confirm frame not received. */
    TMO_RTB_AWAIT_RESULT_REQ_FRAME = RTB_AWAIT_RESULT_REQ_FRAME         /**< Range Result Request frame not received. */
} range_error_t;



/** General ranging data structure for parameter storage. */
typedef struct range_param_tag
{
    /** Address spec of Reflector */
    wpan_addr_spec_t ReflectorAddrSpec;

    /** Address spec of Initiator */
    wpan_addr_spec_t InitiatorAddrSpec;

#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
    /** Address spec of Coordinator */
    wpan_addr_spec_t CoordinatorAddrSpec;
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */

    /** Ranging Method used for current ranging procedure */
    uint8_t method;

    /**
     * Requested value of transmit power using IEEE defined format
     * of phyTransmitPower.
     */
    uint8_t req_tx_power;

    /** Transceiver capabilities negotiated between Initiator and Reflector */
    uint8_t caps;

#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
    /** Remote range capabilities requested by Coordinator for Initiator */
    uint8_t remote_caps;
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */
} range_param_t;



/** PMU specific data structure for parameter storage. */
typedef struct range_param_pmu_tag
{
    /**
     * Measurement start index in frequency table
     * for current ranging procedure.
     */
    int16_t f_start;

    /**
     * Measurement step size in frequency table
     * for current ranging procedure.
     */
    uint8_t f_step;

    /**
     * Measurement stop index in frequency table
     * for current ranging procedure.
     */
    int16_t f_stop;

    /**
     * Status of applying minimum threshold during
     * weighted distance calculation.
     */
    bool apply_min_dist_threshold;

    /**
     * Number of antennas measurements used depending on utilized atenna
     * diversity scheme:
     <pre>
     * +--------------------------+-----------------------+--------------------------+
     * | antenna_measurement_nos  | Initiator Antenna Div | Initiator no Antenna Div |
     * +--------------------------+-----------------------+--------------------------+
     * | Reflector Antenna Div    |           4           |            2             |
     * +--------------------------+-----------------------+--------------------------+
     * | Reflector no Antenna Div |           2           |            1             |
     * +--------------------------+-----------------------+--------------------------+
     </pre>
     */
    uint8_t antenna_measurement_nos;

#if (ANTENNA_DIVERSITY == 1) || defined(DOXYGEN)
    /**
     * Array indicating the antenna to be used by this node for the measurement
     * indicated by be index.
     *
     * This works as follows:
     *
     * 1)
     * In case both the Initiator and the Reflector do NOT use antenna
     * diversity, the antenna array contains only one entry which is based
     * on the default antenna of this node.
     *
     *
     * 2)
     * In case either the Initiator or the Reflector use antenna diversity
     * (but not both nodes), the antenna array contains two entries.
     *
     * If this node uses antenna diversity (no matter if this node is the
     * Initiator or the Reflector), the antenna array is filled as
     * index = 0: Use antenna 0
     * index = 1: Use antenna 1
     *
     * If this node does NOT use antenna diverstiy, the antenna array is filled
     * using the default antenna of this node.
     * index = 0 & 1: Use default antenna of this node
     *
     *
     * 3)
     * In case both the Initiator and the Reflector use antenna diversity,
     * the antenna array contains four entries.
     *
     <pre>
     * +---------------+---------------+---------------+
     * | Index of      | Value of      | Value of      |
     * | antenna_array | antenna_array | antenna_array |
     * |               | at Initiator  | at Reflector  |
     * +---------------+---------------+---------------+
     * | 0             |       0       |       0       |
     * +---------------+---------------+---------------+
     * | 1             |       0       |       1       |
     * +---------------+---------------+---------------+
     * | 2             |       1       |       1       |
     * +---------------+---------------+---------------+
     * | 3             |       1       |       0       |
     * +---------------+---------------+---------------+
     </pre>
     */
    uint8_t antenna_array[PMU_MAX_NO_ANTENNAS];
#endif  /* (ANTENNA_DIVERSITY == 1)  || defined(DOXYGEN) */
} range_param_pmu_t;



/** PMU measurement status structure */
typedef struct range_status_tag
{
    /* Calculation results */
    /** Measured distance in cm */
    uint32_t distance_cm;
    /** Distance quality factor in % */
    uint8_t dqf;

    /** Current error status */
    range_error_t range_error;
} range_status_t;




/** PMU measurement status structure */
typedef struct range_status_pmu_tag
{
    /** Result exchange ongoing for current antenna measurement value. */
    uint8_t curr_antenna_measurement_no;

    /** Array for measured distance in cm. */
    uint32_t measured_distance_cm[PMU_MAX_NO_ANTENNAS];
    /** Array for measured DQF in %. */
    uint8_t measured_dqf[PMU_MAX_NO_ANTENNAS];
} range_status_pmu_t;

/** RTB dispatch handler type */
typedef void (*handler_rtb_t)(uint8_t *);

/** Typedef for Range-Confirm generation in error case. */
typedef enum conf_on_error_tag
{
    /**
     * No confirm generation in error case
     * (i.e. node is Reflector or Initiator in remote ranging).
     */
    NO_CONF = 0,
    /**
     * Generate regular Range-Confirm (i.e. node is Initiator
     * in Local Ranging).
     */
    LOCAL_CONF = 1
#if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN)
                 ,
    /**
     * Generate Remote-Range-Confirm (i.e. node is Gateyway
     * in Remote Ranging). */
    REMOTE_CONF = 2
#endif  /* #if defined(ENABLE_RTB_REMOTE) || defined(DOXYGEN) */
} SHORTENUM conf_on_error_t;

/* === Externals ============================================================ */

/* Global data variables */
extern rtb_state_t last_rtb_state;
extern range_param_t range_param;
extern range_param_pmu_t range_param_pmu;
extern range_status_t range_status;
extern range_status_pmu_t range_status_pmu;
extern int8_t rtb_dist_offset;
extern rtb_pib_t rtb_pib;
extern result_frame_ie_t req_result_type;
extern rtb_role_t rtb_role;
extern queue_t tal_rtb_q;
extern rtb_state_t rtb_state;
extern uint8_t rtb_static_frame_buffer[];
extern bool rtb_tx_in_progress;
#ifdef RTB_WITHOUT_MAC
extern parse_t mac_parse_data;
#else   /* #ifdef RTB_WITHOUT_MAC */
extern mac_pib_t mac_pib;
#endif  /* #ifdef RTB_WITHOUT_MAC */
extern uint8_t orig_tal_transmit_power;
extern volatile bool timer_is_synced;

/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

    void configure_ranging(void);
    void handle_range_frame_error(uint8_t error);

    bool pmu_check_pmu_params(void);
    void pmu_extract_no_of_req_result_values(uint16_t received_value_cnt);
    void pmu_fill_initial_start_addr(uint8_t *ptr_to_frame);
    void pmu_fill_result_data(uint16_t no_of_values,
                              uint8_t *ptr_to_frame);
    uint16_t pmu_get_no_of_results_to_be_sent(void);
    void pmu_handle_received_pmu_values(uint8_t *curr_frame_ptr);
    bool pmu_more_results_to_be_expected(void);
    bool pmu_no_more_pmu_data_available(void);
    void pmu_set_pmu_result_idx_done(void);
#ifndef RTB_WITHOUT_MAC
    void pmu_result_presentation(void);
#endif  /* #ifndef RTB_WITHOUT_MAC */
    void pmu_reset_fec_vars(void);
    void pmu_reset_pmu_result_vars(void);
    bool pmu_update_result_ptr(void);

    void range_assemble_and_tx_frame_csma(frame_msgtype_t msgtype,
                                          range_cmd_t cmd_type,
                                          rtb_state_t next_rtb_state,
                                          conf_on_error_t generate_range_conf_on_error);
    void range_build_frame(range_cmd_t cmd, frame_info_t *frame);
    void range_calc_aver_pmu4(uint8_t *p_pmu,
                              int freq_N,
                              int fec,
                              uint8_t *pmu_avg);
    void range_exit(void);
#ifdef ENABLE_RTB_REMOTE
    void range_gen_rtb_remote_range_conf(uint8_t status,
                                         uint32_t distance,
                                         uint8_t dqf,
                                         uint8_t no_of_provided_meas_pairs,
                                         measurement_pair_t *provided_meas_pairs);
#endif  /* ENABLE_RTB_REMOTE */
    void range_gen_rtb_range_conf(uint8_t status,
                                  uint32_t distance,
                                  uint8_t dqf);
    void range_process_tal_tx_status(retval_t tx_status,  frame_info_t *frame);
    void range_result_presentation(void);
    void range_start_await_timer(rtb_state_t current_state);
    void range_stop_await_timer(void);
    void range_t_await_frame_cb(void *callback_parameter);
    void range_tx_range_accept_frame(void);
#ifdef ENABLE_RTB_REMOTE
    void range_tx_remote_range_conf_frame(void);
#endif  /* ENABLE_RTB_REMOTE */
    void range_tx_result_conf_frame(void);
    void range_tx_result_req_frame(void);
    void reset_pmu_average_data(void);

    void rtb_exit_rx_tx_end_irq(void);
    void rtb_sync_handler_cb(void);
    void rtb_init_rx_end_irq(void);
    void rtb_init_tx_end_irq(void);
    void rtb_tstamp_irq_init(void);
    void rtb_tstamp_irq_exit(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTB_INTERNAL_H */
/* EOF */
