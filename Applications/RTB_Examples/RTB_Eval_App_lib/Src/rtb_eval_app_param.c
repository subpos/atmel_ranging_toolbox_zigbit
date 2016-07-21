/**
 * @file rtb_eval_app_param.c
 *
 * @brief Helper file to store ranging application relevant data into EEPROM
 *
 * This file implements the required functions to store ranging application
 * relevant data into EEPROM.
 *
 * $Id: rtb_eval_app_param.c 34344 2013-02-22 12:13:28Z sschneid $
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

/* === Types ===============================================================*/


/* === Macros ==============================================================*/

#define EEPROM_RECORD_OFFSET (16)

/* Length of buffer for user input. */
#define LENGTH_OF_USER_INPUT_BUF        (18)//(11) //To allow long long int input

/* === Globals =============================================================*/


/* === Prototypes ==========================================================*/


/* === Implementation ======================================================*/

void fill_range_addresses(wpan_rtb_range_req_t *wrrr, bool is_remote)
{
    wrrr->InitiatorAddr = 0;
    wrrr->ReflectorAddr = 0;

    switch (app_data.app_addressing.range_addr_scheme)
    {
        case RANGE_INIT_SHORT_REFL_SHORT:
            {
                wrrr->InitiatorAddrMode = WPAN_ADDRMODE_SHORT;
                if (is_remote)
                {
                    ADDR_COPY_DST_SRC_16(wrrr->InitiatorAddr,
                                         app_data.app_addressing.init_short_addr_for_rem);
                }
                else
                {
                    ADDR_COPY_DST_SRC_16(wrrr->InitiatorAddr,
                                         tal_pib.ShortAddress);
                }

                wrrr->ReflectorAddrMode = WPAN_ADDRMODE_SHORT;
                ADDR_COPY_DST_SRC_16(wrrr->ReflectorAddr,
                                     app_data.app_addressing.refl_short_addr);
            }
            break;

        case RANGE_INIT_SHORT_REFL_LONG:
            {
                wrrr->InitiatorAddrMode = WPAN_ADDRMODE_SHORT;
                if (is_remote)
                {
                    ADDR_COPY_DST_SRC_16(wrrr->InitiatorAddr,
                                         app_data.app_addressing.init_short_addr_for_rem);
                }
                else
                {
                    ADDR_COPY_DST_SRC_16(wrrr->InitiatorAddr,
                                         tal_pib.ShortAddress);
                }

                wrrr->ReflectorAddrMode = WPAN_ADDRMODE_LONG;
                ADDR_COPY_DST_SRC_64(wrrr->ReflectorAddr,
                                     app_data.app_addressing.refl_long_addr);
            }
            break;

        case RANGE_INIT_LONG_REFL_SHORT:
            {
                wrrr->InitiatorAddrMode = WPAN_ADDRMODE_LONG;
                if (is_remote)
                {
                    ADDR_COPY_DST_SRC_64(wrrr->InitiatorAddr,
                                         app_data.app_addressing.init_long_addr_for_rem);
                }
                else
                {
                    ADDR_COPY_DST_SRC_64(wrrr->InitiatorAddr,
                                         tal_pib.IeeeAddress);
                }

                wrrr->ReflectorAddrMode = WPAN_ADDRMODE_SHORT;
                ADDR_COPY_DST_SRC_16(wrrr->ReflectorAddr,
                                     app_data.app_addressing.refl_short_addr);
            }
            break;

        case RANGE_INIT_LONG_REFL_LONG:
        default:
            {
                wrrr->InitiatorAddrMode = WPAN_ADDRMODE_LONG;
                if (is_remote)
                {
                    ADDR_COPY_DST_SRC_64(wrrr->InitiatorAddr,
                                         app_data.app_addressing.init_long_addr_for_rem);
                }
                else
                {
                    ADDR_COPY_DST_SRC_64(wrrr->InitiatorAddr, tal_pib.IeeeAddress);
                }

                wrrr->ReflectorAddrMode = WPAN_ADDRMODE_LONG;
                ADDR_COPY_DST_SRC_64(wrrr->ReflectorAddr,
                                     app_data.app_addressing.refl_long_addr);
            }
            break;

    }
    wrrr->InitiatorPANId = tal_pib.PANId;
    wrrr->ReflectorPANId = tal_pib.PANId;
}



/**
 * Read an integer value from SIO.
 *
 * The procedure is terminated after the ENTER-key is pressed.
 *
 * @return converted integer value.
 */
