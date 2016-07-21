/**
 * @file rtb_eval_app.c
 *
 * @brief Ranging Toolbox (RTB) Evaluation Application based on RTB library.
 *
 * $Id: rtb_eval_app.c 34344 2013-02-22 12:13:28Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2011, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === Includes ============================================================ */

#include "rtb_eval_app_param.h"

/* === Types =============================================================== */


/* === Macros ============================================================== */

/* Current RTB version ID */
#define VERSION "V 1.1.7"

#define BUILD_NO "$Id: rtb_eval_app.c 34344 2013-02-22 12:13:28Z sschneid $"

#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
/* Timeout value to change node type during initialization phase. */
#   define NODE_TYPE_CHANGE_US          (1500000)

/* Timeout value to clear all LEDs after node type detection. */
#   define NODE_TYPE_LEDS_OFF_US        (10000000)
#endif  /* #if (AUTOMATIC_NODE_DETECTION_RTB == 1) */

/* === Globals ============================================================= */

app_data_t app_data;
app_state_t app_state = APP_IDLE;
uint8_t gate_way_addr_mode = COORDINATOR_SHORT_ADDR;
static bool load_factory_in_progress = false;
#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
static node_type_t node_type;
#endif  /* #if (AUTOMATIC_NODE_DETECTION_RTB == 1) */
/*
 * Status variable indicating whether continuous ranging is going on or not.
 *
 * This variable is set, once the continuous ranging is started
 * (by pressing 'm'/'M' once in case 'n' > 1).
 *
 * This variable is reset, once the continuous ranging is stopped
 * (by 'm'/'M' a 2nd time).
 */
bool cont_ranging_ongoing = false;
/*
 * Timestamp history array (in ms) for speed calculation during
 * continuous ranging
 */
uint16_t time_history[SPEED_CALC_ARRAY_LEN] = {0, 0};
/* Timestamp history array index */
uint8_t time_history_idx = 0;

char build_string[sizeof(BUILD_NO)] = BUILD_NO;
char build_no[20];
char *build_no_start, *build_no_end;

/* === Prototypes ========================================================== */

static void flash_all_leds(void);
static bool handle_user_input(int user_input);
static void rtb_eval_app_help_menu(void);
static void rtb_eval_app_param_menu(void);
static void rtb_eval_app_task(void);
void timeout_remote_ranging_cb(void *parameter);
#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
static bool button_pressed(void);
static void node_type_selection(void);
static void set_node_type_coordinator(void);
static void set_node_type_initiator(void);
static void set_node_type_reflector(void);
static void timeout_led_off_cb(void *parameter);
static void timeout_update_node_type_cb(void *parameter);
#endif  /* #if (AUTOMATIC_NODE_DETECTION_RTB == 1) */
#if (DEBUG > 0)
static void set_distance_offset(void);
#endif

/* === Externals =========================================================== */

#if (DEBUG > 0)
extern int8_t rtb_dist_offset;
#endif

/* === Implementation ====================================================== */

/**
 * Main function of the RTB Eval App.
 */
int main(void)
{
    /* Initialize the MAC layer and its underlying layers, like PAL, TAL, BMM. */
    if (wpan_init() != MAC_SUCCESS)
    {
        /*
         * Stay here; we need a valid IEEE address.
         * Check kit documentation how to create an IEEE address
         * and to store it into the EEPROM.
         */
        pal_alert();
    }

    /* Initialize LEDs. */
    pal_led_init();
    pal_led(LED_NODE_1, LED_OFF);
    pal_led(LED_NODE_2, LED_OFF);
    pal_led(LED_RANGING_ONGOING, LED_OFF);

    /*
     * The stack is initialized above, hence the global interrupts are enabled
     * here.
     */
    pal_global_irq_enable();

    /* Initialize the serial interface used for communication with terminal program. */
    if (pal_sio_init(SIO_CHANNEL) != MAC_SUCCESS)
    {
        /* Something went wrong during initialization. */
        pal_alert();
    }

#if ((!defined __ICCAVR__) && (!defined __ICCARM__) && (!defined __GNUARM__) && \
     (!defined __ICCAVR32__) && (!defined __AVR32__))
    fdevopen(_sio_putchar, _sio_getchar);
