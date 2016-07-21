/**
 * @file main.c
 *
 * @brief  RTB MAC Star Demo based on RTB library
 *
 * This is the source code of a simple Ranging Toobox example for a
 * MAC Star topology application utilizing ranging features
 * based on RTB library.
 *
 * $Id: main.c 33806 2012-11-09 15:53:06Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2011, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* === INCLUDES ============================================================ */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "pal.h"
#include "tal.h"
#include "mac_api.h"
#include "rtb_api.h"
#include "app_config.h"
#include "ieee_const.h"
#include "sio_handler.h"

/* === TYPES =============================================================== */

typedef struct associated_device_tag
{
    uint16_t short_addr;
    uint64_t ieee_addr;
}
/** This type definition of a structure can store the short address and the
 *  extended address of a device.
 */
associated_device_t;

/* === MACROS ============================================================== */

#ifndef RADIO_CHANNEL
/* IEEE radio channel for handshake communication. */
#   define RADIO_CHANNEL                (16)
#endif

#define DEFAULT_CHANNEL_PAGE            (0)

/** Defines the PAN ID of the network. */
#ifdef PAN_ID
#   define DEFAULT_PAN_ID               (PAN_ID)
#else
#   define DEFAULT_PAN_ID               CCPU_ENDIAN_TO_LE16(0xBABE)
#endif
/** Defines the short address of the coordinator. */
#define COORD_SHORT_ADDR                (0x0000)
/** Defines the maximum number of devices the coordinator will handle. */
#define MAX_NUMBER_OF_DEVICES           (2)
/** This is the time period in micro seconds for data transmissions. */
#define RANGING_PERIOD                  (50000)
#define LED_PERIOD                      (5000)

/** Defines the scan duration time. */
#define SCAN_DURATION                   (4)
/** Defines the maximum number of scans before starting own network. */
#define MAX_NUMBER_OF_SCANS             (3)

#define SIZE_OF_PAYLOAD                 (sizeof(uint32_t) + sizeof(uint8_t))

#if (NO_OF_LEDS >= 3)
#define LED_NWK_SETUP                   (LED_0)
#define LED_DATA                        (LED_1)
#define LED_RANGE                       (LED_2)
#elif (NO_OF_LEDS == 2)
#define LED_NWK_SETUP                   (LED_0)
#define LED_DATA                        (LED_0)
#define LED_RANGE                       (LED_1)
#else
#define LED_NWK_SETUP                   (LED_0)
#define LED_DATA                        (LED_0)
#define LED_RANGE                       (LED_0)
#endif

/* === GLOBALS ============================================================= */

/** Number of done network scans */
static uint8_t number_of_scans;
/** This array stores all device related information. */
static associated_device_t device_list[MAX_NUMBER_OF_DEVICES];
/** Number of assigned short addresses at coordinator */
static uint8_t no_of_assigned_short_addr = 0;
/** Number of associated devices at coordinator */
static uint8_t no_of_assoc_devs = 0;
/** Range device counter. */
static uint8_t range_with_dev_no = 0;
/** Structure holding RTB Range Request parameter. */
static wpan_rtb_range_req_t wrrr;
/** Address specifications for Reflector for data transmission. */
static wpan_addr_spec_t refl_addr;
static uint8_t payload[SIZE_OF_PAYLOAD];

/* === PROTOTYPES ========================================================== */

static void app_task(void);
static void app_timer_cb(void *parameter);
static bool assign_new_short_addr(uint64_t addr64, uint16_t *addr16);
static void data_exchange_led_off_cb(void *parameter);
#ifdef SIO_HUB
static void handle_range_conf(uint8_t status, uint32_t distance, uint8_t dqf);
#endif  /* SIO_HUB */
static void network_scan_indication_cb(void *parameter);

/* === IMPLEMENTATION ====================================================== */

