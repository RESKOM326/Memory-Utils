/**
 * @file mu_io.h
 * @author Mark Dervishaj
 * @brief Operations and algorithms for searching, reading, and writing in memory
 * @version 0.1
 * @date 2022-08-18
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _MU_IO
#define _MU_IO

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif  /* _GNU_SOURCE */

#include "mu_types.h"

/**
 * @brief Reads the data from a target's memory chunk. REMEMBER TO FREE read data
 * 
 * @param target PID of the target process
 * @param chunk Chunk which to read data from
 * @return UCHAR array with the data read
 */
extern UCHAR* read_chunk_data(PID target, MU_MEM_CHUNK chunk);

/**
 * @brief Writes data into a target's memory chunk
 * 
 * @param target PID of the target process
 * @param address Starting memory address 
 * @param data Data to write
 * @return Error code indicating this operation status 
 */
extern MU_ERROR write_chunk_data(PID target, INT64 address, UCHAR *data);

/**
 * @brief Modifies the final matches with the wanted value
 * 
 * @param target PID of the target process
 * @param addresses Starting memory addresses of the matches
 * @param data Data to write
 * @param size Number of matches to modify
 * @return Error code indicating this operation status 
 */
extern MU_ERROR modify_value(PID target, INT64 *addresses, UCHAR *data, INT size);

#endif  /* _MU_IO */