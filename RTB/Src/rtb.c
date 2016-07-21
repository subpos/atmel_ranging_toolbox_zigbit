/**
 * @file rtb.c
 *
 * @brief Ranging Toolbox
 *
 * This file implements main functionality of the Ranging Toolbox.
 *
 * $Id: rtb.c 34343 2013-02-22 11:45:08Z sschneid $
 *
 */
/**
 * @author
 *      Atmel Corporation: http://www.atmel.com
 *      Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2011, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

#ifdef ENABLE_RTB

/* === Includes ============================================================ */

#include "tal.h"
#include "ieee_const.h"
#include "rtb.h"
#include "rtb_msg_types.h"
#include "rtb_internal.h"
#ifdef ENABLE_RP
#   include "rp_api.h"
#endif  /* #ifdef ENABLE_RP */

/* === Macros ============================================================== */


/* === Types =============================================================== */


/* === Globals ============================================================= */

/**
 * Array of pointers defining the start of the averaged PMU values per
 * antenna measurement pair.
 */
/* DO NOT CHANGE THIS */
pmu_avg_data_t pmu_avg_data;

/**
 * Current role of node in ranging procedure
 */
rtb_role_t rtb_role = RTB_ROLE_NONE;

/**
 * Current state of the RTB state machine.
 */
rtb_state_t rtb_state = RTB_IDLE;

/**
 * Last state of the RTB when a timer has been started to have
 * knowledge about RTB state once the is expired.
 */
rtb_state_t last_rtb_state;

/** Parameter variable, it holds all general ranging parameters. */
range_param_t range_param;

/** PMU Parameter variable, it holds all PMU related ranging parameters. */
range_param_pmu_t range_param_pmu;

/** Status variable, it holds all general measurement data. */
range_status_t range_status;

/** Status variable, it holds all PMU related measurement data. */
range_status_pmu_t range_status_pmu;

/** Status variable hold current requested or confirmed result data type. */
result_frame_ie_t req_result_type;

/**
 * Queue used by RTB for its internal operation. TAL pushes the incoming frames
 * in this queue.
 */
queue_t tal_rtb_q;

#ifdef RTB_WITHOUT_MAC
/**
 * NHLE to RTB queue in which NHLE pushes all the requests to the RTB layer
 * in case the RTB is the highest stack layer
 * (i.e. MAC layer is not available).
 */
queue_t nhle_rtb_q;

/**
 * RTB parse data
 */
parse_t mac_parse_data;
#endif  /* #ifdef RTB_WITHOUT_MAC */

/*
 * Static buffer pointer for later utilization for Range confirm message.
 * The buffer is stored to always ensure a valid buffer for
 * Range confirm, no matter what is going on, to avoid hanging of
 * ongoing ranging procedure in case buffers are out.
 * By storing this buffer explicitly for range confirm,
 * range procedure can always be finished somehow.
 */
buffer_t *range_confirm_msg_ptr = NULL;

/*
 * Static buffer used for ranging frame transmission.
 */
uint8_t rtb_static_frame_buffer[LARGE_BUFFER_SIZE];

/*
 * Distance offset:
 * Usually this parameter is a board specific define, but this predefined value
 * can be overwritten in order to fine tune this during development.
 */
int8_t rtb_dist_offset = DISTANCE_OFFSET;

/*
 * Original TAL Transmit Power for later restoring.
 * This is required while applying the RTB Ranging Transmit Power at the
 * receiver during the PMU measurement.
 */
uint8_t orig_tal_transmit_power;

/** IRQ flags for time synchronization */
volatile bool timer_is_synced = false;

/*
 * Status variable  indicating whether an RTB based frame transmission
 * is currently going on. This is required in order to prevent further
 * frame initiations while the previous transmission has not finished yet.
 */
bool rtb_tx_in_progress = false;

/* === Prototypes ========================================================== */

static void range_prepare_result_exchange(void);
static void range_result_calculation(void);
static void range_start_initiator(void);
#ifdef ENABLE_RTB_REMOTE
static void range_start_remote(uint16_t coordinator_addr_mode);
#endif  /* ENABLE_RTB_REMOTE */
static void store_range_req_parameter(wpan_rtb_range_req_t *wrrr);

/* === Implementation ====================================================== */

/**
 * @brief RTB Initialization
 *
 * This function initializes the RTB.
 */
