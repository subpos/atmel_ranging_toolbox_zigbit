/**
 * @file mac_data_extract_mhr.c
 *
 * @brief Implements common functionality for MAC
 *
 * This file implements common MAC functionality to be used
 * by other blocks, such as extracting the MHR from a received frame.
 *
 * $Id: mac_data_extract_mhr.c 29158 2011-11-08 08:48:47Z sschneid $
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

#include "mac_internal.h"

/* === Macros =============================================================== */


/* === Globals ============================================================= */


/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

/*
 * @brief Helper function to extract the complete address information
 *        of the received frame
 *
 * @param frame_ptr Pointer to first octet of Addressing fields of received frame
 *        (See IEEE 802.15.4-2006 Figure 41)
 *
 * @return bool Length of Addressing fields
 */
uint8_t mac_extract_mhr_addr_info(uint8_t *frame_ptr)
{
    uint16_t fcf = mac_parse_data.fcf;
    uint8_t src_addr_mode = (fcf >> FCF_SOURCE_ADDR_OFFSET) & FCF_ADDR_MASK;
    uint8_t dst_addr_mode = (fcf >> FCF_DEST_ADDR_OFFSET) & FCF_ADDR_MASK;
    bool intra_pan = fcf & FCF_PAN_ID_COMPRESSION;
    uint8_t addr_field_len = 0;

    if (dst_addr_mode != 0)
    {
        mac_parse_data.dest_panid = convert_byte_array_to_16_bit(frame_ptr);
        frame_ptr += PAN_ID_LEN;
        addr_field_len += PAN_ID_LEN;

        if (FCF_SHORT_ADDR == dst_addr_mode)
        {
            /*
             * First initialize the complete long address with zero, since
             * later only 16 bit are actually written.
             */
            mac_parse_data.dest_addr.long_address = 0;
            mac_parse_data.dest_addr.short_address = convert_byte_array_to_16_bit(frame_ptr);
            frame_ptr += SHORT_ADDR_LEN;
            addr_field_len += SHORT_ADDR_LEN;
        }
        else if (FCF_LONG_ADDR == dst_addr_mode)
        {
            mac_parse_data.dest_addr.long_address = convert_byte_array_to_64_bit(frame_ptr);
            frame_ptr += EXT_ADDR_LEN;
            addr_field_len += EXT_ADDR_LEN;
        }
    }

    if (src_addr_mode != 0)
    {
        if (!intra_pan)
        {
            /*
             * Source PAN ID is present in the frame only if the intra-PAN bit
             * is zero and src_addr_mode is non zero.
             */
            mac_parse_data.src_panid = convert_byte_array_to_16_bit(frame_ptr);
            frame_ptr += PAN_ID_LEN;
            addr_field_len += PAN_ID_LEN;
        }
        else
        {
            /*
             * The received frame does not contain a source PAN ID, hence
             * source PAN ID is updated with the destination PAN ID.
             */
            mac_parse_data.src_panid = mac_parse_data.dest_panid;
        }

        /* The source address is updated. */
        if (FCF_SHORT_ADDR == src_addr_mode)
        {
            /*
             * First initialize the complete long address with zero, since
             * later only 16 bit are actually written.
             */
            mac_parse_data.src_addr.long_address = 0;
            mac_parse_data.src_addr.short_address = convert_byte_array_to_16_bit(frame_ptr);
            frame_ptr += SHORT_ADDR_LEN;

            addr_field_len += SHORT_ADDR_LEN;
        }
        else if (FCF_LONG_ADDR == src_addr_mode)
        {
            mac_parse_data.src_addr.long_address = convert_byte_array_to_64_bit(frame_ptr);
            frame_ptr += EXT_ADDR_LEN;
            addr_field_len += EXT_ADDR_LEN;
        }
    }

    /*
     * The length of the Addressing Field is known, so the length of the
     * MAC payload can be calcluated.
     * The actual MAC payload length is calculated from
     * the length of the mpdu minus 2 octets FCS, minus 1 octet sequence
     * number, minus the length of the addressing fields, minus 2 octet FCS.
     */
    mac_parse_data.mac_payload_length = mac_parse_data.mpdu_length -
                                        FCF_LEN -
                                        SEQ_NUM_LEN -
                                        addr_field_len -
                                        FCS_LEN;

    mac_parse_data.src_addr_mode = src_addr_mode;
    mac_parse_data.dest_addr_mode = dst_addr_mode;

    return (addr_field_len);
}

/* EOF */
