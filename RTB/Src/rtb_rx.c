/**
 * @file rtb_rx.c
 *
 * @brief Frame reception handling of RTB
 *
 * This file implements frame reception handling functionality of the RTB.
 *
 * $Id: rtb_rx.c 34342 2013-02-22 10:44:18Z sschneid $
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
#include "mac_msg_types.h"
#include "rtb_msg_types.h"
#include "rtb_internal.h"

/* === Macros ============================================================== */


/* === Types =============================================================== */


/* === Globals ============================================================= */


/* === Prototypes ========================================================== */

static void handle_received_pmu_values(uint8_t *curr_frame_ptr);
/* Prototypes for specific frame handling functions. */
static void handle_pmu_time_sync_frame(void);
static void handle_range_acpt_frame(uint8_t *curr_frame_ptr);
static void handle_range_req_frame(uint8_t *curr_frame_ptr);
static void handle_result_conf_frame(uint8_t *curr_frame_ptr);
static void handle_result_req_frame(uint8_t *curr_frame_ptr);
static bool handle_rx_rtb_frame_type(frame_info_t *rx_frame_ptr);
#ifdef ENABLE_RTB_REMOTE
static void handle_remote_range_conf_frame(uint8_t *curr_frame_ptr);
static void handle_remote_range_req_frame(uint8_t *curr_frame_ptr);
#endif  /* ENABLE_RTB_REMOTE */

/* === Implementation ====================================================== */

/**
 * @brief Callback function called by TAL on frame reception if RTB is used.
 *
 * This function pushes an event into the TAL-RTB queue, indicating a
 * frame reception.
 *
 * @param frame Pointer to recived frame
 */
void rtb_rx_frame_cb(frame_info_t *frame)
{
    frame->msg_type = (frame_msgtype_t)RTB_DATA_INDICATION;

    if (NULL == frame->buffer_header)
    {
#if (DEBUG > 0)
        ASSERT("Null frame From TAL" == 0);
#endif
        return;
    }

    qmm_queue_append(&tal_rtb_q, frame->buffer_header);
}



void rtb_process_data_ind(uint8_t *msg)
{
    buffer_t *buf_ptr = (buffer_t *)msg;
    frame_info_t *frameptr = (frame_info_t *)BMM_BUFFER_POINTER(buf_ptr);

    mac_parse_data.mpdu_length = frameptr->mpdu[0];

    /* First extract LQI. */
    mac_parse_data.ppdu_link_quality = frameptr->mpdu[mac_parse_data.mpdu_length + LQI_LEN];

#ifdef RTB_WITHOUT_MAC
    /* Handle the received frame in case the frame is an RTB frame. */
    handle_rx_rtb_frame_type(frameptr);

    /*
     * Release buffer in any case, since it is not forwarded
     * to any higher layer.
     */
    bmm_buffer_free(buf_ptr);
#else   /* #ifdef RTB_WITHOUT_MAC */
    /* Handle the received frame in case the frame is an RTB frame. */
    if (!handle_rx_rtb_frame_type(frameptr))
    {
        /* This is a not an RTB frame, so it is forwarded to the MAC. */
        frameptr->msg_type = (frame_msgtype_t)TAL_DATA_INDICATION;
        qmm_queue_append(&tal_mac_q, buf_ptr);
    }
    else
    {
        bmm_buffer_free(buf_ptr);
    }
#endif  /* #ifdef RTB_WITHOUT_MAC */
}



/*
 * @brief Handles received RTB frames
 *
 * This function parses a received MPDU and checks whether this frame
 * is dedicated to the RTB.
 *
 * @param rx_frame_ptr Pointer to frame received from TAL
 *
 * @return bool True if frame is for RTB, or false if frame is for MAC.
 */