/**
 * @brief Main function of the device application
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
    pal_led(LED_NWK_SETUP, LED_OFF);    // indicating network setup or joing
    pal_led(LED_DATA, LED_OFF);         // indicating successfull data transmission
    pal_led(LED_RANGE, LED_OFF);        // indicating ranging procedure
    /*
     * The stack is initialized above, hence the global interrupts are enabled
     * here.
     */
    pal_global_irq_enable();

#ifdef SIO_HUB
    /* Initialize the serial interface used for communication with terminal program */
    if (pal_sio_init(SIO_CHANNEL) != MAC_SUCCESS)
    {
        // something went wrong during initialization
        pal_alert();
    }

#if ((!defined __ICCAVR__) && (!defined __ICCARM__) && (!defined __GNUARM__) && \
     (!defined __ICCAVR32__) && (!defined __AVR32__))
    fdevopen(_sio_putchar, _sio_getchar);
#endif

    printf("\nRTB MAC Star Demo\n\n");
#endif  /* SIO_HUB */

    /*
     * Reset the MAC layer to the default values
     * This request will cause a mlme reset confirm message ->
     * usr_mlme_reset_conf
     */
    wpan_mlme_reset_req(true);

    /* Main loop */
    while (1)
    {
        wpan_task();

        app_task();
    }
}


/**
 * @brief Application task
 */
static void app_task(void)
{

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
        wpan_mlme_scan_req(MLME_SCAN_TYPE_ACTIVE,
                           1UL << RADIO_CHANNEL,
                           SCAN_DURATION,
                           DEFAULT_CHANNEL_PAGE);

#ifdef SIO_HUB
        uint8_t current_channel = (uint8_t)RADIO_CHANNEL;
        printf("Searching for existing networks on channel %" PRIu8 ".\n\n", current_channel);
#endif  /* SIO_HUB */

        // Indicate network scanning by a LED flashing
        pal_timer_start(TIMER_LED_OFF,
                        LED_PERIOD,
                        TIMEOUT_RELATIVE,
                        (FUNC_PTR())network_scan_indication_cb,
                        NULL);
    }
    else
    {
        // something went wrong; restart
        wpan_mlme_reset_req(true);
    }
}


/**
 * @brief Callback function usr_mlme_scan_conf
 *
 * @param status            Result of requested scan operation
 * @param ScanType          Type of scan performed
 * @param ChannelPage       Channel page on which the scan was performed
 * @param UnscannedChannels Bitmap of unscanned channels
 * @param ResultListSize    Number of elements in ResultList
 * @param ResultList        Pointer to array of scan results
 */
