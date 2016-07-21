/**
 * @file rtb_eval_app_param.h
 *
 * @brief Header file for RTB evaluation application related EEPROM functions
 *
 * $Id: rtb_eval_app_param.h 34339 2013-02-22 10:11:19Z sschneid $
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
#ifndef RTB_EVAL_APP_PARAM_H
#define RTB_EVAL_APP_PARAM_H

/* === Includes ============================================================= */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include "pal.h"
#include "tal.h"
#include "sio_handler.h"
#include "mac_api.h"
#include "rtb_api.h"
#include "app_config.h"
#include "ieee_const.h"
#include "mac_internal.h"
#include "rtb_api.h"

/* === Macros =============================================================== */

/*
 * Check whether automatic node detection via button and LEDs can be used
 * on this board.
 */
#if ((NO_OF_LEDS < 3) || (NO_OF_BUTTONS < 1))
#   define AUTOMATIC_NODE_DETECTION_RTB (0)
#   if (NO_OF_LEDS >= 3)
#       define LED_NODE_1               (LED_0)
#       define LED_NODE_2               (LED_1)
#       define LED_RANGING_ONGOING      (LED_2)
#   elif (NO_OF_LEDS == 2)
#       define LED_NODE_1               (LED_0)
#       define LED_NODE_2               (LED_0)
#       define LED_RANGING_ONGOING      (LED_1)
#   elif (NO_OF_LEDS == 1)
#       define LED_NODE_1               (LED_0)
#       define LED_NODE_2               (LED_0)
#       define LED_RANGING_ONGOING      (LED_0)
#   else
#       error "At least 1 LED required"
#   endif
#else
#   define AUTOMATIC_NODE_DETECTION_RTB (1)
#   if (NO_OF_LEDS >= 3)
#       define LED_NODE_1               (LED_0)
#       define LED_NODE_2               (LED_1)
#       define LED_RANGING_ONGOING      (LED_2)
#   else
#       error "At least 3 LEDs required"
#   endif
#endif

#ifndef RADIO_CHANNEL
/* IEEE radio channel for handshake communication. */
#   define RADIO_CHANNEL                (26)
#endif

/** Identifier of the network the node belongs to. */
#define DEFAULT_PAN_ID                  (0xCAFE)

/* Addressing during regular ranging */
/*
 * This is the Gateyway in a remote ranging procedure,
 * or the Initiator in a local ranging procedure.
 */
#define DEFAULT_COORDINATOR_SHORT_ADDR  (0x0000)

/*
 * This is the Initiator remote ranging procedure.
 */
#define DEFAULT_INITIATOR_LONG_ADDR_REMOTE  (0x000425FFFF175C7DUL)
#define DEFAULT_INITIATOR_SHORT_ADDR_REMOTE (0x0001)

/*
 * This is the reflector in a regular ranging procedure.
 */
#define DEFAULT_REFLECTOR_LONG_ADDR     (0x000425FFFF175C9DUL)
#define DEFAULT_REFLECTOR_SHORT_ADDR    (0x0002)

/* Default transmit power. */
#define DEFAULT_TX_POWER                (-17)

/* Max. filtering length during continuous ranging. */
/*
 * Keep this a value of power of 2 for unexpansive module operations
 * during index calculations.
 */
#define MAX_LEN_OF_FILTERING_CONT              (16U)

/* Default filtering length during continuous ranging. */
#define DEFAULT_LEN_OF_FILTERING_CONT          (5U)

/*
 * Number of values utilzed for speed calculation during continuous
 * ranging.
 */
#define SPEED_CALC_ARRAY_LEN            (2U)

/* Length of speed history array */
#define SPEED_HISTORY_LEN               (4U)

/* Timeout value to watch ongoing remote ranging measurement in us. */
#define REMOTE_RANGING_TIMEOUT_US       (1000000)

/* Timeout value before starting next ranging in continuous ranging in ms. */
#define CONT_RANGING_PERIOD_MS          (100UL)
#if (CONT_RANGING_PERIOD_MS > 200)
#   error "Unreasonable value for ranging period for speed calculation."
#endif
#if (CONT_RANGING_PERIOD_MS > INT16_MAX)
#   error "Value for ranging period too large for timers for speed calculation."
#endif

/* === Types ================================================================ */

/* Ranging application state type */
typedef enum app_state_tag
{
    APP_IDLE,
    APP_LOCAL_RANGING,
    APP_REMOTE_RANGING,
    APP_CONT_LOCAL_RANGING_ONGOING,
    APP_CONT_LOCAL_RANGING_NEXT,
    APP_CONT_REMOTE_RANGING_ONGOING,
    APP_CONT_REMOTE_RANGING_NEXT
} SHORTENUM app_state_t;

