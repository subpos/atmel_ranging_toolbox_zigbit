/**
 * @file rtb_pib.c
 *
 * @brief Implements the RTB PIB attribute handling.
 *
 * $Id: rtb_pib.c 34339 2013-02-22 10:11:19Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2011, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

#if (defined ENABLE_RTB) || (defined ENABLE_RH)

/* === Includes ============================================================ */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pal.h"
#include "return_val.h"
#include "bmm.h"
#include "qmm.h"
#include "tal.h"
#include "ieee_const.h"
#include "rtb_pib.h"
#ifdef ENABLE_RH
#   include "mac_internal.h"    /* Required for mac_pib */
#else
#   include "rtb_msg_types.h"
#   include "rtb_internal.h"
#endif  /* #ifndef ENABLE_RH */

/* === Macros ============================================================== */


/* === Globals ============================================================= */

/** Holds the values of all RTB related PIB attributes. */
rtb_pib_t rtb_pib;

/* Size constants for RTB PIB attributes */
static FLASH_DECLARE(uint8_t rtb_pib_size[]) =
{
    sizeof(bool),           // RTB_NATIVE_PIB_START + 0x00: RTB_PIB_RANGING_ENABLED
    sizeof(uint8_t),        // RTB_NATIVE_PIB_START + 0x01: RTB_PIB_RANGE_METHOD
    sizeof(uint16_t),       // RTB_NATIVE_PIB_START + 0x02: RTB_PIB_PMU_FREQ_START
    sizeof(uint8_t),        // RTB_NATIVE_PIB_START + 0x03: RTB_PIB_PMU_FREQ_STEP
    sizeof(uint16_t),       // RTB_NATIVE_PIB_START + 0x04: RTB_PIB_PMU_FREQ_STOP
    sizeof(uint8_t),        // RTB_NATIVE_PIB_START + 0x05: RTB_PIB_PMU_VERBOSE_LEVEL
    sizeof(bool),           // RTB_NATIVE_PIB_START + 0x06: RTB_PIB_DEFAULT_ANTENNA
    sizeof(bool),           // RTB_NATIVE_PIB_START + 0x07: RTB_PIB_ENABLE_ANTENNA_DIV
    sizeof(bool),           // RTB_NATIVE_PIB_START + 0x08: RTB_PIB_PROVIDE_ANTENNA_DIV_RESULTS
    sizeof(uint8_t),        // RTB_NATIVE_PIB_START + 0x09: RTB_PIB_RANGING_TX_POWER
    sizeof(bool),           // RTB_NATIVE_PIB_START + 0x0A: RTB_PIB_PROVIDE_RANGING_TX_POWER
    sizeof(bool),           // RTB_NATIVE_PIB_START + 0x0B: RTB_PIB_APPLY_MIN_DIST_THRESHOLD
};

/* Update this once the array rtb_pib_size is updated. */
#define MIN_RTB_PIB_ATTRIBUTE_ID        (RTB_PIB_RANGING_ENABLED)
#define MAX_RTB_PIB_ATTRIBUTE_ID        (RTB_PIB_APPLY_MIN_DIST_THRESHOLD)

/* === Prototypes ========================================================== */

extern uint8_t limit_tx_pwr(uint8_t tal_pib_TransmitPower);

/* === Implementation ====================================================== */

/**
 * @brief Setting of RTB PIB attributes via functional access
 *
 * In case the highest stack layer is above RTB (e.g. NWK or even
 * higher), it is not efficient to change PIB attributes using
 * the standard request / confirm primitive concept via the NHLE_RTB
 * queue. In order to allow a more efficient way to change PIB attributes
 * residing in RTB or TAL, this function replaces the standard primitive
 * access via a functional interface.
 *
 * An additional parameter allows for forcing the transceiver back to sleep
 * after PIB setting. Otherwise the transceiver will stay awake (if it has been
 * woken up before).
 * This enables the higher layer to change several PIB attributes without
 * waking up the transceiver and putting it back to sleep several times.
 *
 * @param attribute PIB attribute to be set
 * @param attribute_value Attribute value to be set
 * @param set_trx_to_sleep Set TRX back to sleep after this PIB access if it was
 *        before starting this TRX access. Other the transceiver state will
 *        remain as it is, i.e. in case the transceiver was woken up, it stays
 *        awake.
 *        The default value for just changing one variable is true, i.e. the
 *        transceiver will put back to sleep if it has been woken up.
 *
 * @return Status of the attempt to set the RTB PIB attribute:
 *         RTB_UNSUPPORTED_ATTRIBUTE if the PIB attribute was not found
 *         RTB_SUCCESS if the attempt to set the PIB attribute was successful
 *         TAL_BUSY if the TAL is not in an idle state to change PIB attributes
 */
