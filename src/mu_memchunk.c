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
#include "inc/mu_utils.h"
#include "inc/mu_diag.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

#define ALL_CHUNKS          0
#define MODIFIABLE_CHUNKS   1
#define LINE_BUFFER         256
#define IS_MODIFIABLE(chnk) (chnk.is_readable && chnk.is_writable && chnk.is_private)
#define REQ_MATCHES         6
#define OFFSET_CHNK_NAME    73

/**
 * @brief Parses a line from the maps file. REMEMBER TO FREE returned chunk
 * 
 * @param maps_line Line from the maps file
 * @return Pointer to a memory chunk struct, containing all neccessary fields
 */
static MU_MEM_CHUNK parse_maps_line(CHAR* maps_line)
{
    diag_trace trace;
    MU_ERROR is_ok = ERR_OK;
    regex_t regex;
    MU_MEM_CHUNK chunk;
    const CHAR *re = "([0-9A-Fa-f]+)-([0-9A-Fa-f]+) ([-r])([-w])[-x]([sp]).*";

    if(regcomp(&regex, re, REG_EXTENDED) != 0){
        is_ok = ERR_GENERIC;
        sprintf(trace, "%s | Error compiling regular expression!", __func__);
        diag_critical(trace, is_ok);
        exit(is_ok);
    }
    regmatch_t matches[REQ_MATCHES]; 
    if(regexec(&regex, maps_line, REQ_MATCHES, matches, 0) != 0){
        is_ok = ERR_GENERIC;
        sprintf(trace, "%s | Error in memory map line format!", __func__);
        diag_critical(trace, is_ok);
        exit(is_ok);
    }

    /* Get starting chunk memory address */
    CHAR *start = malloc(matches[1].rm_eo - matches[1].rm_so + 1);
    memcpy(start, maps_line + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);

    /* Get ending chunk memory address */
    CHAR *end = malloc(matches[2].rm_eo - matches[2].rm_so + 1);
    memcpy(end, maps_line + matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);

    /* Get read permissions */
    CHAR *r = malloc(matches[3].rm_eo - matches[3].rm_so + 1);
    memcpy(r, maps_line + matches[3].rm_so, matches[3].rm_eo - matches[3].rm_so);

    /* Get write permissions */
    CHAR *w = malloc(matches[4].rm_eo - matches[4].rm_so + 1);
    memcpy(w, maps_line + matches[4].rm_so, matches[4].rm_eo - matches[4].rm_so);
    
    /* Get private status */
    CHAR *p = malloc(matches[5].rm_eo - matches[5].rm_so + 1); 
    memcpy(p, maps_line + matches[5].rm_so, matches[5].rm_eo - matches[5].rm_so);

    /* Get name of the region */
    INT sz = matches[0].rm_eo - matches[0].rm_so;
    if(sz > OFFSET_CHNK_NAME)
    {
        sz = matches[0].rm_eo - OFFSET_CHNK_NAME - 1;
        chunk.chunk_name = malloc(sz + 1);
        memcpy(chunk.chunk_name, maps_line + OFFSET_CHNK_NAME, sz);
        chunk.chnk_name_sz = sz;
    }
    else
    {
        INT to_reserve = 4; /* "NULL has 4 bytes" */
        chunk.chunk_name = malloc(to_reserve + 1);
        memcpy(chunk.chunk_name, "NULL", to_reserve);
        chunk.chnk_name_sz = to_reserve;
    }

    ULONG addr_start = (ULONG) strtoll(start, NULL, 16);
    ULONG addr_end = (ULONG) strtoll(end, NULL, 16);
    ULONG size = addr_end - addr_start;

    BOOL readable = r[0] == 'r';
    BOOL writable = w[0] == 'w';
    BOOL priv = p[0] == 'p';

    chunk.addr_start = addr_start; 
    chunk.chunk_size = size; 
    chunk.is_readable = readable;
    chunk.is_writable = writable; 
    chunk.is_private = priv;

    free(start);
    free(end);
    free(r);
    free(w);
    free(p);

    return chunk;
}

