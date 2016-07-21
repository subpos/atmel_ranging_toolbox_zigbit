/**
 * @file rtb_tx.c
 *
 * @brief Frame transmission handling of RTB
 *
 * This file implements frame transmission handling functionality of the RTB.
 *
 * $Id: rtb_tx.c 34343 2013-02-22 11:45:08Z sschneid $
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

/* === Macros ============================================================== */


/* === Types =============================================================== */


/* === Globals ============================================================= */


/* === Prototypes ========================================================== */

static void build_pmu_time_sync_req_frame(uint8_t *curr_frame_ptr);
#ifdef  ENABLE_RTB_REMOTE
static uint8_t build_remote_range_conf_frame(uint8_t *curr_frame_ptr,
                                             uint8_t curr_frame_len);
static void build_remote_range_req_frame(uint8_t *curr_frame_ptr);
#endif  /* ENABLE_RTB_REMOTE */
static void build_result_conf_frame(uint8_t *curr_frame_ptr,
                                    uint16_t values_to_be_sent);
static void build_result_req_frame(uint8_t *curr_frame_ptr);
static void build_range_acpt_frame(uint8_t *curr_frame_ptr);
static void build_range_req_frame(uint8_t *curr_frame_ptr);

/* === Implementation ====================================================== */

void range_tx_range_accept_frame(void)
{
    /* Send Range Accept frame using CSMA/CA. */
    range_assemble_and_tx_frame_csma(RTB_CMD_RANGE_ACPT,        // Internal RTB message type
                                     CMD_RANGE_ACPT,            // External RTB command frame type
                                     RTB_RANGE_ACPT_FRAME_DONE, // Next RTB state
                                     /*
                                      * Do NOT Generate an error message if required,
                                      * since this is a Reflector.
                                      */
                                     NO_CONF);
}



/** Initiation of a Result Request frame. */
void range_tx_result_req_frame(void)
{
    if (RTB_ROLE_INITIATOR == rtb_role)
    {
        /* Send Result Request frame using CSMA/CA. */
        range_assemble_and_tx_frame_csma(RTB_CMD_RESULT_REQ,        // Internal RTB message type
                                         CMD_RESULT_REQ,            // External RTB command frame type
                                         RTB_RESULT_REQ_FRAME_DONE, // Next RTB state
                                         /*
                                          * Generate a Range-Confirm if required,
                                          * since this is an Initiator.
                                          */
                                         LOCAL_CONF);
    }
}



/** Initiation of a Result Confirm frame. */
void range_tx_result_conf_frame(void)
{
    /* This is an PMU based ranging measurement. */
    if (pmu_update_result_ptr())
    {
        /* Send Result Confirm frame using CSMA/CA. */
        range_assemble_and_tx_frame_csma(RTB_CMD_RESULT_CONF,        // Internal RTB message type
                                         CMD_RESULT_CONF,            // External RTB command frame type
                                         RTB_RESULT_CONF_FRAME_DONE, // Next RTB state
                                         /*
                                          * Do NOT Generate an error message if required,
                                          * since this is a Reflector.
                                          */
                                         NO_CONF);
    }
}



#ifdef ENABLE_RTB_REMOTE
void range_tx_remote_range_conf_frame(void)
{
    /* Send Remote Range Confirm frame using CSMA/CA. */
    range_assemble_and_tx_frame_csma(RTB_CMD_REMOTE_RANGE_CONF,        // Internal RTB message type
                                     CMD_REMOTE_RANGE_CONF,            // External RTB command frame type
                                     RTB_REMOTE_RANGE_CONF_FRAME_DONE, // Next RTB state
                                     /*
                                      * Do NOT Generate an error message if required,
                                      * since this is an Initator sending the frame
                                      * to the Coordinator.
                                      */
                                     NO_CONF);
}
#endif  /* ENABLE_RTB_REMOTE */



/**
 * @brief Callback function from RTB after the frame is transmitted
 *
 * This is a callback function from the RTB. It is used when an attempt
 * to transmit a frame is finished.
 *
 * @param status Status of transmission
 * @param frame Specifies pointer to the transmitted frame
 */
void rtb_tx_frame_done_cb(retval_t status, frame_info_t *frame)
{
#ifndef RTB_WITHOUT_MAC
    if (RTB_ROLE_NONE == rtb_role)
    {
        /*
         * Ranging currently not ongoing.
         * A regular TAL frame received which is to be forwarded
         * to the MAC directly.
         */
        tal_tx_frame_done_cb(status, frame);
    }
    else
#endif  /* #ifndef RTB_WITHOUT_MAC */
    {
        /* Ranging currently ongoing. */
        range_process_tal_tx_status(status, frame);
    }
}



/**
 * Helper function to assemble and transmit RTB frames using CSMA-CA.
 *
 * @param msgtype Internal RTB message type
 * @param cmd_type External RTB command frame type to be transmitted
 * @param next_rtb_state Next RTB state after transmission
 * @param generate_range_conf_on_error
 *   NO_CONF: No confirm generation in error case
 *   LOCAL_CONF: Generate regular Range-Confirm
 *   REMOTE_CONF: Generate Remote-Range-Confirm (if remote ranging is enabled)
 *
 */