#endif

#if ((defined __ICCAVR32__) || (defined __AVR32__))
    sio_getchar();
#endif

    printf("\nRanging Toolbox Evaluation Application\n");
    printf("(Library build)\n");
    printf(VERSION" (Build no. ");

    /* Search for 1st occurence of space. */
    build_no_start = memchr(build_string, ' ', sizeof(BUILD_NO));
    build_no_start++;

    /* Search for 2nd occurence of space; the build number starts here. */
    build_no_start = memchr(build_no_start, ' ', sizeof(BUILD_NO));
    build_no_start++;

    /* Search for 3rd occurence of space; the build number ends here. */
    build_no_end = memchr(build_no_start, ' ', sizeof(BUILD_NO));

    strncpy(build_no, build_no_start, build_no_end - build_no_start);
    printf("%s)\n\n\n", build_no);
#if (DEBUG > 0)
    printf(" DEBUG BUILD\n\n");
#endif

#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
    /* Button Initialization for node type detection */
    pal_button_init();

    /* Select the node type by pressing the button */
    node_type_selection();

    if (INITIATOR == node_type)
    {
        /* Start timer to select proper node type. */
        printf("Detecting node type\n");
        printf("Release button if node type is correct\n");
        printf(".");

        /*
         * Start timer for stepping through potential node types.
         * In order to save timers, the remote ranging timer is
         * re-used here, since it is mutual exclusive with the
         * real remote ranging.
         */
        pal_timer_start(RANGING_APP_TIMER,
                        NODE_TYPE_CHANGE_US,
                        TIMEOUT_RELATIVE,
                        (FUNC_PTR())timeout_update_node_type_cb,
                        NULL);
    }
    else
    {
        /*
         * Reset the MAC layer to the default values.
         * This request will cause a mlme reset confirm message ->
         * usr_mlme_reset_conf
         */
        wpan_mlme_reset_req(true);
    }
#else   /* (AUTOMATIC_NODE_DETECTION_RTB == 0) */
    /*
     * Reset the MAC layer to the default values.
     * This request will cause a mlme reset confirm message ->
     * usr_mlme_reset_conf
     */
    wpan_mlme_reset_req(true);
#endif  /* (AUTOMATIC_NODE_DETECTION_RTB == 1/0) */

    /* Main loop */
    while (1)
    {
        wpan_task();
        rtb_eval_app_task();
    }
}



/**
 * @brief Application task
 *
 * Handler for the application commands received from the SIO.
 *
 * This function is called periodically in the main loop.
 * it processes the SIO input characters.
 */
void rtb_eval_app_task(void)
{
    bool eeprom_to_be_updated = false;

    if (APP_IDLE == app_state)
    {
        int input = sio_getchar_nowait();

        if (input > -1)
        {
            eeprom_to_be_updated = handle_user_input(input);

            if (eeprom_to_be_updated)
            {
                eeprom_to_be_updated = false;
                range_store_param();
            }

            printf("\n");
        }
    }

    if (cont_ranging_ongoing)
    {
        int input = sio_getchar_nowait();

        if ((input == 'm') || (input == 'M'))
        {
            /*
             * Continuous Ranging was already ongoing
             * and will be stopped now.
             */
            cont_ranging_ongoing = false;

            pal_timer_stop(RANGING_APP_TIMER);
            pal_timer_stop(RANGING_APP_TIMER_CONT_RANGING);

            app_state = APP_IDLE;

            pal_led(LED_RANGING_ONGOING, LED_OFF);
        }
    }

    if (APP_CONT_LOCAL_RANGING_NEXT == app_state)
    {
        // local ranging measurement
        continue_ranging(false,
                         APP_CONT_LOCAL_RANGING_ONGOING);
    }

    if (APP_CONT_REMOTE_RANGING_NEXT == app_state)
    {
        // remote ranging measurement
        continue_ranging(true,
                         APP_CONT_REMOTE_RANGING_ONGOING);
    }
}



