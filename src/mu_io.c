/**
 * @file mu_io.c
 * @author Mark Dervishaj
 * @brief Implementation of mu_io.h
 * @version 0.1
 * @date 2022-08-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "inc/mu_io.h"
#include "inc/mu_memchunk.h"
#include "inc/mu_diag.h"
#include <stdio.h>
#include <sys/uio.h>
#include <errno.h>
#include <string.h>

UCHAR* read_chunk_data(PID target, MU_MEM_CHUNK chunk)
{
    diag_trace trace;
    MU_ERROR is_ok = ERR_OK;

    struct iovec local[1];
    struct iovec remote[1];

    ULONG to_read = chunk.chunk_size;
    UCHAR *r_buffer = malloc(to_read);

    local[0].iov_base = r_buffer;
    local[0].iov_len = to_read;
    remote[0].iov_base = (void *) chunk.addr_start;
    remote[0].iov_len = to_read;

    INT64 n_read = process_vm_readv(target, local, 1, remote, 1, 0);

    /* If n_read < 0, it enters. If not, value is unsigned and can be safely casted to size_t for comparision */
    if(n_read < 0 || (ULONG) n_read != to_read)
    {
        is_ok = ERR_GENERIC;
        sprintf(trace, "%s | Error reading memory of target process!", __func__);
        diag_critical(trace, is_ok);
        exit(is_ok);
    }

    return r_buffer;
}