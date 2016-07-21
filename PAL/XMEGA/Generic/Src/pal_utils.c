/**
 * @file pal_utils.c
 *
 * @brief Utilities for PAL for AVR ATxmega MCUs
 *
 * This file implementes utilities for the PAL module for AVR ATxmega MCUs.
 *
 * $Id: pal_utils.c 33251 2012-08-27 07:39:46Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */
/* === Includes ============================================================ */

#if (DEBUG > 0)
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "pal.h"
#ifdef TEST_HARNESS
#include "sio_handler.h"
#endif  /* TEST_HARNESS */

/* === Macros ============================================================== */

#ifdef TEST_HARNESS
#define ASSERT_BUFFER_SIZE          (172)

#define ASSERT_INDICATION           (0x9F)

/*
 * Message length is limited to LARGE_BUFFER_SIZE - 5 to include room for
 * SOT, EOT, Length, total buffer length and command code
 */
#define ASSERT_MESSAGE_LENGTH       (ASSERT_BUFFER_SIZE - 5)
#else
/**
 * The event payload can be max 255 bytes, 1 byte goes as length byte
 * for octetstring and 1 byte as command code
 */
#define MAX_OCTETSTRING_SIZE        (253)
#endif  /* TEST_HARNESS */

/* === Globals ============================================================= */

#ifdef TEST_HARNESS
static uint8_t assert_msg[sizeof(assert_t) + ASSERT_BUFFER_SIZE - sizeof(uint8_t)];
#else
/* Holds the assert message to be printed. */
static char tmpbuf[MAX_OCTETSTRING_SIZE];
#endif  /* TEST_HARNESS */

/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

/**
 * @brief Tests for Assertion
 *
 * This function tests the assertion of a given expression and
 * if the expression fails, a message is printed. This function
 * is implemented similar to the C library function except
 * that the processing will not be aborted if the assertion fails.
 *
 * @param expression To be tested for assertion
 * @param message Data to be printed over SIO
 * @file file File in which assertion has to be tested.
 * @line line Line number on which assertion has to be tested.
 */
void pal_assert(bool expression,
                FLASH_STRING_T message,
                int8_t *file,
                uint16_t line)
{
    /*
     * Assert for the expression. This expression should be true always,
     * false indicates that something went wrong
     */
    if (!expression)
    {
#ifdef TEST_HARNESS
        assert_t *assert_details = (assert_t *)&assert_msg[0];
        uint8_t assert_string[ASSERT_BUFFER_SIZE];

        /* Only used for test environment */
        assert_details->assert_cmdcode = ASSERT_INDICATION;

        /* Copy the assertion message to RAM */
        PGM_STRCPY((char *)assert_string, message);

        /*
         * Put total length of message to be printed in data[0] of echo
         * indication. Data will be copied from failure_msg->data[1]
         * The function snprintf is used for copying variable
         * number of charaters into failure_msg->data
         */
        assert_details->data[0] = snprintf((char *) & (assert_details->data[1]),
                                           ASSERT_MESSAGE_LENGTH, "%s, line %d: assertion %s failed -",
                                           (char *)file, (uint8_t)line, (char *)assert_string);

        /*
         * The function snprintf returns the number of characters that
         * would have been printed if there was enough room.
         * Take the actual number of bytes that are printed
         */
        assert_details->data[0] =
            strlen((const char *) & (assert_details->data[1]));

        /* Total size of echo indication */
        assert_details->size = sizeof(assert_t) + assert_details->data[0] -
                               sizeof(assert_details->size);

        /* Write into UART/USB */
        sio_write((uint8_t *)assert_details);
#else
        /* Standard for all applications */
        PGM_STRCPY(tmpbuf, message);
        tmpbuf[PGM_STRLEN(message) + 1] = '\0';
        PRINTF("Assertion Failed on File %s, line %d, expression %s\n",
               file, line, tmpbuf);
#endif  /* TEST_HARNESS */
    }
}

#endif  /* (DEBUG > 0) */


/* EOF */
