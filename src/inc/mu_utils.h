/**
 * @file mu_scan_utils.h
 * @author Mark Dervishaj
 * @brief Utilities used by the memory scanner
 * @version 0.1
 * @date 2022-08-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _MU_UTILS
#define _MU_UTILS

#include "mu_types.h"

/**
 * @brief Check if the targeted process exists
 * 
 * @param target PID of the target process
 * @return True if the process exists, false otherwise 
 */
extern MU_ERROR pid_exists(PID target);

/**
 * @brief Get the path of the memory maps for the target process. 
 * REMEMBER TO FREE memory used by the path
 * 
 * @param target Process PID whose maps are needed
 * @return CHAR* Path to maps file
 */
extern CHAR* get_maps_path(PID target);

/**
 * @brief Get the path of the memory file for the target process. 
 * REMEMBER TO FREE memory used by the path to mem file
 * 
 * @param target Process PID whose mem file is needed
 * @return CHAR* Path to mem file
 */
extern CHAR* get_mem_path(PID target);

/**
 * @brief Get the path of the binary file corresponding to PID
 * 
 * @param target Process PID whose path to executable is needed
 * @return CHAR* Path to executable
 */
extern CHAR* get_exe_path(PID target);

#endif  /* _MU_UTILS */