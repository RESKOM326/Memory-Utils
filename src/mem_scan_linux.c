/**
 * @file mem_scan_linux.c
 * @author Mark Dervishaj 
 * @brief Main program of the Linux Memory Scanner
 * @version 0.1
 * @date 2022-08-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "inc/mu_types.h"
#include "inc/mu_utils.h"
#include "inc/mu_io.h"
#include "inc/mu_scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <time.h>

#define OPT_1BYTE   0
#define OPT_INT16   1
#define OPT_INT32   2
#define OPT_INT64   3
#define OPT_REALF   4
#define OPT_REALD   5
#define OPT_STRNG   6

#define ASK_FILTER  0
#define ASK_SCAN    1

#define MAX_STR_SZ  1023

#define BILLION     1000000000.0

#define NEWL_TO_NUL(buf)    if(buf[strlen(buf) - 1] == '\n'){buf[strlen(buf) - 1] = '\0';}

void show_help();
void show_types();
BOOL ask_for_more(INT option);
INT ask_data(INT type_index, UCHAR **data);

/**
 * @brief Main workflow
 * 
 * @param argc Number of arguments
 * @param argv List of arguments
 * @return Error code
 */
INT main(int argc, char **argv)
{
    /* Benchmark */
    struct timespec start;
    struct timespec end;
    REAL64 elapsed_time;

    /* CHECK ARGUMENTS ------------------------------------------------------------------- */

    if(argc != 2)
    {
        fprintf(stderr, "Error in arguments. See 'mem_scan_linux --help' for usage\n");
        exit(ERR_ARGS_MAIN);
    }
    PID target = atoi(argv[argc - 1]);
    if(target == 0)
    {
        INT ret = strcmp(argv[argc - 1], "--help");
        if(ret == 0)
        {
            show_help();
            exit(ERR_ARGS_MAIN);
        }
        else
        {
            fprintf(stderr, "Error in arguments. See 'mem_scan_linux --help' for usage\n");
            exit(ERR_ARGS_MAIN);
        }
    }

    /* CHECK IF PID EXISTS --------------------------------------------------------------- */

    if(pid_exists(target)){
        exit(ERR_FUNC_OPT);
    }

    /* ASK VALUE TYPE -------------------------------------------------------------------- */

    const CHAR *data_types[] = {"8-Bit Integer", "16-Bit Integer", "32-Bit Integer", "64-Bit Integer", "Float", "Double", "String"};

    BOOL keep_scan = true;
    while(keep_scan)
    {
        BOOL go_to_end = false;
        printf("Available data types:\n");
        show_types();
        fflush(stdin);
        printf("Please, select the value type: ");
        BOOL selected = false;
        INT c;
        while(!selected)
        {
            /* Use to read input and parse numbers */
            CHAR input_buff[MAX_STR_SZ];
            CHAR *thrash;
            fgets(input_buff, MAX_STR_SZ, stdin);
            NEWL_TO_NUL(input_buff);   
            c = (INT32) strtol(input_buff, &thrash, 10);
            if(*thrash != '\0' || c < 1 || c > 7)
            {
                printf("Please, select a correct value type (choice between 1 and 7): ");
            } 
            else selected = true;
        }
        printf("Type selected: %s\n\n", data_types[c - 1]);

    /* ASK DATA VALUE  ------------------------------------------------------------------- */

        INT type_index = c - 1;
        printf("Please, select the value to search: ");

        UCHAR *data;
        INT data_size;
        ULONG *matches;
        INT n_matches;

        data_size = ask_data(type_index, &data);

    /* SCANNING -------------------------------------------------------------------------- */

        printf("Please wait...\n\n");
        clock_gettime(CLOCK_MONOTONIC, &start);
        matches = execute_scanner(target, data, data_size, &n_matches);
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / BILLION;
        printf("ELAPSED TIME: %f\n", elapsed_time);
        free(data);
        if(n_matches == 0)
        {
            printf("No matches found\n");
            go_to_end = true;
        }
        else
        {
            printf("%i address<es> matching the value\n", n_matches);
        }

    /* FILTERING ------------------------------------------------------------------------- */

        if(!go_to_end)
        {
            BOOL done_filter = true;
            BOOL stop_filter = false;
            if(ask_for_more(ASK_FILTER))
            {
                while(!stop_filter)
                {
                    printf("\nPlease, select the value to search: ");
                    data_size = ask_data(type_index, &data);
                    printf("Please wait...\n\n");
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    execute_filtering(target, &matches, data, data_size, &n_matches);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / BILLION;
                    printf("ELAPSED TIME: %f\n", elapsed_time);
                    free(data);
                    if(n_matches == 0)
                    {
                        printf("No matches found\n");
                        done_filter = false;
                        break;
                    }
                    printf("%i address<es> matching the value\n", n_matches);
                    if(!ask_for_more(ASK_FILTER))
                    {
                        stop_filter = true;
                    }
                }
            }
            if(done_filter)
            {
                for(INT i = 0; i < n_matches; i++)
                {
                    printf("Address: %#lx\n", matches[i]);
                }
            }

        /* MODIFY VALUES --------------------------------------------------------------------- */

            if(done_filter)
            {
                printf("\nPlease, enter the value for the new address<es>: ");
                data_size = ask_data(type_index, &data);
                printf("Please wait...\n\n");
                modify_values(target, matches, n_matches, data, data_size);
                printf("Value<s> modified\n\n");
            }
        }
        free(matches);

        if(!ask_for_more(ASK_SCAN))
        {
            keep_scan = false;
        }
    }
    return ERR_OK;
}