void range_assemble_and_tx_frame_csma(frame_msgtype_t msgtype,
                                      range_cmd_t cmd_type,
                                      rtb_state_t next_rtb_state,
                                      conf_on_error_t generate_range_conf_on_error)
{
    retval_t status = FAILURE;

    buffer_t *buf_ptr = bmm_buffer_alloc(LARGE_BUFFER_SIZE);
    frame_info_t *transmit_frame;

    if (NULL == buf_ptr)
    {
        if (LOCAL_CONF == generate_range_conf_on_error)
        {
            /* Return range request. */
            range_gen_rtb_range_conf((uint8_t)RTB_OUT_OF_BUFFERS,
                                     INVALID_DISTANCE, DQF_ZERO);
        }
#ifdef ENABLE_RTB_REMOTE
        else if (REMOTE_CONF == generate_range_conf_on_error)
        {
            /* Return range request. */
            range_gen_rtb_remote_range_conf((uint8_t)RTB_OUT_OF_BUFFERS,
                                            INVALID_DISTANCE,
                                            DQF_ZERO,
                                            0,
                                            NULL);
        }
#endif  /* ENABLE_RTB_REMOTE */

        /* Clean-up RTB */
        range_exit();
        return;
    }

    /* Now all is fine, continue... */
    transmit_frame = (frame_info_t *)BMM_BUFFER_POINTER(buf_ptr);

    /* Store the message type - The frame will be an RTB data frame. */
    transmit_frame->msg_type = msgtype;

    /* Build the requested RTB Frame. */
    range_build_frame(cmd_type, transmit_frame);

    transmit_frame->buffer_header = buf_ptr;

    /* Update RTB state */
    rtb_state = next_rtb_state;

    /* Transmission should be done with CSMA-CA and with frame retries. */
#ifdef BEACON_SUPPORT
    csma_mode_t cur_csma_mode;

    if (NON_BEACON_NWK == tal_pib.BeaconOrder)
    {
        /* In Nonbeacon network the frame is sent with unslotted CSMA-CA. */
        cur_csma_mode = CSMA_UNSLOTTED;
    }
    else
    {
        /* In Beacon network the frame is sent with slotted CSMA-CA. */
        cur_csma_mode = CSMA_SLOTTED;
    }

    status = tal_tx_frame(transmit_frame, cur_csma_mode, true);
#else   /* No BEACON_SUPPORT */
    /* In Nonbeacon build the frame is sent with unslotted CSMA-CA. */
    status = tal_tx_frame(transmit_frame, CSMA_UNSLOTTED, true);
#endif  /* BEACON_SUPPORT / No BEACON_SUPPORT */
    if (MAC_SUCCESS != status)
    {
        /*
         * Release buffer due to failed transmission, since
         * the local range confirm uses the already kept buffer
         * belonging to range_gen_rtb_range_conf,
         * and the remote range confirm allocates a new buffer.
         */
        bmm_buffer_free(buf_ptr);

        /* Transmission to TAL failed. */
        if (NO_CONF != generate_range_conf_on_error)
        {
#ifdef ENABLE_RTB_REMOTE
            if (RTB_ROLE_COORDINATOR == rtb_role)
            {
                /* Generate remote range confirm message. */
                range_gen_rtb_remote_range_conf((uint8_t)MAC_CHANNEL_ACCESS_FAILURE,
                                                INVALID_DISTANCE,
                                                DQF_ZERO,
                                                0,
                                                NULL);
            }
            else
#endif  /* ENABLE_RTB_REMOTE */
            {
                /* Generate range confirm message. */
                range_gen_rtb_range_conf((uint8_t)MAC_CHANNEL_ACCESS_FAILURE,
                                         INVALID_DISTANCE, DQF_ZERO);
            }

        }

        /* Clean-up RTB */
        range_exit();
    }
    else
    {
        /* Indicate started frame transmission, awaiting TRX_END IRQ. */
        rtb_tx_in_progress = true;
    }
}



/*
 * @brief Process rtb_tx_frame_done_cb status
 *
 * This function is called, if an ACK is requested in the last transmitted frame.
 * According to the frame type that has previously been sent, the
 * corresponding actions are taken and the RTB returns to its standard state.
 *
 * @param tx_status Status of transmission
 * @param frame Pointer to the transmitted frame
 */