void usr_mlme_scan_conf(uint8_t status,
                        uint8_t ScanType,
                        uint8_t ChannelPage,
                        uint32_t UnscannedChannels,
                        uint8_t ResultListSize,
                        void *ResultList)
{
    number_of_scans++;

    if (status == MAC_SUCCESS)
    {
        wpan_pandescriptor_t *coordinator;
        uint8_t i;

        /*
         * Analyze the ResultList.
         * Assume that the first entry of the result list is our coodinator.
         */
        coordinator = (wpan_pandescriptor_t *)ResultList;
        for (i = 0; i < ResultListSize; i++)
        {
            /*
             * Check if the PAN descriptor belongs to our coordinator.
             * Check if coordinator allows association.
             */
            if ((coordinator->LogicalChannel == RADIO_CHANNEL) &&
                (coordinator->ChannelPage == DEFAULT_CHANNEL_PAGE) &&
                (coordinator->CoordAddrSpec.PANId == DEFAULT_PAN_ID) &&
                ((coordinator->SuperframeSpec & ((uint16_t)1 << ASSOC_PERMIT_BIT_POS)) == ((uint16_t)1 << ASSOC_PERMIT_BIT_POS))
               )
            {
#ifdef SIO_HUB
                printf("Network found.\nJoin network.\n\n");
#endif  /* SIO_HUB */
                wpan_mlme_associate_req(coordinator->LogicalChannel,
                                        coordinator->ChannelPage,
                                        &(coordinator->CoordAddrSpec),
                                        WPAN_CAP_ALLOCADDRESS);
                return;
            }

            // Get the next PAN descriptor
            coordinator++;
        }

        /*
         * If here, the result list does not contain our expected coordinator.
         * Let's scan again.
         */
        if (number_of_scans < MAX_NUMBER_OF_SCANS)
        {
            wpan_mlme_scan_req(MLME_SCAN_TYPE_ACTIVE,
                               1UL << RADIO_CHANNEL,
                               SCAN_DURATION,
                               DEFAULT_CHANNEL_PAGE);
        }
        else
        {
#ifdef SIO_HUB
            printf("Our network could be found.\nStart new network.\n\n");
#endif  /* SIO_HUB */
            /*
             * Set the short address of this node.
             * Use: bool wpan_mlme_set_req(uint8_t PIBAttribute,
             *                             void *PIBAttributeValue);
             *
             * This request leads to a set confirm message -> usr_mlme_set_conf
             */
            uint8_t short_addr[2];

            short_addr[0] = (uint8_t)COORD_SHORT_ADDR;          // low byte
            short_addr[1] = (uint8_t)(COORD_SHORT_ADDR >> 8);   // high byte
            wpan_mlme_set_req(macShortAddress, short_addr);
        }
    }
    else if (status == MAC_NO_BEACON)
    {
        /*
         * No beacon is received; no coordiantor is located.
         * Scan again, but used longer scan duration.
         */
        if (number_of_scans < MAX_NUMBER_OF_SCANS)
        {
            wpan_mlme_scan_req(MLME_SCAN_TYPE_ACTIVE,
                               1UL << RADIO_CHANNEL,
                               SCAN_DURATION,
                               DEFAULT_CHANNEL_PAGE);
        }
        else
        {
#ifdef SIO_HUB
            printf("No network found.\nStart new network.\n\n");
#endif  /* SIO_HUB */
            /*
             * Set the short address of this node.
             * Use: bool wpan_mlme_set_req(uint8_t PIBAttribute,
             *                             void *PIBAttributeValue);
             *
             * This request leads to a set confirm message -> usr_mlme_set_conf
             */
            uint8_t short_addr[2];

            short_addr[0] = (uint8_t)COORD_SHORT_ADDR;          // low byte
            short_addr[1] = (uint8_t)(COORD_SHORT_ADDR >> 8);   // high byte
            wpan_mlme_set_req(macShortAddress, short_addr);
        }
    }
    else
    {
        // Something went wrong; restart
        wpan_mlme_reset_req(true);
    }

    /* Keep compiler happy. */
    ScanType = ScanType;
    UnscannedChannels = UnscannedChannels;
    ChannelPage = ChannelPage;
}


/**
 * @brief Callback function indicating network search
 */
static void network_scan_indication_cb(void *parameter)
{
    pal_led(LED_NWK_SETUP, LED_TOGGLE);

    // Re-start led timer again
    pal_timer_start(TIMER_LED_OFF,
                    LED_PERIOD,
                    TIMEOUT_RELATIVE,
                    (FUNC_PTR())network_scan_indication_cb,
                    NULL);

    parameter = parameter; /* Keep compiler happy. */
}


/* === Node starts new network === */


/**
 * @brief Callback function usr_mlme_set_conf
 *
 * @param status        Result of requested PIB attribute set operation
 * @param PIBAttribute  Updated PIB attribute
 */