retval_t rtb_init(void)
{
    rtb_role = RTB_ROLE_NONE;

    /* Initialize the TAL to RTB queue */
    qmm_queue_init(&tal_rtb_q);

    /*
     * Reset the PMU average data since no valid PMU average data are available
     * at this stage.
     */
    reset_pmu_average_data();

#ifdef ENABLE_RTB_REMOTE
    /*
     * No remote ranging ongoing:
     * In case the Coordinator address mode is zero, it is indicated that
     * no remote ranging is ongoing.
     */
    range_param.CoordinatorAddrSpec.AddrMode = 0;
#endif  /* ENABLE_RTB_REMOTE */

    /* General PIB attribute default values */
    rtb_pib.RangingEnabled = true;      // Ranging is enabled.
    rtb_pib.DefaultAntenna = false;     // Use antenna 0 as default.
#if (ANTENNA_DIVERSITY == 1)
    rtb_pib.EnableAntennaDiv = true;    // Enable antenna diversity as default.
#else
    rtb_pib.EnableAntennaDiv = false;   // Disable antenna diversity as default.
#endif

#ifndef RTB_WITHOUT_MAC
    /*
     * Disable provisioning of antenna diversity measurement results,
     * if any node uses antenna diversity.
     */
    rtb_pib.ProvideAntennaDivResults = false;

    rtb_pib.RangingMethod = RTB_TYPE;
#endif  /* #ifndef RTB_WITHOUT_MAC */

    /* PMU specific PIB attribute default values */
    rtb_pib.PMUFreqStart = PMU_START_FREQ_DEFAULT;
    rtb_pib.PMUFreqStep = PMU_STEP_FREQ_DEFAULT;
    rtb_pib.PMUFreqStop = PMU_STOP_FREQ_DEFAULT;

#ifndef RTB_WITHOUT_MAC
    rtb_pib.PMUVerboseLevel = 0;
#endif  /* #ifndef RTB_WITHOUT_MAC */

    /*
     * Default Ranging Transmit Power is set to lowest possible value.
     */
    rtb_pib.RangingTransmitPower = RTB_TRANSMIT_POWER_DEFAULT;
    /*
     * Provisioning of own Ranging Transmit Power to other nodes
     * during Ranging is enabled.
     */
    rtb_pib.ProvideRangingTransmitPower = true;

    /*
     * Application of minimum threshold during weighted distance calculation
     * is enabled.
     */
    rtb_pib.ApplyMinDistThreshold = true;

#if (defined RTB_WITHOUT_MAC) && !defined(ENABLE_RP)
    /* Enable receiver to allow for frame reception. */
    /*
     * This shall NOT be done for an RP system, since access to the TRX
     * is not available at this stage.
     */
    /*
     * This is done since in case the RTB is the highest stack layer,
     * there is no dedicated interface to enable the receiver.
     * Therefore this is done implicitly.
     */
    tal_rx_enable(PHY_RX_ON);
#endif  /* #ifdef RTB_WITHOUT_MAC */

    return RTB_SUCCESS;
}



/**
 * State machine of the RTB ranging procedure.
 *
 * This function implements the basic RTB state machine.
 * It is called periodically from wpan_task() (in case the app
 * is residing on the MAC layer) or directly from the main loop of the app
 * (in case the APP is residing on the TAL layer).
 */
void rtb_task(void)
{
    uint8_t *event = NULL;

#ifdef RTB_WITHOUT_MAC
    if (RTB_IDLE == rtb_state)
    {
        /* Check whether queue is empty */
        if (nhle_rtb_q.size != 0)
        {
            event = (uint8_t *)qmm_queue_remove(&nhle_rtb_q, NULL);

            /* If an event has been detected, handle it. */
            if (NULL != event)
            {
                /* Process event due to NHLE requests */
                dispatch_rtb_event(event);
            }
        }
    }
#endif  /* #ifdef RTB_WITHOUT_MAC */

    /*
     * Internal RTB event queue should be dispatched
     * irrespective of the dispatcher state.
     */
    /* Check whether queue is empty */
    if (tal_rtb_q.size != 0)
    {
        event = (uint8_t *)qmm_queue_remove(&tal_rtb_q, NULL);

        /* If an event has been detected, handle it. */
        if (NULL != event)
        {
            dispatch_rtb_event(event);
        }
    }

    /*
     * The RTB shall only handle its state machine if no other RTB initiated
     * frame transmission is ongoing, otherwise we may run into serious issues
     * caused by trying to transmit RTB frames while the TAL is still in
     * tx mode.
     */
    if (!rtb_tx_in_progress)
    {
        switch (rtb_state)
        {
            case RTB_INIT_RANGE_REQ_FRAME:
                /*
                 * This state can be reached by:
                 * 1) reception of a regular range request, or
                 * 2) reception of a remote range request frame.
                 */
                range_start_initiator();
                break;

            case RTB_INIT_RANGE_ACPT_FRAME:
                /* State occurs at Reflector. */
                range_tx_range_accept_frame();
                break;

            case RTB_INIT_TIME_SYNC_REQ_FRAME:
                /* State occurs at Initiator. */
                pmu_tx_pmu_time_sync_frame();
                break;

            case RTB_INIT_PMU_START_FRAME:
                /* State occurs at Reflector. */
                pmu_perform_pmu_measurement();
                break;

            case RTB_PREPARE_RESULT_EXCHANGE:
                range_prepare_result_exchange();
                break;

            case RTB_INIT_RESULT_REQ_FRAME:
                range_tx_result_req_frame();
                break;

            case RTB_INIT_RESULT_CONF_FRAME:
                range_tx_result_conf_frame();
                break;

            case RTB_RESULT_CALC:
                range_result_calculation();
                range_result_presentation();
                break;

#ifdef ENABLE_RTB_REMOTE
            case RTB_INIT_REMOTE_RANGE_CONF_FRAME:
                range_tx_remote_range_conf_frame();
                break;
#endif  /* ENABLE_RTB_REMOTE */

            default:
                break;
        }
    }
}



