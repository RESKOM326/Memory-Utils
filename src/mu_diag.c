/**
 * @file mu_diag.h
 * @author Mark Dervishaj
 * @brief Implementation of mu_diag.h
 * @version 0.1
 * @date 2022-08-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "inc/mu_diag.h"
#include <stdio.h>

void diag_info(diag_trace msg)
{
    fprintf(stderr, "INFO: %s\n", msg);
}

void diag_error(diag_trace msg, MU_ERROR err)
{
    fprintf(stderr, "ERROR: %d -> %s\n", err, msg);
}

void diag_critical(diag_trace msg, MU_ERROR err)
{
    fprintf(stderr, "CRITICAL: %d -> %s\n", err, msg);
}