/**
 * @file mu_scanner.h
 * @author Mark Dervishaj
 * @brief Scanning and filtering in memory of the searched value
 * @version 0.1
 * @date 2022-08-19
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _MU_SCANNER_H
#define _MU_SCANNER_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif  /* _GNU_SOURCE */

#include "mu_types.h"

/**
 * @brief Scans through the target memory in search of the desired value. 
 * This version uses Boyer-Moore algorithm.
 * 
 * @param target PID of the target process
 * @param data Data bytes to search
 * @param data_size Size in bytes of the data
 * @param n_matches Stores the number of matching addresses
 * @return List of addresses that match the desired value
 */
extern ULONG* execute_scanner_BM(PID target, UCHAR *data, INT data_size, INT *n_matches);

/**
 * @brief Scans through the target memory in search of the desired value. 
 * This version uses optimized search with "memmem" from feature test macros.
 * 
 * @param target PID of the target process
 * @param data Data bytes to search
 * @param data_size Size in bytes of the data
 * @param n_matches Stores the number of matching addresses
 * @return List of addresses that match the desired value
 */
extern ULONG* execute_scanner_STD(PID target, UCHAR *data, INT data_size, INT *n_matches);

/**
 * @brief Scans through the target memory in search of the desired value. 
 * Same as STD version, but also using multithreading.
 * 
 * @param target PID of the target process
 * @param data Data bytes to search
 * @param data_size Size in bytes of the data
 * @param n_matches Stores the number of matching addresses
 * @return List of addresses that match the desired value
 */
extern ULONG* execute_scanner_STD_MTH(PID target, UCHAR *data, INT data_size, INT *n_matches);

/**
 * @brief Filters a list of addresses to narrow down the required address/es
 * 
 * @param target PID of the target process
 * @param addresses List of potential addresses narrowed down
 * @param data Data bytes to search
 * @param data_size Size in bytes of the data
 * @param n_matches Stores the number of matching addresses
 * @return MU_ERROR 
 */
extern MU_ERROR execute_filtering(PID target, ULONG **addresses, UCHAR *data, ULONG data_size, INT *n_matches);

#endif  /* _MU_SCANNER_H */