/**
 * @brief Starts a new ranging request
 *
 * @param msg Pointer to the RTB-RANGE.request parameter
 */
void rtb_range_request(uint8_t *msg)
{
    rtb_range_req_t rrr;
    memcpy(&rrr, BMM_BUFFER_POINTER((buffer_t *)msg), sizeof(rtb_range_req_t));

    wpan_rtb_range_req_t *wrrr = &rrr.range_req;

#ifdef ENABLE_RTB_REMOTE
    if (wrrr->CoordinatorAddrMode != FCF_NO_ADDR)
    {
        /*
         * Release current buffer in case of remote ranging.
         */
        bmm_buffer_free((buffer_t *)msg);
    }
    else
#endif  /* ENABLE_RTB_REMOTE */
    {
        /*
         * Store msg pointer for later utilization for Range-Confirm message
         * in local ranging.
         * The message buffer is stored to always ensure a valid buffer for
         * Range confirm, no matter what is going on, to avoid hanging of
         * ongoing ranging procedure in case buffers are out.
         * By storing this message buffer explicitly for range confirm,
         * range procedure can always be finished somehow.
         */
        /* In remote ranging we
         * - might not get an answer back from the Initiator if he is busy,
         * - might have several parallel remote ranging requests ongoing,
         * so this buffer must be released and cannot be re-used for the
         * Remote-Range-Confirm.
         */
        range_confirm_msg_ptr = (buffer_t *)msg;
    }

    if (!rtb_pib.RangingEnabled)
    {
#ifdef ENABLE_RTB_REMOTE
        /* Ranging is currently disabled, reject new request. */
        if (wrrr->CoordinatorAddrMode != FCF_NO_ADDR)
        {
            /* This node is Coordinator in a remote ranging procedure. */
            store_range_req_parameter(wrrr);

            range_gen_rtb_remote_range_conf((uint8_t)RTB_UNSUPPORTED_RANGING,
                                            INVALID_DISTANCE,
                                            DQF_ZERO,
                                            0,
                                            NULL);
        }
        else
#endif  /* ENABLE_RTB_REMOTE */
        {
            /* This node is Initiator in regular ranging procedure. */
            range_gen_rtb_range_conf((uint8_t)RTB_UNSUPPORTED_RANGING,
                                     INVALID_DISTANCE,
                                     DQF_ZERO);
        }
        return;
    }

    if (RTB_ROLE_NONE != rtb_role)
    {
        ASSERT("(Local) Ranging procedure already ongoing" == 0);

        /* Ranging procedure in progress, reject new request. */
        range_gen_rtb_range_conf((uint8_t)RTB_RANGING_IN_PROGRESS,
                                 INVALID_DISTANCE,
                                 DQF_ZERO);

        return;
    }

#ifdef ENABLE_RTB_REMOTE
    if (wrrr->CoordinatorAddrMode != FCF_NO_ADDR)
    {
        /* This node is Coordinator in a remote ranging procedure. */

        /*
         * Check whether the Coordinator was requested to use its short address,
         * but does currently not have a valid short address.
         */
        if (wrrr->CoordinatorAddrMode == FCF_SHORT_ADDR)
        {
            if (((BROADCAST) == tal_pib.ShortAddress) ||
                ((MAC_NO_SHORT_ADDR_VALUE) == tal_pib.ShortAddress))
            {
                /*
                 * Invalid Coordinator short address. Reject remote range request.
                 */
                store_range_req_parameter(wrrr);

                range_gen_rtb_remote_range_conf((uint8_t)MAC_NO_SHORT_ADDRESS,
                                                INVALID_DISTANCE,
                                                DQF_ZERO,
                                                0,
                                                NULL);
                return;
            }
        }

        /*
         * First make MAC busy, to prevent further tasks from MAC beeing done,
         * until this ranging procedure is finished.
         */
        MAKE_MAC_BUSY();

        store_range_req_parameter(wrrr);

        /* Remote range request. */
        range_start_remote(wrrr->CoordinatorAddrMode);
    }
    else if ((wrrr->InitiatorPANId == tal_pib.PANId) &&
             (((wrrr->InitiatorAddrMode == FCF_SHORT_ADDR) &&
               (wrrr->InitiatorAddr == tal_pib.ShortAddress)) ||
              ((wrrr->InitiatorAddrMode == FCF_LONG_ADDR) &&
               (wrrr->InitiatorAddr == tal_pib.IeeeAddress))))
    {
        /* This node is Initiator. */

        /*
         * First make MAC busy, to prevent further tasks from MAC beeing done,
         * until this ranging procedure is finished.
         */
        MAKE_MAC_BUSY();

        store_range_req_parameter(wrrr);

        /* Start a regular ranging procedure. */
        range_status.range_error = RANGE_OK;
        rtb_state = RTB_INIT_RANGE_REQ_FRAME;
    }
    else
    {
        /*
         * Invalid addresses received. Reject range request.
         */
        range_gen_rtb_range_conf((uint8_t)RTB_INVALID_PARAMETER,
                                 INVALID_DISTANCE,
                                 DQF_ZERO);
        return;
    }
#else   /* ENABLE_RTB_REMOTE */
    /*
     * This is the code without remote ranging,
     * i.e. there exist only local ranging.
     */
    /* This node is Initiator. */
    if ((wrrr->InitiatorAddrMode == FCF_SHORT_ADDR) &&
        ((BROADCAST == tal_pib.ShortAddress) || (MAC_NO_SHORT_ADDR_VALUE == tal_pib.ShortAddress)))
    {
        /*
         * Invalid addresses received. Reject range request.
         */
        range_gen_rtb_range_conf((uint8_t)RTB_INVALID_PARAMETER,
                                 INVALID_DISTANCE, DQF_ZERO);
        return;
    }
    else
    {
#ifndef RTB_WITHOUT_MAC
        /*
         * First make MAC busy, to prevent further tasks from MAC beeing done,
         * until this ranging procedure is finished.
         */
        MAKE_MAC_BUSY();
#endif  /* #ifndef RTB_WITHOUT_MAC */

        store_range_req_parameter(wrrr);

        /* Start a regular ranging procedure. */
        range_status.range_error = RANGE_OK;
        rtb_state = RTB_INIT_RANGE_REQ_FRAME;
    }
#endif  /* ENABLE_RTB_REMOTE */
}