void range_process_tal_tx_status(retval_t tx_status,  frame_info_t *frame)
{
    /* Indicate finished frame transmission. */
    rtb_tx_in_progress = false;

    /* Next action has to be initiated  depending on the ranging command id. */
    switch (frame->msg_type)
    {
        case RTB_CMD_RANGE_REQ:
            {
                ASSERT(rtb_state == RTB_RANGE_REQ_FRAME_DONE);

                if ((MAC_NO_ACK == tx_status) || (MAC_CHANNEL_ACCESS_FAILURE == tx_status))
                {
                    handle_range_frame_error(tx_status);
                }
                else
                {
                    configure_ranging();

                    rtb_state = RTB_AWAIT_RANGE_ACPT_FRAME;

                    /* Start timer in case Range Accept frame is not received. */
                    range_start_await_timer(RTB_AWAIT_RANGE_ACPT_FRAME);
                }
            }
            break;

        case RTB_CMD_RANGE_ACPT:
            {
                ASSERT(rtb_state == RTB_RANGE_ACPT_FRAME_DONE);

                if ((MAC_NO_ACK == tx_status) ||
                    (MAC_CHANNEL_ACCESS_FAILURE == tx_status) ||
                    (RANGE_OK != range_status.range_error)
                   )
                {
                    /* Clean-up RTB */
                    range_exit();
                }
                else
                {
                    rtb_state = RTB_AWAIT_TIME_SYNC_REQ_FRAME;

                    /* Start timer in case Time Sync Request frame is not received. */
                    range_start_await_timer(RTB_AWAIT_TIME_SYNC_REQ_FRAME);
                }
            }
            break;

        case RTB_CMD_PMU_TIME_SYNC_REQ:
            {
                ASSERT(rtb_state == RTB_TIME_SYNC_REQ_FRAME_DONE);

                if ((MAC_NO_ACK == tx_status) || (MAC_CHANNEL_ACCESS_FAILURE == tx_status))
                {
                    handle_range_frame_error(tx_status);
                }
                else
                {
                    rtb_state = RTB_AWAIT_PMU_START_FRAME;

                    /*
                     * Start timer in case the PMU Start frame is not received.
                     * The timer is not immediately cancelled once the
                     * PMU Start is received from the Reflector, since we do not
                     * have time for this as this point of time due to the
                     * tight synchronization.
                     * The timer is actually cancelled once the entire PMU
                     * measurement has finished and regular operation is
                     * in place again.
                     */
                    range_start_await_timer(RTB_AWAIT_PMU_START_FRAME);

                    /* Now the PMU Start frame is expected and handled. */
                    pmu_perform_pmu_measurement();
                }
            }
            break;

        case RTB_CMD_RESULT_REQ:
            {
                ASSERT(rtb_state == RTB_RESULT_REQ_FRAME_DONE);

                if ((MAC_NO_ACK == tx_status) || (MAC_CHANNEL_ACCESS_FAILURE == tx_status))
                {
                    handle_range_frame_error(tx_status);
                }
                else
                {
                    rtb_state = RTB_AWAIT_RESULT_CONF_FRAME;

                    /* Start timer in case Result Confirm frame is not received. */
                    range_start_await_timer(RTB_AWAIT_RESULT_CONF_FRAME);
                }
            }
            break;

        case RTB_CMD_RESULT_CONF:
            {
                ASSERT(rtb_state == RTB_RESULT_CONF_FRAME_DONE);

                if ((MAC_NO_ACK == tx_status) || (MAC_CHANNEL_ACCESS_FAILURE == tx_status))
                {
                    /* Clean-up RTB */
                    range_exit();
                }
                else
                {
                    if (pmu_no_more_pmu_data_available())
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
                            rtb_state = RTB_AWAIT_RESULT_REQ_FRAME;
                            pmu_reset_pmu_result_vars();
                            /*
                             * Start timer in case the next Result Confirm frame
                             * is not received.
                             */
                            range_start_await_timer(RTB_AWAIT_RESULT_REQ_FRAME);
                        }
                        else
                        {
                            /* Clean-up RTB */
                            range_exit();
                        }
                    }
                    else
                    {
                        rtb_state = RTB_AWAIT_RESULT_REQ_FRAME;

                        /*
                         * Start timer in case the next Result Confirm frame
                         * is not received.
                         */
                        range_start_await_timer(RTB_AWAIT_RESULT_REQ_FRAME);
                    }
                }
            }
            break;

#ifdef ENABLE_RTB_REMOTE
        case RTB_CMD_REMOTE_RANGE_REQ:
            {
                ASSERT(rtb_state == RTB_REMOTE_RANGE_REQ_FRAME_DONE);

                if ((MAC_NO_ACK == tx_status) || (MAC_CHANNEL_ACCESS_FAILURE == tx_status))
                {
                    /* Create the RTB REMOTE RANGE confirmation message */
                    range_gen_rtb_remote_range_conf(tx_status,
                                                    INVALID_DISTANCE,
                                                    DQF_ZERO,
                                                    0,
                                                    NULL);
                }
                else
                {
                    /*
                     * Fire and forget. Whenever a Remote Range Confirm frame
                     * is received, the corresponding confirm callback will be
                     * invoked.
                     */
                }
                /* Clean-up RTB */
                range_exit();
            }
            break;
#endif  /* ENABLE_RTB_REMOTE */

#ifdef ENABLE_RTB_REMOTE
        case RTB_CMD_REMOTE_RANGE_CONF:
            {
                ASSERT(rtb_state == RTB_REMOTE_RANGE_CONF_FRAME_DONE);

                /*
                 * This is the last frame at the Initiator during remote
                 * ranging, so always exit the ranging procedure.
                 */
                /* Clean-up RTB */
                range_exit();
            }
            break;
#endif  /* ENABLE_RTB_REMOTE */

        default:
#if (DEBUG > 0)
            ASSERT("Unknown RTB command frame.");
#endif
            break;
    }

    buffer_t *rtb_buf = frame->buffer_header;

    /* Release current buffers. */
    bmm_buffer_free(rtb_buf);
} /* range_process_tal_tx_status() */



