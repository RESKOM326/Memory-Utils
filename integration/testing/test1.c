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
#include "../../src/inc/mu_memchunk.h"
#include "../../src/inc/mu_io.h"
#include "../../src/inc/mu_scanner.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define ALL_CHUNKS      0
#define MOD_CHUNKS      1

#define OPT_MAPS        0
#define OPT_MEM         1
#define OPT_EXE         2

#define ALG_BOYER_MOORE 0
#define ALG_GNU_MEMMEM  1
#define ALG_MEMMEM_MTH  2

#define BILLION         1000000000.0

MU_ERROR test_pid_exists(PID target)
{
    return pid_exists(target);
}

MU_ERROR test_get_paths(PID target, INT option)
{
    MU_ERROR is_ok = ERR_OK;

    /* Options: 0 for maps, 1 for mem, 2 for exe */
    CHAR *path = (option == OPT_MAPS) ? get_maps_path(target) : (option == OPT_MEM) ? get_mem_path(target) : get_exe_path(target);
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
        CHAR *path_exe = get_exe_path(target);
        if(path_maps == NULL || path_mem == NULL || path_exe == NULL)
        {
            is_ok = ERR_GENERIC;
        }
        else
        {
            printf("%s - %s\n", __func__, path_maps);
            printf("%s - %s\n", __func__, path_mem);
            printf("%s - %s\n", __func__, path_exe);
        }
        free(path_maps);
        free(path_mem);
        free(path_exe);
    }

    return is_ok;
}

MU_ERROR test_memchunks(PID target, INT option)
{
    MU_ERROR is_ok = ERR_OK;
    INT size = 0;
    MU_MEM_CHUNK *chunks = get_memory_chunks(target, option, &size);
    
    if(chunks == NULL)
    {
        is_ok = ERR_GENERIC;
    }
    else{
        for(int i = 0; i < size; i++)
        {
            MU_MEM_CHUNK chunk = chunks[i];
            printf("CHUNK %-2d | Start Addr: %-19ld, Size: %-7ld, R: %d, W: %d, P: %d, Name: %s (%ld)\n", i,
            chunk.addr_start, chunk.chunk_size, chunk.is_readable, chunk.is_writable,
            chunk.is_private, chunk.chunk_name, chunk.chnk_name_sz);

            free(chunk.chunk_name);
        }
        free(chunks);
    }

    return is_ok;
}

MU_ERROR test_filter_chunks(PID target, INT option)
{
    MU_ERROR is_ok = ERR_OK;
    INT size = 0;
    MU_MEM_CHUNK *chunks = get_memory_chunks(target, option, &size);
    MU_MEM_CHUNK *filtered = filter_memory_chunks(target, chunks, &size);
    
    if(filtered == NULL)
    {
        is_ok = ERR_GENERIC;
    }
    else
    {
        for(int i = 0; i < size; i++)
        {
            MU_MEM_CHUNK chunk = filtered[i];
            printf("CHUNK %-2d | Start Addr: %-19ld, Size: %-7ld, R: %d, W: %d, P: %d, Name: %s (%ld)\n", i,
            chunk.addr_start, chunk.chunk_size, chunk.is_readable, chunk.is_writable,
            chunk.is_private, chunk.chunk_name, chunk.chnk_name_sz);

            free(chunk.chunk_name);
        }
        free(filtered);
    }

    return is_ok;
}

MU_ERROR test_read_chunk_data(PID target)
{
    MU_ERROR is_ok = ERR_OK;
    INT size = 0;
    MU_MEM_CHUNK *chunks = get_memory_chunks(target, 1, &size);
    MU_MEM_CHUNK *filtered = filter_memory_chunks(target, chunks, &size);
    
    if(filtered == NULL)
    {
        is_ok = ERR_GENERIC;
    }
    else
    {
        for(int i = 0; i < size; i++)
        {
            MU_MEM_CHUNK chunk = filtered[i+1];
            UCHAR *r_buff = read_chunk_data(target, chunk);
            INT j = 0;
            printf("BYTES READ CHNK %d: ", i);
            while(j < 16)
            {
                printf("%02X, ", r_buff[j]);
                if(j == 15) printf("...\n");
                j++;
            }
            free(chunk.chunk_name);
            free(r_buff);
        }
        free(filtered);
    }

    return is_ok;  
}

MU_ERROR test_modify_values(PID target)
{
    MU_ERROR is_ok = ERR_OK;
    INT32 new_value = 999999;
    ULONG addresses[2];
    addresses[0] = (ULONG) 0x7ffc1b6bbab8;
    addresses[1] = (ULONG) 0x7ffc1b6bbabc;
    INT size = sizeof(INT32);
    UCHAR bytes[size];
    memcpy(bytes, &new_value, size);

    printf("Writing 999999 as new silver and bronze values in the simulator...\n");

    is_ok = modify_values(target, addresses, 2, bytes, size);

    return is_ok;
}