static bool handle_rx_rtb_frame_type(frame_info_t *rx_frame_ptr)
{
    uint8_t payload_index = 0;
    uint16_t fcf;
    range_cmd_t rcmd;
    bool rtb_frame_handled = false;
    uint8_t *temp_frame_ptr = &(rx_frame_ptr->mpdu[1]);
    /* temp_frame_ptr points now to first octet of FCF. */

    /* Extract the FCF. */
    fcf = convert_byte_array_to_16_bit(temp_frame_ptr);
    fcf = CLE16_TO_CPU_ENDIAN(fcf);
    mac_parse_data.fcf = fcf;
    temp_frame_ptr += 2;

    /* Extract the Sequence Number. */
    mac_parse_data.sequence_number = *temp_frame_ptr++;

    /* Extract the complete address information from the MHR. */
    temp_frame_ptr += mac_extract_mhr_addr_info(temp_frame_ptr);
    /*
     * Note: temp_frame_ptr points now to the first octet of the MAC payload
     * if available.
     */

    mac_parse_data.frame_type = FCF_GET_FRAMETYPE(fcf);

    if (FCF_FRAMETYPE_DATA == mac_parse_data.frame_type)
    {
        /* This is a data frame - continue. */
        if (mac_parse_data.mac_payload_length)
        {
            /*
             * In case the device got a frame with a corrupted payload
             * length
             */
            if (mac_parse_data.mac_payload_length >= aMaxMACPayloadSize)
            {
                mac_parse_data.mac_payload_length = aMaxMACPayloadSize;
            }

            /*
             * Copy the pointer to the data frame payload for
             * further processing later.
             */
            mac_parse_data.mac_payload_data.data.payload = &temp_frame_ptr[payload_index];

            /* Check RTB frame identifier. */
            if ((*temp_frame_ptr++ == RTB_FRAME_ID_1) &&
                (*temp_frame_ptr++ == RTB_FRAME_ID_2) &&
                (*temp_frame_ptr++ == RTB_FRAME_ID_3)
               )
            {
                /* This frame is an RTB frame. */

                /* Frame is data frame with payload. */
                /* Check whether the data frame is a valid ranging frame. */
                rcmd = (range_cmd_t)(*temp_frame_ptr);
                temp_frame_ptr++;

                switch (rcmd)
                {
                    case CMD_RANGE_REQ:
                        {
                            handle_range_req_frame(temp_frame_ptr);

                            /* Frame has been handled within RTB. */
                            rtb_frame_handled = true;
                        }
                        break;

                    case CMD_RANGE_ACPT:
                        {
                            handle_range_acpt_frame(temp_frame_ptr);

                            /* Frame has been handled within RTB. */
                            rtb_frame_handled = true;
                        }
                        break;

#ifdef ENABLE_RTB_REMOTE
                    case CMD_REMOTE_RANGE_REQ:
                        {
                            /* This is a remote range request frame. */
                            handle_remote_range_req_frame(temp_frame_ptr);

                            /* Frame has been handled within RTB. */
                            rtb_frame_handled = true;
                        }
                        break;
#endif  /* ENABLE_RTB_REMOTE */

#ifdef ENABLE_RTB_REMOTE
                    case CMD_REMOTE_RANGE_CONF:
                        {
                            handle_remote_range_conf_frame(temp_frame_ptr);

                            /* Frame has been handled within RTB. */
                            rtb_frame_handled = true;
                        }
                        break;
#endif  /* ENABLE_RTB_REMOTE */

                    case CMD_PMU_TIME_SYNC_REQ:
                        {
                            if (RTB_ROLE_REFLECTOR == rtb_role)
                            {
                                handle_pmu_time_sync_frame();

                                /* Frame has been handled within RTB. */
                                rtb_frame_handled = true;

                                /* Buffer is freed up in the calling function. */
                            }
                        }
                        break;

                    case CMD_RESULT_REQ:
                        {
                            /* This is a result request frame. */
                            if (RTB_ROLE_REFLECTOR == rtb_role)
                            {
                                handle_result_req_frame(temp_frame_ptr);

                                /* Frame has been handled within RTB. */
                                rtb_frame_handled = true;

                                /* Buffer is freed up in the calling function. */
                            }
                        }
                        break;

                    case CMD_RESULT_CONF:
                        {
                            /* This is a result confirm frame. */
                            if ((RTB_ROLE_INITIATOR == rtb_role) &&
                                (RTB_AWAIT_RESULT_CONF_FRAME == rtb_state))
                            {
                                handle_result_conf_frame(temp_frame_ptr);

                                /* Frame has been handled within RTB. */
                                rtb_frame_handled = true;

                                /* Buffer is freed up in the calling function. */
                            }
                        }
                        break;

                    default:
                        break;
                }   /* switch (rcmd) */
            }   /* This frame is an RTB frame. */
        }
        else
        {
            mac_parse_data.mac_payload_length = 0;
            /* Data frame without payload is NOT for RTB. */
        }
    }

    return rtb_frame_handled;
} /* handle_rx_rtb_frame_type() */