/** Build ranging command frames */
void range_build_frame(range_cmd_t cmd, frame_info_t *frame)
{
    uint8_t frame_len = 0;
    uint8_t *frame_ptr = NULL;
    uint8_t *temp_frame_ptr = NULL;
    uint16_t fcf = 0;
    wpan_addr_spec_t *src_addr_spec = NULL;
    wpan_addr_spec_t *dst_addr_spec = NULL;

    /* Get the payload pointer. */
    switch (cmd)
    {
        case CMD_RANGE_REQ:
            {
                frame_ptr = temp_frame_ptr =
                                (uint8_t *)frame +
                                LARGE_BUFFER_SIZE -
                                CMD_RANGE_REQ_LEN
                                - 2;    /* Add 2 octets for FCS. */

                build_range_req_frame(frame_ptr);

                frame_ptr = temp_frame_ptr;

                /* Update the length. */
                frame_len = CMD_RANGE_REQ_LEN +
                            2 + // 2 octets for FCS
                            2 + // 2 octets for short source address
                            2 + // 2 octets for short destination address
                            2 + // 2 octets for destination PAN-Id
                            3;  // 3 octets DSN and FCF

                if (rtb_pib.ProvideRangingTransmitPower)
                {
                    /* Add octets for the Requested Ranging Transmit Power. */
                    frame_len += IE_REQ_RANGING_TX_POWER_LEN;
                }

                /* Update the FCF. */
                fcf = FCF_ACK_REQUEST;

                /* Set address specs for further frame MHR completion. */
                src_addr_spec = &range_param.InitiatorAddrSpec;
                dst_addr_spec = &range_param.ReflectorAddrSpec;

            }
            break;

        case CMD_RANGE_ACPT:
            {
                frame_ptr = temp_frame_ptr =
                                (uint8_t *)frame +
                                LARGE_BUFFER_SIZE -
                                CMD_RANGE_ACPT_LEN
                                - 2;    /* Add 2 octets for FCS. */

                build_range_acpt_frame(frame_ptr);

                frame_ptr = temp_frame_ptr;

                /* Update the length. */
                frame_len = CMD_RANGE_ACPT_LEN +
                            2 + // 2 octets for FCS
                            2 + // 2 octets for short source address
                            2 + // 2 octets for short destination address
                            2 + // 2 octets for destination PAN-Id
                            3;  // 3 octets DSN and FCF

                /* Update the FCF. */
                fcf = FCF_ACK_REQUEST;

                /* Set address specs for further frame MHR completion. */
                src_addr_spec = &range_param.ReflectorAddrSpec;
                dst_addr_spec = &range_param.InitiatorAddrSpec;
            }
            break;

#ifdef ENABLE_RTB_REMOTE
        case CMD_REMOTE_RANGE_REQ:
            {
                frame_ptr = temp_frame_ptr =
                                (uint8_t *)frame +
                                LARGE_BUFFER_SIZE -
                                CMD_REMOTE_RANGE_REQ_LEN
                                - 2;    /* Add 2 octets for FCS. */

                /* Update the length. */
                frame_len = CMD_REMOTE_RANGE_REQ_LEN +
                            2 + // 2 octets for FCS
                            2 + // 2 octets for short source address
                            2 + // 2 octets for short destination address
                            2 + // 2 octets for destination PAN-Id
                            3;  // 3 octets DSN and FCF

                if (rtb_pib.ProvideRangingTransmitPower)
                {
                    /* Add octets for the Requested Ranging Transmit Power. */
                    frame_len += IE_REQ_RANGING_TX_POWER_LEN;
                }

                if (range_param.ReflectorAddrSpec.AddrMode == FCF_LONG_ADDR)
                {
                    /*
                     * Add 6 extra octets since long address is larger
                     * than short address.
                     */
                    frame_ptr -= 6;
                    temp_frame_ptr = frame_ptr;
                    frame_len += 6;
                }

                build_remote_range_req_frame(frame_ptr);

                frame_ptr = temp_frame_ptr;

                /* Update the FCF. */
                fcf = FCF_ACK_REQUEST;

                /* Set address specs for further frame MHR completion. */
                src_addr_spec = &range_param.CoordinatorAddrSpec;
                dst_addr_spec = &range_param.InitiatorAddrSpec;
            }
            break;
#endif  /* ENABLE_RTB_REMOTE */

#ifdef ENABLE_RTB_REMOTE
        case CMD_REMOTE_RANGE_CONF:
            {
                /* Additional length octets to be considered. */
                uint8_t add_length_octets = 0;
                {
                    add_length_octets =
                        range_param_pmu.antenna_measurement_nos *
                        (sizeof(uint32_t) + sizeof(uint8_t));    // distance and DQF
                }

                frame_ptr = temp_frame_ptr =
                                (uint8_t *)frame +
                                LARGE_BUFFER_SIZE -
                                CMD_REMOTE_RANGE_CONF_LEN
                                /* Add enough space for potential antenna diversity. */
                                - add_length_octets
                                - 2;    /* Add 2 octets for FCS. */

                /* Update the length. */
                frame_len = CMD_REMOTE_RANGE_CONF_LEN +
                            2 + // 2 octets for FCS
                            2 + // 2 octets for short source address
                            2 + // 2 octets for short destination address
                            2 + // 2 octets for destination PAN-Id
                            3;  // 3 octets DSN and FCF

                if (range_param.ReflectorAddrSpec.AddrMode == FCF_LONG_ADDR)
                {
                    /*
                     * Add 6 extra octets since long address is larger
                     * than short address.
                     */
                    frame_ptr -= 6;
                    temp_frame_ptr = frame_ptr;
                    frame_len += 6;
                }

                frame_len = build_remote_range_conf_frame(frame_ptr, frame_len);

                frame_ptr = temp_frame_ptr;

                /* Update the FCF. */
                fcf = FCF_ACK_REQUEST;

                /* Set address specs for further frame MHR completion. */
                src_addr_spec = &range_param.InitiatorAddrSpec;
                dst_addr_spec = &range_param.CoordinatorAddrSpec;
            }
            break;
#endif  /* ENABLE_RTB_REMOTE */

        case CMD_PMU_TIME_SYNC_REQ:
            {
                frame_ptr = temp_frame_ptr =
                                (uint8_t *)frame +
                                LARGE_BUFFER_SIZE -
                                CMD_PMU_SYNC_REQ_LEN
                                - 2;    /* Add 2 octets for FCS. */

                build_pmu_time_sync_req_frame(frame_ptr);

                frame_ptr = temp_frame_ptr;

                /* Update the length. */
                frame_len = CMD_PMU_SYNC_REQ_LEN +
                            2 + // 2 octets for FCS
                            2 + // 2 octets for short source address
                            2 + // 2 octets for short destination address
                            2 + // 2 octets for destination PAN-Id
                            3;  // 3 octets DSN and FCF

                /* Set address specs for further frame MHR completion. */
                src_addr_spec = &range_param.InitiatorAddrSpec;
                dst_addr_spec = &range_param.ReflectorAddrSpec;
            }
            break;


        case CMD_PMU_START:
            {
                frame_ptr =
                    (uint8_t *)frame +
                    LARGE_BUFFER_SIZE -
                    CMD_PMU_START_LEN
                    - 2;    /* Add 2 octets for FCS. */

                /* Set ranging command type. */
                *frame_ptr = cmd;

                /* Update the length. */
                frame_len = CMD_PMU_START_LEN +
                            2 + // 2 octets for FCS
                            2 + // 2 octets for short source address
                            2 + // 2 octets for short destination address
                            2 + // 2 octets for destination PAN-Id
                            3;  // 3 octets DSN and FCF

                /* Set address specs for further frame MHR completion. */
                src_addr_spec = &range_param.ReflectorAddrSpec;
                dst_addr_spec = &range_param.InitiatorAddrSpec;
            }
            break;


        case CMD_RESULT_REQ:
            {
                frame_ptr = temp_frame_ptr =
                                (uint8_t *)frame +
                                LARGE_BUFFER_SIZE -
                                CMD_RESULT_REQ_LEN
                                - 2;    /* Add 2 octets for FCS. */

                /* Update the length. */
                frame_len = CMD_RESULT_REQ_LEN +
                            2 + // 2 octets for FCS
                            2 + // 2 octets for short source address
                            2 + // 2 octets for short destination address
                            2 + // 2 octets for destination PAN-Id
                            3;  // 3 octets DSN and FCF

                build_result_req_frame(frame_ptr);

                frame_ptr = temp_frame_ptr;

                /* Update the FCF. */
                fcf = FCF_ACK_REQUEST;

                /* Set address specs for further frame MHR completion. */
                src_addr_spec = &range_param.InitiatorAddrSpec;
                dst_addr_spec = &range_param.ReflectorAddrSpec;
            }
            break;


        case CMD_RESULT_CONF:
            {
                uint16_t result_values_to_be_sent;

                frame_ptr =
                    (uint8_t *)frame +
                    LARGE_BUFFER_SIZE -
                    CMD_RESULT_CONF_LEN
                    - 2;    /* Add 2 octets for FCS. */

                result_values_to_be_sent = pmu_get_no_of_results_to_be_sent();
                if (result_values_to_be_sent > MAX_RESULT_VALUES_PER_FRAME)
                {
                    result_values_to_be_sent = MAX_RESULT_VALUES_PER_FRAME;
                }

                /*
                 * Adjust frame pointer with number of result values
                 * to be sent.
                 */
                frame_ptr -= result_values_to_be_sent;
                temp_frame_ptr = frame_ptr;

                build_result_conf_frame(frame_ptr, result_values_to_be_sent);

                frame_ptr = temp_frame_ptr;

                /* Update the length. */
                frame_len = CMD_RESULT_CONF_LEN +
                            result_values_to_be_sent +   // Number of result values to be sent
                            2 + // 2 octets for FCS
                            2 + // 2 octets for short source address
                            2 + // 2 octets for short destination address
                            2 + // 2 octets for destination PAN-Id
                            3;  // 3 octets DSN and FCF

                /* Update the FCF. */
                fcf = FCF_ACK_REQUEST;

                /* Set address specs for further frame MHR completion. */
                src_addr_spec = &range_param.ReflectorAddrSpec;
                dst_addr_spec = &range_param.InitiatorAddrSpec;
            }
            break;

        default:
            break;
    }

    /* Set RTB frame identifier. */
    frame_ptr--;
    *frame_ptr-- = RTB_FRAME_ID_3;
    *frame_ptr-- = RTB_FRAME_ID_2;
    *frame_ptr = RTB_FRAME_ID_1;

    /* Continue with MHR portion which is identical for all ranging frames. */
    /* Source address */
    if (src_addr_spec->AddrMode == FCF_SHORT_ADDR)
    {
        frame_ptr -= 2;
        convert_16_bit_to_byte_array(src_addr_spec->Addr.short_address, frame_ptr);
    }
    else
    {
        frame_ptr -= 8;
        frame_len += 6; // Add further 6 octets for long source Address
        convert_64_bit_to_byte_array(src_addr_spec->Addr.long_address, frame_ptr);
    }

    /* Source PAN-Id */
    /* Shall the Intra-PAN bit set? */
    if (src_addr_spec->PANId == dst_addr_spec->PANId)
    {
        /*
         * Both address are present and both PAN-Ids are identical.
         * Set intra-PAN bit.
         */
        fcf |= FCF_PAN_ID_COMPRESSION;
    }
    else
    {
        /* Set Source PAN-Id. */
        frame_ptr -= 2;
        frame_len += 2;
        convert_16_bit_to_byte_array(src_addr_spec->PANId, frame_ptr);
    }

    /* Destination address */
    if (dst_addr_spec->AddrMode == FCF_SHORT_ADDR)
    {
        frame_ptr -= 2;
        convert_16_bit_to_byte_array(dst_addr_spec->Addr.short_address, frame_ptr);
    }
    else
    {
        frame_ptr -= 8;
        frame_len += 6; // Add further 6 octets for long destination Address
        convert_64_bit_to_byte_array(dst_addr_spec->Addr.long_address, frame_ptr);
    }

    /* Destination PAN-Id */
    frame_ptr -= 2;
    convert_16_bit_to_byte_array(dst_addr_spec->PANId, frame_ptr);

    /* Set DSN. */
    frame_ptr--;
#ifdef RTB_WITHOUT_MAC
    /* No DSN available, simply set to zero. */
    *frame_ptr = 0;
#else   /* #ifdef RTB_WITHOUT_MAC */
    uint8_t temp_dsn = mac_pib.mac_DSN;
    *frame_ptr = temp_dsn++;
    mlme_set(macDSN, (pib_value_t *)&temp_dsn, false);
#endif  /* #ifdef RTB_WITHOUT_MAC */

    /* Update and set the FCF. */
    frame_ptr -= 2;
    fcf |= FCF_SET_FRAMETYPE(FCF_FRAMETYPE_DATA);

    /* Set FCFs address mode */
    fcf |= FCF_SET_SOURCE_ADDR_MODE(src_addr_spec->AddrMode);
    fcf |= FCF_SET_DEST_ADDR_MODE(dst_addr_spec->AddrMode);

    convert_spec_16_bit_to_byte_array(fcf, frame_ptr);

    /* First element shall be length of PHY frame. */
    frame_ptr--;
    *frame_ptr = frame_len;

    /* Finished building of frame. */
    frame->mpdu = frame_ptr;
}