retval_t rtb_set(uint8_t attribute, pib_value_t *attribute_value, bool set_trx_to_sleep)
{
    /*
     * Variables indicates whether the transceiver has been woken up for
     * setting a PIB attribute.
     */
    static bool trx_pib_wakeup;

    retval_t status = RTB_SUCCESS;

    switch (attribute)
    {
        case RTB_PIB_RANGING_ENABLED:
            rtb_pib.RangingEnabled = attribute_value->pib_value_bool;
            break;

#if (!defined RTB_WITHOUT_MAC) && (!defined ENABLE_RH)
        case RTB_PIB_RANGE_METHOD:
            /*
             * This must not be changed, unless another ranging method than
             * PMU has been implemented.
             */
            status = MAC_READ_ONLY;
            break;
#endif  /* #if (!defined RTB_WITHOUT_MAC) && (!defined ENABLE_RH) */

        case RTB_PIB_PMU_FREQ_START:
            if (attribute_value->pib_value_16bit >=
                rtb_pib.PMUFreqStop - PMU_STEP_FREQ_MAX_IN_MHZ)
            {
                /* f_start must be smaller than f_stop - PMU_STEP_FREQ_MAX_IN_MHZ */
                status = RTB_INVALID_PARAMETER;
            }
            else if ((attribute_value->pib_value_16bit >= PMU_MIN_FREQ) &&
                     (attribute_value->pib_value_16bit <= PMU_MAX_FREQ))
            {
                rtb_pib.PMUFreqStart = attribute_value->pib_value_16bit;
            }
            else
            {
                status = RTB_INVALID_PARAMETER;
            }
            break;

        case RTB_PIB_PMU_FREQ_STEP:
            if ((PMU_STEP_FREQ_500kHz != attribute_value->pib_value_8bit) &&
                (PMU_STEP_FREQ_1MHz != attribute_value->pib_value_8bit) &&
                (PMU_STEP_FREQ_2MHz != attribute_value->pib_value_8bit) &&
                (PMU_STEP_FREQ_4MHz != attribute_value->pib_value_8bit)
               )
            {
                status = RTB_INVALID_PARAMETER;
            }
            else
            {
                rtb_pib.PMUFreqStep = attribute_value->pib_value_8bit;
            }
            break;

        case RTB_PIB_PMU_FREQ_STOP:
            if (attribute_value->pib_value_16bit <=
                rtb_pib.PMUFreqStart + PMU_STEP_FREQ_MAX_IN_MHZ)
            {
                /* f_stop must be larger than f_start + PMU_STEP_FREQ_MAX_IN_MHZ */
                status = RTB_INVALID_PARAMETER;
            }
            else if ((attribute_value->pib_value_16bit >= PMU_MIN_FREQ) &&
                     (attribute_value->pib_value_16bit <= PMU_MAX_FREQ))
            {
                rtb_pib.PMUFreqStop = attribute_value->pib_value_16bit;
            }
            else
            {
                status = RTB_INVALID_PARAMETER;
            }
            break;

#if (!defined RTB_WITHOUT_MAC) && (!defined ENABLE_RH)
        case RTB_PIB_PMU_VERBOSE_LEVEL:
            if (attribute_value->pib_value_8bit <= aRTBMaxPMUVerboseLevel)
            {
                /*
                 * The above comparison requires the type of
                 * rtb_pib.PMUVerboseLevel to be uint8_t, otherwise
                 * an additional check for the variable to be not negative
                 * would be required.
                 */
                rtb_pib.PMUVerboseLevel = attribute_value->pib_value_8bit;
            }
            else
            {
                status = RTB_INVALID_PARAMETER;
            }
            break;
#endif  /* #if (!defined RTB_WITHOUT_MAC) && (!defined ENABLE_RH) */

        case RTB_PIB_DEFAULT_ANTENNA:
            rtb_pib.DefaultAntenna = attribute_value->pib_value_bool;
            break;

#if (ANTENNA_DIVERSITY == 1)
        case RTB_PIB_ENABLE_ANTENNA_DIV:
            /*
             * This PIB attribute can only be changed
             * if antenna diversity is enabled.
             */
            rtb_pib.EnableAntennaDiv = attribute_value->pib_value_bool;
            break;
#endif  /* (ANTENNA_DIVERSITY == 1/0) */

#if (!defined RTB_WITHOUT_MAC) && (!defined ENABLE_RH)
        case RTB_PIB_PROVIDE_ANTENNA_DIV_RESULTS:
            /*
             * This PIB attribute can only be changed
             * if MAC layer is included.
             */
            rtb_pib.ProvideAntennaDivResults = attribute_value->pib_value_bool;
            break;
#endif  /* #if (!defined RTB_WITHOUT_MAC) && (!defined ENABLE_RH) */

        case RTB_PIB_RANGING_TX_POWER:
            rtb_pib.RangingTransmitPower = attribute_value->pib_value_8bit;
            /* Limit rtb_pib.RangingTransmitPower to max/min trx values */
            rtb_pib.RangingTransmitPower = limit_tx_pwr(rtb_pib.RangingTransmitPower);
            break;

        case RTB_PIB_PROVIDE_RANGING_TX_POWER:
            rtb_pib.ProvideRangingTransmitPower = attribute_value->pib_value_bool;
            break;

        case RTB_PIB_APPLY_MIN_DIST_THRESHOLD:
            rtb_pib.ApplyMinDistThreshold = attribute_value->pib_value_bool;
            break;

#ifdef RTB_WITHOUT_MAC
            /*
             * MAC standard PIB attributes residing in the TAL required for the RTB
             * are addressed here and directly forwarded to the TAL.
             */
        case macPANId:
        case macShortAddress:
        case phyCurrentChannel:
        case phyTransmitPower:
        case macIeeeAddress:    //For test purposes only...
            {
                /* Now only TAL PIB attributes are handled anymore. */
                status = tal_pib_set(attribute, attribute_value);

                if (status == TAL_TRX_ASLEEP)
                {
                    /*
                     * Wake up the transceiver and repeat the attempt
                     * to set the TAL PIB attribute.
                     */
                    tal_trx_wakeup();
                    status = tal_pib_set(attribute, attribute_value);
                    if (status == MAC_SUCCESS)
                    {
                        /*
                         * Set flag indicating that the trx has been woken up
                         * during PIB setting.
                         */
                        trx_pib_wakeup = true;
                    }
                }
            }
            break;
#endif  /* #ifdef RTB_WITHOUT_MAC */

        default:
            status = RTB_UNSUPPORTED_ATTRIBUTE;
            break;
    }

    /*
     * In case the transceiver shall be forced back to sleep and
     * has been woken up, it is put back to sleep again.
     */
#ifdef RTB_WITHOUT_MAC
    /* No MAC or higher layer inlcuded */
    if (set_trx_to_sleep && trx_pib_wakeup)
#else   /* #ifdef RTB_WITHOUT_MAC */
    /* MAC or higher layer included */
    if (set_trx_to_sleep && trx_pib_wakeup && !mac_pib.mac_RxOnWhenIdle)
#endif  /* #ifdef RTB_WITHOUT_MAC */
    {
        tal_trx_sleep(SLEEP_MODE_1);
        trx_pib_wakeup = false;
    }

    return status;
}



