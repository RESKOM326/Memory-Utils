/**
 * @file mu_diag.h
 * @author Mark Dervishaj
 * @brief Generates traces about program execution
 * @version 0.1
 * @date 2022-08-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _MU_DIAG_H
#define _MU_DIAG_H

#include "mu_types.h"

#define MAX_TRACE_SIZE  200

typedef CHAR diag_trace[MAX_TRACE_SIZE];

/**
 * @brief Gives information about the process' execution
 * 
 * @param msg Trace to generate to the user
 */
extern void diag_info(diag_trace msg);

/**
 * @brief Gives information about non-fatal errors (execution does not stop)
 * 
 * @param msg Trace to generate to the user
 * @param err Error code
 */
extern void diag_error(diag_trace msg, MU_ERROR err);

/**
 * @brief Gives information about critical errors (execution stops)
 * 
 * @param msg Trace to generate to the user
 * @param err Error code
 */
extern void diag_critical(diag_trace msg, MU_ERROR err);

#endif  /* _MU_DIAG_H */