static void build_pmu_time_sync_req_frame(uint8_t *curr_frame_ptr)
{
    /* Set ranging command type. */
    *curr_frame_ptr++ = CMD_PMU_TIME_SYNC_REQ;

    /* Set RTB Protocol Version. */
    *curr_frame_ptr++ = RTB_PROTOCOL_VERSION_01;
}



#ifdef ENABLE_RTB_REMOTE
static uint8_t build_remote_range_conf_frame(uint8_t *curr_frame_ptr,
                                             uint8_t curr_frame_len)
{
    /* Set ranging command type. */
    *curr_frame_ptr++ = CMD_REMOTE_RANGE_CONF;

    /* Set Reflector address specification. */
    /* Reflector Address Mode */
    *curr_frame_ptr++ = range_param.ReflectorAddrSpec.AddrMode;
    /* Reflector PAN-Id */
    convert_16_bit_to_byte_array(range_param.ReflectorAddrSpec.PANId, curr_frame_ptr);
    curr_frame_ptr += 2;
    /* Reflector Address */
    if (range_param.ReflectorAddrSpec.AddrMode == FCF_SHORT_ADDR)
    {
        convert_16_bit_to_byte_array(range_param.ReflectorAddrSpec.Addr.short_address, curr_frame_ptr);
        curr_frame_ptr += 2;
    }
    else
    {
        convert_64_bit_to_byte_array(range_param.ReflectorAddrSpec.Addr.long_address, curr_frame_ptr);
        curr_frame_ptr += 8;
    }

    /*
     * Check current error status whether the requested
     * remote ranging procedure is accepted.
     */
    if (RANGE_OK == range_status.range_error)
    {
        /* The ranging request from the Initiator is accepted. */
        /* Set status accepted. */
        *curr_frame_ptr++ = RTB_SUCCESS;

        /* Range reject reason is dont't care here. */
        *curr_frame_ptr++ = 0;

        /* Set ranging results. */
        convert_32_bit_to_byte_array(range_status.distance_cm, curr_frame_ptr);
        curr_frame_ptr += 4;
        *curr_frame_ptr++ = range_status.dqf;
    }
    else
    {
        /* The ranging request from the Initiator is NOT accepted. */
        /* Set status rejected. */
        *curr_frame_ptr++ = RTB_REJECT;

        /* Set the corresponding reject reason. */
        *curr_frame_ptr++ = range_status.range_error;

        /* Set ranging results indicating an error. */
        convert_32_bit_to_byte_array(INVALID_DISTANCE, curr_frame_ptr);    // distance
        curr_frame_ptr += 4;
        *curr_frame_ptr++ = DQF_ZERO; // DQF
    }

    /*
     * Check whether provisioning of antenna diversity results
     * was requested and add the actual measurement pairs
     * consisting of distance and dqf if required.
     */
    if (((range_param.remote_caps & PMU_REM_CAP_PROV_ANT_DIV_RES) != 0) &&
        (range_param_pmu.antenna_measurement_nos > 1))
    {
        /*
         * Further antenna diversity measurement result shall be provided
         * AND there actually are more than one measurement results available
         * (i.e. one of the two nodes uses antenna diversity), so append the
         * available measurement pairs.
         */
        *curr_frame_ptr++ = ANT_DIV_MEAS_RESULTS;    // Length already taken care of
        *curr_frame_ptr++ = range_param_pmu.antenna_measurement_nos;
        curr_frame_len += 1;

        for (uint8_t i = 0; i < range_param_pmu.antenna_measurement_nos; i++)
        {
            memcpy(curr_frame_ptr,
                   &range_status_pmu.measured_distance_cm[i],
                   sizeof(uint32_t));
            curr_frame_ptr += sizeof(uint32_t);
            *curr_frame_ptr++ = range_status_pmu.measured_dqf[i];
            curr_frame_len += sizeof(uint32_t) + sizeof(uint8_t);
        }
    }
    else
    {
        /* No further additional results to be appended. */
        *curr_frame_ptr = NO_ADDITIONAL_RESULTS;
    }

    /* Return calculated length */
    return (curr_frame_len);
}
#endif  /* ENABLE_RTB_REMOTE */