/* Helper function to store the requested ranging address parameter. */
static void store_range_req_parameter(wpan_rtb_range_req_t *wrrr)
{
    /* Set proper ranging address parameter. */
    /* Set local address as Initiator address. */
    if (wrrr->InitiatorAddrMode == FCF_SHORT_ADDR)
    {
        range_param.InitiatorAddrSpec.AddrMode = FCF_SHORT_ADDR;
#ifndef ENABLE_RTB_REMOTE
        range_param.InitiatorAddrSpec.Addr.short_address = tal_pib.ShortAddress;
#endif  /* ifndef ENABLE_RTB_REMOTE */
    }
    else
    {
        range_param.InitiatorAddrSpec.AddrMode = FCF_LONG_ADDR;
#ifndef ENABLE_RTB_REMOTE
        range_param.InitiatorAddrSpec.Addr.long_address = tal_pib.IeeeAddress;
#endif  /* ifndef ENABLE_RTB_REMOTE */
    }
#ifdef ENABLE_RTB_REMOTE
    ADDR_COPY_DST_SRC_64(range_param.InitiatorAddrSpec.Addr.long_address, wrrr->InitiatorAddr);
    range_param.InitiatorAddrSpec.PANId = wrrr->InitiatorPANId;
#else   /* ENABLE_RTB_REMOTE */
    range_param.InitiatorAddrSpec.PANId = tal_pib.PANId;
#endif  /* ENABLE_RTB_REMOTE */

    /* Set peer address as Reflector address. */
    if (wrrr->ReflectorAddrMode == FCF_SHORT_ADDR)
    {
        range_param.ReflectorAddrSpec.AddrMode = FCF_SHORT_ADDR;
    }
    else
    {
        range_param.ReflectorAddrSpec.AddrMode = FCF_LONG_ADDR;
    }
    ADDR_COPY_DST_SRC_64(range_param.ReflectorAddrSpec.Addr.long_address, wrrr->ReflectorAddr);
    range_param.ReflectorAddrSpec.PANId = wrrr->ReflectorPANId;
}



/*
 * @brief Initiates rtb range confirm message
 *
 * This function creates the RTB range confirm structure,
 * and appends it into internal event queue.
 *
 * @param buf Buffer for RTB range confirmation.
 * @param status Ranging status.
 */