void usr_mlme_set_conf(uint8_t status, uint8_t PIBAttribute)
{
    if ((status == MAC_SUCCESS) && (PIBAttribute == macShortAddress))
    {
        /*
         * Allow other devices to associate to this coordinator.
         * Use: bool wpan_mlme_set_req(uint8_t PIBAttribute,
         *                             void *PIBAttributeValue);
         *
         * This request leads to a set confirm message -> usr_mlme_set_conf
         */
        uint8_t association_permit = true;

        wpan_mlme_set_req(macAssociationPermit, &association_permit);
    }
    else if ((status == MAC_SUCCESS) && (PIBAttribute == macAssociationPermit))
    {
        /*
         * Start a nonbeacon-enabled network
         * Use: bool wpan_mlme_start_req(uint16_t PANId,
         *                               uint8_t LogicalChannel,
         *                               uint8_t ChannelPage,
         *                               uint8_t BeaconOrder,
         *                               uint8_t SuperframeOrder,
         *                               bool PANCoordinator,
         *                               bool BatteryLifeExtension,
         *                               bool CoordRealignment)
         *
         * This request leads to a start confirm message -> usr_mlme_start_conf
         */
        wpan_mlme_start_req(DEFAULT_PAN_ID,
                            RADIO_CHANNEL,
                            DEFAULT_CHANNEL_PAGE,
                            15, 15,
                            true, false, false);
    }
    else if ((status == MAC_SUCCESS) && (PIBAttribute == macRxOnWhenIdle))
    {
        /* Do nothing special here... */
    }
    else
    {
        // something went wrong; restart
        wpan_mlme_reset_req(true);
    }
}


/**
 * @brief Callback function usr_mlme_start_conf
 *
 * @param status        Result of requested start operation
 */
void usr_mlme_start_conf(uint8_t status)
{
    if (status == MAC_SUCCESS)
    {
#ifdef SIO_HUB
        uint16_t pan_id = DEFAULT_PAN_ID;
        printf("Selected PAN-Id 0x%04" PRIX16 ".\n\n", pan_id);
#endif  /* SIO_HUB */

        // Stop timer used for search indication
        pal_timer_stop(TIMER_LED_OFF);
        pal_led(LED_NWK_SETUP, LED_ON);

        wrrr.InitiatorAddrMode = WPAN_ADDRMODE_SHORT;

        /*
         * Set reflector address spec (except address itself)
         * for later ranging procedures.
         */
        wrrr.ReflectorAddr = 0;
        wrrr.ReflectorAddrMode = WPAN_ADDRMODE_SHORT;
        wrrr.ReflectorPANId = tal_pib.PANId;
        /*
         * Set reflector address spec (except address itself)
         * for data transmissions.
         */
        refl_addr.Addr.long_address = 0;
        refl_addr.AddrMode = WPAN_ADDRMODE_SHORT;
        refl_addr.PANId = tal_pib.PANId;

        /* Reset range device counter. */
        range_with_dev_no = 0;

        /*
         * Set RX on when idle to enable the receiver as default.
         * Use: bool wpan_mlme_set_req(uint8_t PIBAttribute,
         *                             void *PIBAttributeValue);
         *
         * This request leads to a set confirm message -> usr_mlme_set_conf
         */
        bool rx_on_when_idle = true;

        wpan_mlme_set_req(macRxOnWhenIdle, &rx_on_when_idle);

        /*
         * Start a timer that initiates a ranging procedure to all connected
         * devices with the period RANGING_PERIOD.
         */
        pal_timer_start(TIMER_INIT_RANGING,
                        RANGING_PERIOD,
                        TIMEOUT_RELATIVE,
                        (FUNC_PTR())app_timer_cb,
                        NULL);
    }
    else
    {
        // something went wrong; restart
        wpan_mlme_reset_req(true);
    }
}


/**
 * @brief Callback function usr_mlme_associate_ind
 *
 * @param DeviceAddress         Extended address of device requesting association
 * @param CapabilityInformation Capabilities of device requesting association
 */
void usr_mlme_associate_ind(uint64_t DeviceAddress,
                            uint8_t CapabilityInformation)
{
    /*
     * Any device is allowed to join the network
     * Use: bool wpan_mlme_associate_resp(uint64_t DeviceAddress,
     *                                    uint16_t AssocShortAddress,
     *                                    uint8_t status);
     *
     * This response leads to comm status indication -> usr_mlme_comm_status_ind
     * Get the next available short address for this device
     */
    uint16_t associate_short_addr = macShortAddress_def;

    if (assign_new_short_addr(DeviceAddress, &associate_short_addr) == true)
    {
        wpan_mlme_associate_resp(DeviceAddress,
                                 associate_short_addr,
                                 ASSOCIATION_SUCCESSFUL);
    }
    else
    {
        wpan_mlme_associate_resp(DeviceAddress,
                                 associate_short_addr,
                                 PAN_AT_CAPACITY);
    }

    /* Keep compiler happy. */
    CapabilityInformation = CapabilityInformation;
}