#ifdef ENABLE_RTB_REMOTE
static void build_remote_range_req_frame(uint8_t *curr_frame_ptr)
{
    /* Pointer to Lenght of Range Request Frame octet. */
    uint8_t *ptr_to_len_field;

    /* Set ranging command type. */
    *curr_frame_ptr++ = CMD_REMOTE_RANGE_REQ;

    /* Store pointer to lenght of Range Request Frame octet. */
    ptr_to_len_field = curr_frame_ptr++;
    *ptr_to_len_field = IE_PMU_RANGING_LEN +
                        IE_REFLECTOR_ADDR_SPEC_LEN_MIN;

    /* Set RTB Protocol Version. */
    *curr_frame_ptr++ = RTB_PROTOCOL_VERSION_01;

    /* Set Reflector address specification. */
    /* Reflector Address Mode */
    *curr_frame_ptr++ = range_param.ReflectorAddrSpec.AddrMode;
    /* Reflector PAN-Id */
    convert_16_bit_to_byte_array(range_param.ReflectorAddrSpec.PANId, curr_frame_ptr);
    curr_frame_ptr += 2;

    if (range_param.ReflectorAddrSpec.AddrMode == FCF_SHORT_ADDR)
    {
        convert_16_bit_to_byte_array(range_param.ReflectorAddrSpec.Addr.short_address, curr_frame_ptr);
        curr_frame_ptr += 2;
    }
    else
    {
        convert_64_bit_to_byte_array(range_param.ReflectorAddrSpec.Addr.long_address, curr_frame_ptr);
        curr_frame_ptr += 8;
        /* Update of length field due to Extended Address usage. */
        *ptr_to_len_field += 6;
    }

    /* Set ranging method. */
    *curr_frame_ptr++ = rtb_pib.RangingMethod;
    range_param.method = rtb_pib.RangingMethod;

    /* Set Requested Ranging Transmit Power. */
    range_param.req_tx_power = rtb_pib.RangingTransmitPower;

    /* Set PMU specific parameter for current ranging procedure. */
    range_param_pmu.f_start = rtb_pib.PMUFreqStart;
    range_param_pmu.f_step = rtb_pib.PMUFreqStep;
    range_param_pmu.f_stop = rtb_pib.PMUFreqStop;
    range_param_pmu.apply_min_dist_threshold = rtb_pib.ApplyMinDistThreshold;
    /*
     * The remote capabilities at the Coordinator are set here.
     *
     * Note: The actual capabilities between initator and Reflector
     * are independent from the remote capabilities.
     */
    range_param.remote_caps = 0;

    if (rtb_pib.ProvideAntennaDivResults)
    {
        range_param.remote_caps |= PMU_REM_CAP_PROV_ANT_DIV_RES;
    }

    if (rtb_pib.ApplyMinDistThreshold)
    {
        range_param.remote_caps |= PMU_REM_CAP_APPLY_MIN_DIST_THRSHLD;
    }

    /* Fill frame with PMU parameters. */
    *curr_frame_ptr++ = range_param_pmu.f_start;
    *curr_frame_ptr++ = (range_param_pmu.f_start >> 8);

    *curr_frame_ptr++ = range_param_pmu.f_step;

    *curr_frame_ptr++ = range_param_pmu.f_stop;
    *curr_frame_ptr++ = (range_param_pmu.f_stop >> 8);

    /* Local caps are not used within Remote Range request. */
    *curr_frame_ptr++ = range_param.remote_caps;

    if (rtb_pib.ProvideRangingTransmitPower)
    {
        /* Add octets for the Requested Ranging Transmit Power. */
        *curr_frame_ptr++ = REQ_RANGING_TX_POWER_IE;
        *curr_frame_ptr++ = range_param.req_tx_power;
        /* Update Lenght of Range Request Frame octet. */
        *ptr_to_len_field += IE_REQ_RANGING_TX_POWER_LEN;
    }
}
#endif  /* ENABLE_RTB_REMOTE */



