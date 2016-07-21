/**
 * @file rtb_eval_app_ranging.c
 *
 * @brief Ranging related functions of RTB Evaluation Application.
 *
 * $Id: rtb_eval_app_ranging.c 34322 2013-02-21 09:50:35Z sschneid $
 *
 * @author    Atmel Corporation: http://www.atmel.com
 * @author    Support email: avr@atmel.com
 */
/*
 * Copyright (c) 2012, Atmel Corporation All rights reserved.
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
#include <math.h>
#include <ctype.h>
#include "rtb_eval_app_param.h"

/* === Types =============================================================== */

/* Ranging distance error type */
typedef enum distance_error_tag
{
    DIST_OK = 0,
    TRANSACT_ERROR,
    DQF_TOO_LOW,
    DIST_TOO_SHORT,
    DIST_TOO_LONG
} SHORTENUM distance_error_t;

/* === Macros ============================================================== */

/*
 * Threshold for DQF to discard measured distance values during
 * continuous ranging. */
#define Q_THRESHOLD                     (10)

/* Max allowed walking speed in [cm/sec] */
#define SPEED_MAX_CM_PER_S              (200U)   // i.e. 7.2 km/h

/* === Globals ============================================================= */

static const float THRESHOLD = 100.0;   /* In case the distance is measured in cm */
/*
 * Status variable indicating whether at least one successful ranging
 * measurements during a continuous ranging has been received.
 */
static bool fill_status = false;
/* Index during a continuous ranging. */
static uint8_t ranging_array_idx = 0;
/* Array to hold the ranging distance results during a continuous ranging. */
static uint32_t dist_array[MAX_LEN_OF_FILTERING_CONT];
/* Array to hold the ranging DQF results during a continuous ranging. */
static uint8_t dqf_array[MAX_LEN_OF_FILTERING_CONT];
/* Filtered distance for continuous ranging. */
static uint32_t dist_filt = 0;
/* Filtered DQF for continuous ranging. */
static uint8_t dqf_filt = 0;
/* Array to hold the ranging distance results history for speed calculation. */
static uint32_t dist_history[SPEED_CALC_ARRAY_LEN];
/* Array to hold the speed history. */
static float speed_array[SPEED_HISTORY_LEN];
/*
 * Filtered speed derived from calculation based on speed history array values.
 * Note: The value can both be positive and negative(!).
 * A negative speed value logically indicates an approaching node.
 */
static int8_t speed_filt = 0;
/* Last error during continuous ranging. */
static distance_error_t last_error = DIST_OK;
/*
 * Time difference in ms taken from last 2 timestamps within
 * timestamp history array.
 */
static uint16_t time_diff_dist_ms;

/* === Prototypes ========================================================== */

static void calc_distance_history(void);
static void calc_filt_aver(uint8_t filter_len);
static void calc_filt_max(uint8_t filter_len);
static void calc_filt_median(uint8_t filter_len);
static void calc_filt_min(uint8_t filter_len);
static void calc_filt_min_var(uint8_t filter_len);
static void calc_filt_distance_and_dqf(uint8_t filter_len);
static distance_error_t check_distance(uint32_t curr_distance,
                                       uint8_t curr_dqf,
                                       uint32_t *curr_checked_dist,
                                       uint16_t curr_time_diff_dist);
static uint32_t get_median_dist(uint32_t *temp_dist_array,
                                uint8_t len_of_dist_array);
static uint8_t get_median_dqf(uint8_t *temp_dqf_array,
                              uint8_t len_of_dqf_array);
static int compare_uin32_t(const void *f1, const void *f2);

/* === Externals =========================================================== */


/* === Implementation ====================================================== */

/* Helper function to calculate a previous array index. */
FORCE_INLINE(uint8_t, calc_prev_array_idx, uint8_t curr_idx, uint8_t offset, uint8_t arr_len)
{
    uint8_t temp_idx = curr_idx - offset;
    uint8_t temp_idx_2 = temp_idx % arr_len;
    return temp_idx_2;
}



/* Implements required compare function fpr uint32_t variables for qsort(). */
static int compare_uin32_t(const void *f1, const void *f2)
{
    return ( *(uint32_t *)f1 > *(uint32_t *)f2) ? 1 : -1;
}