/* Helper function to handle user input. */
static bool handle_user_input(int user_input)
{
    bool eeprom_to_be_updated = false;

    switch (user_input)
    {
        case 'n':
            eeprom_to_be_updated = set_filtering_length_cont();
            break;

        case 'f':
            eeprom_to_be_updated = set_filtering_method_cont();
            break;

        case 'm':
            {
                if (app_data.app_filtering_len_cont > 1)
                {
                    /* Continuous Ranging is started now. */
                    cont_ranging_ongoing = true;
                }
                // local ranging measurement
                init_ranging(false);
            }
            break;

        case 'M':
            {
                if (app_data.app_filtering_len_cont > 1)
                {
                    /* Continuous Ranging is started now. */
                    cont_ranging_ongoing = true;
                }
                // remote ranging measurement
                init_ranging(true);
            }
            break;

        case 'h':
            rtb_eval_app_help_menu();
            break;

        case 'p':
            rtb_eval_app_param_menu();
            break;

        case '1':
            eeprom_to_be_updated = set_freq_start();
            break;

        case '2':
            eeprom_to_be_updated = set_freq_step();
            break;

        case '3':
            eeprom_to_be_updated = set_freq_stop();
            break;

#if (DEBUG > 0)
        case 'O':
            set_distance_offset();
            break;
#endif  /* (DEBUG > 0) */

        case 'd':
            eeprom_to_be_updated = set_default_antenna();
            break;

        case 'a':
            eeprom_to_be_updated = set_antenna_diversity();
            break;

        case 'e':
            eeprom_to_be_updated = set_provisioning_of_results();
            break;

        case 'w':
            eeprom_to_be_updated = set_application_min_threshold();
            break;

        case 'c':
            eeprom_to_be_updated = set_channel();
            break;

        case 'P':
            eeprom_to_be_updated = set_pan_id();
            break;

        case 'o':
            eeprom_to_be_updated = set_short_addr();
            break;

        case 'i':
            eeprom_to_be_updated = set_init_short_addr();
            break;

        case 'I':
            eeprom_to_be_updated = set_init_long_addr();
            break;

        case 'r':
            eeprom_to_be_updated = set_refl_short_addr();
            break;

        case 'R':
            eeprom_to_be_updated = set_refl_long_addr();
            break;

        case 'v':
            eeprom_to_be_updated = set_verbose_level();
            break;

        case 's':
            eeprom_to_be_updated = set_addr_scheme();
            break;

        case 'g':
            eeprom_to_be_updated = set_coordinator_addr_mode();
            break;

        case 't':
            eeprom_to_be_updated = set_transmit_power();
            break;

        case 'T':
            eeprom_to_be_updated = set_provisioning_of_tx_power();
            break;

        case 'F':
            {
                printf("Reload factory parameters");
                /* Make sure the stored EEPROM data are NOT loaded later. */
                load_factory_in_progress = true;
                wpan_mlme_reset_req(true);
            }
            break;

        default:
            break;
    }

    return eeprom_to_be_updated;
}



/**
 * @brief Callback function usr_rtb_range_conf
 */
