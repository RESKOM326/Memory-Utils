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
#include "../../src/inc/mu_utils.h"
#include <stdio.h>

MU_ERROR test_pid_exists(PID target)
{
    return pid_exists(target);
}

MU_ERROR test_get_paths(PID target, INT option)
{
    MU_ERROR is_ok = ERR_OK;
    CHAR *path = (option == 0) ? get_maps_path(target) : get_mem_path(target);
    if(path == NULL)
    {
        is_ok = ERR_GENERIC;
    }
    else
    {
        printf("%s - %s\n", __func__, path);
    }
    free(path);

    return is_ok; 
}

MU_ERROR test_combined(PID target)
{
    MU_ERROR is_ok = ERR_OK;
    is_ok = pid_exists(target);
    if(!is_ok)
    {
        CHAR *path_maps = get_maps_path(target);
        CHAR *path_mem = get_mem_path(target);
        if(path_maps == NULL || path_mem == NULL)
        {
            is_ok = ERR_GENERIC;
        }
        else
        {
            printf("%s - %s\n", __func__, path_maps);
            printf("%s - %s\n", __func__, path_mem);
        }
        free(path_maps);
        free(path_mem);
    }

    return is_ok;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
INT main(INT argc, CHAR **argv)
{
    INT target = atoi(argv[1]);
    printf("RUN TEST PID_EXISTS:\t%d\n\n", test_pid_exists(target));
    printf("RUN TEST GET_MAPS:\t%d\n\n", test_get_paths(target, 0));
    printf("RUN TEST GET_MEM:\t%d\n\n", test_get_paths(target, 1));
    printf("RUN TEST COMBINED:\t%d\n\n", test_combined(target));

    return EXIT_SUCCESS;
}
#pragma GCC diagnostic pop