/* Returns median distance. */
static uint32_t get_median_dist(uint32_t *temp_dist_array,
                                uint8_t len_of_dist_array)
{
    uint8_t median_index = len_of_dist_array / 2;

    /* First sort the distance array. */
    qsort(temp_dist_array, len_of_dist_array, sizeof(uint32_t), compare_uin32_t);

    /* Get actual median distance value. */
    if (len_of_dist_array % 2)
    {
        /*
         * Odd number of distance values, so the median is the
         * value in the middle of the sorted array.
         */
        return temp_dist_array[median_index];
    }
    else
    {
        /*
         * Even number of distance values, so the median is to be calculated.
         */
        uint32_t temp = temp_dist_array[median_index - 1] + temp_dist_array[median_index];
        return (temp / 2);
    }
}



static uint8_t get_median_dqf(uint8_t *temp_dqf_array,
                              uint8_t len_of_dqf_array)
{
    uint8_t median_index = len_of_dqf_array / 2;

    /* First sort the distance array. */
    qsort(temp_dqf_array, len_of_dqf_array, sizeof(uint8_t), compare_uin32_t);

    /* Get actual median distance value. */
    if (len_of_dqf_array % 2)
    {
        /*
         * Odd number of distance values, so the median is the
         * value in the middle of the sorted array.
         */
        return temp_dqf_array[median_index];
    }
    else
    {
        /*
         * Even number of distance values, so the median is to be calculated.
         */
        uint32_t temp = temp_dqf_array[median_index - 1] + temp_dqf_array[median_index];
        return (temp / 2);
    }
}



/*
 * Helper function to calculate distance history array values
 * for speed calculation.
 */
static void calc_distance_history(void)
{
    dist_history[0] = (dist_array[ranging_array_idx] +
                       dist_array[calc_prev_array_idx(ranging_array_idx, 1, MAX_LEN_OF_FILTERING_CONT)])
                      / 2;
    dist_history[1] = (dist_array[calc_prev_array_idx(ranging_array_idx, 2, MAX_LEN_OF_FILTERING_CONT)] +
                       dist_array[calc_prev_array_idx(ranging_array_idx, 3, MAX_LEN_OF_FILTERING_CONT)])
                      / 2;
}



/* Helper function to calculate average distance and DQF. */
static void calc_filt_aver(uint8_t filter_len)
{
    /* No. of result values in array to be considered for filtering. */
    uint8_t index_cnt = filter_len;
    /* Start index in result array for filtering. */
    uint8_t curr_array_idx = ranging_array_idx;
    /* Sum of distances during calculation. */
    uint32_t dist_sum = 0;
    /* Sum of dqfs during calculation. */
    uint16_t dqf_sum = 0;

    while (index_cnt)
    {
        dist_sum += dist_array[curr_array_idx];
        dqf_sum += dqf_array[curr_array_idx];

        if (curr_array_idx == 0)
        {
            /* Perform wrap-around. */
            curr_array_idx = MAX_LEN_OF_FILTERING_CONT;
        }
        curr_array_idx--;
        index_cnt--;
    }

    dist_filt = dist_sum / filter_len;
    dqf_filt = (uint8_t)(round((float)dqf_sum / filter_len));
}



/* Helper function to calculate median distance and DQF. */
static void calc_filt_median(uint8_t filter_len)
{
    /* No. of result values in array to be considered for filtering. */
    uint8_t index_cnt = filter_len;
    /* Start index in result array for filtering. */
    uint8_t curr_array_idx = ranging_array_idx;
    /* Local copy of distance and DQF arrays. */
    uint32_t temp_distance_array[MAX_LEN_OF_FILTERING_CONT];
    uint8_t temp_dqf_array[MAX_LEN_OF_FILTERING_CONT];

    /*
     * Create a temp copy of the distance and DQF value arrays in order
     * be able to sort and find the median.
     */
    while (index_cnt)
    {
        temp_distance_array[index_cnt - 1] = dist_array[curr_array_idx];
        temp_dqf_array[index_cnt - 1] = dqf_array[curr_array_idx];

        if (curr_array_idx == 0)
        {
            /* Perform wrap-around. */
            curr_array_idx = MAX_LEN_OF_FILTERING_CONT;
        }
        curr_array_idx--;
        index_cnt--;
    }

    /* Now sort the distances and DQFs within the temp array. */
    dist_filt = get_median_dist(temp_distance_array, filter_len);
    dqf_filt = get_median_dqf(temp_dqf_array, filter_len);
}