static void handle_received_pmu_values(uint8_t *curr_frame_ptr)
{
    pmu_handle_received_pmu_values(curr_frame_ptr);

    /* Check whether we should expect further values. */
    if (pmu_more_results_to_be_expected())
    {
        /* More result values pending. */
        rtb_state = RTB_INIT_RESULT_REQ_FRAME;
    }
    else
    {
        /*
         * No more result values pending for this
         * antenna measurement value.
         */
        /*
         * Check whether results for more antenna
         * values available.
         */
        if (range_status_pmu.curr_antenna_measurement_no <
            (range_param_pmu.antenna_measurement_nos - 1))
        {
            /*
             * Continue with next result values for
             * next antenna combination.
             */
            range_status_pmu.curr_antenna_measurement_no++;
            pmu_reset_pmu_result_vars();
            rtb_state = RTB_INIT_RESULT_REQ_FRAME;
        }
        else
        {
            /*
             * No more result values pending for further
             * antenna values.
             */
            /* Regular handling, no more result data to be requested. */
            pmu_set_pmu_result_idx_done();
            rtb_state = RTB_RESULT_CALC;
        }
    }
}



static void handle_pmu_time_sync_frame(void)
{
    /*
     * The RTB Protocol Version is not yet evaluated here.
     * This will be added, once further
     * RTB Procotols are implemented.
     */
    /* Cancel running timer. */
    range_stop_await_timer();

    rtb_state = RTB_INIT_PMU_START_FRAME;

    /* Start timer in case Result Request frame is not received. */
    range_start_await_timer(RTB_INIT_PMU_START_FRAME);
}



static void handle_range_acpt_frame(uint8_t *curr_frame_ptr)
{
    if (RTB_ROLE_INITIATOR == rtb_role)
    {
        /* Cancel running timer. */
        range_stop_await_timer();

        /* This is a Range Accept frame. */
        /* Check range acceptance status. */
        if (RTB_SUCCESS == *curr_frame_ptr++)
        {
            /* Skip range reject reason. */
            curr_frame_ptr++;
            if (RTB_TYPE == *curr_frame_ptr++)
            {
                /*
                 * Update capabilities received from Reflector.
                 * This may overwrite our initially requested
                 * antenna diversity request.
                 */
                range_param.caps = *curr_frame_ptr++;
                pmu_configure_antenna();
            }

            /* Ranging Request is accepted. */
            rtb_state = RTB_INIT_TIME_SYNC_REQ_FRAME;
        }
        else
        {
            range_error_t range_reject_reason =
                (range_error_t)(*curr_frame_ptr++);
#ifdef ENABLE_RTB_REMOTE
            if (range_param.CoordinatorAddrSpec.AddrMode != FCF_NO_ADDR)
            {
                /*
                 * This is the Initiator during a remote ranging.
                 * The Reflector does not allow ranging, so return
                 * a Remote Range Confirm frame.
                 */
                range_status.range_error = range_reject_reason;
                rtb_state = RTB_INIT_REMOTE_RANGE_CONF_FRAME;
            }
            else
#endif  /* ENABLE_RTB_REMOTE */
            {
                /*
                 * This is the Initiator during a regular ranging.
                 * Ranging request is NOT accepted.
                 * Return range confirm.
                 */
                range_gen_rtb_range_conf(range_reject_reason,
                                         INVALID_DISTANCE, DQF_ZERO);

                /* Clean-up RTB */
                range_exit();
            }
        }
    }
}