int get_int(void)
{
    char buf[LENGTH_OF_USER_INPUT_BUF], *p;
    p = buf;
    do
    {
        *p = sio_getchar();
        if (*p == 0x0d || *p == 0x0a || p >= &buf[LENGTH_OF_USER_INPUT_BUF - 1])
        {
            *p = 0;
            break;
        }
        sio_putchar(*p++);
    }
    while (1);
    return atoi(buf);
}

/**
 * Read an integer value from SIO.
 *
 * The procedure is terminated after the ENTER-key is pressed.
 *
 * @return converted integer value.
 */
uint64_t get_longint(void)
{
    char buf[LENGTH_OF_USER_INPUT_BUF], *p;
    
    p = buf;
    do
    {
        *p = sio_getchar();
        if (*p == 0x0d || *p == 0x0a || p >= &buf[LENGTH_OF_USER_INPUT_BUF - 1])
        {
            *p = 0;
            break;
        }
        sio_putchar(*p++);
    }
    while (1);
    
    return atoll(buf);
}

uint64_t atoll(char *instr)
{
  uint64_t retval;
  //int i;

  retval = 0;
  for (; *instr; instr++) {
    retval = 10*retval + (*instr - '0');
  }
  return retval;
}

void print_range_addresses(bool was_remote)
{
    switch (app_data.app_addressing.range_addr_scheme)
    {
        case RANGE_INIT_SHORT_REFL_SHORT:
            {
                if (was_remote)
                {
                    printf(" 0x%" PRIX16,
                           app_data.app_addressing.init_short_addr_for_rem);
                }
                else
                {
                    printf(" 0x%" PRIX16,
                           tal_pib.ShortAddress);
                }
                printf(" 0x%" PRIX16, app_data.app_addressing.refl_short_addr);
            }
            break;

        case RANGE_INIT_SHORT_REFL_LONG:
            {
                if (was_remote)
                {
                    printf(" 0x%" PRIX16,
                           app_data.app_addressing.init_short_addr_for_rem);
                }
                else
                {
                    printf(" 0x%" PRIX16,
                           tal_pib.ShortAddress);
                }
                printf(" 0x%08" PRIX32 "%08" PRIX32,
                       (uint32_t)((app_data.app_addressing.refl_long_addr >> 32) & 0xffffffffUL),
                       (uint32_t)(app_data.app_addressing.refl_long_addr  & 0xffffffffUL));
            }
            break;

        case RANGE_INIT_LONG_REFL_SHORT:
            {
                if (was_remote)
                {
                    printf(" 0x%08" PRIX32 "%08" PRIX32,
                           (uint32_t)((app_data.app_addressing.init_long_addr_for_rem >> 32) & 0xffffffffUL),
                           (uint32_t)(app_data.app_addressing.init_long_addr_for_rem  & 0xffffffffUL));
                }
                else
                {
                    printf(" 0x%08" PRIX32 "%08" PRIX32,
                           (uint32_t)((tal_pib.IeeeAddress >> 32) & 0xffffffffUL),
                           (uint32_t)(tal_pib.IeeeAddress  & 0xffffffffUL));
                }
                printf(" 0x%" PRIX16, app_data.app_addressing.refl_short_addr);
            }
            break;

        case RANGE_INIT_LONG_REFL_LONG:
        default:
            {
                if (was_remote)
                {
                    printf(" 0x%08" PRIX32 "%08" PRIX32,
                           (uint32_t)((app_data.app_addressing.init_long_addr_for_rem >> 32) & 0xffffffffUL),
                           (uint32_t)(app_data.app_addressing.init_long_addr_for_rem  & 0xffffffffUL));
                }
                else
                {
                    printf(" 0x%08" PRIX32 "%08" PRIX32,
                           (uint32_t)((tal_pib.IeeeAddress >> 32) & 0xffffffffUL),
                           (uint32_t)(tal_pib.IeeeAddress  & 0xffffffffUL));
                }
                printf(" 0x%08" PRIX32 "%08" PRIX32,
                       (uint32_t)((app_data.app_addressing.refl_long_addr >> 32) & 0xffffffffUL),
                       (uint32_t)(app_data.app_addressing.refl_long_addr  & 0xffffffffUL));
            }
            break;
    }
}



