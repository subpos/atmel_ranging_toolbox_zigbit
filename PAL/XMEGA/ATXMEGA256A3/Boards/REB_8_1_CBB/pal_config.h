/**
 * @file pal_config.h
 *
 * @brief PAL configuration for ATxmega256A3
 *
 * This header file contains configuration parameters for ATxmega256A3.
 *
 * $Id: pal_config.h 32539 2012-07-04 06:20:12Z sschneid $
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
#ifndef PAL_CONFIG_H
#define PAL_CONFIG_H

/* === Includes =============================================================*/

#include "pal_boardtypes.h"

#if (BOARD_TYPE == REB_8_1_CBB)

/*
 * This header file is required since a function with
 * return type retval_t is declared
 */
#include "return_val.h"

/* === Types ================================================================*/

/** Enumerations used to identify LEDs. */
typedef enum led_id_tag
{
    LED_0,
    LED_1,
    LED_2
} SHORTENUM led_id_t;

/** Number of LEDs on this board. */
#define NO_OF_LEDS                      (3)


/** Enumerations used to identify buttons */
typedef enum button_id_tag
{
    BUTTON_0
} SHORTENUM button_id_t;

/** Number of buttons on this board. */
#define NO_OF_BUTTONS                   (1)

/* === Externals ============================================================*/


/* === Macros ===============================================================*/

/**
 * The default CPU clock (if not defined differently
 * in project files of makefiles).
 */
#ifndef F_CPU
#define F_CPU                   (16000000UL)
#endif

/* Currently only the following system clock frequencies are supported */
#if ((F_CPU != (32000000UL)) && (F_CPU != (16000000UL)) && (F_CPU != (8000000UL)) && (F_CPU != (4000000UL)))
#error "Unsupported F_CPU value"
#endif


/**
 * This board uses Antenna Diversity.
 * The antenna diversity support can be overwritten by Makefile setting.
 */
#ifndef ANTENNA_DIVERSITY
#define ANTENNA_DIVERSITY               (1)
#endif

/**
 * This board uses the timestamp interrupt, since the DIG2 pin from the
 * the transceiver is connected to the MCU.
 * If the following value is set to 0, the timestamp IRQ/DIG2 is used.
 */

#ifndef DISABLE_TSTAMP_IRQ
#define DISABLE_TSTAMP_IRQ              (0)
#endif

/** Autonomous antenna selection is used as default. */
#ifndef ANTENNA_DEFAULT
#define ANTENNA_DEFAULT                 (ANT_CTRL_0)
#endif

/**
 * Define the CCA ED threshold register value.
 * Enabled the following define only if it differs from the reset value.
 */
/* #define CCA_ED_THRESHOLD                (7) */

/**
 * Value of an external PA gain.
 * If no external PA is available, the value is 0.
 */
#define EXTERN_PA_GAIN                  (0)

/*
 * IRQ macros for ATxmega256A3
 */

/** Mapping of TRX interrupts to ISR vectors */
#define TRX_MAIN_ISR_VECTOR             (PORTC_INT0_vect)
#if ((defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)) && (DISABLE_TSTAMP_IRQ == 0)
#define TRX_TSTAMP_ISR_VECTOR           (PORTC_INT1_vect)
#endif

/** Enables the transceiver main interrupt. */
#define ENABLE_TRX_IRQ()                (PORTC.INTCTRL |= PORT_INT0LVL_gm)

/** Disables the transceiver main interrupt. */
#define DISABLE_TRX_IRQ()               (PORTC.INTCTRL &= ~PORT_INT0LVL_gm)

/** Clears the transceiver main interrupt. */
#define CLEAR_TRX_IRQ()                 (PORTC.INTFLAGS = PORT_INT0IF_bm)


#if ((defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)) && (DISABLE_TSTAMP_IRQ == 0)
/** Enables the transceiver timestamp interrupt. */
#define ENABLE_TRX_IRQ_TSTAMP()         (PORTC.INTCTRL |= PORT_INT1LVL_gm)

/** Disables the transceiver timestamp interrupt. */
#define DISABLE_TRX_IRQ_TSTAMP()        (PORTC.INTCTRL &= ~PORT_INT1LVL_gm)

/** Clears the transceiver timestamp interrupt. */
#define CLEAR_TRX_IRQ_TSTAMP()          (PORTC.INTFLAGS = PORT_INT1IF_bm)
#endif  /* #if ((defined BEACON_SUPPORT) || (defined ENABLE_TSTAMP)) && (DISABLE_TSTAMP_IRQ == 0) */