static void handle_range_req_frame(uint8_t *curr_frame_ptr)
{
    /*
     * Store addresses for the ongoing transaction.
     * This is required for all cases, also for
     * erroneous cases.
     */
    /* This node is the Reflector. */
    range_param.ReflectorAddrSpec.AddrMode = mac_parse_data.dest_addr_mode;
    range_param.ReflectorAddrSpec.PANId = mac_parse_data.dest_panid;
    /* Long address also covers short address here. */
    ADDR_COPY_DST_SRC_64(range_param.ReflectorAddrSpec.Addr.long_address,
                         mac_parse_data.dest_addr.long_address);

    /* The transmitter of this frame is the Initiator. */
    range_param.InitiatorAddrSpec.AddrMode = mac_parse_data.src_addr_mode;
    range_param.InitiatorAddrSpec.PANId = mac_parse_data.src_panid;
    /* Long address also covers short address here. */
    ADDR_COPY_DST_SRC_64(range_param.InitiatorAddrSpec.Addr.long_address,
                         mac_parse_data.src_addr.long_address);

    if (RTB_ROLE_NONE != rtb_role)
    {
        /* Ranging is currently already ongoing, do nothing. */
    }
    else if (!rtb_pib.RangingEnabled)
    {
        /* Ranging is currently disabled, reject new request. */
        range_status.range_error = (range_error_t)RTB_UNSUPPORTED_RANGING;
        rtb_state = RTB_INIT_RANGE_ACPT_FRAME;
        /*
         * The role needs to be updated even in error
         * case to  be able to properly release the
         * frame buffer after the transmission of the
         * Range Accept frame back to the Initiator.
         */
        rtb_role = RTB_ROLE_REFLECTOR;

        /*
         * Reset the PMU average data since no valid PMU average data are available
         * at this stage.
         */
        reset_pmu_average_data();
    }
    else
    {
        uint8_t frame_len;

        rtb_role = RTB_ROLE_REFLECTOR;

        /*
         * Reset the PMU average data since no valid PMU average data are available
         * at this stage.
         */
        reset_pmu_average_data();

        /*
         * Generally ranging is allowed ...
         * Now check requested ranging parameters.
         */

        /* Read Frame length field. */
        frame_len = *curr_frame_ptr++;

        if (RTB_PROTOCOL_VERSION_01 == *curr_frame_ptr++)
        {
            /* Currently only one RTB Protocol Version is supported. */

            /* Check whether requested ranging method is supported. */
            if (RTB_TYPE == *curr_frame_ptr++)
            {
                range_param.method = RTB_TYPE;

                range_param_pmu.f_start = convert_byte_array_to_16_bit(curr_frame_ptr);
                curr_frame_ptr += 2;
                range_param_pmu.f_step = *curr_frame_ptr++;
                range_param_pmu.f_stop = convert_byte_array_to_16_bit(curr_frame_ptr);;
                curr_frame_ptr += 2;

                range_param.caps = *curr_frame_ptr++;
                /*
                 * Update the received capabilities from the Initiator
                 * to match our own capabilities.
                 *
                 * Note: Other capabilty bits than used for antenna
                 * diversity are not touched here.
                 */
#if (ANTENNA_DIVERSITY == 1)
                /*
                 * If we allow antenna diversity generally,
                 * use antenna diversity value received from the initator,
                 * and simply update our own.
                 */
                if (rtb_pib.EnableAntennaDiv)
                {
                    /*
                     * Reflector uses antenna diversity.
                     * Keep Initiator antenna diversity as is.
                     */
                    range_param.caps |= PMU_CAP_REFLECTOR_ANT;
                }
                else
                {
                    /*
                     * Reflector temporarily does not use antenna diversity,
                     * but keep Initiator antenna diversity as is.
                     */
                    range_param.caps &= ~(PMU_CAP_REFLECTOR_ANT);
                }
#else
                /*
                 * Reflector generally does not use antenna diversity,
                 * but keep Initiator antenna diversity as is.
                 */
                range_param.caps &= ~(PMU_CAP_REFLECTOR_ANT);

#endif  /* (ANTENNA_DIVERSITY == 1) */

                /*
                 * Initialize the Ranging Transmit Power to be applied at the
                 * Reflector in case this parameter is not included properly
                 * in this frame.
                 */
                range_param.req_tx_power = rtb_pib.RangingTransmitPower;

                /* Check wether Requested Ranging Transmit Power IE is available. */
                if (frame_len == IE_PMU_RANGING_LEN + IE_REQ_RANGING_TX_POWER_LEN)
                {
                    /*
                     * Extract and set requested Ranging Transmit Power
                     * (if changed).
                     */
                    if (REQ_RANGING_TX_POWER_IE == *curr_frame_ptr++)
                    {
                        /* Overwrite Ranging Transmit Power. */
                        range_param.req_tx_power = *curr_frame_ptr++;
                    }
                }

                pmu_configure_antenna();

                if (!pmu_check_pmu_params())
                {
                    /* Unsupported ranging parameters, reject new request. */
                    range_status.range_error = (range_error_t)RTB_INVALID_PARAMETER;
                    rtb_state = RTB_INIT_RANGE_ACPT_FRAME;
                }
                else
                {
#if (RTB_TYPE == RTB_PMU_233R)
                    /* Prepare FEC measurement */
                    pmu_enable_fec_measurement();
#endif  /* (RTB_TYPE == RTB_PMU_233R) */

                    configure_ranging();

#ifndef RTB_WITHOUT_MAC
                    /*
                     * Block MAC from doing anything else than ranging
                     * until this procedure is finished.
                     */
                    MAKE_MAC_BUSY();
#endif  /* #ifndef RTB_WITHOUT_MAC */

                    /* Next a Range Accept frame needs to be assembled. */
                    range_status.range_error = RANGE_OK;
                    rtb_state = RTB_INIT_RANGE_ACPT_FRAME;
                }
            }
            else
            {
                /* Unsupported ranging method, reject new request. */
                range_status.range_error = (range_error_t)RTB_UNSUPPORTED_METHOD;
                rtb_state = RTB_INIT_RANGE_ACPT_FRAME;
            }
        }
        else    /* if (RTB_PROTOCOL_VERSION_01 == *curr_frame_ptr++) */
        {
            /* Unsupported RTB Protocol Version, reject new request. */
            range_status.range_error = (range_error_t)RTB_UNSUPPORTED_PROTOCOL;
            rtb_state = RTB_INIT_RANGE_ACPT_FRAME;
        }
    }
}