void range_gen_rtb_range_conf(uint8_t status, uint32_t distance, uint8_t dqf)
{
    rtb_range_conf_t *rrc = (rtb_range_conf_t *)BMM_BUFFER_POINTER(range_confirm_msg_ptr);

    rrc->cmdcode = RTB_RANGE_CONFIRM;
    rrc->range_conf.ranging_type = RTB_LOCAL_RANGING;
    rrc->range_conf.results.local.status = status;
    rrc->range_conf.results.local.distance = distance;
    rrc->range_conf.results.local.dqf = dqf;

#ifndef RTB_WITHOUT_MAC
    /* Add the actual measurement pairs consisting of distance and dqf if required. */
    /* This is only allowed in case the MAC layer is included. */
    if ((rtb_pib.ProvideAntennaDivResults) &&
        (range_param_pmu.antenna_measurement_nos > 1))
    {
        /*
         * Further antenna diversity measurement result shall be provided
         * AND there actually are more than one measurement results available
         * (i.e. one of the two nodes uses antenna diversity), so append the
         * available measurement pairs.
         */
        rrc->range_conf.results.local.no_of_provided_meas_pairs =
            range_param_pmu.antenna_measurement_nos;
        for (uint8_t i = 0; i < range_param_pmu.antenna_measurement_nos; i++)
        {
            rrc->range_conf.results.local.provided_meas_pairs[i].distance =
                range_status_pmu.measured_distance_cm[i];
            rrc->range_conf.results.local.provided_meas_pairs[i].dqf =
                range_status_pmu.measured_dqf[i];
        }
    }
    else
    {
        rrc->range_conf.results.local.no_of_provided_meas_pairs = 0;
    }
#endif  /* #ifndef RTB_WITHOUT_MAC */

#ifdef RTB_WITHOUT_MAC
    /* Append the RTB range confirmation message to the RTB-NHLE queue */
    qmm_queue_append(&rtb_nhle_q, range_confirm_msg_ptr);
#else   /* #ifdef RTB_WITHOUT_MAC */
    /* MAC layer is available */
    /* Append the RTB range confirmation message to the MAC-NHLE queue */
    qmm_queue_append(&mac_nhle_q, range_confirm_msg_ptr);
#endif  /* #ifdef RTB_WITHOUT_MAC */
}



#ifdef ENABLE_RTB_REMOTE
/*
 * @brief Initiates RTB remote range confirm message
 *
 * This function creates the RTB remote range confirm structure,
 * and appends it into internal event queue.
 *
 */
void range_gen_rtb_remote_range_conf(uint8_t status,
                                     uint32_t distance,
                                     uint8_t dqf,
                                     uint8_t no_of_provided_meas_pairs,
                                     measurement_pair_t *provided_meas_pairs)
{
    buffer_t *buffer_header = bmm_buffer_alloc(LARGE_BUFFER_SIZE);

    /* Check for buffer availability */
    if (NULL != buffer_header)
    {
        rtb_range_conf_t *rrc = (rtb_range_conf_t *)BMM_BUFFER_POINTER(buffer_header);

        rrc->cmdcode = RTB_RANGE_CONFIRM;
        rrc->range_conf.ranging_type = RTB_REMOTE_RANGING;

        rrc->range_conf.results.remote.InitiatorAddrMode =
            range_param.InitiatorAddrSpec.AddrMode;
        rrc->range_conf.results.remote.InitiatorPANId =
            range_param.InitiatorAddrSpec.PANId;
        ADDR_COPY_DST_SRC_64(rrc->range_conf.results.remote.InitiatorAddr,
                             range_param.InitiatorAddrSpec.Addr.long_address);

        rrc->range_conf.results.remote.ReflectorAddrMode =
            range_param.ReflectorAddrSpec.AddrMode;
        rrc->range_conf.results.remote.ReflectorPANId =
            range_param.ReflectorAddrSpec.PANId;
        ADDR_COPY_DST_SRC_64(rrc->range_conf.results.remote.ReflectorAddr,
                             range_param.ReflectorAddrSpec.Addr.long_address);

        rrc->range_conf.results.remote.status =
            status;
        rrc->range_conf.results.remote.distance =
            distance;
        rrc->range_conf.results.remote.dqf =
            dqf;

        /* Add the actual measurement pairs consisting of distance and dqf if required. */
        if (no_of_provided_meas_pairs)
        {
            /*
             * Further antenna diversity measurement result shall be provided
             * AND there actually are more than one measurement results available
             * (i.e. one of the two nodes uses antenna diversity), so append the
             * available measurement pairs.
             */
            rrc->range_conf.results.remote.no_of_provided_meas_pairs =
                no_of_provided_meas_pairs;
            for (uint8_t i = 0; i < no_of_provided_meas_pairs; i++)
            {
                rrc->range_conf.results.remote.provided_meas_pairs[i].distance =
                    provided_meas_pairs[i].distance;
                rrc->range_conf.results.remote.provided_meas_pairs[i].dqf =
                    provided_meas_pairs[i].dqf;
            }
        }
        else
        {
            rrc->range_conf.results.remote.no_of_provided_meas_pairs = 0;
        }

#ifdef RTB_WITHOUT_MAC
        /* Append the RTB remote range confirmation message to the RTB-NHLE queue */
        qmm_queue_append(&rtb_nhle_q, buffer_header);
#else   /* #ifdef RTB_WITHOUT_MAC */
        /* MAC layer is available */
        /* Append the RTB remote range confirmation message to the MAC-NHLE queue */
        qmm_queue_append(&mac_nhle_q, buffer_header);
#endif  /* #ifdef RTB_WITHOUT_MAC */
    }
}
#endif  /* ENABLE_RTB_REMOTE */