void usr_rtb_range_conf(usr_rtb_range_conf_t *urrc)
{
    if (urrc->ranging_type == RTB_LOCAL_RANGING)
    {
        if (APP_LOCAL_RANGING == app_state)
        {
            app_state = APP_IDLE;

            handle_range_conf(false,    /* Local ranging */
                              urrc->results.local.status,
                              urrc->results.local.distance,
                              urrc->results.local.dqf,
                              urrc->results.local.no_of_provided_meas_pairs,
                              urrc->results.local.provided_meas_pairs);

            pal_led(LED_RANGING_ONGOING, LED_OFF);  // Indicates ranging has finished
        }
        else if (APP_CONT_LOCAL_RANGING_ONGOING == app_state)
        {
            handle_cont_range_conf(urrc->results.local.status,
                                   urrc->results.local.distance,
                                   urrc->results.local.dqf);
        }
    }
    else if (urrc->ranging_type == RTB_REMOTE_RANGING)
    {
        /* Stop application timer for remote ranging. */
        pal_timer_stop(RANGING_APP_TIMER);

        if (APP_REMOTE_RANGING == app_state)
        {
            app_state = APP_IDLE;

            handle_range_conf(true,     /* Remote ranging */
                              urrc->results.remote.status,
                              urrc->results.remote.distance,
                              urrc->results.remote.dqf,
                              urrc->results.remote.no_of_provided_meas_pairs,
                              urrc->results.remote.provided_meas_pairs);

            pal_led(LED_RANGING_ONGOING, LED_OFF);  // Indicates remote ranging has finished
        }
        else if (APP_CONT_REMOTE_RANGING_ONGOING == app_state)
        {
            handle_cont_range_conf(urrc->results.remote.status,
                                   urrc->results.remote.distance,
                                   urrc->results.remote.dqf);
        }
    }
}



/**
 * @brief Callback function usr_mlme_reset_conf
 *
 * @param status Result of the reset procedure
 */
void usr_mlme_reset_conf(uint8_t status)
{
    if (status == MAC_SUCCESS)
    {
        /* Always enable receiver. */
        bool rx_on_when_idle = true;
        mlme_set(macRxOnWhenIdle,
                 (pib_value_t *)&rx_on_when_idle,
                 false);

        /* Reset RTB. */
        wpan_rtb_reset_req();
    }
    else
    {
        // something went wrong; restart
        wpan_mlme_reset_req(true);
    }
}



/**
 * @brief Callback function usr_mlme_set_conf
 *
 * @param status        Result of requested MAC PIB attribute set operation
 * @param PIBAttribute  Updated MAC PIB attribute
 */
void usr_mlme_set_conf(uint8_t status, uint8_t PIBAttribute)
{
    if (status != MAC_SUCCESS)
    {
        /* Something went wrong; restart. */
        wpan_mlme_reset_req(true);
    }

    /* Keep compiler happy. */
    PIBAttribute = PIBAttribute;
}



/**
 * @brief Callback function usr_rtb_reset_conf
 *
 * @param urrc  Pointer to usr_rtb_reset_conf_t result structure.
 */