static void build_result_conf_frame(uint8_t *curr_frame_ptr,
                                    uint16_t values_to_be_sent)
{
    /* Set ranging command type. */
    *curr_frame_ptr++ = CMD_RESULT_CONF;

    /* Set IE of requested result data type. */
    *curr_frame_ptr++ = req_result_type;

    /*
     * Set requested antenna combination,
     * i.e. 0, 1, 2, 3, depending on used
     * antenna diversity scheme.
     */
    *curr_frame_ptr++ = range_status_pmu.curr_antenna_measurement_no;

    /* Send number of included result values. */
    *curr_frame_ptr++ = values_to_be_sent;
    *curr_frame_ptr++ = (values_to_be_sent >> 8);

    /*
     * Include correct data depending on the type of result data
     * to be sent.
     */
    pmu_fill_result_data(values_to_be_sent,
                         curr_frame_ptr);
}



static void build_result_req_frame(uint8_t *curr_frame_ptr)
{
    /* Set ranging command type. */
    *curr_frame_ptr++ = CMD_RESULT_REQ;

    /* Set IE of requested result data type. */
    /* Request measured PMU values. */
    *curr_frame_ptr++ = req_result_type;

    /*
     * Set requested antenna combination,
     * i.e. 0, 1, 2, 3, depending on used
     * antenna diversity scheme.
     */
    *curr_frame_ptr++ = range_status_pmu.curr_antenna_measurement_no;

    /* Send initial start address for result exchange. */
    pmu_fill_initial_start_addr(curr_frame_ptr);
}