/**
 * @brief Prints a sensible usage message
 * 
 */
void show_help()
{
    printf("Usage: mem_scan_linux <pid_of_target>\n");
}

/**
 * @brief Shows data types available to scan
 * 
 */
void show_types()
{
    printf("\n1) 8-Bit Integer     (1 Byte)\n");
    printf("2) 16-Bit Integer    (2 Bytes)\n");
    printf("3) 32-Bit Integer    (4 Bytes)\n");
    printf("4) 64-Bit Integer    (8 Bytes)\n");
    printf("5) Float             (4 Bytes)\n");
    printf("6) Double            (8 Bytes)\n");
    printf("7) String            (Up to 1023 characters)\n");
}

/**
 * @brief Asks if the user wants more scanning or filtering
 * 
 * @param option ASK_FILTER for filtering. ASK_SCAN for scanning
 * @return True if user wants more filter or scanning. False otherwise
 */
BOOL ask_for_more(INT option)
{
    BOOL correct_choice = false;
    INT keep_searching = 0;
    CHAR input_buff[MAX_STR_SZ];
    CHAR *thrash;
    while(!correct_choice)
    {
        if(option == ASK_FILTER)
        {
            printf("Do you want to filter the matches? (0: no; 1: yes): ");
        }
        else printf("Do you want to scan more values? (0: no; 1: yes): ");

        fgets(input_buff, MAX_STR_SZ, stdin);
        NEWL_TO_NUL(input_buff);
        keep_searching = (INT32) strtol(input_buff, &thrash, 10);

        if(*thrash != '\0' || keep_searching < 0 || keep_searching > 1)
        {
            printf("\nPlease, select a valid choice (1 for YES, 0 for NO): ");
        } 
        else correct_choice = true;
    }
    return (keep_searching == 1) ? true : false;
}

/**
 * @brief Asks the user for the data value, checks if value is within type limits, 
 * and provides with a byte array representation of the value. REMEMBER TO FREE data array
 * 
 * @param type_index Data type
 * @param data Pointer to byte array where the value will be stored.
 * @return Data size in bytes of the value.
 */