/**
 * @brief Callback function usr_mlme_comm_status_ind
 *
 * @param SrcAddrSpec      Pointer to source address specification
 * @param DstAddrSpec      Pointer to destination address specification
 * @param status           Result for related response operation
 */
void usr_mlme_comm_status_ind(wpan_addr_spec_t *SrcAddrSpec,
                              wpan_addr_spec_t *DstAddrSpec,
                              uint8_t status)
{
    if (status == MAC_SUCCESS)
    {
        /*
         * Now the association of the device has been successful and its
         * information, like address, could  be stored.
         * But for the sake of simple handling it has been done
         * during assignment of the short address within the function
         * assign_new_short_addr()
         */
        no_of_assoc_devs = no_of_assigned_short_addr;
    }

    /* Keep compiler happy. */
    SrcAddrSpec = SrcAddrSpec;
    DstAddrSpec = DstAddrSpec;
}


/**
 * @brief Callback function usr_mcps_data_ind
 *
 * @param SrcAddrSpec      Pointer to source address specification
 * @param DstAddrSpec      Pointer to destination address specification
 * @param msduLength       Number of octets contained in MSDU
 * @param msdu             Pointer to MSDU
 * @param mpduLinkQuality  LQI measured during reception of the MPDU
 * @param DSN              DSN of the received data frame.
 * @param Timestamp        The time, in symbols, at which the data were received
 *                         (only if timestamping is enabled).
 */
void usr_mcps_data_ind(wpan_addr_spec_t *SrcAddrSpec,
                       wpan_addr_spec_t *DstAddrSpec,
                       uint8_t msduLength,
                       uint8_t *msdu,
                       uint8_t mpduLinkQuality,
#ifdef ENABLE_TSTAMP
                       uint8_t DSN,
                       uint32_t Timestamp)
#else
                       uint8_t DSN)
#endif  /* ENABLE_TSTAMP */
{
    /*
     * Check RTB frame identifier for RTB frames routed to MAC
     * due to error conditions (No Acks, timeouts, etc.).
     */
    if ((msdu[0] == 'R') &&
        (msdu[1] == 'T') &&
        (msdu[2] == 'B'))
    {
#ifdef SIO_HUB
        printf("RTB frame routed to MAC\n\n");
#endif  /* SIO_HUB */

        return;
    }

    /*
     * Ranging data has been received successfully.
     * Application code could be added here ...
     */
    pal_led(LED_DATA, LED_ON);

    /* Extract received data. */
    uint32_t distance;
    uint8_t dqf;

    memcpy(&distance, msdu, sizeof(uint32_t));
    dqf = msdu[sizeof(uint32_t)];

#ifdef SIO_HUB
    if (INVALID_DISTANCE != distance)
    {
        printf("Received Distance to anchor = %" PRIu32 " cm (DQF = %" PRIu8" %%)\n\n",
               distance, dqf);
    }
    else
    {
        printf("Received Invalid Distance to anchor\n\n");
    }
#endif  /* SIO_HUB */

    // Start a timer switching off the LED
    pal_timer_start(TIMER_LED_OFF,
                    LED_PERIOD,
                    TIMEOUT_RELATIVE,
                    (FUNC_PTR())data_exchange_led_off_cb,
                    NULL);

    /* Keep compiler happy. */
    SrcAddrSpec = SrcAddrSpec;
    DstAddrSpec = DstAddrSpec;
    msduLength = msduLength;
    msdu = msdu;
    mpduLinkQuality = mpduLinkQuality;
    DSN = DSN;
#ifdef ENABLE_TSTAMP
    Timestamp = Timestamp;
#endif  /* ENABLE_TSTAMP */
}


/**
 * @brief Application specific function to assign a short address
 *
 */