static void handle_result_req_frame(uint8_t *curr_frame_ptr)
{
    /* Cancel running timer. */
    range_stop_await_timer();

    /* Check received IE to indicate requested result data. */
    req_result_type = *(result_frame_ie_t *)curr_frame_ptr;
    curr_frame_ptr++;

    if (RESULT_IE_PMU_VALUES == req_result_type)
    {
        /* PMU result values are requested and handled. */
        /* Which antenna measurement value is requested? */
        range_status_pmu.curr_antenna_measurement_no = *curr_frame_ptr++;

        pmu_extract_no_of_req_result_values(*(uint16_t *)curr_frame_ptr);

        /*
         * Check whether invalid antenna measurement
         * value has been requested.
         */
        if (range_status_pmu.curr_antenna_measurement_no >=
            range_param_pmu.antenna_measurement_nos)
        {
            /*
             * This should NEVER happen, because we
             * would access memory, we are not supposed
             * to read out.
             * Does somebody try to compromise our
             * system?
             * Get out of here ...
             */
            /* Immediately stop the ranging procedure. */
            range_exit();
        }
        else
        {
            /* Everything is as expected. */
            rtb_state = RTB_INIT_RESULT_CONF_FRAME;
        }
    }
    else
    {
        /* Unsupported result data type requested. */
        range_exit();
    }
}



