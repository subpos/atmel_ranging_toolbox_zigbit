/**
 * @file main.c
 *
 * @brief Ranging Toolbox (RTB) Application MACless RTB Demo based on TAL.
 *
 * $Id: main.c 34334 2013-02-22 08:51:48Z sschneid $
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
#include "rtb_api.h"
#include "app_config.h"
#include "ieee_const.h"

/* === Types =============================================================== */


/* === Macros ============================================================== */

/* Current RTB version ID */
#define VERSION "V 1.1.7"

/** Identifier of the network the node belongs to. */
#define DEFAULT_PAN_ID                  (0xCAFE)

/* Addressing during ranging */
/*
 * This address is used by all nodes running this application.
 * The value itself is set on purpose in order to allow for easy acting as
 * Reflector in conjuction with other RTB evaluation applications.
 */
#define DEFAULT_SHORT_ADDR              (0x0002)

#ifndef RADIO_CHANNEL
/* IEEE radio channel for handshake communication. */
#   define RADIO_CHANNEL                (16)
#endif

#if (NO_OF_LEDS >= 3)
#define LED_START                       (LED_0)
#define LED_RANGING_INITIATOR           (LED_1)
#define LED_RANGING_REMOTE              (LED_2)
#elif (NO_OF_LEDS == 2)
#define LED_START                       (LED_0)
#define LED_RANGING_INITIATOR           (LED_0)
#define LED_RANGING_REMOTE              (LED_1)
#else
#define LED_START                       (LED_0)
#define LED_RANGING_INITIATOR           (LED_0)
#define LED_RANGING_REMOTE              (LED_0)
#endif

/* === Globals ============================================================= */

static bool ranging_in_progress = false;
static wpan_rtb_range_req_t wrrr;

/* === Prototypes ========================================================== */

static void configure_pibs(void);
static void handle_range_conf(uint8_t status,
                              uint32_t distance,
                              uint8_t dqf);

/* === Externals =========================================================== */


/* === Implementation ====================================================== */

/**
 * @brief Application task
 *
 * Handler for the application commands received from the SIO.
 *
 * This function is called periodically in the main loop.
 * it processes the SIO input characters.
 */
void app_task()
{
    uint8_t input;

    /* Get input from terminal program / user. */
    input =  sio_getchar_nowait();

    if (input == 'r')
    {
        if (!ranging_in_progress)
        {
            if (wpan_rtb_range_req(&wrrr))
            {
                ranging_in_progress = true;
                pal_led(LED_RANGING_INITIATOR, LED_ON);   // Indicates ranging has started
            }
        }
    }
}


/**
 * Main function of the ranging application.
 */
int main(void)
{
    /* Initialize the RTB layer and its underlying layers, like PAL, TAL, BMM. */
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
    pal_led(LED_START, LED_ON);                 // indicating application is started
    pal_led(LED_RANGING_INITIATOR, LED_OFF);    // indicating ranging has started as initiator
    pal_led(LED_RANGING_REMOTE, LED_OFF);       // indicating remote ranging has started

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

    printf("\nMACless Ranging Toolbox Demo (w/o MAC layer)\n");
    printf("(Library build)\n");
    printf(VERSION"\n");
    printf("!!! Run only 2 nodes !!!\n\n");

    printf("Press 'r' to start ranging\n\n");

    configure_pibs();

    wrrr.InitiatorAddrMode = WPAN_ADDRMODE_SHORT;

    /* Set reflector address spec for later ranging procedures. */
    wrrr.ReflectorAddr = 0;
    wrrr.ReflectorAddrMode = WPAN_ADDRMODE_SHORT;
    ADDR_COPY_DST_SRC_16(wrrr.ReflectorAddr, tal_pib.ShortAddress);
    wrrr.ReflectorPANId = tal_pib.PANId;

    /*
     * Iniator and Reflector address are identical for easy demonstration
     * purpose.
     * This limit the application to 2 nodes and needs to be updated to real
     * addresses in the real application.
     */

    /* Main loop */
    while (1)
    {
        wpan_task();
        app_task();
    }
}



/**
 * @brief Callback function usr_rtb_range_conf
 */
void usr_rtb_range_conf(usr_rtb_range_conf_t *urrc)
{
    if (ranging_in_progress)
    {
        ranging_in_progress = false;

        handle_range_conf(urrc->results.local.status,
                          urrc->results.local.distance,
                          urrc->results.local.dqf);

        pal_led(LED_RANGING_INITIATOR, LED_OFF);    // Indicates ranging has finished
    }
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



static void handle_range_conf(uint8_t status,
                              uint32_t distance,
                              uint8_t dqf)
{
    if (RTB_SUCCESS == status)
    {
        printf("RTB_SUCCESS\n");

        printf("Distance = %"PRIu32" cm\n", distance);
        printf("DQF = %"PRIu8" %%\n\n", dqf);
    }
    else
    {
        /* An error occured during the ranging procedure. */

        /* Human reading oriented formatting. */
        printf("ERROR: 0x%" PRIX8 "\n", status);
    }
}


/**
 * @brief Configure the TAL PIbs
 */
static void configure_pibs(void)
{
    uint16_t temp_value_16;
    uint8_t temp_value_8;

    temp_value_16 = CPU_ENDIAN_TO_LE16(DEFAULT_SHORT_ADDR);
    tal_pib_set(macShortAddress, (pib_value_t *)&temp_value_16);

    temp_value_16 = CPU_ENDIAN_TO_LE16(DEFAULT_PAN_ID);
    tal_pib_set(macPANId, (pib_value_t *)&temp_value_16);

    /* Set channel. */
    temp_value_8 = (uint8_t)RADIO_CHANNEL;
    tal_pib_set(phyCurrentChannel, (pib_value_t *)&temp_value_8);
}

/* EOF */
