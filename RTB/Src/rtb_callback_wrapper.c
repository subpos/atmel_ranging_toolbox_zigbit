/**
 * @file rtb_callback_wrapper.c
 *
 * @brief Wrapper code for RTB callback functions on MAC level.
 *
 * $Id: rtb_callback_wrapper.c 33203 2012-08-23 13:59:03Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2011, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

#ifdef ENABLE_RTB

/* === Includes ============================================================ */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pal.h"
#include "tal.h"
#include "return_val.h"
#include "bmm.h"
#include "qmm.h"
#include "rtb_msg_const.h"
#include "rtb_api.h"
#include "rtb_msg_types.h"

/* === Macros ============================================================== */


/* === Globals ============================================================= */


/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

/**
 * @brief Wrapper function for messages of type rtb_range_conf_t
 *
 * This function is a callback for rtb range confirm.
 *
 * @param m Pointer to message structure
 */
void rtb_range_conf(uint8_t *msg)
{
    rtb_range_conf_t *pmsg;
    usr_rtb_range_conf_t *purrc;

    /* Get the buffer body from buffer header */
    pmsg = (rtb_range_conf_t *)BMM_BUFFER_POINTER(((buffer_t *)msg));

    purrc = (usr_rtb_range_conf_t *)(&(pmsg->range_conf.ranging_type));

    usr_rtb_range_conf(purrc);

    /* Free the buffer */
    bmm_buffer_free((buffer_t *)msg);
}



/**
 * @brief Wrapper function for messages of type rtb_reset_conf_t
 *
 * This function is a callback for rtb reset confirm.
 *
 * @param m Pointer to message structure
 */
#ifndef RTB_WITHOUT_MAC
void rtb_reset_conf(uint8_t *msg)
{
    rtb_reset_conf_t *pmsg;
    usr_rtb_reset_conf_t *purrc;

    /* Get the buffer body from buffer header */
    pmsg = (rtb_reset_conf_t *)BMM_BUFFER_POINTER(((buffer_t *)msg));

    purrc = (usr_rtb_reset_conf_t *)(&(pmsg->reset_conf));

    usr_rtb_reset_conf(purrc);

    /* Free the buffer */
    bmm_buffer_free((buffer_t *)msg);
}
#endif  /* #ifndef RTB_WITHOUT_MAC */



/**
 * @brief Wrapper function for messages of type rtb_set_conf_t
 *
 * This function is a callback for rtb set confirm.
 *
 * @param m Pointer to message structure
 */
void rtb_set_conf(uint8_t *msg)
{
    rtb_set_conf_t *pmsg;
    usr_rtb_set_conf_t *pursc;

    /* Get the buffer body from buffer header */
    pmsg = (rtb_set_conf_t *)BMM_BUFFER_POINTER(((buffer_t *)msg));

    pursc = (usr_rtb_set_conf_t *)(&(pmsg->set_conf));

    usr_rtb_set_conf(pursc);

    /* Free the buffer */
    bmm_buffer_free((buffer_t *)msg);
}



#ifndef RTB_WITHOUT_MAC
/**
 * @brief Wrapper function for messages of type rtb_pmu_validity_ind_t
 *
 * This function is a callback for rtb pmu validity indication.
 *
 * @param m Pointer to message structure
 */
void rtb_pmu_validitiy_ind(uint8_t *msg)
{
    rtb_pmu_validity_ind_t *pmsg;
    usr_rtb_pmu_validity_ind_t *purpv;

    /* Get the buffer body from buffer header */
    pmsg = (rtb_pmu_validity_ind_t *)BMM_BUFFER_POINTER(((buffer_t *)msg));

    purpv = (usr_rtb_pmu_validity_ind_t *)(&(pmsg->pmu_validity));

    usr_rtb_pmu_validity_ind(purpv);

    /* Free the buffer */
    bmm_buffer_free((buffer_t *)msg);
}
#endif  /* #ifndef RTB_WITHOUT_MAC */

#endif  /* #ifdef ENABLE_RTB */

/* EOF */

