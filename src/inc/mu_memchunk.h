/**
 * @file mu_memchunk.h
 * @author Mark Dervishaj
 * @brief Retrieves information of a memory chunk based on maps file
 * @version 0.1
 * @date 2022-08-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _MU_MEMCHUNK
#define _MU_MEMCHUNK

#include "mu_types.h"

/**
 * @brief Get the memory chunks from maps file. REMEMBER TO FREE the memory of the chunks array and chunk_name attrib
 * 
 * @param path_maps Absolute path of maps file
 * @param option 0 for all chunks, 1 for modifiable chunks
 * @param size pointer to store the size of the chunks array
 * @return MU_MEM_CHUNK[] Array with all the memory chunks retrieved. Null if an user-error occurred
 */
extern MU_MEM_CHUNK* get_memory_chunks(PID target, INT option, INT *size);

#endif  /* _MU_MEMCHUNK */