bool range_load_param(void)
{
    app_data_t stored_app_data;
    bool ret;
    uint8_t *p, i;

    if (FAILURE == pal_ps_get(INTERN_EEPROM, EEPROM_RECORD_OFFSET, sizeof(stored_app_data), &stored_app_data))
    {
        return false;
    }

    uint16_t crc16 = 0;
    p = (uint8_t *) &stored_app_data;

    for (i = 0; i < sizeof(stored_app_data); i++)
    {
        crc16 = CRC_CCITT_UPDATE(crc16, *p++);
    }

    if (crc16 == 0)
    {
        /* Update application data. */
        memcpy(&app_data, &stored_app_data, sizeof(app_data_t));

        /* Set RTB PIB attributes. */
        memcpy(&rtb_pib, &stored_app_data.app_rtb_pib, sizeof(rtb_pib_t));

        /* Set transceiver related PIB attributes. */
        /* Set proper channel for ranging. */
        mlme_set(phyCurrentChannel, (pib_value_t *) & (stored_app_data.app_curr_channel), false);
        /* Set new PAN-Id. */
        mlme_set(macPANId, (pib_value_t *) & (stored_app_data.app_addressing.pan_id), false);
        /* Set new own short address. */
        mlme_set(macShortAddress, (pib_value_t *) & (stored_app_data.app_addressing.own_short_addr), false);

        ret = true;
    }
    else
    {
        ret = false;
    }

    return ret;
}



#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
void range_set_default_addr(node_type_t cur_node_type)
#else
void range_set_default_addr(void)
#endif
{
    app_data.app_addressing.pan_id = DEFAULT_PAN_ID;
    app_data.app_addressing.init_long_addr_for_rem = DEFAULT_INITIATOR_LONG_ADDR_REMOTE;
    app_data.app_addressing.refl_long_addr = DEFAULT_REFLECTOR_LONG_ADDR;
    app_data.app_addressing.range_addr_scheme = RANGE_INIT_SHORT_REFL_SHORT;
	#ifdef ENABLE_RTB_REMOTE
    gate_way_addr_mode = COORDINATOR_SHORT_ADDR;
	#endif

#if (AUTOMATIC_NODE_DETECTION_RTB == 1)
    if (INITIATOR == cur_node_type)
    {
        app_data.app_addressing.own_short_addr = DEFAULT_INITIATOR_SHORT_ADDR_REMOTE;
        app_data.app_addressing.init_short_addr_for_rem = DEFAULT_COORDINATOR_SHORT_ADDR;
        app_data.app_addressing.refl_short_addr = DEFAULT_REFLECTOR_SHORT_ADDR;
    }
    else if (REFLECTOR == cur_node_type)
    {
        app_data.app_addressing.own_short_addr = DEFAULT_REFLECTOR_SHORT_ADDR;
        app_data.app_addressing.init_short_addr_for_rem = DEFAULT_COORDINATOR_SHORT_ADDR;
        app_data.app_addressing.refl_short_addr = DEFAULT_INITIATOR_SHORT_ADDR_REMOTE;
    }
    else    /* Coordinator or General */
    {
        app_data.app_addressing.own_short_addr = DEFAULT_COORDINATOR_SHORT_ADDR;
        app_data.app_addressing.init_short_addr_for_rem = DEFAULT_INITIATOR_SHORT_ADDR_REMOTE;
        app_data.app_addressing.refl_short_addr = DEFAULT_REFLECTOR_SHORT_ADDR;
    }
#else
    app_data.app_addressing.own_short_addr = DEFAULT_COORDINATOR_SHORT_ADDR;
    app_data.app_addressing.init_short_addr_for_rem = DEFAULT_INITIATOR_SHORT_ADDR_REMOTE;
    app_data.app_addressing.refl_short_addr = DEFAULT_REFLECTOR_SHORT_ADDR;
#endif
}



void range_store_param(void)
{
    app_data_t app_data_to_be_stored;
    uint8_t *p, i;


    memcpy(&app_data_to_be_stored, &app_data, sizeof(app_data_t));
    /*
     * Need to udpate current rtb_pib values as well, since they might be
     * not in sync with current app_data.app_rtb_pib values to save code.
     */
    memcpy(&app_data_to_be_stored.app_rtb_pib, &rtb_pib, sizeof(rtb_pib_t));

    /* Update PIB attributes with actual values before storing. */
    app_data_to_be_stored.app_addressing.pan_id = tal_pib.PANId;
    app_data_to_be_stored.app_addressing.own_short_addr = tal_pib.ShortAddress;
    app_data_to_be_stored.app_curr_channel = tal_pib.CurrentChannel;

    p = (uint8_t *) &app_data_to_be_stored;

    app_data_to_be_stored.crc = 0;

    for (i = 0; i < sizeof(app_data_to_be_stored) - sizeof(app_data_to_be_stored.crc); i++)
    {
        app_data_to_be_stored.crc = CRC_CCITT_UPDATE(app_data_to_be_stored.crc, *p++);
    }

    pal_ps_set (EEPROM_RECORD_OFFSET,
                sizeof(app_data_to_be_stored),
                &app_data_to_be_stored);
}