static void handle_result_conf_frame(uint8_t *curr_frame_ptr)
{
    /* Cancel running timer. */
    range_stop_await_timer();

    /* A Range Result frame is actually expected. */

    /* Check whether the proper result data are received. */
    if (*curr_frame_ptr++ == req_result_type)
    {
        /* Handle reception of PMU result data. */
        handle_received_pmu_values(curr_frame_ptr);
    }
}



#ifdef ENABLE_RTB_REMOTE
static void handle_remote_range_conf_frame(uint8_t *curr_frame_ptr)
{
    if (RTB_ROLE_NONE == rtb_role)
    {
        /*
         * Only if the Coordinator is currently not
         * actively involved itself, the Remote-Range-Confirm
         * is handled, otherwise it is ignored, because this
         * would corrupt our current local ranging related
         * address information.
         */
        /* Set proper ranging address parameter. */
        /* Set received source address address as Initiator address. */
        range_param.InitiatorAddrSpec.AddrMode = mac_parse_data.src_addr_mode;
        range_param.InitiatorAddrSpec.PANId = mac_parse_data.src_panid;
        /* Long address also covers short address here. */
        ADDR_COPY_DST_SRC_64(range_param.InitiatorAddrSpec.Addr.long_address,
                             mac_parse_data.src_addr.long_address);

        refl_addr_t *ra = (refl_addr_t *)curr_frame_ptr;

        /*
         * Set Reflector address received in frame as
         * Reflector address.
         */
        range_param.ReflectorAddrSpec.PANId = ra->refl_pan_id;

        range_remote_answer_t *rra;

        if (ra->refl_addr_mode == FCF_SHORT_ADDR)
        {
            range_param.ReflectorAddrSpec.AddrMode = FCF_SHORT_ADDR;
            range_param.ReflectorAddrSpec.Addr.long_address = 0;    // Init long address first
            ADDR_COPY_DST_SRC_16(range_param.ReflectorAddrSpec.Addr.short_address,
                                 ra->refl_addr.short_address);
            rra = (range_remote_answer_t *)((uint8_t *) & (ra->refl_addr) + 2);
        }
        else
        {
            range_param.ReflectorAddrSpec.AddrMode = FCF_LONG_ADDR;
            ADDR_COPY_DST_SRC_64(range_param.ReflectorAddrSpec.Addr.long_address,
                                 ra->refl_addr.long_address);
            rra = (range_remote_answer_t *)((uint8_t *) & (ra->refl_addr) + 8);
        }

        if (RTB_SUCCESS == rra->status)
        {
            /* Remote ranging was successful. */
            /* Check Additional Results IE type. */
            if (NO_ADDITIONAL_RESULTS == rra->additional_result_ie)
            {
                /* Return remote range confirm. */
                range_gen_rtb_remote_range_conf(RTB_SUCCESS,
                                                rra->distance_cm,
                                                rra->dqf,
                                                0,
                                                NULL);
            }
            else if (ANT_DIV_MEAS_RESULTS == rra->additional_result_ie)
            {
                /* Return remote range confirm with measured values. */
                range_gen_rtb_remote_range_conf(RTB_SUCCESS,
                                                rra->distance_cm,
                                                rra->dqf,
                                                rra->additional_result_fields.
                                                prov_antenna_div_results.
                                                no_of_provided_meas_pairs,
                                                rra->additional_result_fields.
                                                prov_antenna_div_results.
                                                provided_meas_pairs);
            }
        }
        else
        {
            /* Ranging request is NOT accepted. */
            /* Return remote range confirm with error values. */
            range_gen_rtb_remote_range_conf(rra->range_reject_reason,
                                            INVALID_DISTANCE,
                                            DQF_ZERO,
                                            0,
                                            NULL);
        }

        /*
         * Note: range_exit() must not be called here;
         * The remote range confirm is just notified to the
         * application.
         */
    }
}
#endif  /* ENABLE_RTB_REMOTE */