void usr_rtb_reset_conf(usr_rtb_reset_conf_t *urrc)
{
    if (load_factory_in_progress)
    {
        /* The stored parameters from EEPROM are NOT loaded. */
        load_factory_in_progress = false;

        set_default_non_addr_param();
#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
        range_set_default_addr(GENERAL);
#else
        range_set_default_addr();
#endif  /* (AUTOMATIC_NODE_DETECTION_RTB == 1) */

        write_pib();
        /* Store parameters if changed. */
        range_store_param();
        flash_all_leds();
        return;
    }

#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
    if (GENERAL == node_type)
    {
        /* Attempt to load stored parameters. */
        if (!range_load_param())
        {
            /* No valid parameters available. */
            set_default_non_addr_param();
            /* Set default address values. */
            range_set_default_addr(GENERAL);
        }
        else
        {
            /*
             * Valid parameters already available.
             * Check for any of the pre-defined node types.
             */
            if (RANGE_INIT_SHORT_REFL_SHORT == app_data.app_addressing.range_addr_scheme)
            {
                if (DEFAULT_INITIATOR_SHORT_ADDR_REMOTE == app_data.app_addressing.own_short_addr)
                {
                    set_node_type_initiator();
                }
                else if (DEFAULT_REFLECTOR_SHORT_ADDR == app_data.app_addressing.own_short_addr)
                {
                    set_node_type_reflector();
                }
                else if (DEFAULT_COORDINATOR_SHORT_ADDR == app_data.app_addressing.own_short_addr)
                {
                    set_node_type_coordinator();
                }
                /*
                 * Overwrite default address values in case of any of the
                 * pre-defined node types was already stored to finally have consistent
                 * addressing values.
                 */
                if (GENERAL != node_type)
                {
                    range_set_default_addr(node_type);
                    /*
                     * Start timer to later turn off LEDs again to reuse LEDs
                     * for other purposes.
                     */
                    pal_timer_start(RANGING_APP_TIMER,
                                    NODE_TYPE_LEDS_OFF_US,
                                    TIMEOUT_RELATIVE,
                                    (FUNC_PTR())timeout_led_off_cb,
                                    NULL);
                }
            }
            else
            {
                /* Leave all values as stored. */
            }
        }
    }
    else
    {
        /* Node type is one of Initiator, Reflector, or Coordinator. */
        /* Attempt to load stored parameters. */
        if (!range_load_param())
        {
            /* No valid parameters available. */
            set_default_non_addr_param();
        }
        /* Set node type dependent address values. */
        range_set_default_addr(node_type);
    }

    printf("\nNode type: ");
    if (INITIATOR == node_type)
    {
        printf("Initiator\n");
    }
    else if (REFLECTOR == node_type)
    {
        printf("Reflector\n");
    }
    else if (COORDINATOR == node_type)
    {
        printf("Coordinator\n");
    }
    else
    {
        flash_all_leds();
        printf("General\n");
    }

#else   /* (AUTOMATIC_NODE_DETECTION_RTB == 0) */
    /*
     * In case automatic node detection is disabled, the node type is always
     * "Default" (even if not explicitly set).
     */
    /* Attempt to load stored parameters. */
    if (!range_load_param())
    {
        /* No valid parameters available. */
        set_default_non_addr_param();
        /* Set default address values. */
        range_set_default_addr();
    }
    else
    {
        /* Leave all values as stored. */
    }

    flash_all_leds();
#endif  /* (AUTOMATIC_NODE_DETECTION_RTB == 1/0) */

    write_pib();

    /* Store parameters if changed. */
    range_store_param();

    /* Continuous ranging is currently not going on. */
    cont_ranging_ongoing = false;

    /* Keep compiler happy. */
    urrc = urrc;
}



/**
 * @brief Callback function usr_rtb_set_conf
 *
 * @param ursc Pointer to usr_rtb_set_conf_t result structure.
 */
void usr_rtb_set_conf(usr_rtb_set_conf_t *ursc)
{
    /* Keep compiler happy. */
    ursc = ursc;
}



/**
 * @brief Callback function usr_rtb_pmu_validity_ind
 */
void usr_rtb_pmu_validity_ind(usr_rtb_pmu_validity_ind_t *urpv)
{
    uint8_t i, idx = 0;
    int16_t i_freq;
    uint8_t i_step;
    char line[25];
    uint8_t bit_pos = 0;
    uint8_t octet_no = 0;
    bool validity;

    i_freq = (rtb_pib.PMUFreqStart * 2);
    i_step = (1 << rtb_pib.PMUFreqStep);

    idx = 0;

    printf("[PMU_VALID]\n");
    printf("[ANTENNA_MEASUREMENT_VALUE_%" PRIu8 "]\n", urpv->PMUAntennaMeasurementValue);
    printf("PMU validity vector for antenna measurement value %" PRIu8 "\n",
           urpv->PMUAntennaMeasurementValue);

    for (i = 0; i < urpv->PMUValidityValueNo; i++)
    {
        if (bit_pos == 8)
        {
            bit_pos = 0;
            octet_no++;
        }

        validity = (urpv->PMUValidityValues[octet_no] >> bit_pos) & 0x1;
        bit_pos++;

        line[idx++] = validity ? '1' : '0';

        if ((idx == 5) || (idx == 11) || (idx == 17))
        {
            line[idx++] = ' ';
        }

        if ((idx == 23) || (i == (urpv->PMUValidityValueNo - 1)))
        {
            line[idx++] = '\0';
            printf("% 5d.%1d %s\n", i_freq / 2, (i_freq & 1) * 5, line);
            i_freq +=  20 * i_step;
            idx = 0;
        }
    }

    printf("[PMU_VALID_END]\n");
    printf("\n");
}



