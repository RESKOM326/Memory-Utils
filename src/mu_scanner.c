/**
 * @file mu_scanner.c
 * @author Mark Dervishaj
 * @brief Implementation of mu_scanner.h
 * @version 0.1
 * @date 2022-08-19
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif  /* _GNU_SOURCE */

#define MODIF_CHNKS     1

ULONG* execute_scanner_STD(PID target, UCHAR *data, INT data_size, INT *n_matches)
{
    MU_ERROR is_ok = ERR_OK;
    diag_trace trace;
    INT size = 0;
    INT nmatches = 0;
    ULONG *matches = malloc((sizeof *matches)*(nmatches + 1));
    MU_MEM_CHUNK *filtered = get_memory_chunks(target, 1, &size);

    for(INT i = 0; i < size; i++)
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

    return matches;
}

ULONG* execute_scanner_STD_MTH(PID target, UCHAR *data, INT data_size, INT *n_matches)
{
    MU_ERROR is_ok = ERR_OK;
    diag_trace trace;
    INT size = 0;
    INT nmatches = 0;
    ULONG *matches = malloc((sizeof *matches)*(nmatches + 1));
    MU_MEM_CHUNK *filtered = get_memory_chunks(target, MODIF_CHNKS, &size);

    for(INT i = 0; i < size; i++)
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

    return matches;
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