/**
 * @file mu_memchunk.c
 * @author Mark Dervishaj
 * @brief Implementation of mu_memchunk.h
 * @version 0.1
 * @date 2022-08-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "inc/mu_memchunk.h"
#include "inc/mu_types.h"
#include "inc/mu_utils.h"
#include "inc/mu_diag.h"
#include <stdio.h>

#define ALL_CHUNKS          0
#define MODIFIABLE_CHUNKS   1
#define IS_MODIFIABLE(chnk) (chnk->is_readable && chnk->is_writable && chnk->is_private)

MU_MEM_CHUNK* get_memory_chunks(PID target, INT option)
{
    diag_trace trace;
    MU_ERROR is_ok = ERR_OK;
    CHAR *path_maps = get_maps_path(target);
    if(option != ALL_CHUNKS && option != MODIFIABLE_CHUNKS)
    {
        is_ok = ERR_GENERIC;
        sprintf(trace, "%s | Wrong option! Valid options are 0 and 1", __func__);
        diag_error(trace, is_ok);
    }
    /* chnk = parse_maps_while (loop) -> malloc mem_chunks */
    for (size_t i = 0; i < 0; i++)
    {
        /* code */
    }
    
    if(option == MODIFIABLE_CHUNKS)
    {
        /* Get modifiable memory chunks */
        /* if(IS_MODIFIABLE(chnk)) {do stuff;} */
    }
    free(path_maps);
}