#ifndef ENABLE_RH
/**
 * @brief Handles an RTB-SET.request primitive
 *
 * This function handles the RTB-SET.request. The RTB-SET.request primitive
 * attempts to write the given value to the indicated PIB attribute.
 *
 * @param msg Pointer to the request structure
 */
void rtb_set_request(uint8_t *msg)
{
    rtb_set_req_t  *rsr = (rtb_set_req_t *)BMM_BUFFER_POINTER((buffer_t *)msg);

    /* Do the actual PIB attribute set operation */
    {
        pib_value_t *attribute_value = &rsr->set_req.PIBAttributeValue;
        retval_t status = RTB_SUCCESS;
        rtb_set_conf_t *rsc;

        /*
         * Call internal PIB attribute handling function. Always force
         * the trx back to sleep when using request primitives via the
         * MLME queue.
         */
        status = rtb_set(rsr->set_req.PIBAttribute, attribute_value, true);

        rsc = (rtb_set_conf_t *)rsr;
        rsc->set_conf.PIBAttribute  = rsr->set_req.PIBAttribute;
        rsc->cmdcode                = RTB_SET_CONFIRM;
        rsc->set_conf.status        = status;
    }

#ifdef RTB_WITHOUT_MAC
    /* Append the rtb set confirmation message to the RTB-NHLE queue */
    qmm_queue_append(&rtb_nhle_q, (buffer_t *)msg);
#else   /* #ifdef RTB_WITHOUT_MAC */
    /* MAC layer is available */
    /* Append the rtb set confirmation message to the MAC-NHLE queue */
    qmm_queue_append(&mac_nhle_q, (buffer_t *)msg);
#endif  /* #ifdef RTB_WITHOUT_MAC */
}
#endif  /* #ifndef ENABLE_RH */