bool set_addr_scheme(void)
{
    int input;

    printf("Ranging Addressing Scheme\n");
    printf("  0: Initiator short addr; Reflector short addr\n");
    printf("  1: Initiator short addr; Reflector long addr\n");
    printf("  2: Initiator long addr; Reflector short addr\n");
    printf("  3: Initiator long addr; Reflector long addr\n");
    printf("Enter new Addressing Scheme [0...3]");
    input = get_int() & 0xFF;
    if (input >= RANGE_INIT_SHORT_REFL_SHORT && input <= RANGE_INIT_LONG_REFL_LONG)
    {
        app_data.app_addressing.range_addr_scheme = (range_addr_scheme_t)input;
        return true;
    }

    return false;
}



bool set_antenna_diversity(void)
{
    int input;

    printf("Antenna Diversity selection [0,1]: ");
    input = get_int();
    if (input >= 0 && input <= 1)
    {
        rtb_set(RTB_PIB_ENABLE_ANTENNA_DIV, (pib_value_t *)&input, false);
        return true;
    }

    return false;
}



bool set_application_min_threshold(void)
{
    int input;

    printf("Apply Minimum Threshold during weighted distance calculation [0,1]: ");
    input = get_int();
    if (input >= 0 && input <= 1)
    {
        rtb_set(RTB_PIB_APPLY_MIN_DIST_THRESHOLD, (pib_value_t *)&input, false);
        return true;
    }

    return false;
}



bool set_channel(void)
{
    int input;

    printf("Channel: ");
    input = get_int();
    if (input >= MIN_CHANNEL && input <= MAX_CHANNEL)
    {
        /* Set proper channel for ranging. */
        mlme_set(phyCurrentChannel, (pib_value_t *)&input, false);
        return true;
    }

    return false;
}



bool set_default_antenna(void)
{
    int input;

    printf("Default Antenna selection (AD disabled only) [0,1]: ");
    input = get_int();
    if (input >= 0 && input <= 1)
    {
        rtb_set(RTB_PIB_DEFAULT_ANTENNA, (pib_value_t *)&input, false);
        return true;
    }

    return false;
}



/* Helper function for updating non-address parameters to default values. */
void set_default_non_addr_param(void)
{
    /* Set default channel. */
    app_data.app_curr_channel = RADIO_CHANNEL;
    /* Set default RTB parameters. */
    memcpy(&app_data.app_rtb_pib, &rtb_pib, sizeof(rtb_pib_t));
    /* Set default filtering length during continuous ranging. */
    app_data.app_filtering_len_cont = DEFAULT_LEN_OF_FILTERING_CONT;
    /* Set default ranging result filter type for continuous ranging. */
    app_data.app_filtering_method_cont = FILT_AVER;
}



bool set_freq_start(void)
{
    int input;

    printf("f_start: ");
    input = get_int();
    rtb_set(RTB_PIB_PMU_FREQ_START, (pib_value_t *)&input, false);
    return true;
}



bool set_filtering_length_cont(void)
{
    int input;

    printf("Filtering Length during continuous Ranging (1...%" PRIu8 "): ",
           (uint8_t)MAX_LEN_OF_FILTERING_CONT);
    input = get_int();
    if ((input < 1) || (input > (int)MAX_LEN_OF_FILTERING_CONT))
    {
        /* In case of erroneous input set filtering length. to 1. */
        input = 1;
    }
    app_data.app_filtering_len_cont = (uint8_t)input;
    return true;
}



bool set_filtering_method_cont(void)
{
    int input;

    printf("Filtering Method for continuous Ranging\n");
    printf("  0: Average of distance and DQF)\n");
    printf("  1: Median of distance and DQF)\n");
    printf("  2: Minimum of distance and DQF)\n");
    printf("  3: Minimum of distance and DQF considerung variance)\n");
    printf("  4: Maximum of distance and DQF)\n");
    printf("Enter new Filtering Method [%" PRIu8 "...%" PRIu8 "]",
           (uint8_t)FILT_AVER,
           (uint8_t)FILT_MAX);
    input = get_int();
    if ((input < FILT_AVER) || (input > FILT_MAX))
    {
        /* In case of erroneous input set filtering method. to average filtering. */
        input = FILT_AVER;
    }
    app_data.app_filtering_method_cont = (filtering_method_t)input;
    return true;
}



bool set_freq_step(void)
{
    int input;

    printf("f_step: ");
    input = get_int();
    if ((input >= 0) && (input <= 3))
    {
        /* Set proper frequence step. */
        rtb_set(RTB_PIB_PMU_FREQ_STEP, (pib_value_t *)&input, false);
        return true;
    }

    return false;
}