/* Helper function to calculate minimum distance and DQF. */
static void calc_filt_min(uint8_t filter_len)
{
    /* No. of result values in array to be considered for filtering. */
    uint8_t index_cnt = filter_len;
    /* Start index in result array for filtering. */
    uint8_t curr_array_idx = ranging_array_idx;
    /* Min. distances during calculation. */
    uint32_t temp_dist = (uint32_t) - 1;
    /* Min. of dqfs during calculation. */
    uint8_t temp_dqf = 100;

    while (index_cnt)
    {
        if (dist_array[curr_array_idx] < temp_dist)
        {
            /* Store new minimum distance value. */
            temp_dist = dist_array[curr_array_idx];
            /* Store the DQF of the new minimum distance value. */
            temp_dqf = dqf_array[curr_array_idx];
        }

        if (curr_array_idx == 0)
        {
            /* Perform wrap-around. */
            curr_array_idx = MAX_LEN_OF_FILTERING_CONT;
        }
        curr_array_idx--;
        index_cnt--;
    }

    dist_filt = temp_dist;
    dqf_filt = temp_dqf;
}



/* Helper function to calculate minimum distance and DQF considering variance. */
static void calc_filt_min_var(uint8_t filter_len)
{
    /* No. of result values in array to be considered for filtering. */
    uint8_t index_cnt = filter_len;
    /* Start index in result array for filtering. */
    uint8_t curr_array_idx = ranging_array_idx;

    /* Min. distances during calculation. */
    uint32_t dist_min = (uint32_t) - 1;
    uint32_t dist_aver, dist_deviation, dist_var;
    uint32_t dist_sum = 0;
    uint32_t dist_sum_sqr = 0;

    /* Min. of DQFs during calculation. */
    uint8_t dqf_min = 100;
    uint8_t dqf_aver, dqf_deviation, dqf_var;
    uint16_t dqf_sum = 0;
    uint16_t dqf_sum_sqr = 0;

    float b;

    while (index_cnt)
    {
        if (dist_array[curr_array_idx] < dist_min)
        {
            /* Store new minimum distance value. */
            dist_min = dist_array[curr_array_idx];
        }
        if (dqf_array[curr_array_idx] < dqf_min)
        {
            /* Store new minimum DQF value. */
            dqf_min = dqf_array[curr_array_idx];
        }

        /* Calculate sum of distance values for mean distance value. */
        dist_sum += dist_array[curr_array_idx];
        /* Calculate sum of DQF values for mean DQF value. */
        dqf_sum += dqf_array[curr_array_idx];

        if (curr_array_idx == 0)
        {
            /* Perform wrap-around. */
            curr_array_idx = MAX_LEN_OF_FILTERING_CONT;
        }
        curr_array_idx--;
        index_cnt--;
    }


    /* Mean distance value */
    dist_aver = dist_sum / filter_len;
    /* Mean DQF value */
    dqf_aver = dqf_sum / filter_len;


    /* Calculate variance */
    curr_array_idx = ranging_array_idx;

    while (index_cnt)
    {
        dist_deviation = dist_array[curr_array_idx] - dist_aver;
        dist_sum_sqr += dist_deviation * dist_deviation;

        dqf_deviation = dqf_array[curr_array_idx] - dqf_aver;
        dqf_sum_sqr += dqf_deviation * dqf_deviation;

        if (curr_array_idx == 0)
        {
            /* Perform wrap-around. */
            curr_array_idx = MAX_LEN_OF_FILTERING_CONT;
        }
        curr_array_idx--;
        index_cnt--;
    }

    dist_var = dist_sum_sqr / filter_len;
    dqf_var = dqf_sum_sqr / filter_len;

    b = THRESHOLD / (THRESHOLD + dist_var);
    dist_filt = (uint32_t)(b * dist_aver + (1 - b) * dist_min);
    b = THRESHOLD / (THRESHOLD + dqf_var);
    dqf_filt = (uint32_t)(b * dqf_aver + (1 - b) * dqf_min);
}