/** Enables the global interrupts. */
#define ENABLE_GLOBAL_IRQ()             sei()

/** Disables the global interrupts. */
#define DISABLE_GLOBAL_IRQ()            cli()

/**
 * This macro saves the global interrupt status.
 */
#define ENTER_CRITICAL_REGION()         {uint8_t sreg = SREG; cli()

/**
 *  This macro restores the global interrupt status.
 */
#define LEAVE_CRITICAL_REGION()         SREG = sreg;}

/**
 * This macro saves the trx interrupt status and disables the trx interrupt.
 */
#define ENTER_TRX_REGION()      { uint8_t irq_mask = PORTC.INTCTRL; PORTC.INTCTRL &= ~PORT_INT0LVL_gm

/**
 *  This macro restores the transceiver interrupt status.
 */
#define LEAVE_TRX_REGION()      PORTC.INTCTRL = irq_mask; }


/*
 * GPIO macros for ATxmega256A3
 */

/**
 * This board uses an SPI-attached transceiver.
 */
#define PAL_USE_SPI_TRX                 (1)

/* Actual Ports */
/**
 * The data direction register for the transceiver
 */
#define TRX_PORT1_DDR                   (PORTC.DIR)

/**
 * The transceiver port
 */
#define TRX_PORT1                       (PORTC)

/**
 * RESET pin of transceiver (connected to @ref TRX_PORT1)
 */
#define TRX_RST                         (0)

/**
 * Sleep Transceiver pin (connected to @ref TRX_PORT1)
 */
#define SLP_TR                          (3)

/**
 * Slave select pin (connected to @ref TRX_SPI_PORT)
 */
#define SEL                             (4)

/**
 * SPI Bus Master Output/Slave Input pin (connected to @ref TRX_SPI_PORT)
 */
#define MOSI                            (5)

/**
 * SPI Bus Master Input/Slave Output pin (connected to @ref TRX_SPI_PORT)
 */
#define MISO                            (6)

/**
 * SPI serial clock pin (connected to @ref TRX_SPI_PORT)
 */
#define SCK                             (7)

/** CLKM of transceiver is on separate port. */
#define TRX_CLKM_PORT                   (PORTD)
/** CLKM pin from transceiver (connected to @ref TRX_CLKM_PORT) */
#define CLKM_PIN                        (PIN0_bp)

/**
 * IRQ pin direct GPIO access
 *
 * use pin 2 directly to avoid additional macro "TRX_IRQ"
 */
#define IRQ_PINGET()                    (TRX_PORT1.IN & _BV(2))

/*
 * Set TRX GPIO pins.
 */
/** Set TRX_RST pin to high. */
#define RST_HIGH()                      (TRX_PORT1.OUTSET = _BV(TRX_RST))
/** Set TRX_RST pin to low. */
#define RST_LOW()                       (TRX_PORT1.OUTCLR = _BV(TRX_RST))
/** Set SLP_TR pin to high. */
#define SLP_TR_HIGH()                   (TRX_PORT1.OUTSET = _BV(SLP_TR))
/** Set SLP_TR pin to low. */
#define SLP_TR_LOW()                    (TRX_PORT1.OUTCLR = _BV(SLP_TR))

/**
 * PORT where LEDs are connected.
 */
#define LED_PORT                        (PORTB)

/*
 * PINs where LEDs are connected
 */
/** LED pin 0 */
#define LED_PIN_0                       (0)
/** LED pin 1 */
#define LED_PIN_1                       (1)
/** LED pin 2 */
#define LED_PIN_2                       (2)

/**
 * PORT where button is connected.
 */
#define BUTTON_PORT                     (PORTB)

/**
 * PINs where buttons are connected.
 */
#define BUTTON_PIN_0                    (3)

/**
 * PORT Control register for specific button
 */
#define BUTTON_PIN_0_CTRL               (BUTTON_PORT.PIN3CTRL)


/**
 * UART 0 port and pin defines
 * here: UART 0 is located at port D, pin 2 and 3
 */
#if defined(UART0) || defined(DOXYGEN)
/** Port for UART0 */
#define UART0_PORT                      PORTD
#define UART0_TX_PIN                    PIN3_bm
#define UART0_RX_PIN                    PIN2_bm
#define UART0_REG                       USARTD0
#define UART0_RX_ISR_VECT               USARTD0_RXC_vect
#define UART0_TX_ISR_VECT               USARTD0_TXC_vect
#endif

