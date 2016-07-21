/**
 * @file rtb_msg_types.h
 *
 * @brief This file defines all message structures for the RTB.
 *
 * $Id: rtb_msg_types.h 33204 2012-08-23 14:14:44Z sschneid $
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
#ifndef RTB_MSG_TYPES_H
#define RTB_MSG_TYPES_H

/* === Includes ============================================================= */

#include "rtb_msg_const.h"
#include "rtb_api.h"

/* === Macros =============================================================== */


/* === Types ================================================================ */


/* === RTB-SAP messages ==================================================== */

/**
 * @brief This is the RTB-RANGE.request message structure.
 */
typedef struct rtb_range_req_tag
{
    /** This identifies the message as \ref RTB_RANGE_REQUEST */
    rtb_msg_code_t cmdcode;
    /** The parameters of the current ranging. */
    wpan_rtb_range_req_t range_req;
} rtb_range_req_t;

/**
 * @brief This is the RTB-RANGE.confirm message structure.
 */
typedef struct rtb_range_conf_tag
{
    /** This identifies the message as \ref RTB_RANGE_CONFIRM */
    rtb_msg_code_t cmdcode;
    /** The results of the current ranging. */
    usr_rtb_range_conf_t range_conf;
} rtb_range_conf_t;

#ifndef RTB_WITHOUT_MAC
/**
 * @brief This is the RTB-RESET.request message structure.
 */
typedef struct rtb_reset_req_tag
{
    /** This identifies the message as \ref RTB_RESET_REQUEST */
    rtb_msg_code_t cmdcode;
} rtb_reset_req_t;
#endif  /* #ifndef RTB_WITHOUT_MAC */

#ifndef RTB_WITHOUT_MAC
/**
 * @brief This is the RTB-RESET.confirm message structure.
 */
typedef struct rtb_reset_conf_tag
{
    /** This identifies the message as \ref RTB_RESET_CONFIRM */
    rtb_msg_code_t cmdcode;
    /** The result of the reset operation. */
    usr_rtb_reset_conf_t reset_conf;
} rtb_reset_conf_t;
#endif  /* #ifndef RTB_WITHOUT_MAC */

/**
 * @brief This is the RTB-SET.request message structure.
 */
typedef struct rtb_set_req_tag
{
    /** This identifies the message as \ref RTB_SET_REQUEST */
    rtb_msg_code_t cmdcode;
    /** The parameters of the current RTB PIB attribute set request. */
    wpan_rtb_set_req_t set_req;
} rtb_set_req_t;

/**
 * @brief This is the RTB-SET.confirm message structure.
 */
typedef struct rtb_set_conf_tag
{
    /** This identifies the message as \ref RTB_SET_CONFIRM */
    rtb_msg_code_t cmdcode;
    /** The results of the current RTB PIB attribute set confirm. */
    usr_rtb_set_conf_t set_conf;
} rtb_set_conf_t;

#ifndef RTB_WITHOUT_MAC
/**
 * @brief This is the RTB-PMU-VALIDITIY.indication message structure.
 */
typedef struct rtb_pmu_validity_ind_tag
{
    /** This identifies the message as \ref RTB_PMU_VALIDITY_INDICATION */
    rtb_msg_code_t cmdcode;
    /** The parameters of the current PMU Validity indication. */
    usr_rtb_pmu_validity_ind_t pmu_validity;
} rtb_pmu_validity_ind_t;
#endif  /* #ifndef RTB_WITHOUT_MAC */

/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTB_MSG_TYPES_H */
/* EOF */
