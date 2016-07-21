/**
 * @file pal_trx_access_spi.h
 *
 * @brief SPI based TRX access API
 *
 * This header file declares prototypes of the SPI based TRX access API.
 *
 * $Id: pal_trx_access_spi.h 33175 2012-08-22 13:08:53Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2012, Atmel Corporation All rights reserved.
 *
 * Licensed under Atmel's Limited License Agreement --> EULA.txt
 */

/* Prevent double inclusion */
#ifndef PAL_TRX_ACCESS_SPI_H
#define PAL_TRX_ACCESS_SPI_H

#if defined(PAL_USE_SPI_TRX) || defined(DOXYGEN)

/* === Includes ============================================================ */

#include <stdbool.h>
#include <stdint.h>
#include "pal_types.h"
#include "pal_config.h"

/* === Macros =============================================================== */


/* === Types =============================================================== */


/* === Externals ============================================================ */


/* === Prototypes =========================================================== */

#ifdef __cplusplus
extern "C" {
#endif

    /*
     * Prototypes for transceiver access.  Some PALs define these as
     * macros instead of implementing them as functions, so only declare
     * them here if they are not implemented as macros.
     */
    /**
     * @brief Reads frame buffer of the transceiver
     *
     * This function reads the frame buffer of the transceiver.
     *
     * @param[out] data Pointer to the location to store frame
     * @param[in] length Number of bytes to be read from the frame
     * buffer.
     * @ingroup apiPalApi
     */
    void pal_trx_frame_read(uint8_t *data, uint8_t length);

    /**
     * @brief Writes data into frame buffer of the transceiver
     *
     * This function writes data into the frame buffer of the transceiver
     *
     * @param[in] data Pointer to data to be written into frame buffer
     * @param[in] length Number of bytes to be written into frame buffer
     * @ingroup apiPalApi
     */
    void pal_trx_frame_write(uint8_t *data, uint8_t length);

    /**
     * @brief Reads current value from a transceiver register
     *
     * This function reads the current value from a transceiver register.
     *
     * @param addr Specifies the address of the trx register
     * from which the data shall be read
     *
     * @return value of the register read
     * @ingroup apiPalApi
     */
    uint8_t pal_trx_reg_read(uint8_t addr);

    /**
     * @brief Writes data into a transceiver register
     *
     * This function writes a value into transceiver register.
     *
     * @param addr Address of the trx register
     * @param data Data to be written to trx register
     *
     * @ingroup apiPalApi
     */
    void pal_trx_reg_write(uint8_t addr, uint8_t data);

    /**
     * @brief Subregister read
     *
     * @param   addr  offset of the register
     * @param   mask  bit mask of the subregister
     * @param   pos   bit position of the subregister
     *
     * @return  value of the read bit(s)
     * @ingroup apiPalApi
     */
    uint8_t pal_trx_bit_read(uint8_t addr, uint8_t mask, uint8_t pos);

    /**
     * @brief Subregister write
     *
     * @param[in]   reg_addr  Offset of the register
     * @param[in]   mask  Bit mask of the subregister
     * @param[in]   pos   Bit position of the subregister
     * @param[out]  new_value  Data, which is muxed into the register
     * @ingroup apiPalApi
     */
    void pal_trx_bit_write(uint8_t reg_addr, uint8_t mask, uint8_t pos, uint8_t new_value);

#if defined(ENABLE_TRX_SRAM) || defined(ENABLE_TRX_SRAM_READ) || defined(DOXYGEN)
    /**
     * @brief Reads data from SRAM of the transceiver
     *
     * This function reads from the SRAM of the transceiver
     *
     * @param[in] addr Start address in SRAM for read operation
     * @param[out] data Pointer to the location where data stored
     * @param[in] length Number of bytes to be read from SRAM
     * @ingroup apiPalApi
     */
    void pal_trx_sram_read(uint8_t addr, uint8_t *data, uint8_t length);
#endif

#if defined(ENABLE_TRX_SRAM) || defined(DOXYGEN)
    /**
     * @brief Writes data into SRAM of the transceiver
     *
     * This function writes data into the SRAM of the transceiver
     *
     * @param addr Start address in the SRAM for the write operation
     * @param data Pointer to the data to be written into SRAM
     * @param length Number of bytes to be written into SRAM
     * @ingroup apiPalApi
     */
    void pal_trx_sram_write(uint8_t addr, uint8_t *data, uint8_t length);
#endif

#if defined(ENABLE_TRX_SRAM) || defined(DOXYGEN)
    /**
     * @brief Writes and reads data into/from SRAM of the transceiver
     *
     * This function writes data into the SRAM of the transceiver and
     * simultaneously reads the bytes.
     *
     * @param addr Start address in the SRAM for the write operation
     * @param idata Pointer to the data written/read into/from SRAM
     * @param length Number of bytes written/read into/from SRAM
     * @ingroup apiPalApi
     */
    void pal_trx_aes_wrrd(uint8_t addr, uint8_t *idata, uint8_t length);
#endif  /* #ifdef ENABLE_TRX_SRAM */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #if defined(PAL_USE_SPI_TRX) || defined(DOXYGEN) */

#endif  /* PAL_TRX_ACCESS_SPI_H */
/* EOF */