/**
 * UART 1 port and pin defines
 * here: UART 1 is located at port D, pin 6 and 7
 */
#if defined(UART1) || defined(DOXYGEN)
/** Port for UART1 */
#define UART1_PORT                      PORTD
#define UART1_TX_PIN                    PIN7_bm
#define UART1_RX_PIN                    PIN6_bm
#define UART1_REG                       USARTD1
#define UART1_RX_ISR_VECT               USARTD1_RXC_vect
#define UART1_TX_ISR_VECT               USARTD1_TXC_vect
#endif


/*
 * PORT where over-the-air update Serial Flash is connected
 */
#define OTAU_FLASH_PORT                 (PORTD)

/*
 * OTAU flash chip select signal
 * bit position
 */
#define OTAU_FLASH_CS_bp                (4)
#define OTAU_FLASH_CS_bm                (1<<OTAU_FLASH_CS_bp)

/*
 * OTAU flash serial data in
 * bit position
 */
#define OTAU_FLASH_SI_bp                (5)
#define OTAU_FLASH_SI_bm                (1<<OTAU_FLASH_SI_bp)

/*
 * OTAU flash serial data out
 * bit position
 */
#define OTAU_FLASH_SO_bp                (6)
#define OTAU_FLASH_SO_bm                (1<<OTAU_FLASH_SO_bp)

/*
 * OTAU flash serial clock signal
 * bit position
 */
#define OTAU_FLASH_SCK_bp               (7)
#define OTAU_FLASH_SCK_bm               (1<<OTAU_FLASH_SCK_bp)

/*
 * Timer macros for ATxmega256A3
 */
#define WAIT_2_NOPS()                   {nop(); nop();}
#define WAIT_4_NOPS()                   {WAIT_2_NOPS(); WAIT_2_NOPS();}
#define WAIT_8_NOPS()                   {WAIT_4_NOPS(); WAIT_4_NOPS();}
#define WAIT_16_NOPS()                  {WAIT_8_NOPS(); WAIT_8_NOPS();}
#define WAIT_32_NOPS()                  {WAIT_16_NOPS(); WAIT_16_NOPS();}


/*
 * These macros are placeholders for delay functions for high speed processors.
 *
 * The following delay are not reasonbly implemented via delay functions,
 * but rather via a certain number of NOP operations.
 * The actual number of NOPs for each delay is fully MCU and frequency
 * dependent, so it needs to be updated for each board configuration.
 */
#if (F_CPU == (32000000UL))
/*
 * ATxmega256A3 @ 32MHz
 */
/*
 * Wait for 65 ns.
 * time t7: SLP_TR time (see data sheet or SWPM).
 * In case the system clock is 32 MHz we need to have this value filled,
 * otherwise frame transmission may not be initiated properly.
 */
#define PAL_WAIT_65_NS()                nop(); nop(); nop()
/* Wait for 500 ns. */
#define PAL_WAIT_500_NS()               WAIT_16_NOPS()
/* Wait for 1 us. */
#define PAL_WAIT_1_US()                 WAIT_32_NOPS()

#elif (F_CPU == (16000000UL))
/*
 * ATxmega256A3 @ 16MHz
 */
/*
 * Wait for 65 ns.
 * time t7: SLP_TR time (see data sheet or SWPM).
 */
#define PAL_WAIT_65_NS()                nop(); nop()
/* Wait for 500 ns. */
#define PAL_WAIT_500_NS()               WAIT_8_NOPS()
/* Wait for 1 us. */
#define PAL_WAIT_1_US()                 WAIT_16_NOPS()

#elif (F_CPU == (8000000UL))
/*
 * ATxmega256A3 @ 8MHz
 */
/*
 * Wait for 65 ns.
 * time t7: SLP_TR time (see data sheet or SWPM).
 */
#define PAL_WAIT_65_NS()                nop()
/* Wait for 500 ns. */
#define PAL_WAIT_500_NS()               WAIT_4_NOPS()
/* Wait for 1 us. */
#define PAL_WAIT_1_US()                 WAIT_8_NOPS()

#elif (F_CPU == (4000000UL))
/*
 * ATxmega256A3 @ 4MHz
 */
/*
 * Wait for 65 ns.
 * time t7: SLP_TR time (see data sheet or SWPM).
 * In case the system clock is slower than 32 MHz we do not need
 * to have this value filled.
 */