/* Helper function to calculate minimum distance and DQF. */
static void calc_filt_max(uint8_t filter_len)
{
    /* No. of result values in array to be considered for filtering. */
    uint8_t index_cnt = filter_len;
    /* Start index in result array for filtering. */
    uint8_t curr_array_idx = ranging_array_idx;
    /* Min. distances during calculation. */
    uint32_t temp_dist = 0;
    /* Min. of dqfs during calculation. */
    uint8_t temp_dqf = 0;

    while (index_cnt)
    {
        if (dist_array[curr_array_idx] > temp_dist)
        {
            /* Store new maximum distance value. */
            temp_dist = dist_array[curr_array_idx];
            /* Store the DQF of the new maximum distance value. */
            temp_dqf = dqf_array[curr_array_idx];
        }

        if (curr_array_idx == 0)
        {
            /* Perform wrap-around. */
            curr_array_idx = MAX_LEN_OF_FILTERING_CONT;
        }
        curr_array_idx--;
        index_cnt--;
    }

    dist_filt = temp_dist;
    dqf_filt = temp_dqf;
}



static void calc_filt_distance_and_dqf(uint8_t filter_len)
{
    /* Check current filter method. */
    switch (app_data.app_filtering_method_cont)
    {
        case FILT_AVER:
        default:
            /* Average of distance and DQF */
            calc_filt_aver(filter_len);
            break;

        case FILT_MEDIAN:
            /* Median of distance and DQF */
            calc_filt_median(filter_len);
            break;

        case FILT_MIN:
            /* Minimum of distance and DQF */
            calc_filt_min(filter_len);
            break;

        case FILT_MIN_VAR:
            /* Minimum of distance and DQF considerung variance */
            calc_filt_min_var(filter_len);
            break;

        case FILT_MAX:
            /* Maximum of distance and DQF */
            calc_filt_max(filter_len);
            break;
    }
}



/* Helper function to check measured distance for sanity. */
static distance_error_t check_distance(uint32_t curr_distance,
                                       uint8_t curr_dqf,
                                       uint32_t *curr_checked_dist,
                                       uint16_t curr_time_diff_dist)
{
    distance_error_t distance_error = (distance_error_t)RTB_SUCCESS;
    /*
     * Maximum or minimum allowed distance change limit (in cm) based on
     * maximum allowed speed limit.
     */
    uint16_t dist_limit = curr_time_diff_dist * SPEED_MAX_CM_PER_S / 1000;

    *curr_checked_dist = dist_filt;

    if (curr_distance == (uint32_t) - 1)
    {
        /*
         * An error occured during ranging.
         * Store the last filtered distance.
         */
        distance_error = TRANSACT_ERROR;
    }
    else if (curr_dqf < (uint8_t)Q_THRESHOLD)
    {
        /*
         * Current DQF is really bad. Ignore distance with low DQF.
         * Store the last filtered distance.
         */
        distance_error = DQF_TOO_LOW;
    }
    else if (curr_distance > (dist_filt + dist_limit))
    {
        /*
         * Measured distance is larger than maximum expected distance.
         *
         * Eqalize too large distance.
         */
        *curr_checked_dist = dist_filt + dist_limit;
        distance_error = DIST_TOO_LONG;
    }
    else if ((dist_filt > dist_limit) &&
             ((dist_filt - dist_limit) > curr_distance))
    {
        /*
         * Prerequisite: Filtered distance is larger than minimum
         * expected distance.
         *
         * Measured distance is smaller than minimum expected distance.
         *
         * Eqalize too short distance.
         */
        *curr_checked_dist = dist_filt - dist_limit;
        distance_error = DIST_TOO_SHORT;
    }
    else
    {
        /*
         * Regular distance and DQF.
         * Keep current distance.
         */
        *curr_checked_dist = curr_distance;
        distance_error = DIST_OK;
    }

    return distance_error;
}