static void build_range_acpt_frame(uint8_t *curr_frame_ptr)
{
    /* Set ranging command type. */
    *curr_frame_ptr++ = CMD_RANGE_ACPT;

    /*
     * Check current error status whether the requested ranging
     * procedure is accepted.
     */
    if (RANGE_OK == range_status.range_error)
    {
        /* The ranging request from the Initiator is accepted. */
        /* Set status accepted. */
        *curr_frame_ptr++ = RTB_SUCCESS;

        /* Range reject reason is dont't care here. */
        *curr_frame_ptr++ = 0;

        /* Set ranging method. */
        *curr_frame_ptr++ = range_param.method;

        /*
         * Set PMU specific parameter for current ranging
         * procedure.
         */
        *curr_frame_ptr++ = range_param.caps;
    }
    else
    {
        /* The ranging request from the Initiator is NOT accepted. */
        /* Set status rejected. */
        *curr_frame_ptr++ = RTB_REJECT;

        /* Set the corresponding reject reason. */
        *curr_frame_ptr = range_status.range_error;

        /* Don't care about further content of frame. */
    }
}



static void build_range_req_frame(uint8_t *curr_frame_ptr)
{
    /* Pointer to Lenght of Range Request Frame octet. */
    uint8_t *ptr_to_len_field;

    /* Set ranging command type. */
    *curr_frame_ptr++ = CMD_RANGE_REQ;

    /* Store pointer to Lenght of Range Request Frame octet. */
    ptr_to_len_field = curr_frame_ptr++;
    *ptr_to_len_field = IE_PMU_RANGING_LEN;

#ifdef ENABLE_RTB_REMOTE
    if (range_param.CoordinatorAddrSpec.AddrMode == FCF_NO_ADDR)
#endif  /* ENABLE_RTB_REMOTE */
    {
        /*
         * This is the Initiator during a local ranging.
         *
         * The parameter for this ranging procedure are taken
         * purely from the Initiator's PIB attributes.
         */

        /*
         * Note: In case of remote ranging,
         * the parameter for this ranging procedure have already
         * been extracted and stored at reception of the
         * Remote Range Request frame from the Coordinator,
         * so the PIB attributes are not used.
         */

        /* Store ranging method. */
#ifdef RTB_WITHOUT_MAC
        range_param.method = RTB_TYPE;
#else
        range_param.method = rtb_pib.RangingMethod;
#endif  /* #ifdef RTB_WITHOUT_MAC */

        /* Set Requested Ranging Transmit Power. */
        range_param.req_tx_power = rtb_pib.RangingTransmitPower;

        /* Set PMU specific parameter for current ranging procedure. */
        range_param_pmu.f_start = rtb_pib.PMUFreqStart;
        range_param_pmu.f_step = rtb_pib.PMUFreqStep;
        range_param_pmu.f_stop = rtb_pib.PMUFreqStop;
        range_param_pmu.apply_min_dist_threshold =
            rtb_pib.ApplyMinDistThreshold;
        SET_INITIATOR_CAPS(range_param.caps);
    }

    /* Set RTB Protocol Version. */
    *curr_frame_ptr++ = RTB_PROTOCOL_VERSION_01;

    /* Set ranging method. */
    *curr_frame_ptr++ = range_param.method;

    /* Fill frame with PMU parameters. */
    *curr_frame_ptr++ = range_param_pmu.f_start;
    *curr_frame_ptr++ = (range_param_pmu.f_start >> 8);

    *curr_frame_ptr++ = range_param_pmu.f_step;

    *curr_frame_ptr++ = range_param_pmu.f_stop;
    *curr_frame_ptr++ = (range_param_pmu.f_stop >> 8);

    *curr_frame_ptr++ = range_param.caps;

    if (rtb_pib.ProvideRangingTransmitPower)
    {
        /* Add octets for the Requested Ranging Transmit Power. */
        *curr_frame_ptr++ = REQ_RANGING_TX_POWER_IE;
        *curr_frame_ptr++ = range_param.req_tx_power;
        /* Update Lenght of Range Request Frame octet. */
        *ptr_to_len_field += IE_REQ_RANGING_TX_POWER_LEN;
    }
}

#endif /* #ifdef ENABLE_RTB */

/* EOF */
