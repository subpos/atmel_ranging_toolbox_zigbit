/**
 * @file sio_handler.h
 *
 * @brief This file contains the prototypes for UART related functions.
 *
 * $Id: sio_handler.h 34062 2013-01-09 14:08:23Z uwalter $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2009, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef SIO_HANDLER_H
#define SIO_HANDLER_H

/* === Includes ============================================================= */

#include <stdio.h>
#include <pal_types.h>

/* === Macros =============================================================== */

#if (PAL_GENERIC_TYPE == SAM4)
#ifdef ENABLE_UART0
#define SIO_CHANNEL     SIO_0
#endif
#ifdef ENABLE_UART1
#define SIO_CHANNEL     SIO_1
#endif
#ifdef ENABLE_USART0
#define SIO_CHANNEL     SIO_3
#endif
#ifdef ENABLE_USART1
#define SIO_CHANNEL     SIO_4
#endif
#ifdef ENABLE_USB0
#define SIO_CHANNEL     SIO_2
#endif
#else /* #if (PAL_TYPE == SAM4) */
#ifdef UART0
#define SIO_CHANNEL     SIO_0
#endif
#ifdef UART1
#define SIO_CHANNEL     SIO_1
#endif
#ifdef UART2
#define SIO_CHANNEL     SIO_3
#endif
#ifdef UART3
#define SIO_CHANNEL     SIO_4
#endif
#ifdef USB0
#define SIO_CHANNEL     SIO_2
#endif
#endif /* #if (PAL_TYPE == SAM4) */

/* Function aliases allowing IAR and GCC functions use the same way */
#if ((defined __ICCAVR__) || (defined __ICCARM__) || (defined __ICCAVR32__))
#define sio_putchar(data)       _sio_putchar(data)
#define sio_getchar()           _sio_getchar()
#define sio_getchar_nowait()    _sio_getchar_nowait()
#define fdevopen(a,b)           /* IAR does not use the fdevopen - the __write() (or __read()) must exist instead (here in file write.c) */
#else
#define sio_putchar(data)       _sio_putchar(data, NULL)
#define sio_getchar()           _sio_getchar(NULL)
#define sio_getchar_nowait()    _sio_getchar_nowait(NULL)
#if (PAL_GENERIC_TYPE == ARM7)
#define fdevopen(a,b)
#endif
#endif

/* === Types ================================================================ */


/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif


#if ((defined __ICCAVR__) || (defined __ICCARM__) || (defined __ICCAVR32__))
    int _sio_putchar(char data);
    int _sio_getchar(void);
    int _sio_getchar_nowait(void);
#else
    int _sio_putchar(char data, FILE *dummy);
    int _sio_getchar(FILE *dummy);
    int _sio_getchar_nowait(FILE *dummy);
#endif
    void sio_binarywrite(uint8_t *d, int16_t sz);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SIO_HANDLER_H */
/* EOF */
