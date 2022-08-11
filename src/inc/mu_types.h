/**
 * @file mu_types.h
 * @author Mark Dervishaj
 * @brief Definitions and types for Memory Utils
 * @version 0.1
 * @date 2022-08-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _MU_TYPES_H
#define _MU_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

/* Redefinitions of types for more clarity */
typedef unsigned char   UCHAR;
typedef char            CHAR;
typedef short           INT16;
typedef int             INT32;
typedef int             INT;
typedef long            INT64;
typedef size_t          ULONG;
typedef float           REAL32;
typedef double          REAL64;
typedef bool            BOOL;
typedef pid_t           PID;

/* Union used to change between bytes and data type - MAYBE UNNECESSARY */
typedef union data_types
{
    UCHAR   byte;
    INT16   int16;
    INT32   int32;
    INT64   int64;
    REAL32  real32;
    REAL64  real64;
    UCHAR   bytes16[2];     /* Byte information for 16-bit data type */
    UCHAR   bytes32[4];     /* Byte information for 32-bit data type */
    UCHAR   bytes64[8];     /* Byte information for 64-bit data type */

} MU_TYPES;

/* Enum to store error codes */
typedef enum error_codes
{
    ERR_OK          =   0,          /* No error */
    ERR_EPERM       =   100,        /* pid_exists -> No permissions */
    ERR_ESRCH       =   101,        /* pid_exists -> Does not exist */
    ERR_GENERIC     =   500         /* Generic error for C/Sys function calls */

} MU_ERROR;

/* struct to store information about a memory chunk */
typedef struct memory_chunk
{
    INT64   addr_start;
    ULONG   chunk_size;
    BOOL    is_readable;
    BOOL    is_writable;
    BOOL    is_private;
    BOOL    modifiable;
    
} MU_MEM_CHUNK;

#endif  /* _MU_TYPES_H */