static bool assign_new_short_addr(uint64_t addr64, uint16_t *addr16)
{
    uint8_t i;

    // Check if device has been associated before
    for (i = 0; i < MAX_NUMBER_OF_DEVICES; i++)
    {
        if (device_list[i].short_addr == 0x0000)
        {
            // If the short address is 0x0000, it has not been used before
            continue;
        }
        if (device_list[i].ieee_addr == addr64)
        {
            // Assign the previously assigned short address again
            *addr16 = device_list[i].short_addr;
            return true;
        }
    }

    for (i = 0; i < MAX_NUMBER_OF_DEVICES; i++)
    {
        if (device_list[i].short_addr == 0x0000)
        {
            *addr16 = CPU_ENDIAN_TO_LE16(i + 0x0001);
            device_list[i].short_addr = CPU_ENDIAN_TO_LE16(i + 0x0001); // get next short address
            device_list[i].ieee_addr = addr64;      // store extended address
            no_of_assigned_short_addr++;
            return true;
        }
    }

    // If we are here, no short address could be assigned.
    return false;
}


/* ===  Node joined existing network === */


/**
 * @brief Callback function usr_mlme_associate_conf
 *
 * @param AssocShortAddress    Short address allocated by the coordinator
 * @param status               Result of requested association operation
 */
void usr_mlme_associate_conf(uint16_t AssocShortAddress, uint8_t status)
{
    if (status == MAC_SUCCESS)
    {
        // Stop timer used for search indication (same as used for data transmission)
        pal_timer_stop(TIMER_LED_OFF);
        pal_led(LED_NWK_SETUP, LED_ON);

#ifdef SIO_HUB
        printf("Joined with Short Address 0x%04" PRIX16 ".\n\n", AssocShortAddress);
#endif  /* SIO_HUB */

        /*
         * Set RX on when idle to enable the receiver as default.
         * Use: bool wpan_mlme_set_req(uint8_t PIBAttribute,
         *                             void *PIBAttributeValue);
         *
         * This request leads to a set confirm message -> usr_mlme_set_conf
         */
        bool rx_on_when_idle = true;

        wpan_mlme_set_req(macRxOnWhenIdle, &rx_on_when_idle);
    }
    else
    {
        // Something went wrong; restart
        wpan_mlme_reset_req(true);
    }

    /* Keep compiler happy. */
    AssocShortAddress = AssocShortAddress;
}


/**
 * @brief Callback function for the application timer
 */
static void app_timer_cb(void *parameter)
{
    if (no_of_assoc_devs > 0)
    {
        /* Initiate ranging to the associated devices round-robin. */
        if (range_with_dev_no == no_of_assoc_devs)
        {
            /* Ranging was done with each device, start all over... */
            range_with_dev_no = 0;
        }

        /*
         * Set actual reflector address based on currently selected
         * associated device.
         */
        ADDR_COPY_DST_SRC_16(wrrr.ReflectorAddr,
                             device_list[range_with_dev_no].short_addr);
        ADDR_COPY_DST_SRC_16(refl_addr.Addr.short_address,
                             device_list[range_with_dev_no].short_addr);

        if (wpan_rtb_range_req(&wrrr))
        {
            range_with_dev_no++;
            pal_led(LED_RANGE, LED_ON); // Indicates ranging has started
        }
    }
    else
    {
        /* Start timer for next ranging procedure. */
        pal_timer_start(TIMER_INIT_RANGING,
                        RANGING_PERIOD,
                        TIMEOUT_RELATIVE,
                        (FUNC_PTR())app_timer_cb,
                        NULL);
    }

    parameter = parameter;  /* Keep compiler happy. */
}


/**
 * @brief Callback function usr_rtb_reset_conf
 *
 * @param status        Result of requested RTB reset operation
 */
void usr_rtb_reset_conf(usr_rtb_reset_conf_t *urrc)
{
    /* Keep compiler happy. */
    urrc = urrc;
}



/**
 * @brief Callback function usr_rtb_range_conf
 */
