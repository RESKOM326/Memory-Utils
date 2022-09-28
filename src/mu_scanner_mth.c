/**
 * @file mu_scanner_mth.c
 * @author Mark Dervishaj
 * @brief Multithreaded implementation of mu_scanner.h
 * @version 0.1
 * @date 2022-09-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "inc/mu_scanner.h"
#include "inc/mu_memchunk.h"
#include "inc/mu_io.h"
#include "inc/mu_diag.h"
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/uio.h>
#include <sched.h>
#include <pthread.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif  /* _GNU_SOURCE */

#define MODIF_CHNKS     1
#define NUM_CORES       16

/* GLOBAL VARIABLES USED BY ALL FUNCTIONS */
MU_OFFSETS offsets[NUM_CORES];
MU_MEM_CHUNK *filtered;
ULONG *matches;
INT mt_curr_index;  /* To know where is the next available position to write a match */
pthread_mutex_t mt_mutex;
pthread_t tid[NUM_CORES];

void *finder(void *arg)
{
    diag_trace trace;
    INT *core_id = (INT *) arg;
    pthread_t id = pthread_self();
    cpu_set_t cpuset;

    if(*core_id < 0 || *core_id >= NUM_CORES)
    {
        sprintf(trace, "%s | Invalid core ID!", __func__);
        diag_critical(trace, ERR_GENERIC);
        return NULL;
    }

    CPU_ZERO(&cpuset);
    CPU_SET(*core_id, &cpuset);

    INT rv = pthread_setaffinity_np(id, sizeof(cpu_set_t), &cpuset);
    if(rv != 0)
    {
        sprintf(trace, "%s | Cannot set thread affinity to core!", __func__);
        diag_critical(trace, ERR_GENERIC);
        return NULL;
    }

    /* Find matches only in chunks assigned to this core */
    ULONG start = offsets[*core_id].off_start;
    ULONG end = start + offsets[*core_id].off_len;
    for(INT i = start; i < end; i++)
    {
        pthread_mutex_lock(&mt_mutex);
        /**
         * @TODO: Find matches and store them
         */
        mt_curr_index++;
        pthread_mutex_unlock(&mt_mutex);
    }

}

ULONG* execute_scanner(PID target, UCHAR *data, INT data_size, INT *n_matches)
{
    MU_ERROR is_ok = ERR_OK;
    diag_trace trace;
    INT size = 0;
    INT nmatches = 0;
    ULONG *matches = malloc((sizeof *matches)*(nmatches + 1));
    filtered = get_memory_chunks(target, 1, &size);

    /* Get number of threads of the CPU running this program */
    INT64 n_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if(n_cores != NUM_CORES)
    {
        is_ok = ERR_GENERIC;
        sprintf(trace, "%s | Mismatch between number of configured cores and real available cores!", __func__);
        diag_critical(trace, is_ok);
        exit(is_ok); 
    }

    /* INT id[NUM_CORES]; */
    INT n_elems = size/NUM_CORES;
    INT remnant = size%NUM_CORES;
    INT n_elem_in_index[NUM_CORES];

    /* Distribute number of memory chunks into the threads */
    for(INT i = 0; i < NUM_CORES; i++)
    {
        n_elem_in_index[i] = n_elems;
    }
    if(remnant != 0)
    {
        for(INT i = 0; i < remnant; i++)
        {
            n_elem_in_index[i]++;
        }
    }

    /* Pre-process chunks to create an array of offsets and lengths */
    INT offst = 0;
    for(INT i = 0; i < NUM_CORES; i++)
    {
        offsets[i].off_start = offst;
        INT len = n_elem_in_index[i];
        offsets[i].off_len = len;
        offst += len;
    }

    /* Create threads */
    for(INT i = 0; i < NUM_CORES; i++)
    {
        /* id[i] = i; */  /* Use id to pass a different pointer to each thread */
        INT rv = phtread_create(&(tid[i]), NULL, &finder, (void *)(i));
        if(rv != 0)
        {
            is_ok = ERR_GENERIC;
            sprintf(trace, "%s | Error creating thread %d!", __func__, i);
            diag_critical(trace, is_ok);
            exit(is_ok); 
        }
    }

    /* Join threads */
    for(INT i = 0; i < NUM_CORES; i++)
    {
        pthread_join(tid[i], NULL);
    }

    pthread_exit(NULL);

    return matches;

/*     for(INT i = 0; i < size; i++)
    {
        MU_MEM_CHUNK chnk = filtered[i];
        UCHAR *bytes = read_chunk_data(target, chnk);
        UCHAR *ptr = NULL;
        ptr = memmem(bytes, chnk.chunk_size, data, data_size);

        while(ptr)
        {
            ULONG offset = ptr - bytes;
            ULONG match = chnk.addr_start + offset;
            matches = realloc(matches, ((sizeof *matches)*(nmatches + 1)));
            if(matches == NULL)
            {
                is_ok = ERR_GENERIC;
                sprintf(trace, "%s | Error writing into memory of target process!", __func__);
                diag_critical(trace, is_ok);
                exit(is_ok);
            }
            matches[nmatches] = match;
            nmatches++;
            ptr = memmem(&bytes[offset + data_size], (chnk.chunk_size - offset - data_size), data, data_size);
        }
        free(chnk.chunk_name);
        free(bytes);
    }
    *n_matches = nmatches;
    free(filtered);

    return matches; */
}

MU_ERROR execute_filtering(PID target, ULONG **addresses, UCHAR *data, ULONG data_size, INT *n_matches)
{
    MU_ERROR is_ok = ERR_OK;
    diag_trace trace;
    INT nmatches = 0;
    INT og_n_matches = *n_matches;
    ULONG *matches = malloc((sizeof *matches)*(nmatches + 1));
    INT i = 0;
    while(i < og_n_matches)
    {
        struct iovec local[1];
        struct iovec remote[1];
        UCHAR read[data_size];

        local[0].iov_base = read;
        local[0].iov_len = data_size;
        remote[0].iov_base = (void *) (*addresses)[i];
        remote[0].iov_len = data_size;

        INT64 n_read = process_vm_readv(target, local, 1, remote, 1, 0);

        if(n_read < 0 || (ULONG) n_read != data_size)
        {
            is_ok = ERR_GENERIC;
            sprintf(trace, "%s | Error writing into memory of target process!", __func__);
            diag_critical(trace, is_ok);
            exit(is_ok);
        }
        int cmp = memcmp(read, data, data_size);
        if(cmp == 0)
        {
            ULONG match = (*addresses)[i];
            matches = realloc(matches, ((sizeof *matches)*(nmatches + 1)));
            if(matches == NULL)
            {
                is_ok = ERR_GENERIC;
                sprintf(trace, "%s | Error writing into memory of target process!", __func__);
                diag_critical(trace, is_ok);
                exit(is_ok);
            }
            matches[nmatches] = match;
            nmatches++;
        }
        i++;
    }

    *n_matches = nmatches;

    free(*addresses);
 
    *addresses = malloc((sizeof *addresses)*(nmatches + 1));
    memcpy(*addresses, matches, (sizeof *matches)*(nmatches + 1));
    free(matches);

    return ERR_OK;
}