/**
 * @file test1.c
 * @author Mark Dervishaj
 * @brief Tests for mu_scan_utils
 * @version 0.1
 * @date 2022-08-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../../src/inc/mu_types.h"
#include "../../src/inc/mu_scan_utils.h"
#include <stdio.h>

MU_ERROR test_pid_exists(PID target)
{
    return pid_exists(target);
}

MU_ERROR test_get_maps(PID target)
{
    MU_ERROR is_ok = ERR_OK;
    CHAR *path = get_maps_path(target);
    if(path != NULL)
    {
        printf("%s - %s\n", __func__, path);
        is_ok = ERR_GENERIC;
    }

    return is_ok; 
}

INT main(INT argc, CHAR **argv)
{
    INT target = atoi(argv[1]);
    printf("RUN TEST PID_EXISTS:\t%d\n", test_pid_exists(target));
    printf("RUN TEST GET_MAPS:\t%d\n", test_get_maps(target));

    return EXIT_SUCCESS;
}