void continue_ranging(bool is_remote,
                      app_state_t next_app_state)
{
    wpan_rtb_range_req_t wrrr;

    fill_range_addresses(&wrrr, is_remote);
    #ifdef ENABLE_RTB_REMOTE
	if (is_remote)
    {
        wrrr.CoordinatorAddrMode = gate_way_addr_mode;
    }
    else
    {
        wrrr.CoordinatorAddrMode = NO_COORDINATOR;
    }
	#endif

    /* Update timestamp for continuous ranging. */
    uint32_t curr_time = 0;
    pal_get_current_time(&curr_time);
    /* Get timestamp in ms. */
    time_history[time_history_idx] = (uint16_t)(curr_time / 1000);

    if (wpan_rtb_range_req(&wrrr))
    {
        app_state = next_app_state;
        pal_led(LED_RANGING_ONGOING, LED_ON);   // Indicates ranging has started
        if (is_remote)
        {
            /*
             * Start application timer to watch remote ranging, since
             * remote ranging may fail without any return frame.
             * Therefore the application needs to check this by starting
             * its own application timer.
             */
            pal_timer_start(RANGING_APP_TIMER,
                            REMOTE_RANGING_TIMEOUT_US,
                            TIMEOUT_RELATIVE,
                            (FUNC_PTR())timeout_remote_ranging_cb,
                            NULL);
        }
    }
    else
    {
        app_state = APP_IDLE;
    }
}



void handle_range_conf(bool was_remote,
                       uint8_t status,
                       uint32_t distance,
                       uint8_t dqf,
                       uint8_t no_of_provided_meas_pairs,
                       measurement_pair_t *provided_meas_pairs)
{
    if (RTB_SUCCESS == status)
    {
        /* Python oriented formatting. */
        printf("[RESULT]");
        printf(" %" PRIu32 " %" PRIu8 " ", distance, dqf);
        print_range_addresses(was_remote);
        printf("\n");

        /*
         * First time handle measurement pairs formatted for python
         * controlled operation.
         */
        if (no_of_provided_meas_pairs != 0)
        {
            /*
             * Further measurement pairs are included into the range confirm
             * message.
             */
            for (uint8_t i = 0; i < no_of_provided_meas_pairs; i++)
            {
                printf("[PAIR_NO_%" PRIu8 "]", i);
                printf(" %" PRIu32" %" PRIu8 "\n",
                       provided_meas_pairs[i].distance,
                       provided_meas_pairs[i].dqf);
            }
        }
        printf("[DONE]\n");

        /* Human reading oriented formatting. */
        printf("RTB_SUCCESS\n");
        if (no_of_provided_meas_pairs == 0)
        {
            printf("Distance = %"PRIu32" cm\n", distance);
            printf("DQF = %"PRIu8" %%\n\n", dqf);
        }
        else
        {
            printf("Weighted Distance = %"PRIu32" cm\n", distance);
            printf("Weighted DQF = %"PRIu8" %%\n\n", dqf);
        }
    }
    else
    {
        /* An error occured during the ranging procedure. */

        /* Python oriented formatting. */
        printf("[ERROR]");
        printf(" -1 0 ");
        print_range_addresses(was_remote);
        printf(" 0x%" PRIX8 "\n", status);
        printf("[DONE]\n");

        /* Human reading oriented formatting. */
        printf("ERROR: 0x%" PRIX8 "\n", status);

        print_status(status);
    }
}