/**
 * This function displays the main application help menu.
 */
static void rtb_eval_app_help_menu(void)
{
    printf("\nCommands:\n"
           "  h : Print help\n"
           "\n Ranging Measurements:\n"
           "  m : Run local ranging measurement\n"
           "  M : Run remote ranging measurement\n"
           "\n Parameters:\n"
           "  p : Print ranging parameters\n"
           "  F : Reload factory default parameters\n"
          );
}



/**
 * This function displays the paramter menu.
 */
static void rtb_eval_app_param_menu(void)
{
    printf("\n[PARAM]\nCommunication Parameters:\n");

    printf("  c : Channel = %" PRIu8 " [%" PRIu8 "...%" PRIu8 "]\n",
           tal_pib.CurrentChannel,
           MIN_CHANNEL,
           MAX_CHANNEL);

    printf("  o : Own Short Address = 0x%04" PRIX16 " (%" PRIu16 ")\n",
           tal_pib.ShortAddress, tal_pib.ShortAddress);
    printf("      Own Long Address = 0x%08"PRIX32"%08"PRIX32"\n",
           (uint32_t)((tal_pib.IeeeAddress >> 32) & 0xffffffffUL),
           (uint32_t)(tal_pib.IeeeAddress  & 0xffffffffUL));

    printf("  i : Initiator Short Address for Remote Ranging = 0x%04"PRIX16 " (%" PRIu16 ")\n",
           app_data.app_addressing.init_short_addr_for_rem,
           app_data.app_addressing.init_short_addr_for_rem);
    printf("  I : Initiator Long Address for Remote Ranging = 0x%08"PRIX32"%08"PRIX32"\n",
           (uint32_t)((app_data.app_addressing.init_long_addr_for_rem >> 32) & 0xffffffffUL),
           (uint32_t)(app_data.app_addressing.init_long_addr_for_rem  & 0xffffffffUL));

    printf("  r : Reflector Short Address = 0x%04" PRIX16 " (%" PRIu16 ")\n",
           app_data.app_addressing.refl_short_addr,
           app_data.app_addressing.refl_short_addr);
    printf("  R : Reflector Long Address = 0x%08"PRIX32"%08"PRIX32"\n",
           (uint32_t)((app_data.app_addressing.refl_long_addr >> 32) & 0xffffffffUL),
           (uint32_t)(app_data.app_addressing.refl_long_addr  & 0xffffffffUL));

    printf("  P : PAN_Id = 0x%04" PRIX16 " (%" PRIu16 ")\n",
           tal_pib.PANId, tal_pib.PANId);

    printf("  s : Ranging Addressing Scheme = %" PRIu8 " [0,1,2,3]\n", app_data.app_addressing.range_addr_scheme);
    printf("      (0 - Initiator short address, Reflector short address)\n");
    printf("      (1 - Initiator short address, Reflector long address)\n");
    printf("      (2 - Initiator long address, Reflector short address)\n");
    printf("      (3 - Initiator long address, Reflector long address)\n");

    printf("  g : Coordinator Addressing Mode = %" PRIu8 " [2,3]\n", gate_way_addr_mode);
    printf("      (2 - Short address)\n");
    printf("      (3 - Long address)\n");


    printf("\nRanging Parameters:\n");

    printf("  n : Filtering length during continuous Ranging = %" PRIu8 " [1...%" PRIu8 "]\n",
           app_data.app_filtering_len_cont,
           (uint8_t)MAX_LEN_OF_FILTERING_CONT);

    printf("  f : Filtering method for continuous Ranging = ");
    if (FILT_AVER == app_data.app_filtering_method_cont)
    {
        printf("Average of distance and DQF\n");
    }
    else if (FILT_MEDIAN == app_data.app_filtering_method_cont)
    {
        printf("Median of distance and DQF\n");
    }
    else if (FILT_MIN == app_data.app_filtering_method_cont)
    {
        printf("Min. of distance and DQF\n");
    }
    else if (FILT_MIN_VAR == app_data.app_filtering_method_cont)
    {
        printf("Min. of distance and DQF considerung variance\n");
    }
    else if (FILT_MAX == app_data.app_filtering_method_cont)
    {
        printf("Max. of distance and DQF)\n");
    }
    else
    {
        printf("Undefined Filtering method\n");
    }

    printf("  d : Default Antenna = %"PRIu8" [0,1] (AD disabled only)\n", rtb_pib.DefaultAntenna);

#if (defined(ANTENNA_DIVERSITY) && (ANTENNA_DIVERSITY == 1))
    printf("  a : Antenna Diversity = %"PRIu8" [0,1]\n", rtb_pib.EnableAntennaDiv);
#else
    printf("  a : Antenna Diversity = 0 (feature disabled on this Board/Configuration\n");
#endif

    printf("  e : Provide all Measurement Results = %"PRIu8" [0,1]\n",
           rtb_pib.ProvideAntennaDivResults);

    printf("  w : Apply Minimum Threshold during weighted Distance Calc = %"PRIu8" [0,1]\n",
           rtb_pib.ApplyMinDistThreshold);

    printf("      Ranging Method = %X -> ", rtb_pib.RangingMethod);

    if (rtb_pib.RangingMethod == RTB_PMU_233R)
    {
        printf("PMU based on AT86RF233\n");
    }
    printf("  1 : Frequency Start = %" PRIu16 " MHz [%" PRIu16 "...%" PRIu16 "]\n"
           "  2 : Frequency Step = %" PRIu8 " -> %0.1f MHz [0,1,2,3]\n"
           "  3 : Frequency Stop = %" PRIu16 " MHz [%" PRIu16 "...%" PRIu16 "]\n",
           rtb_pib.PMUFreqStart, (uint16_t)PMU_MIN_FREQ, (uint16_t)PMU_MAX_FREQ,
           rtb_pib.PMUFreqStep, ((1 << rtb_pib.PMUFreqStep) * 0.5),
           rtb_pib.PMUFreqStop, (uint16_t)PMU_MIN_FREQ, (uint16_t)PMU_MAX_FREQ
          );

#if (DEBUG > 0)
    printf("  O : Distance Offset = %" PRId8 " cm\n", rtb_dist_offset);
#else
    printf("      Distance Offset = %" PRId8 " cm\n", DISTANCE_OFFSET);
#endif

    printf("\nMisc. Parameters:\n");
    printf("  v : Verbose = %" PRIu8 " [0...%" PRIu8 "]\n",
           rtb_pib.PMUVerboseLevel, 1);

    printf("\nRadio Parameters:\n");
    /* Print tx power settings */
    {
        int8_t tx_pwr_dbm;
        tx_pwr_dbm = CONV_phyTransmitPower_TO_DBM(rtb_pib.RangingTransmitPower);
        printf("  t : Tx Power during Ranging = %" PRId8 " dBm\n", tx_pwr_dbm);

        printf("  T : Provide Ranging Tx Power for next Ranging = %"PRIu8" [0,1]\n",
               rtb_pib.ProvideRangingTransmitPower);
    }

    printf("[PARAM_END]\n");
}



