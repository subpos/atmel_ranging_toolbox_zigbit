/**
 * @file bmm.c
 *
 * @brief This file implements the functions for initializing buffer module,
 *  allocating and freeing up buffers.
 *
 * $Id: bmm.c 33946 2012-11-30 13:44:42Z sschneid $
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

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pal.h"
#include "return_val.h"
#include "bmm.h"
#include "qmm.h"
#include "tal.h"
#include "ieee_const.h"
#include "app_config.h"

#if (TOTAL_NUMBER_OF_BUFS > 0)

/*
 * Check if the buffer configuration does not exceed the queue limits.
 * Note: The queue's size parameter is a 8bit value.
 */
#if (TOTAL_NUMBER_OF_BUFS > 255)
#error "Number of buffer exceeds its limit"
#endif


/* === Types =============================================================== */


/* === Macros ============================================================== */

#if (TOTAL_NUMBER_OF_SMALL_BUFS > 0)
/**
 * Checks whether the buffer pointer provided is of small buffer or of a large
 * buffer
 */
#define IS_SMALL_BUF(p) ((p)->body >= \
                         (buf_pool + LARGE_BUFFER_SIZE * TOTAL_NUMBER_OF_LARGE_BUFS))
#endif

/* === Globals ============================================================= */

/**
 * Common Buffer pool holding the buffer user area
 */
#if (TOTAL_NUMBER_OF_SMALL_BUFS > 0)
static uint8_t buf_pool[(TOTAL_NUMBER_OF_LARGE_BUFS *LARGE_BUFFER_SIZE) +
                        (TOTAL_NUMBER_OF_SMALL_BUFS *SMALL_BUFFER_SIZE)];
#else
static uint8_t buf_pool[(TOTAL_NUMBER_OF_LARGE_BUFS *LARGE_BUFFER_SIZE)];
#endif
/*
 * Array of buffer headers
 */
static buffer_t buf_header[TOTAL_NUMBER_OF_LARGE_BUFS + TOTAL_NUMBER_OF_SMALL_BUFS];

/*
 * Queue of free large buffers
 */
#if (TOTAL_NUMBER_OF_LARGE_BUFS > 0)
static queue_t free_large_buffer_q;
#endif

/*
 * Queue of free small buffers
 */
#if (TOTAL_NUMBER_OF_SMALL_BUFS > 0)
static queue_t free_small_buffer_q;
#endif

/* === Prototypes ========================================================== */


/* === Implementation ====================================================== */

/**
 * @brief Initializes the buffer module.
 *
 * This function initializes the buffer module.
 * This function should be called before using any other functionality
 * of buffer module.
 */
void bmm_buffer_init(void)
{
    uint8_t index;

    /* Initialize free buffer queue for large buffers */
#if (TOTAL_NUMBER_OF_LARGE_BUFS > 0)
#ifdef ENABLE_QUEUE_CAPACITY
    qmm_queue_init(&free_large_buffer_q, TOTAL_NUMBER_OF_LARGE_BUFS);
#else
    qmm_queue_init(&free_large_buffer_q);
#endif  /* ENABLE_QUEUE_CAPACITY */
#endif

    /* Initialize free buffer queue for small buffers */
#if (TOTAL_NUMBER_OF_SMALL_BUFS > 0)
#ifdef ENABLE_QUEUE_CAPACITY
    qmm_queue_init(&free_small_buffer_q, TOTAL_NUMBER_OF_SMALL_BUFS);
#else
    qmm_queue_init(&free_small_buffer_q);
#endif  /* ENABLE_QUEUE_CAPACITY */
#endif

#if (TOTAL_NUMBER_OF_LARGE_BUFS > 0)
    for (index = 0; index < TOTAL_NUMBER_OF_LARGE_BUFS; index++)
    {
        /*
         * Initialize the buffer body pointer with address of the
         * buffer body
         */
        buf_header[index].body = buf_pool + (index * LARGE_BUFFER_SIZE);

        /* Append the buffer to free large buffer queue */
        qmm_queue_append(&free_large_buffer_q, &buf_header[index]);
    }
#endif

#if (TOTAL_NUMBER_OF_SMALL_BUFS > 0)
    for (index = 0; index < TOTAL_NUMBER_OF_SMALL_BUFS; index++)
    {
        /*
         * Initialize the buffer body pointer with address of the
         * buffer body
         */
        buf_header[index + TOTAL_NUMBER_OF_LARGE_BUFS].body =
            buf_pool + (TOTAL_NUMBER_OF_LARGE_BUFS * LARGE_BUFFER_SIZE) +
            (index * SMALL_BUFFER_SIZE);

        /* Append the buffer to free small buffer queue */
        qmm_queue_append(&free_small_buffer_q,
                         &buf_header[index + TOTAL_NUMBER_OF_LARGE_BUFS]);
    }
#endif
}


/**
 * @brief Allocates a buffer
 *
 * This function allocates a buffer and returns a pointer to the buffer.
 * The same pointer should be used while freeing the buffer.User should
 * call BMM_BUFFER_POINTER(buf) to get the pointer to buffer user area.
 *
 * @param size size of buffer to be allocated.
 *
 * @return pointer to the buffer allocated,
 *  NULL if buffer not available.
 */
buffer_t *bmm_buffer_alloc(uint8_t size)
{
    buffer_t *pfree_buffer = NULL;

#if (TOTAL_NUMBER_OF_SMALL_BUFS > 0)
    /*
     * Allocate buffer only if size requested is less than or equal to  maximum
     * size that can be allocated.
     */
    if (size <= LARGE_BUFFER_SIZE)
    {
        /*
         * Allocate small buffer if size is less than small buffer size and if
         * small buffer is available allocate from small buffer pool.
         */
        if ((size <= SMALL_BUFFER_SIZE))
        {
            /* Allocate buffer from free small buffer queue */
            pfree_buffer = qmm_queue_remove(&free_small_buffer_q, NULL);
        }

        /*
         * If size is greater than small buffer size or no free small buffer is
         * available, allocate a buffer from large buffer pool if avialable
         */
        if (NULL == pfree_buffer)
        {
            /* Allocate buffer from free large buffer queue */
            pfree_buffer = qmm_queue_remove(&free_large_buffer_q, NULL);
        }
    }
#else /* no small buffers available at all */
    /* Allocate buffer from free large buffer queue */
    pfree_buffer = qmm_queue_remove(&free_large_buffer_q, NULL);

    size = size;    /* Keep compiler happy. */
#endif

    return pfree_buffer;
}


/**
 * @brief Frees up a buffer.
 *
 * This function frees up a buffer. The pointer passed to this function
 * should be the pointer returned during buffer allocation. The result is
 * unpredictable if an incorrect pointer is passed.
 *
 * @param pbuffer Pointer to buffer that has to be freed.
 */
void bmm_buffer_free(buffer_t *pbuffer)
{
    if (NULL == pbuffer)
    {
        /* If the buffer pointer is NULL abort free operation */
        return;
    }

#if (TOTAL_NUMBER_OF_SMALL_BUFS > 0)
    if (IS_SMALL_BUF(pbuffer))
    {
        /* Append the buffer into free small buffer queue */
        qmm_queue_append(&free_small_buffer_q, pbuffer);
    }
    else
    {
        /* Append the buffer into free large buffer queue */
        qmm_queue_append(&free_large_buffer_q, pbuffer);
    }
#else /* no small buffers available at all */
    /* Append the buffer into free large buffer queue */
    qmm_queue_append(&free_large_buffer_q, pbuffer);
#endif

}

#endif /* (TOTAL_NUMBER_OF_BUFS > 0) */
/* EOF */