void handle_cont_ranging_res(uint8_t status,
                             uint32_t distance,
                             uint8_t dqf)
{
    /* Check whether this was the first successful ranging. */
    if (!fill_status)
    {
        if (RTB_SUCCESS == status)
        {
            /* First successful result in continuous ranging. */
            fill_status = true;

            /* In case this was the first successful ranging,
             * the complete array is filled up (with MAX_LEN_OF_FILTERING_CONT
             * values) taking always the first received results.
             */
            for (uint8_t index = 0;
                 index < MAX_LEN_OF_FILTERING_CONT;
                 index++)
            {
                dist_array[index] = distance;
                dqf_array[index] = dqf;
            }

            /* Initialize the filtered distance and DQF values. */
            dist_filt = dist_array[0];
            dqf_filt = dqf_array[0];

            /*
             * Initialize the (not yet initialized portion of the)
             * timestamp history array.
             * The first element of the array has already been initialized
             * right before this measurement has started.
             * There the loop starts at the 2nd element.
             */
            for (uint8_t index = 1;
                 index < SPEED_CALC_ARRAY_LEN;
                 index++)
            {
                time_history[index] = time_history[time_history_idx];
            }
        }
        else
        {
            /* No successful ranging so far. */
            printf("Err: T\n");
        }
    }
    else
    {
        uint8_t prev_time_history_idx;
        char dir;

        if (time_history_idx == 0)
        {
            /* Index roll-over case. */
            prev_time_history_idx = SPEED_CALC_ARRAY_LEN - 1;
        }
        else
        {
            prev_time_history_idx = time_history_idx - 1;
        }

        /* Calculate time difference based on us timer values. */
        time_diff_dist_ms = time_history[time_history_idx] -
                            time_history[prev_time_history_idx];

        /*
         * The first successful ranging has already been received, so
         * add the received values to the existing arrays.
         * For the distance some sanity calculations are done.
         */
        /* Write next distance value into distance array. */
        last_error = check_distance(distance,
                                    dqf,
                                    &dist_array[ranging_array_idx],
                                    time_diff_dist_ms);

        /* Write next DQF value into DQF array. */
        dqf_array[ranging_array_idx] = dqf;

        /*
         * Calculate the distance history values for speed calculation
         * based on the previous distance array values.
         */
        calc_distance_history();

        /*
         * Calculate filtered distance and DQF
         * based on current filter method.
         */
        calc_filt_distance_and_dqf(app_data.app_filtering_len_cont);

        if (time_diff_dist_ms != 0)
        {
            float speed_array_sum = 0.0;
            /* Speed estimation done in km/h derived from cm/ms. */
            float dist_float = (int32_t)(dist_history[0] - dist_history[1]);

            speed_array[ranging_array_idx % SPEED_HISTORY_LEN] =
                dist_float / time_diff_dist_ms * 36;

            /* Calculate mean value of speed history array. */
            for (uint8_t i = 0; i < SPEED_HISTORY_LEN; i++)
            {
                speed_array_sum += speed_array[i];
            }
            speed_filt = (int)(round(speed_array_sum / SPEED_HISTORY_LEN));
        }
        else
        {
            speed_filt = 0;
        }

        /* Get direction of node. */
        if (speed_filt < -1)
        {
            dir = 'A';  /* Node approaches */
        }
        else if (speed_filt > 1)
        {
            dir = 'L';  /* Node leaves */
        }
        else
        {
            dir = ' ';   /* Constant node position */
        }

        /* Print results. */
        printf("Dist: %5" PRIu32 "cm| Spd: %2" PRIi8 "| Dir: %c| DQF: %3" PRIu8 "%%| Dur: %3" PRIu16 "ms",
               dist_filt,
               speed_filt,
               dir,
               dqf_filt,
               time_diff_dist_ms);

        if (DIST_OK != last_error)
        {
            printf("| Err: ");
        }

        switch (last_error)
        {
            case (DIST_OK):
            default:
                printf(" ");
                break;

            case (TRANSACT_ERROR):
                printf("T");
                break;

            case (DQF_TOO_LOW):
                printf("D");
                break;

            case (DIST_TOO_SHORT):
                printf("S");
                break;

            case (DIST_TOO_LONG):
                printf("L");
                break;
        }

        printf("\n");

        /* Update ranging array index. */
        ranging_array_idx++;
        if (ranging_array_idx == MAX_LEN_OF_FILTERING_CONT)
        {
            /* Reset ranging array index if overflow happened. */
            ranging_array_idx = 0;
        }

        /* Update timestamp history array index. */
        time_history_idx++;
        if (time_history_idx == SPEED_CALC_ARRAY_LEN)
        {
            time_history_idx = 0;
        }
    }
}



