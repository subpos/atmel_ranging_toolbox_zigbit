/**
 * @file rtb_msg_const.h
 *
 * @brief This file defines all message constants for the RTB.
 *
 * $Id: rtb_msg_const.h 34155 2013-01-30 10:34:16Z sschneid $
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
#ifndef RTB_MSG_CONST_H
#define RTB_MSG_CONST_H

/* === Includes ============================================================= */


/* === Macros =============================================================== */

/**
 * This type contains the service primitives of the RTB-layer.
 */
typedef enum rtb_msg_code_tag
{
    RTB_DATA_INDICATION                 = (0xF0), /**< */

    RTB_RANGE_REQUEST                   = (0xF1), /**< */
#ifndef RTB_WITHOUT_MAC
    RTB_RESET_REQUEST                   = (0xF2), /**< */
#endif  /* #ifndef RTB_WITHOUT_MAC */
    RTB_SET_REQUEST                     = (0xF3), /**< */

    RTB_RANGE_CONFIRM                   = (0xF4), /**< */
#ifndef RTB_WITHOUT_MAC
    RTB_RESET_CONFIRM                   = (0xF5), /**< */
#endif  /* #ifndef RTB_WITHOUT_MAC */
    RTB_SET_CONFIRM                     = (0xF6)  /**< */
#ifndef RTB_WITHOUT_MAC
                                          ,
    RTB_PMU_VALIDITY_INDICATION         = (0xF7)  /**< */
#endif  /* #ifndef RTB_WITHOUT_MAC */
} SHORTENUM rtb_msg_code_t;

/*
 * Bump this when extending the list!
 */
/** First defined RTB message */
#define FIRST_RTB_MESSAGE               (RTB_DATA_INDICATION)
#ifndef RTB_WITHOUT_MAC
/** Last defined RTB message for regular RTB */
#   define LAST_RTB_MESSAGE             (RTB_PMU_VALIDITY_INDICATION)
#else
/** Last defined RTB message for Ranging Processor */
#   define LAST_RTB_MESSAGE             (RTB_SET_CONFIRM)
#endif  /* RTB_WITHOUT_MAC */

#ifndef RTB_WITHOUT_MAC
/* The following defines describe the minimum length of a primitive message. */
/** Minimum length of the \ref RTB_PMU_VALIDITY_INDICATION message. */
#   define RTB_PMU_VALIDITY_IND_LEN     (4)
/** Minimum length of the \ref RTB_RESET_CONFIRM message. */
#   define RTB_RESET_CONF_LEN           (2)
#endif  /* #ifndef RTB_WITHOUT_MAC */
/** Minimum length of the \ref RTB_RANGE_CONFIRM message. */
#ifdef RTB_WITHOUT_MAC
#   define RTB_RANGE_CONF_LEN           (9)
#else /* #ifdef RTB_WITHOUT_MAC */
#   define RTB_RANGE_CONF_LEN           (10)
#endif  /* #ifdef RTB_WITHOUT_MAC */
/** Minimum length of the \ref RTB_SET_CONFIRM message. */
#define RTB_SET_CONF_LEN                (3)

/**
 * Length of the PMU fields in a (Remote) Range Request frame.
 *
 * 1+2: f-start
 * 3: f-step
 * 4+5: f-stop
 * 6: Capabilities
 */
#define IE_PMU_INFO_LEN                 (6)

/**
 * Length of the PMU ranging relevant fields in a (Remote) Range Request frame.
 *
 * 1: RTB Protocol Version
 * 2: Ranging Method (PMU)
 * 3..8. PMU fields
 */
#define IE_PMU_RANGING_LEN              (2 + IE_PMU_INFO_LEN)

/** Length of Requested Ranging Transmit Power IE. */
#define IE_REQ_RANGING_TX_POWER_LEN     (2)


/**
 * Minimum length of Reflector Address Spec IE.
 *
 * 1: Reflector Address Mode
 * 2+3: Reflector PAN-Id
 * 4+5: Reflector Short Address
 */
#define IE_REFLECTOR_ADDR_SPEC_LEN_MIN  (5)

/* === Types ================================================================ */


/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTB_MSG_CONST_H */
/* EOF */
