/**
 * @file mu_scan_utils.c
 * @author Mark Dervishaj
 * @brief Implementation for mu_scan_utils.h
 * @version 0.1
 * @date 2022-08-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "inc/mu_scan_utils.h"
#include "inc/mu_diag.h"
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>

#define CHECK_EXISTENCE_AND_PERMS   0
#define MIN_BYTES_MAPS_STR          12
#define MIN_BYTES_MEM_STR           11  

MU_ERROR pid_exists(PID target)
{
    MU_ERROR is_ok = ERR_OK;
    diag_trace trace;
    kill(target, CHECK_EXISTENCE_AND_PERMS);
    if(errno == EPERM)
    {
        is_ok = ERR_EPERM;
        sprintf(trace, "%s | Not enough permissions to send the signal to the target process!", __func__);
        diag_error(trace, is_ok);
    }
    else if(errno == ESRCH)
    {
        is_ok = ERR_ESRCH;
        sprintf(trace, "%s | The target process does not exist!", __func__);
        diag_error(trace, is_ok);
    }
    /* Not necessary to check EINVAL. The signal used is hardcoded and this error cannot happen */
    sprintf(trace, "%s | The target process exists", __func__);
    diag_info(trace);
    return is_ok;
}

/**
 * @brief Gets the number of digits used by the PID (number of bytes as text)
 * 
 * @param target PID of target process
 * @return Number of digits this PID has
 */
static INT get_pid_digits(PID target)
{
    INT pid_digits = 0;
    while(target != 0)
    {
        target = target/10;
        pid_digits++;
    }
    return pid_digits;
}

CHAR* get_maps_path(PID target)
{
    MU_ERROR is_ok = ERR_OK;
    diag_trace trace;

    INT pid_digits = get_pid_digits(target);
    INT bytes_needed = MIN_BYTES_MAPS_STR + pid_digits;
    CHAR *path = malloc(bytes_needed);   /* /proc/$PID/maps */

    is_ok = snprintf(path, bytes_needed, "/proc/%d/maps", target);  /* snprintf writes terminating null-byte */
    if(is_ok != ERR_OK)
    {
        is_ok = ERR_GENERIC;
        sprintf(trace, "%s | Cannot create maps file path!", __func__);
        diag_critical(trace, is_ok);
        path = NULL;
    }

    return path;
}

CHAR* get_mem_path(PID target)
{
    MU_ERROR is_ok = ERR_OK;
    diag_trace trace;

    INT pid_digits = get_pid_digits(target);
    INT bytes_needed = MIN_BYTES_MEM_STR + pid_digits;
    CHAR *path = malloc(bytes_needed);   /* /proc/$PID/mem */

    is_ok = snprintf(path, bytes_needed, "/proc/%d/mem", target);  /* snprintf writes terminating null-byte */
    if(is_ok != ERR_OK)
    {
        is_ok = ERR_GENERIC;
        sprintf(trace, "%s | Cannot create mem file path!", __func__);
        diag_critical(trace, is_ok);
        path = NULL;
    }
    return path;
}