/**
 * @brief Gets the size of a PIB attribute
 *
 * @param attribute PIB attribute
 *
 * @return Size (number of bytes) of the PIB attribute
 */
uint8_t rtb_get_pib_attribute_size(uint8_t pib_attribute_id)
{
#if (defined RTB_WITHOUT_MAC) ||  (defined ENABLE_RH)
    uint8_t return_size = 0;

    /*
     * In case the RTB is the highest stack layer (also valid if
     * Ranging Processor functionality is enabled), the MAC based API function
     * for changing standard MAC PIB attributes is not available.
     * Nevertheless, a number of standard MAC PIB attributes (such as
     * macShortAddress, etc.) is required to be accessed.
     * In order to allow for this access, the RTB based API function is
     * re-used for this.
     * Therefore the MAC based PIB attributes need to be checked here as well.
     */
    if (MIN_RTB_PIB_ATTRIBUTE_ID <= pib_attribute_id && MAX_RTB_PIB_ATTRIBUTE_ID >= pib_attribute_id)
    {
        return(PGM_READ_BYTE(&rtb_pib_size[pib_attribute_id - MIN_RTB_PIB_ATTRIBUTE_ID - RTB_NATIVE_PIB_START]));
    }

    /*
     * Since only a small number of MAC standard PIB attributes (residing in
     * TAL) are used, these PIB attributes are directly checked here.
     * In case this gets too much, this approach needs to be revised later.
     */
    switch (pib_attribute_id)
    {
        case phyCurrentChannel:
        case phyTransmitPower:
            return_size = sizeof(uint8_t);
            break;

        case macPANId:
        case macShortAddress:
            return_size = sizeof(uint16_t);
            break;

        case macIeeeAddress:
            /* For test purposes only... */
            return_size = sizeof(uint64_t);
            break;

        default:
            break;
    }

    return(return_size);

#else   /* #if (defined RTB_WITHOUT_MAC) ||  (defined ENABLE_RH) */
    /*
     * Regular code in case the MAC based API for standard MAC PIB attributes
     * is available.
     * Therefore only RTB based PIB attributes are checked here.
     */
    if (MAX_RTB_PIB_ATTRIBUTE_ID >= pib_attribute_id)
    {
        return(PGM_READ_BYTE(&rtb_pib_size[pib_attribute_id - MIN_RTB_PIB_ATTRIBUTE_ID]));
    }

    return(0);
#endif  /* #ifdef RTB_WITHOUT_MAC */
}

#endif  /* if (defined ENABLE_RTB) || (defined ENABLE_RH) */

/* EOF */