#define PAL_WAIT_65_NS()                // empty
/* Wait for 500 ns. */
#define PAL_WAIT_500_NS()               WAIT_2_NOPS()
/* Wait for 1 us. */
#define PAL_WAIT_1_US()                 WAIT_4_NOPS()

#else
#error Unknown system clock
#endif

/**
 * The smallest timeout in microseconds
 */
#define MIN_TIMEOUT                     (0x80)

/**
 * The largest timeout in microseconds
 */
#define MAX_TIMEOUT                     (0x7FFFFFFF)

/**
 * Minimum time in microseconds, accepted as a delay request
 */
#define MIN_DELAY_VAL                   (5)

/**
 * Timer clock source while radio is awake.
 *
 * T0 & T1 of PORTC
 * clk source is event channel 2 triggered by transceiver clock (CLKM, 1MHz)
 */
#define TIMER_SRC_DURING_TRX_AWAKE() {TCC0_CTRLA = TC0_CLKSEL3_bm | TC0_CLKSEL1_bm; \
        TCC1_CTRLA = TC1_CLKSEL3_bm | TC1_CLKSEL1_bm;}

/**
 * Timer clock source while radio is sleeping.
 *
 * T0 & T1 of PORTC
 * clk source is event channel 0 triggered by system clock with corresponding
 * event system prescaler (see function event_system_init()).
 */
#define TIMER_SRC_DURING_TRX_SLEEP() {TCC0_CTRLA = TC0_CLKSEL3_bm; \
        TCC1_CTRLA = TC1_CLKSEL3_bm;}


/**
 * Maximum numbers of software timers running at a time
 */
#define MAX_NO_OF_TIMERS                (25)
#if (MAX_NO_OF_TIMERS > 255)
#error "MAX_NO_OF_TIMERS must not be larger than 255"
#endif

/**
 * Hardware register that holds Rx timestamp
 */
#define TIME_STAMP_REGISTER             (TCC1_CCA)


/*
 * TRX Access macros for ATxmega256A3
 */

/**
 * Definition of Port for SPI to TRX.
 */
#define TRX_SPI_PORT                    (TRX_PORT1)
/**
 * Definition of SPI to TRX.
 */
#define TRX_SPI                         (SPIC)

/**
 * Bit mask for slave select
 */
#define SS_BIT_MASK                     (1 << SEL)

/**
 * Slave select made low
 */
#define SS_LOW()                        (TRX_SPI_PORT.OUTCLR = SS_BIT_MASK)

/**
 * Slave select made high
 */
#define SS_HIGH()                       (TRX_SPI_PORT.OUTSET = SS_BIT_MASK)

/**
 * Mask for SPIF bit in status register
 */
#define SPIF_MASK                       (SPI_IF_bm)

/**
 * SPI Data Register
 */
#define SPI_DATA_REG                    (TRX_SPI.DATA)

/**
 * Wait for SPI interrupt flag
 */
#define SPI_WAIT()                      do {                        \
        while ((TRX_SPI.STATUS & SPIF_MASK) == 0) { ; }                    \
    } while (0)

/**
 * Dummy value written in SPDR to retrieve data form it
 */
#define SPI_DUMMY_VALUE                 (0x00)

/**
 * TRX Initialization
 */
#if (F_CPU == (32000000UL))
#define TRX_INIT()                      do {                        \
        /* Enable the SPI and make the microcontroller as SPI master */ \
        /* Set the SPI speed to 4MHz. */                                \
        /* CLK2X = 1; PRESCALER = 1 (clkPER/8) */                       \
        /* Resulting SPI speed: 4MHz */                                 \
        TRX_SPI.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | (0 << SPI_MODE0_bp);\
        TRX_SPI.CTRL |= (1 << SPI_CLK2X_bp) | (1 << SPI_PRESCALER0_bp);    \
        /* Set SEL pin to high */                                       \
        TRX_SPI_PORT.OUTSET = _BV(SEL);                                    \
    } while (0)

#elif (F_CPU == (16000000UL))
#define TRX_INIT()                      do {                        \
        /* Enable the SPI and make the microcontroller as SPI master */ \
        /* Set the SPI speed to 4MHz. */                                \
        /* CLK2X = 0; PRESCALER = 0 (clkPER/4) */                       \
        TRX_SPI.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | (0 << SPI_MODE0_bp);\
        /* Set SEL pin to high */                                       \
        TRX_SPI_PORT.OUTSET = _BV(SEL);                                    \
    } while (0)