#ifdef ENABLE_RTB_REMOTE
static void handle_remote_range_req_frame(uint8_t *curr_frame_ptr)
{
    /*
     * Store addresses for the ongoing transaction.
     * This is required for all cases, also for
     * erroneous cases.
     */
    /* This node is the Initiator. */
    range_param.InitiatorAddrSpec.AddrMode = mac_parse_data.dest_addr_mode;
    range_param.InitiatorAddrSpec.PANId = mac_parse_data.dest_panid;
    /* Long address also covers short address here. */
    ADDR_COPY_DST_SRC_64(range_param.InitiatorAddrSpec.Addr.long_address,
                         mac_parse_data.dest_addr.long_address);

    /* The transmitter of this frame is the Coordinator. */
    /*
     * Note: Setting the Coordinator address mode here to a value different from
     * zero actually indicates, that remote ranging is ongoing.
     */
    range_param.CoordinatorAddrSpec.AddrMode = mac_parse_data.src_addr_mode;
    range_param.CoordinatorAddrSpec.PANId = mac_parse_data.src_panid;
    /* Long address also covers short address here. */
    ADDR_COPY_DST_SRC_64(range_param.CoordinatorAddrSpec.Addr.long_address,
                         mac_parse_data.src_addr.long_address);

    if (RTB_ROLE_NONE != rtb_role)
    {
        /* Ranging is currently already ongoing, do nothing. */
    }
    else if (!rtb_pib.RangingEnabled)
    {
        /* Ranging is currently disabled, reject new request. */
        range_status.range_error = (range_error_t)RTB_UNSUPPORTED_RANGING;
        rtb_state = RTB_INIT_REMOTE_RANGE_CONF_FRAME;
        /*
         * The role needs to be updated even in error
         * case to  be able to properly release the
         * frame buffer after the transmission of the
         * Remote Range confirm frame back to the Coordinator.
         */
        rtb_role = RTB_ROLE_INITIATOR;

        /*
         * Reset the PMU average data since no valid PMU average data are available
         * at this stage.
         */
        reset_pmu_average_data();
    }
    else
    {
        uint8_t frame_len;

        /*
         * Generally ranging is allowed ...
         * Now check requested ranging parameters.
         */

        /* Read Frame length field. */
        frame_len = *curr_frame_ptr++;

        if (RTB_PROTOCOL_VERSION_01 == *curr_frame_ptr++)
        {
            uint8_t refl_addr_len = IE_REFLECTOR_ADDR_SPEC_LEN_MIN;

            /* Currently only one RTB Protocol Version is supported. */

            rtb_role = RTB_ROLE_INITIATOR;

            /*
            * Reset the PMU average data since no valid PMU average data are available
            * at this stage.
            */
            reset_pmu_average_data();

            /* Extract Reflector address information from the frame. */
            refl_addr_t *ra =
                (refl_addr_t *)curr_frame_ptr;

            range_param.ReflectorAddrSpec.AddrMode = ra->refl_addr_mode;
            range_param.ReflectorAddrSpec.PANId = ra->refl_pan_id;
            /* Long address also covers short address here. */

            if (ra->refl_addr_mode == FCF_SHORT_ADDR)
            {
                range_param.ReflectorAddrSpec.Addr.long_address = 0;
                ADDR_COPY_DST_SRC_16(range_param.ReflectorAddrSpec.Addr.short_address,
                                     ra->refl_addr.short_address);
            }
            else
            {
                ADDR_COPY_DST_SRC_64(range_param.ReflectorAddrSpec.Addr.long_address,
                                     ra->refl_addr.long_address);
                refl_addr_len += 6;
            }
            curr_frame_ptr += refl_addr_len;

            /* Check whether requested ranging method is supported. */
            if (RTB_TYPE == *curr_frame_ptr++)
            {
                range_param.method = RTB_TYPE;

                range_param_pmu.f_start = convert_byte_array_to_16_bit(curr_frame_ptr);
                curr_frame_ptr += 2;
                range_param_pmu.f_step = *curr_frame_ptr++;
                range_param_pmu.f_stop = convert_byte_array_to_16_bit(curr_frame_ptr);
                curr_frame_ptr += 2;
                /*
                 * Extract remote range capabilities from
                 * the Coordinator.
                 */
                range_param.remote_caps = *curr_frame_ptr++;
                if (range_param.remote_caps & PMU_REM_CAP_APPLY_MIN_DIST_THRSHLD)
                {
                    range_param_pmu.apply_min_dist_threshold = true;
                }
                else
                {
                    range_param_pmu.apply_min_dist_threshold = false;
                }

                /*
                 * The capabilities for the actual ranging
                 * are not taken from the Coordinator, but
                 * rather from the Initiator itself.
                 */
                SET_INITIATOR_CAPS(range_param.caps);

                /*
                 * Initialize the Ranging Transmit Power to be applied at the
                 * Initiator in case this parameter is not included properly
                 * in this frame.
                 */
                range_param.req_tx_power = rtb_pib.RangingTransmitPower;

                /* Check wether Requested Ranging Transmit Power IE is available. */
                if (frame_len == IE_PMU_RANGING_LEN +
                    refl_addr_len +
                    IE_REQ_RANGING_TX_POWER_LEN)
                {
                    /*
                     * Extract and set requested Ranging Transmit Power
                     * (if changed).
                     */
                    if (REQ_RANGING_TX_POWER_IE == *curr_frame_ptr++)
                    {
                        /* Overwrite Ranging Transmit Power. */
                        range_param.req_tx_power = *curr_frame_ptr++;
                    }
                }

                if (!pmu_check_pmu_params())
                {
                    /* Unsupported ranging parameters, reject new request. */
                    range_status.range_error = (range_error_t)RTB_INVALID_PARAMETER;
                    rtb_state = RTB_INIT_REMOTE_RANGE_CONF_FRAME;
                }
                else
                {
                    /*
                     * Proper Remote Range Request frame received,
                     * so continue as Initiator.
                     */
#if (RTB_TYPE == RTB_PMU_233R)
                    /* Prepare FEC measurement */
                    pmu_enable_fec_measurement();
#endif  /* (RTB_TYPE == RTB_PMU_233R) */

                    configure_ranging();

#ifndef RTB_WITHOUT_MAC
                    /*
                     * Block MAC from doing anything else than ranging
                     * until this procedure is finished.
                     */
                    MAKE_MAC_BUSY();
#endif  /* #ifndef RTB_WITHOUT_MAC */

                    /*
                     * Next a Range Request frame needs to be assembled to be
                     * transmitted to Reflector
                     */
                    range_status.range_error = RANGE_OK;
                    rtb_state = RTB_INIT_RANGE_REQ_FRAME;
                }
            }
            else
            {
                /* Unsupported ranging method, reject new request. */
                range_status.range_error = (range_error_t)RTB_UNSUPPORTED_METHOD;
                rtb_state = RTB_INIT_REMOTE_RANGE_CONF_FRAME;
            }
        }
        else    /* if (RTB_PROTOCOL_VERSION_01 == *curr_frame_ptr++) */
        {
            /* Unsupported RTB Protocol Version, reject new request. */
            range_status.range_error = (range_error_t)RTB_UNSUPPORTED_PROTOCOL;
            rtb_state = RTB_INIT_REMOTE_RANGE_CONF_FRAME;
        }
    }
}
#endif  /* ENABLE_RTB_REMOTE */

#endif /* #ifdef ENABLE_RTB */

/* EOF */