/* This function starts the ranging procedure at the Initiator. */
static void range_start_initiator(void)
{
    rtb_role = RTB_ROLE_INITIATOR;
    range_status.range_error = RANGE_OK;
    range_status.dqf = 0;
    range_status.distance_cm = INVALID_DISTANCE;

    /*
     * Reset the PMU average data since no valid PMU average data are available
     * at this stage.
     */
    reset_pmu_average_data();

    /* Perform PMU type specific initialization of Initiator. */
#if (RTB_TYPE == RTB_PMU_233R)
    /* Prepare FEC measurement */
    pmu_enable_fec_measurement();
#endif  /* (RTB_TYPE == RTB_PMU_233R) */

    /* Send Range Request frame using CSMA/CA. */

    /*
     * This state can be reached by:
     * 1) reception of a regular range request, or
     * 2) reception of a remote range request frame.
     *
     * The confirm message in an error case must only be
     * sent, if this is a regular ranging.
     * In case this is a remote ranging, no error message
     * to its local application shall be generated.
     */

    conf_on_error_t notify_app_if_error;

#ifdef ENABLE_RTB_REMOTE
    if (range_param.CoordinatorAddrSpec.AddrMode != FCF_NO_ADDR)
    {
        /*
         * This is the Initiator during a remote ranging.
         * Initiate a Range Request frame but do NOT notify application
         * in error case.
         */
        notify_app_if_error = NO_CONF;
    }
    else
#endif  /* ENABLE_RTB_REMOTE */
    {
        /*
         * This is the Initiator during a regular ranging.
         * Initiate a Range Request frame AND notify application
         * in error case.
         */
        notify_app_if_error = LOCAL_CONF;
    }
    range_assemble_and_tx_frame_csma(RTB_CMD_RANGE_REQ,           // Internal RTB message type
                                     CMD_RANGE_REQ,               // External RTB command frame type
                                     RTB_RANGE_REQ_FRAME_DONE,    // Next RTB state
                                     /*
                                      * Generate an error message if required,
                                      * since this is an Initiator.
                                      */
                                     notify_app_if_error);
}



#ifdef ENABLE_RTB_REMOTE
static void range_start_remote(uint16_t coordinator_addr_mode)
{
    rtb_role = RTB_ROLE_COORDINATOR;

    range_status.range_error = RANGE_OK;
    range_status.dqf = 0;
    range_status.distance_cm = INVALID_DISTANCE;

    /*
     * Reset the PMU average data since no valid PMU average data are available
     * at this stage.
     */
    reset_pmu_average_data();

    /* Set Coordinator address information. */
    if (coordinator_addr_mode == FCF_SHORT_ADDR)
    {
        range_param.CoordinatorAddrSpec.AddrMode = FCF_SHORT_ADDR;
        range_param.CoordinatorAddrSpec.Addr.long_address = 0;   // Clean up address
        ADDR_COPY_DST_SRC_16(range_param.CoordinatorAddrSpec.Addr.short_address, tal_pib.ShortAddress);
    }
    else
    {
        range_param.CoordinatorAddrSpec.AddrMode = FCF_LONG_ADDR;
        ADDR_COPY_DST_SRC_64(range_param.CoordinatorAddrSpec.Addr.long_address, tal_pib.IeeeAddress);
    }
    range_param.CoordinatorAddrSpec.PANId = tal_pib.PANId;


    /* Send Remote Range Request frame using CSMA/CA. */
    range_assemble_and_tx_frame_csma(RTB_CMD_REMOTE_RANGE_REQ,        // Internal RTB message type
                                     CMD_REMOTE_RANGE_REQ,            // External RTB command frame type
                                     RTB_REMOTE_RANGE_REQ_FRAME_DONE, // Next RTB state
                                     /*
                                      * Generate an error message if required,
                                      * since this is a Coordinator.
                                      */
                                     REMOTE_CONF);
}
#endif  /* ENABLE_RTB_REMOTE */