/**
 * @brief Callback function indicating a timeout of the remote ranging measurement
 */
void timeout_remote_ranging_cb(void *parameter)
{
    if (APP_REMOTE_RANGING == app_state)
    {
        app_state = APP_IDLE;

        handle_range_conf(true,     /* Remote ranging */
                          RTB_TIMEOUT,
                          (uint32_t) - 1, // Invalid distance
                          0,    // DQF
                          0,    // No of provided measurement pairs
                          NULL);

        pal_led(LED_RANGING_ONGOING, LED_OFF);  // Indicates remote ranging has finished
    }
    else if (APP_CONT_REMOTE_RANGING_ONGOING == app_state)
    {
        handle_cont_range_conf(RTB_TIMEOUT,
                               ((uint32_t) - 1),
                               0);
    }

    parameter = parameter; /* Keep compiler happy. */
}



#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
/**
 * @brief Callback function indicating a timeout of node update timer
 */
static void timeout_update_node_type_cb(void *parameter)
{
    if (button_pressed())
    {
        printf(".");

        /* Node type selection continues. */
        switch (node_type)
        {
            case INITIATOR:
                /* Next node type is Reflector. */
                set_node_type_reflector();
                break;

            case REFLECTOR:
                /* Next node type is Coordinator. */
                set_node_type_coordinator();
                break;

            case COORDINATOR:
            default:
                /* Next node type is Initiator. */
                set_node_type_initiator();
                break;
        }

        /* Restart button node update timer. */
        pal_timer_start(RANGING_APP_TIMER,
                        NODE_TYPE_CHANGE_US,
                        TIMEOUT_RELATIVE,
                        (FUNC_PTR())timeout_update_node_type_cb,
                        NULL);
    }
    else
    {
        wpan_mlme_reset_req(true);
    }

    parameter = parameter; /* Keep compiler happy. */
}
#endif