#elif (F_CPU == (8000000UL))
#define TRX_INIT()                      do {                        \
        /* Enable the SPI and make the microcontroller as SPI master */ \
        /* Set the SPI speed to 4MHz. */                                \
        /* CLK2X = 1; PRESCALER = 0 (clkPER/2) */                       \
        TRX_SPI.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | (0 << SPI_MODE0_bp);\
        TRX_SPI.CTRL |= (1 << SPI_CLK2X_bp);                               \
        /* Set SEL pin to high */                                       \
        TRX_SPI_PORT.OUTSET = _BV(SEL);                                    \
    } while (0)

#elif (F_CPU == (4000000UL))
#define TRX_INIT()                      do {                        \
        /* Enable the SPI and make the microcontroller as SPI master */ \
        /* Set the SPI speed to 2MHz. */                                \
        /* CLK2X = 1; PRESCALER = 0 (clkPER/2) */                       \
        TRX_SPI.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | (0 << SPI_MODE0_bp);\
        TRX_SPI.CTRL |= (1 << SPI_CLK2X_bp);                               \
        /* Set SEL pin to high */                                       \
        TRX_SPI_PORT.OUTSET = _BV(SEL);                                    \
    } while (0)

#else
#error Unknown system clock
#endif

#if defined(__GNUC__)
#include <util/crc16.h>
#define CRC_CCITT_UPDATE(crc, data) _crc_ccitt_update(crc, data)
#endif /* defined(__GNUC__) */

#if defined(__ICCAVR__)
#define CRC_CCITT_UPDATE(crc, data) crc_ccitt_update(crc, data)
/* Internal helper function for CRC_CCITT_UPDATE. */
uint16_t crc_ccitt_update(uint16_t crc, uint8_t data);
#endif /* __ICCAVR__ */

/**
 * This board has an external eeprom that stores the IEEE address
 * and other information.
 */
#ifndef EXTERN_EEPROM_AVAILABLE
#define EXTERN_EEPROM_AVAILABLE            (1)
#endif

/**
 * Storage location for crystal trim value - within external EEPROM
 */
#define EE_XTAL_TRIM_ADDR                  (21)

/**
 * Alert initialization
 */
#define ALERT_INIT()                    do {    \
        LED_PORT.OUT    = 0;                    \
        LED_PORT.DIRSET = 0xFF;                 \
    } while (0)

/**
 * Alert indication
 */
#define ALERT_INDICATE()                (LED_PORT.OUTTGL = 0xFF)

/**
 * Sleep configurations
 */

/**
 * Sleep mode control register
 */
#define SLEEP_CTRL_REG                  (SLEEP.CTRL)

/**
 * Bitmask for Sleep Enable bit
 */
#define SLEEP_ENABLE_MASK SLEEP_SEN_bm

/*
 * Bitmask for all sleep mode bits
 */
#define SLEEP_MODE_MASK_GRP             (SLEEP_SMODE_gm)

/**
 * Configure sleep ctrl register.
 */
#define CONFIGURE_SLEEP( sleep_mode ) \
    { \
        SLEEP_CTRL_REG = (SLEEP_CTRL_REG & ~SLEEP_MODE_MASK_GRP) | \
                         (sleep_mode) | SLEEP_ENABLE_MASK; \
    }

/**
 * Configure sleep ctrl register to disable sleep
 */
#define DISABLE_SLEEP()                 (SLEEP_CTRL_REG &= ~SLEEP_ENABLE_MASK)

/**
 * Handling of sleep modes
 */
#define pal_pwr_mode(x)                 pal_sleep_mode(x)
/*
 * Sleep fuction for AVR and GCC
 */
#if defined(__ICCAVR__)
#   define CPU_SLEEP()                  __sleep()
#elif defined(__GNUC__)
#   define CPU_SLEEP()                  sleep_cpu()
#endif


/**
 * If ranging is enabled define the distance offset for this board in cm.
 * The valid range is -128...127 (cm).
 */
#if defined(ENABLE_RTB) || defined(DOXYGEN)
#define DISTANCE_OFFSET                 (-50)
#if ((DISTANCE_OFFSET < INT8_MIN) || (DISTANCE_OFFSET > INT8_MAX))
#   error "Invalid Distance Offset"
#endif
#endif  /* ENABLE_RTB */

/* === Prototypes ===========================================================*/
#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* REB_8_1_CBB */

#endif  /* PAL_CONFIG_H */
/* EOF */