/**
 * @brief Resets the RTB
 *
 * @param msg Pointer to the RTB-RESET.request parameter
 */
#ifndef RTB_WITHOUT_MAC
void rtb_reset_request(uint8_t *msg)
{
    uint8_t status;

    status = tal_reset(true);

    if (status == MAC_SUCCESS)
    {
        /* TAL could be reset successfully. */
        /* Enable receiver to allow for frame reception. */
        /*
         * This is done since in case the RTB is the highest stack layer,
         * there is no dedicated interface to enable the receiver.
         * Therefore this is done implicitly.
         */
        tal_rx_enable(PHY_RX_ON);

        /* Init RTB. */
        status = rtb_init();
    }


    rtb_reset_conf_t *rrc = (rtb_reset_conf_t *)BMM_BUFFER_POINTER((buffer_t *)msg);

    rrc->cmdcode = RTB_RESET_CONFIRM;
    rrc->reset_conf.status = status;

#ifdef RTB_WITHOUT_MAC
    /* Append the RTB reset confirmation message to the RTB-NHLE queue */
    qmm_queue_append(&rtb_nhle_q, (buffer_t *)msg);
#else   /* #ifdef RTB_WITHOUT_MAC */
    /* MAC layer is available */
    /* Append the RTB reset confirmation message to the MAC-NHLE queue */
    qmm_queue_append(&mac_nhle_q, (buffer_t *)msg);
#endif  /* #ifdef RTB_WITHOUT_MAC */
}
#endif  /* #ifndef RTB_WITHOUT_MAC */



/** Ranging procedure clean-up function */
void range_exit(void)
{
    if ((RTB_ROLE_INITIATOR == rtb_role) || (RTB_ROLE_REFLECTOR == rtb_role))
    {
        /* Restore regular Transmit Power. */
        tal_pib_set(phyTransmitPower,
                    (pib_value_t *)&orig_tal_transmit_power);
    }

#ifdef ENABLE_RP
    if (RTB_ROLE_REFLECTOR == rtb_role)
    {
        usr_rtb_range_end_ind();
    }
#endif  /* #ifdef ENABLE_RP */

    /* Reset internal variables */
    rtb_role = RTB_ROLE_NONE;
    rtb_state = RTB_IDLE;
    rtb_tx_in_progress = false;
    pmu_reset_pmu_result_vars();
    pmu_reset_fec_vars();

    timer_is_synced = false;
#ifdef ENABLE_RTB_REMOTE
    /* Reset remote ranging indication */
    range_param.CoordinatorAddrSpec.AddrMode = 0;
#endif  /* ENABLE_RTB_REMOTE */

#ifndef RTB_WITHOUT_MAC
    /*
     * Release MAC from being busy.
     */
    MAKE_MAC_NOT_BUSY();
#endif  /* #ifndef RTB_WITHOUT_MAC */

    range_stop_await_timer();

#if (RTB_TYPE == RTB_PMU_233R)
    /* Disable FEC measurement. */
    pmu_disable_fec_measurement();
#endif  /* (RTB_TYPE == RTB_PMU_233R) */

#ifndef RTB_WITHOUT_MAC
    /* Set radio to sleep if allowed */
    mac_sleep_trans();
#endif  /* #ifndef RTB_WITHOUT_MAC */
}



void configure_ranging(void)
{
    pmu_configure_ranging();
}



/** Initial preparation of result exchange procedure. */
static void range_prepare_result_exchange(void)
{
    /* Prepare for exchange of PMU values. */
    pmu_prepare_result_exchange(RESULT_IE_PMU_VALUES);

    if (RTB_ROLE_INITIATOR == rtb_role)
    {
        rtb_state = RTB_INIT_RESULT_REQ_FRAME;
    }
    else if (RTB_ROLE_REFLECTOR == rtb_role)
    {
        rtb_state = RTB_AWAIT_RESULT_REQ_FRAME;
    }
}



/** Perform the result presentation either to the application or to the Coordinator. */
void range_result_presentation(void)
{
#ifdef ENABLE_RTB_REMOTE
    /*
     * In case this was a remote ranging the results need to be sent back
     * to the Coordinator.
     */
    if (range_param.CoordinatorAddrSpec.AddrMode != FCF_NO_ADDR)
    {
        /*
         * Do not notify the application,
         * but sent the data back to the Coordinator.
         */
        range_status.range_error = RANGE_OK;
        rtb_state = RTB_INIT_REMOTE_RANGE_CONF_FRAME;
    }
    else
#endif  /* ENABLE_RTB_REMOTE */
    {
#ifndef RTB_WITHOUT_MAC
        pmu_result_presentation();
#endif  /* #ifndef RTB_WITHOUT_MAC */

        /* Generate range confirm message. */
        range_gen_rtb_range_conf((uint8_t)RTB_SUCCESS,
                                 range_status.distance_cm,
                                 range_status.dqf);

        range_exit();
    }
}