MU_ERROR test_scanner_and_efficiency(PID target)
{
    MU_ERROR is_ok = ERR_OK;
    INT16 to_search = 12500;
    INT size = sizeof(INT16);
    UCHAR bytes[size];
    memcpy(bytes, &to_search, size);

    /* Benchmark */
    struct timespec start;
    struct timespec end;
    REAL64 elapsed_time;

    printf("Finding matches of INT16 - 12500...\n");

    INT n_matches = 0;
    ULONG *matches;
    clock_gettime(CLOCK_MONOTONIC, &start);
    matches = execute_scanner(target, bytes, size, &n_matches);
    clock_gettime(CLOCK_MONOTONIC, &end);

    for(INT i = 0; i < n_matches; i++)
    {
        printf("Match %i: %#lx\n", i+1, matches[i]);
    }
    free(matches);

    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / BILLION;
    printf("ELAPSED TIME: %f\n", elapsed_time);

    return is_ok;   
}

MU_ERROR test_filtering(PID target)
{
    MU_ERROR is_ok = ERR_OK;
    INT32 to_search = 1000;
    INT32 size = sizeof(INT32);
    UCHAR bytes[size];
    memcpy(bytes, &to_search, size);

    printf("Finding matches of INT32 - 1000...\n");

    INT n_matches = 0;
    ULONG *matches;

    /* Scan and wait 5 seconds to change simulator values */
    matches = execute_scanner(target, bytes, size, &n_matches);

    for(INT i = 0; i < n_matches; i++)
    {
        printf("Match %i: %#lx\n", i+1, matches[i]);
    }
    sleep(5);

    printf("\nExecuting filtering with 999...\n\n");
    to_search = 999;
    memcpy(bytes, &to_search, size);

    is_ok = execute_filtering(target, &matches, bytes, size, &n_matches);

    for(INT i = 0; i < n_matches; i++)
    {
        printf("Match %i: %#lx\n", i+1, matches[i]);
    }

    free(matches);

    return is_ok;   
}

MU_ERROR test_scan_filter_modidy(PID target)
{
    MU_ERROR is_ok = ERR_OK;
    INT32 to_search = 999;
    INT32 size = sizeof(INT32);
    UCHAR bytes[size];
    memcpy(bytes, &to_search, size);

    printf("Finding matches of INT32 - 1000...\n");

    INT n_matches = 0;
    ULONG *matches;

    /* Scan and wait 5 seconds to change simulator values */
    matches = execute_scanner(target, bytes, size, &n_matches);

    for(INT i = 0; i < n_matches; i++)
    {
        printf("Match %i: %#lx\n", i+1, matches[i]);
    }
    sleep(5);

    printf("\nExecuting filtering with 999...\n\n");
    to_search = 998;
    memcpy(bytes, &to_search, size);

    is_ok = execute_filtering(target, &matches, bytes, size, &n_matches);

    for(INT i = 0; i < n_matches; i++)
    {
        printf("Match %i: %#lx\n", i+1, matches[i]);
    }

    /* Modify found value to 9999999 */
    INT32 new_val = 9999999;
    memcpy(bytes, &new_val, size);

    printf("\nModify found value to 9999999...\n\n");
    is_ok = modify_values(target, matches, n_matches, bytes, size);

    free(matches);

    return is_ok;   
}

INT main(INT argc, CHAR **argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "NO PID PROVIDED!! Usage: ./test1 <target_pid>\n");
        exit(255);
    }
    INT target = atoi(argv[1]);

    printf("RUN TEST PID_EXISTS:\t%d\n\n", test_pid_exists(target));
    printf("RUN TEST GET_MAPS:\t%d\n\n", test_get_paths(target, OPT_MAPS));
    printf("RUN TEST GET_MEM:\t%d\n\n", test_get_paths(target, OPT_MEM));
    printf("RUN TEST GET_EXE:\t%d\n\n", test_get_paths(target, OPT_EXE));
    printf("RUN TEST COMBINED:\t%d\n\n", test_combined(target));
    printf("****************************************************************"
            "****************************************************************\n\n");
    printf("RUN TEST GET_MEM_CHNKS_ALL:\t%d\n\n", test_memchunks(target, ALL_CHUNKS));
    printf("RUN TEST GET_MEM_CHNKS_MOD:\t%d\n\n", test_memchunks(target, MOD_CHUNKS));
    printf("RUN TEST FILTER_ALL_CHUNKS:\t%d\n\n", test_filter_chunks(target, ALL_CHUNKS));
    printf("RUN TEST FILTER_MOD_CHUNKS:\t%d\n\n", test_filter_chunks(target, MOD_CHUNKS));
    printf("****************************************************************"
            "****************************************************************\n\n");
    printf("RUN TEST READ_CHUNK_DATA:\t%d\n\n", test_read_chunk_data(target));
    printf("RUN TEST MODIFY_VALUES:\t%d\n\n", test_modify_values(target));
    printf("****************************************************************"
            "****************************************************************\n\n");
    printf("RUN TEST EXECUTE_SCAN_AND_EFFICIENCY:\t%d\n\n", test_scanner_and_efficiency(target));
    printf("RUN TEST EXECUTE_FILTERING:\t%d\n\n", test_filtering(target));
    printf("RUN TEST EXECUTE_SCAN_FILTER_MODIFY:\t%d\n\n", test_scan_filter_modidy(target));
    printf("****************************************************************"
            "****************************************************************\n\n");
    printf("N_CORES_ONLN: %ld\n", sysconf(_SC_NPROCESSORS_ONLN));

    return EXIT_SUCCESS;
}