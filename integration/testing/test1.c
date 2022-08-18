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
#include <stdio.h>

MU_ERROR test_pid_exists(PID target)
{
    return pid_exists(target);
}

MU_ERROR test_get_paths(PID target, INT option)
{
    MU_ERROR is_ok = ERR_OK;

    /* Options: 0 for maps, 1 for mem, 2 for exe */
    CHAR *path = (option == 0) ? get_maps_path(target) : (option == 1) ? get_mem_path(target) : get_exe_path(target);
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

INT main(INT argc, CHAR **argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "NO PID PROVIDED!! Usage: ./test1 <target_pid>\n");
        exit(255);
    }
    INT target = atoi(argv[1]);

    printf("RUN TEST PID_EXISTS:\t%d\n\n", test_pid_exists(target));
    printf("RUN TEST GET_MAPS:\t%d\n\n", test_get_paths(target, 0));
    printf("RUN TEST GET_MEM:\t%d\n\n", test_get_paths(target, 1));
    printf("RUN TEST GET_EXE:\t%d\n\n", test_get_paths(target, 2));
    printf("RUN TEST COMBINED:\t%d\n\n", test_combined(target));
    printf("****************************************************************"
            "****************************************************************\n\n");
    printf("RUN TEST GET_MEM_CHNKS_ALL:\t%d\n\n", test_memchunks(target, 0));
    printf("RUN TEST GET_MEM_CHNKS_MOD:\t%d\n\n", test_memchunks(target, 1));
    printf("RUN TEST FILTER_ALL_CHUNKS:\t%d\n\n", test_filter_chunks(target, 0));
    printf("RUN TEST FILTER_MOD_CHUNKS:\t%d\n\n", test_filter_chunks(target, 1));
    printf("****************************************************************"
            "****************************************************************\n\n");
    printf("RUN TEST READ_CHUNK_DATA:\t%d\n\n", test_read_chunk_data(target));

    return EXIT_SUCCESS;
}