void handle_cont_range_conf(uint8_t status,
                            uint32_t distance,
                            uint8_t dqf)
{
    handle_cont_ranging_res(status,
                            distance,
                            dqf);

    /* Start timer before next ranging is initiated. */
    pal_timer_start(RANGING_APP_TIMER_CONT_RANGING,
                    CONT_RANGING_PERIOD_MS * 1000,
                    TIMEOUT_RELATIVE,
                    (FUNC_PTR())continue_ranging_after_timeout_cb,
                    NULL);

    pal_led(LED_RANGING_ONGOING, LED_OFF);  // Indicates ranging has finished
}



void init_ranging(bool is_remote)
{
    wpan_rtb_range_req_t wrrr;

    fill_range_addresses(&wrrr, is_remote);
	if (is_remote)
    {
        wrrr.CoordinatorAddrMode = gate_way_addr_mode;
    }
    else
    {
        wrrr.CoordinatorAddrMode = NO_COORDINATOR;
    }

    ranging_array_idx = 0;
    time_history_idx = 0;
    fill_status = false;

    if (cont_ranging_ongoing)
    {
        if (is_remote)
        {
            /* Start continuous Remote Ranging now. */
            app_state = APP_CONT_REMOTE_RANGING_NEXT;
        }
        else
        {
            /* Start continuous Local Ranging now. */
            app_state = APP_CONT_LOCAL_RANGING_NEXT;
        }
    }
    else if (wpan_rtb_range_req(&wrrr))
    {
        /* Non-continuous ranging. */
        pal_led(LED_RANGING_ONGOING, LED_ON);   // Indicates ranging has started

        if (is_remote)
        {
            /* Start single Remote Ranging now. */
            app_state = APP_REMOTE_RANGING;

            /*
             * Since the system could still be in initializing phase
             * (timer is running to indicate node type via LEDs),
             * the timer must be stopped first.
             */
            pal_timer_stop(RANGING_APP_TIMER);
            /*
             * Start application timer to watch remote ranging, since
             * remote ranging may fail without any return frame.
             * Therefore the application needs to check this by starting
             * its own application timer.
             */
            pal_timer_start(RANGING_APP_TIMER,
                            REMOTE_RANGING_TIMEOUT_US,
                            TIMEOUT_RELATIVE,
                            (FUNC_PTR())timeout_remote_ranging_cb,
                            NULL);
        }
        else
        {
            /* Start single Local Ranging now. */
            app_state = APP_LOCAL_RANGING;
        }
    }
}



/* Helper function to print current ranging status. */
void print_status(uint8_t status)
{
    switch (status)
    {
        case RTB_RANGING_IN_PROGRESS:
            printf("Ranging procedure already in progress\n");
            break;

        case RTB_REJECT:
            printf("Ranging is rejected\n");
            break;

        case RTB_OUT_OF_BUFFERS:
            printf("Ranging measurement out of buffers\n");
            break;

        case RTB_UNSUPPORTED_RANGING:
            printf("Ranging currently not supported\n");
            break;

        case RTB_TIMEOUT:
            printf("Timeout - Reponse frame not received\n");
            break;

        case RTB_INVALID_PARAMETER:
            printf("Invalid ranging parameters\n");
            break;

        case RTB_UNSUPPORTED_PROTOCOL:
            printf("Unsupported RTB protocol\n");
            break;

        case RTB_UNSUPPORTED_METHOD:
            printf("Unsupported ranging method\n");
            break;

        case MAC_CHANNEL_ACCESS_FAILURE:
            printf("Channel access failure during ranging procedure\n");
            break;

        case MAC_NO_ACK:
            printf("No Ack received\n");
            break;

        default:
            printf("Unspecified RTB status\n");
            break;
    }
}



/**
 * @brief Callback function indicating a timeout of ranging initiation timer
 */
void continue_ranging_after_timeout_cb(void *parameter)
{
    if (cont_ranging_ongoing)
    {
        if (APP_CONT_LOCAL_RANGING_ONGOING == app_state)
        {
            app_state = APP_CONT_LOCAL_RANGING_NEXT;
        }
        else if (APP_CONT_REMOTE_RANGING_ONGOING == app_state)
        {
            app_state = APP_CONT_REMOTE_RANGING_NEXT;
        }
    }

    /* Keep compiler happy. */
    parameter = parameter;
}



/* EOF */