/** Perform the actual result calculation. */
static void range_result_calculation(void)
{
#if defined(SIO_HUB) && defined(ENABLE_RTB_PRINT)&& !defined(RTB_WITHOUT_MAC)
    if ((range_status.range_error == RANGE_OK) && (rtb_pib.PMUVerboseLevel > 1))
    {
        pmu_range_pmu_result_dump();
    }
#endif  /* #if defined(SIO_HUB) && defined(ENABLE_RTB_PRINT)&& !defined(RTB_WITHOUT_MAC) */

    pmu_math_pmu_2_dist();
}



void range_start_await_timer(rtb_state_t current_state)
{
    retval_t timer_status;
    last_rtb_state = current_state;

    timer_status = pal_timer_start(T_RTB_Wait_Time,
                                   RTB_AWAIT_FRAME_TIME,
                                   TIMEOUT_RELATIVE,
                                   (FUNC_PTR())range_t_await_frame_cb,
                                   NULL);

#if (DEBUG > 0)
    ASSERT(MAC_SUCCESS == timer_status);
#endif
    if (MAC_SUCCESS != timer_status)
    {
        /* Timer could not be started. */
        range_t_await_frame_cb((void *)current_state);
    }
}



void range_stop_await_timer(void)
{
    pal_timer_stop(T_RTB_Wait_Time);
}



void range_t_await_frame_cb(void *callback_parameter)
{
    switch (last_rtb_state)
    {
        case RTB_AWAIT_RANGE_ACPT_FRAME:
            {
                /* Happens at Initiator. */
                range_status.range_error = TMO_RTB_AWAIT_RANGE_ACPT_FRAME;

                handle_range_frame_error(RTB_TIMEOUT);
            }
            break;

        case RTB_AWAIT_TIME_SYNC_REQ_FRAME:
            {
                /* Happens at Reflector. */
                range_status.range_error = TMO_RTB_AWAIT_TIME_SYNC_REQ_FRAME;

                /* Clean-up RTB */
                range_exit();
            }
            break;

        case RTB_AWAIT_PMU_START_FRAME:
            {
                /* Happens at Initiator. */
                range_status.range_error = TMO_RTB_AWAIT_PMU_START_FRAME;

                handle_range_frame_error(RTB_TIMEOUT);
            }
            break;

        case RTB_INIT_PMU_START_FRAME:
            {
                /* Happens at Reflector. */
                range_status.range_error = TMO_RTB_INIT_PMU_START_FRAME;

                /* Clean-up RTB */
                range_exit();
            }
            break;

        case RTB_AWAIT_RESULT_CONF_FRAME:
            {
                /* Happens at Initiator. */
                range_status.range_error = TMO_RTB_AWAIT_RESULT_CONF_FRAME;

                handle_range_frame_error(RTB_TIMEOUT);
            }
            break;

        case RTB_AWAIT_RESULT_REQ_FRAME:
            {
                /* Happens at Reflector. */
                range_status.range_error = TMO_RTB_AWAIT_RESULT_REQ_FRAME;

                /* Clean-up RTB */
                range_exit();
            }
            break;

        default:
            break;
    }

    /* Keep compiler happy. */
    callback_parameter = callback_parameter;
}



void handle_range_frame_error(uint8_t error)
{
#ifdef ENABLE_RTB_REMOTE
    if (range_param.CoordinatorAddrSpec.AddrMode != FCF_NO_ADDR)
    {
        /*
         * This is the Initiator during a remote ranging.
         * Notify Coordinator in error case.
         */
        range_status.range_error = (range_error_t)error;
        rtb_state = RTB_INIT_REMOTE_RANGE_CONF_FRAME;
    }
    else
#endif  /* ENABLE_RTB_REMOTE */
    {
        /*
         * This is the Initiator during a regular ranging.
         * Notify application in error case.
         */
        /* Create the RTB RANGE confirmation message */
        range_gen_rtb_range_conf(error,
                                 INVALID_DISTANCE, DQF_ZERO);

        /* Clean-up RTB */
        range_exit();
    }
}



/* Helper function to reset the PMU average data. */
/* DO NOT CHANGE THIS */
void reset_pmu_average_data(void)
{
    pmu_avg_data.no_of_ant_meas = pmu_avg_data.no_of_freq =
                                      pmu_avg_data.ant_meas_ptr_offset = 0;

    pmu_avg_data.p_pmu_avg_init = pmu_avg_data.p_pmu_avg_refl = NULL;
}

#endif /* #ifdef ENABLE_RTB */

/* EOF */