#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
/**
 * @brief Callback function indicating a timeout to turn LEDs off
 */
static void timeout_led_off_cb(void *parameter)
{
    pal_led(LED_NODE_1, LED_OFF);
    pal_led(LED_NODE_2, LED_OFF);

    parameter = parameter; /* Keep compiler happy. */
}
#endif



#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
static void set_node_type_initiator(void)
{
    node_type = INITIATOR;
    pal_led(LED_NODE_1, LED_ON);
    pal_led(LED_NODE_2, LED_OFF);
}


static void set_node_type_reflector(void)
{
    node_type = REFLECTOR;
    pal_led(LED_NODE_1, LED_OFF);
    pal_led(LED_NODE_2, LED_ON);
}



static void set_node_type_coordinator(void)
{
    node_type = COORDINATOR;
    pal_led(LED_NODE_1, LED_ON);
    pal_led(LED_NODE_2, LED_ON);
}
#endif



#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
/**
 * Node selction for General, Coordinator, Initiator, or Reflector
 *
 * @return none
 */
static void node_type_selection(void)
{
    if (button_pressed())
    {
        set_node_type_initiator();  // This will be refined later ...
    }
    else
    {
        node_type = GENERAL;        // Configuration as stored in EEPROM is used
    }
}
#endif



#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
/**
 * Determine if button is pressed
 *
 * @return true if button is pressed, else false
 */
static bool button_pressed(void)
{
    if (pal_button_read(BUTTON_0) == BUTTON_PRESSED)
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif



/**
 * Flash all LEDs to indicate General node type
 */
static void flash_all_leds(void)
{
    for (uint8_t i = 0; i < 5; i++)
    {
        pal_led(LED_NODE_1, LED_ON);
        pal_led(LED_NODE_2, LED_ON);
        pal_led(LED_RANGING_ONGOING, LED_ON);

        pal_timer_delay(50000);

        pal_led(LED_NODE_1, LED_OFF);
        pal_led(LED_NODE_2, LED_OFF);
        pal_led(LED_RANGING_ONGOING, LED_OFF);

        pal_timer_delay(50000);
    }
}



#if (DEBUG > 0)
static void set_distance_offset(void)
{
    int input;

    printf("Distance Offset (in cm): ");
    input = get_int();
    if ((input >= INT8_MIN) && (input <= INT8_MAX))
    {
        rtb_dist_offset = input;
    }
}
#endif  /* (DEBUG > 0) */

/* EOF */