#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
typedef enum node_type_tag
{
    /* LED 0    LED 1 */
    GENERAL,    /* off      off   */
    INITIATOR,  /* on       off   */
    REFLECTOR,  /* off      on    */
    COORDINATOR /* on       on    */
} SHORTENUM node_type_t;
#endif

/* Ranging addressging scheme type */
typedef enum range_addr_scheme_tag
{
    RANGE_INIT_SHORT_REFL_SHORT = 0x00,
    RANGE_INIT_SHORT_REFL_LONG = 0x01,
    RANGE_INIT_LONG_REFL_SHORT = 0x02,
    RANGE_INIT_LONG_REFL_LONG = 0x03
} SHORTENUM range_addr_scheme_t;

/* Application relevant addressing type for EEPROM storage */
typedef struct app_addressing_tag
{
    uint16_t pan_id;
    uint16_t own_short_addr;
    uint16_t init_short_addr_for_rem;
    uint64_t init_long_addr_for_rem;
    uint16_t refl_short_addr;
    uint64_t refl_long_addr;
    range_addr_scheme_t range_addr_scheme;
} app_addressing_t;

/* Ranging result filtering type */
typedef enum filtering_method_tag
{
    FILT_AVER = 0,  /**< Average of distance and DQF */
    FILT_MEDIAN,    /**< Median of distance and DQF */
    FILT_MIN,       /**< Minimum of distance and DQF */
    FILT_MIN_VAR,   /**< Minimum of distance and DQF considerung variance */
    FILT_MAX        /**< Maximum of distance and DQF */
} SHORTENUM filtering_method_t;

/* Complete application relevant type for EEPROM storage */
typedef struct app_data_tag
{
    app_addressing_t app_addressing;
    rtb_pib_t app_rtb_pib;
    uint8_t app_curr_channel;
    uint8_t app_filtering_len_cont;
    filtering_method_t app_filtering_method_cont;
    uint16_t crc;
} app_data_t;

/* === Externals ============================================================ */

extern app_data_t app_data;
extern app_state_t app_state;
extern uint8_t gate_way_addr_mode;
extern rtb_pib_t rtb_pib;
extern bool cont_ranging_ongoing;
extern uint16_t time_history[];
extern uint8_t time_history_idx;

/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

    void continue_ranging(bool is_remote,
                          app_state_t next_app_state);
    void continue_ranging_after_timeout_cb(void *parameter);
    void fill_range_addresses(wpan_rtb_range_req_t *wrrr, bool is_remote);
    int get_int(void);
    void handle_range_conf(bool was_remote,
                           uint8_t status,
                           uint32_t distance,
                           uint8_t dqf,
                           uint8_t no_of_provided_meas_pairs,
                           measurement_pair_t *provided_meas_pairs);
    void handle_cont_ranging_res(uint8_t status,
                                 uint32_t distance,
                                 uint8_t dqf);
    void handle_cont_range_conf(uint8_t status,
                                uint32_t distance,
                                uint8_t dqf);
    void init_ranging(bool is_remote);
    void print_range_addresses(bool was_remote);
    void print_status(uint8_t status);
    bool range_load_param(void);
    void range_store_param(void);
#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
    void range_set_default_addr(node_type_t cur_node_type);
#else
    void range_set_default_addr(void);
#endif
    bool set_addr_scheme(void);
    bool set_antenna_diversity(void);
    bool set_channel(void);
    bool set_default_antenna(void);
    void set_default_non_addr_param(void);
    bool set_freq_start(void);
    bool set_freq_step(void);
    bool set_freq_stop(void);
    bool set_coordinator_addr_mode(void);
    bool set_init_long_addr(void);
    bool set_init_short_addr(void);
    bool set_pan_id(void);
    bool set_provisioning_of_results(void);
    bool set_application_min_threshold(void);
    bool set_provisioning_of_tx_power(void);
    bool set_filtering_length_cont(void);
    bool set_filtering_method_cont(void);
    bool set_refl_long_addr(void);
    bool set_refl_short_addr(void);
    bool set_short_addr(void);
    bool set_transmit_power(void);
    bool set_verbose_level(void);
    void timeout_remote_ranging_cb(void *parameter);
    void write_pib(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* RTB_EVAL_APP_PARAM_H */
/* EOF */