INT ask_data(INT type_index, UCHAR **data)
{
    MU_TYPES value;
    INT ret_val;
    BOOL value_ok = false;
    /* Use to read input and parse numbers */
    CHAR input_buff[MAX_STR_SZ];
    CHAR *thrash;
    while(!value_ok)
    {
        switch(type_index)
        {
            case OPT_1BYTE:
                fgets(input_buff, MAX_STR_SZ, stdin);
                NEWL_TO_NUL(input_buff);
                value.int32 = (INT32) strtol(input_buff, &thrash, 10);
                if(*thrash != '\0' || value.int32 < 0 || value.int32 > UCHAR_MAX)
                {
                    printf("Value not in 8-Bit Integer range. Provide a correct value: ");
                    break;
                }
                else
                {
                    ret_val = 1;
                    printf("Selected value: %hhu\n\n", value.byte);
                    *data = malloc(ret_val);
                    memcpy(*data, value.bytes8, ret_val);
                    value_ok = true;
                }
                break;

            case OPT_INT16:
                fgets(input_buff, MAX_STR_SZ, stdin);
                NEWL_TO_NUL(input_buff);
                value.int32 = (INT32) strtol(input_buff, &thrash, 10);
                if(*thrash != '\0' || value.int32 < SHRT_MIN || value.int32 > SHRT_MAX)
                {
                    printf("Value not in 16-Bit Integer range. Provide a correct value: ");
                    break;                    
                } 
                else
                {
                    ret_val = 2;
                    printf("Selected value: %hu\n\n", value.int16);
                    *data = malloc(ret_val);
                    memcpy(*data, value.bytes16, ret_val);
                    value_ok = true;
                }
                break;

            case OPT_INT32:
                fgets(input_buff, MAX_STR_SZ, stdin);
                NEWL_TO_NUL(input_buff);
                value.int32 = (INT32) strtol(input_buff, &thrash, 10);
                if(*thrash != '\0' || value.int32 < INT_MIN || value.int32 > INT_MAX)
                {
                    printf("Value not in 32-Bit Integer range. Provide a correct value: ");
                    break;
                }
                else
                {
                    ret_val = 4;
                    printf("Selected value: %d\n\n", value.int32);
                    *data = malloc(ret_val);
                    memcpy(*data, value.bytes32, ret_val);
                    value_ok = true;
                }
                break;

            case OPT_INT64:
                fgets(input_buff, MAX_STR_SZ, stdin);
                NEWL_TO_NUL(input_buff);
                value.int64 = strtol(input_buff, &thrash, 10);
                if(*thrash != '\0' || value.int64 < LONG_MIN || value.int64 > LONG_MAX)
                {
                    printf("Value not in 64-Bit Integer range. Provide a correct value: ");
                    break;
                } 
                else
                {
                    ret_val = 8;
                    printf("Selected value: %ld\n\n", value.int64);
                    *data = malloc(ret_val);
                    memcpy(*data, value.bytes64, ret_val);
                    value_ok = true;
                }
                break;

            case OPT_REALF:
                fgets(input_buff, MAX_STR_SZ, stdin);
                NEWL_TO_NUL(input_buff);
                value.real32 = strtof(input_buff, &thrash);
                if(*thrash != '\0' || value.real32 < FLT_MIN || value.real32 > FLT_MAX)
                {
                    printf("Value not in Float range. Provide a correct value: ");
                    break;
                } 
                else
                {
                    ret_val = 4;
                    printf("Selected value: %f\n\n", value.real32);
                    *data = malloc(ret_val);
                    memcpy(*data, value.bytes32, ret_val);
                    value_ok = true;
                }
                break;

            case OPT_REALD:
                fgets(input_buff, MAX_STR_SZ, stdin);
                NEWL_TO_NUL(input_buff);
                value.real64 = strtod(input_buff, &thrash);
                if(*thrash != '\0' || value.real64 < DBL_MIN || value.real64 > DBL_MAX)
                {
                    printf("Value not in Float range. Provide a correct value: ");
                    break;
                } 
                else
                {
                    ret_val = 8;
                    printf("Selected value: %le\n\n", value.real64);
                    *data = malloc(ret_val);
                    memcpy(*data, value.bytes64, ret_val);
                    value_ok = true;
                }
                break;

            case OPT_STRNG:
                fgets(input_buff, MAX_STR_SZ, stdin);
                /* Not checking buffer overflow because lazyness */
                NEWL_TO_NUL(input_buff);
                ULONG length = strlen(input_buff);
                ret_val = length + 1;
                *data = malloc(ret_val);
                memcpy(*data, input_buff, ret_val);
                value_ok = true;
            /* END SWITCH */
        }
    }

    return ret_val;
}