MU_MEM_CHUNK* get_memory_chunks(PID target, INT option, INT *size)
{
    diag_trace trace;
    MU_ERROR is_ok = ERR_OK;
    MU_MEM_CHUNK *chunks;

    if(option != ALL_CHUNKS && option != MODIFIABLE_CHUNKS)
    {
        is_ok = ERR_FUNC_OPT;
        sprintf(trace, "%s | Wrong option! Valid options are 0 and 1", __func__);
        diag_error(trace, is_ok);
        chunks = NULL;
    }
    else
    {
        /* Open maps file and parse each line. Add to array if necessary */
        CHAR *path_maps = get_maps_path(target);
        FILE *maps;
        INT n_chunks = 0;

        /* Reserve memory for at least one chunk */
        chunks = malloc(sizeof(*chunks)*(n_chunks + 1));
        CHAR line[LINE_BUFFER];
        maps = fopen(path_maps, "r");
        if(maps == NULL)
        {
            is_ok = ERR_GENERIC;
            sprintf(trace, "%s | Error opening /proc/%d/maps!", __func__, target);
            diag_critical(trace, is_ok);
            exit(is_ok);
        }
        /* It will process chunks as long as they are modifiable AND MODIF option OR ALL option */
        INT flag = option;  /* Store parameter to local variable */
        while(fgets(line, LINE_BUFFER, maps))
        {
            MU_MEM_CHUNK chunk = parse_maps_line(line);
            if((flag == MODIFIABLE_CHUNKS && IS_MODIFIABLE(chunk)) || flag == ALL_CHUNKS)
            {
                chunks = realloc(chunks, ((sizeof(*chunks))*(n_chunks + 1)));
                if(chunks == NULL)
                {
                    is_ok = ERR_GENERIC;
                    sprintf(trace, "%s | Cannot reserve more dynamic memory!", __func__);
                    diag_critical(trace, is_ok);
                    exit(is_ok);
                }
                chunks[n_chunks++] = chunk;
            }
        }
        /* Update size and liberate resources */
        *size = n_chunks;
        fclose(maps);
        free(path_maps);
    }

    return chunks;
}

MU_MEM_CHUNK* filter_memory_chunks(PID target, MU_MEM_CHUNK *chunks, INT *size)
{
    diag_trace trace;
    MU_ERROR is_ok = ERR_OK;
    CHAR *exe_path = get_exe_path(target);
    CHAR exec_name[LINE_BUFFER];
    INT n_filtered = 0;

    MU_MEM_CHUNK *filtered;
    /* Reserve memory for at least one chunk */
    filtered = malloc(sizeof(*filtered)*(n_filtered + 1));

    const CHAR *HEAP = "[heap]";
    const CHAR *STACK = "[stack]";

    INT ret_val = readlink(exe_path, exec_name, LINE_BUFFER);
    
    if(ret_val < 0)
    {
        is_ok = ERR_GENERIC;
        sprintf(trace, "%s | Cannot get absolute path of the target binary!", __func__);
        diag_critical(trace, is_ok);
        exit(is_ok);
    }

    /* Print only chunks EXEC, HEAP and STACK */
    for(int i = 0; i < *size; i++)
    {
        if( !strncmp(exec_name, chunks[i].chunk_name, chunks[i].chnk_name_sz) ||
            !strncmp(HEAP,      chunks[i].chunk_name, chunks[i].chnk_name_sz) ||
            !strncmp(STACK,     chunks[i].chunk_name, chunks[i].chnk_name_sz))
        {
            filtered = realloc(filtered, ((sizeof(*filtered))*(n_filtered + 1)));
            if(filtered == NULL)
            {
                is_ok = ERR_GENERIC;
                sprintf(trace, "%s | Cannot reserve more dynamic memory!", __func__);
                diag_critical(trace, is_ok);
                exit(is_ok);
            }
            filtered[n_filtered++] = chunks[i];
        }
    }
    *size = n_filtered;

    return filtered;
}