void usr_rtb_range_conf(usr_rtb_range_conf_t *urrc)
{
    pal_led(LED_RANGE, LED_OFF);    // Indicates ranging has finished

#ifdef SIO_HUB
    handle_range_conf(urrc->results.local.status,
                      urrc->results.local.distance,
                      urrc->results.local.dqf);
#endif  /* SIO_HUB */

    /* Send measured ranging data to this device. */
    static uint8_t msduHandle = 0;

    memcpy(&payload, &urrc->results.local.distance, sizeof(uint32_t));
    payload[sizeof(uint32_t)] = urrc->results.local.dqf;
    msduHandle++;               // increment handle
    wpan_mcps_data_req(WPAN_ADDRMODE_SHORT,
                       &refl_addr,
                       SIZE_OF_PAYLOAD,
                       payload,
                       msduHandle,
                       WPAN_TXOPT_ACK);

    /* Start timer for next ranging procedure. */
    pal_timer_start(TIMER_INIT_RANGING,
                    RANGING_PERIOD,
                    TIMEOUT_RELATIVE,
                    (FUNC_PTR())app_timer_cb,
                    NULL);
}



#ifdef SIO_HUB
static void handle_range_conf(uint8_t status, uint32_t distance, uint8_t dqf)
{
    switch (status)
    {
        case RTB_SUCCESS:
            {
                printf("RTB success\n");
                printf("Distance = %"PRIu32" in cm\n", distance);
                printf("DQF = %"PRIu8" %%\n", dqf);
            }
            break;

        case RTB_RANGING_IN_PROGRESS:
            printf("Ranging procedure already in progress\n");
            break;

        case RTB_REJECT:
            printf("Error: Ranging is rejected\n");
            break;

        case RTB_OUT_OF_BUFFERS:
            printf("Error: Ranging measurement out of buffers\n");
            break;

        case RTB_UNSUPPORTED_RANGING:
            printf("Error: Ranging currently not supported\n");
            break;

        case RTB_TIMEOUT:
            printf("Error: Timeout - Reponse frame not received\n");
            break;

        case RTB_INVALID_PARAMETER:
            printf("Error: Invalid ranging parameters\n");
            break;

        case MAC_CHANNEL_ACCESS_FAILURE:
            printf("Error: Channel access failure during ranging procedure\n");
            break;

        case MAC_NO_ACK:
            printf("Error: No Ack received\n");
            break;

        default:
            printf("Unspecified RTB status\n");
            break;
    }

    printf("\n");
}
#endif  /* SIO_HUB */



/**
 * Callback function usr_mcps_data_conf
 *
 * @param msduHandle  Handle of MSDU handed over to MAC earlier
 * @param status      Result for requested data transmission request
 * @param Timestamp   The time, in symbols, at which the data were transmitted
 *                    (only if timestamping is enabled).
 *
 */
#ifdef ENABLE_TSTAMP
void usr_mcps_data_conf(uint8_t msduHandle, uint8_t status, uint32_t Timestamp)
#else
void usr_mcps_data_conf(uint8_t msduHandle, uint8_t status)
#endif  /* ENABLE_TSTAMP */
{
    if (status == MAC_SUCCESS)
    {
        /*
         * Ranging data has been transmitted successfully.
         * Application code could be added here ...
         */
        pal_led(LED_DATA, LED_ON);
        // Start a timer switching off the LED
        pal_timer_start(TIMER_LED_OFF,
                        LED_PERIOD,
                        TIMEOUT_RELATIVE,
                        (FUNC_PTR())data_exchange_led_off_cb,
                        NULL);
    }

    /* Keep compiler happy. */
    msduHandle = msduHandle;
#ifdef ENABLE_TSTAMP
    Timestamp = Timestamp;
#endif  /* ENABLE_TSTAMP */
}


/**
 * @brief Callback function switching off the LED
 */
static void data_exchange_led_off_cb(void *parameter)
{
    pal_led(LED_DATA, LED_OFF);

    parameter = parameter;  /* Keep compiler happy. */
}

/* EOF */