bool set_freq_stop(void)
{
    int input;

    printf("f_stop: ");
    input = get_int();
    rtb_set(RTB_PIB_PMU_FREQ_STOP, (pib_value_t *)&input, false);
    return true;
}


#ifdef ENABLE_RTB_REMOTE
bool set_coordinator_addr_mode(void)
{
    int input;

    printf("Coordinator Addressing Mode\n");
    printf("  2: Short addr\n");
    printf("  3: Long addr\n");
    printf("Enter new Coordinator Addressing Mode [2, 3]");
    input = get_int() & 0xFF;
    if ((input == COORDINATOR_SHORT_ADDR) || (input == COORDINATOR_LONG_ADDR))
    {
        gate_way_addr_mode = input;
        return true;
    }

    return false;
}
#endif


bool set_init_long_addr(void)
{
    printf("Initiator Long Address for Remote Ranging [64bit decimal]:");
    app_data.app_addressing.init_long_addr_for_rem = get_longint();
    return true;
}



bool set_init_short_addr(void)
{
    printf("Initiator Short Address for Remote Ranging [16bit decimal]:");
    app_data.app_addressing.init_short_addr_for_rem = get_int() & 0xFFFF;
    return true;
}



bool set_pan_id(void)
{
    int pan_id;

    printf("PAN_Id [16bit decimal]:");
    pan_id = get_int() & 0xFFFF;
    /* Set new PAN-Id. */
    mlme_set(macPANId, (pib_value_t *)&pan_id, false);
    return true;
}



bool set_provisioning_of_results(void)
{
    int input;

    printf("Provide all Measurement Results [0,1]: ");
    input = get_int();
    if (input >= 0 && input <= 1)
    {
        rtb_set(RTB_PIB_PROVIDE_ANTENNA_DIV_RESULTS, (pib_value_t *)&input, false);
        return true;
    }

    return false;
}



bool set_provisioning_of_tx_power(void)
{
    int input;

    printf("Provide Ranging Transmit Power for next Ranging [0,1]: ");
    input = get_int();
    if (input >= 0 && input <= 1)
    {
        rtb_set(RTB_PIB_PROVIDE_RANGING_TX_POWER, (pib_value_t *)&input, false);
        return true;
    }

    return false;
}



bool set_refl_long_addr(void)
{
    printf("Reflector Long Address [64bit decimal]:");
    app_data.app_addressing.refl_long_addr = get_longint();
    return true;
}



bool set_refl_short_addr(void)
{
    printf("Reflector Short Address [16bit decimal]:");
    app_data.app_addressing.refl_short_addr = get_int() & 0xFFFF;
    return true;
}



bool set_short_addr(void)
{
    int short_address;

    printf("Own Short Address [16bit decimal]:");
    short_address = get_int() & 0xFFFF;
    /* Set new own short address. */
    mlme_set(macShortAddress, (pib_value_t *)&short_address, false);
    return true;
}



bool set_transmit_power(void)
{
    int tx_power_dbm;
    uint8_t conv_tx_power;

    printf("Tx Power: (in dBm, e.g. \"-5\"): ");
    tx_power_dbm = get_int();

    /* Set proper Tx power. */
    conv_tx_power = CONV_DBM_TO_phyTransmitPower(tx_power_dbm);
    rtb_set(RTB_PIB_RANGING_TX_POWER,
            (pib_value_t *)&conv_tx_power,
            false);

    return true;
}




bool set_verbose_level(void)
{
    int input;

    printf("Verbose level:");
    input = get_int() & 0xFF;
    if ((input >= 0) && (input <= 1))
    {
        rtb_set(RTB_PIB_PMU_VERBOSE_LEVEL, (pib_value_t *)&input, false);
        return true;
    }

    return false;
}



/* Helper function for actual PIB writing. */
void write_pib(void)
{
    /* Set proper channel for ranging. */
    mlme_set(phyCurrentChannel, (pib_value_t *)&app_data.app_curr_channel, false);

    uint8_t pan_id[2];
    pan_id[0] = (uint8_t)app_data.app_addressing.pan_id;          // low byte
    pan_id[1] = (uint8_t)(app_data.app_addressing.pan_id >> 8);   // high byte
    mlme_set(macPANId, (pib_value_t *)&pan_id, false);

    uint8_t short_addr[2];
    short_addr[0] = (uint8_t)app_data.app_addressing.own_short_addr;          // low byte
    short_addr[1] = (uint8_t)(app_data.app_addressing.own_short_addr >> 8);   // high byte
    mlme_set(macShortAddress, (pib_value_t *)&short_addr, false);
}



/* EOF */
