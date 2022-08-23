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
#include "inc/mu_scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define OPT_1BYTE   0
#define OPT_INT16   1
#define OPT_INT32   2
#define OPT_INT64   3
#define OPT_REALF   4
#define OPT_REALD   5

#define ASK_FILTER  0
#define ASK_SCAN    1

void show_help();
void show_types();
BOOL ask_for_more(INT option);
INT ask_data(INT type_index, UCHAR **data);

INT main(int argc, char **argv)
{

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

    /* CHECK ARGUMENTS DONE -------------------------------------------------------------- */

    /* CHECK PID EXISTS ------------------------------------------------------------------ */

    if(pid_exists(target)){
        exit(ERR_FUNC_OPT);
    }

    /* CHECK PID EXISTS DONE ------------------------------------------------------------- */

    /* ASK VALUE TYPE -------------------------------------------------------------------- */

    const CHAR *data_types[] = {"8-Bit Integer", "16-Bit Integer", "32-Bit Integer", "64-Bit Integer", "Float", "Double"};

    BOOL keep_scan = true;
    while(keep_scan)
    {
        printf("Please, select the value type:\n");
        show_types();
        BOOL selected = false;
        INT c;
        while(!selected)
        {
            scanf("%i", &c);
            if(c < 1 || c > 6)
            {
                printf("Please, select a correct value type (choice between 1 and 6):\n");
            } 
            else selected = true;
        }
        printf("Type selected: %s\n\n", data_types[c - 1]);

    /* ASK VALUE TYPE DONE --------------------------------------------------------------- */

        INT type_index = c - 1;
        printf("Please, select the value to search:\n");
        BOOL value_ok = false;

        // MU_TYPES value;
        UCHAR *data;
        INT data_size;
        ULONG *matches;
        INT n_matches;


        data_size = ask_data(type_index, &data);

        /* Do scanning and filtering */
        printf("AAAAAAAA\n");
        matches = execute_scanner_STD_MTH(target, data, data_size, &n_matches);
        if(n_matches == 0)
        {
            printf("No matches found\n");
            break;
        }
        printf("%i address<es> matching the value\n", n_matches);
        
        if(ask_for_more(ASK_FILTER))
        {
            BOOL done = false;
            while(!done)
            {
                printf("\nPlease, select the value to search:\n");
                break;
            }
        }

        if(!ask_for_more(ASK_SCAN))
        {
            keep_scan = false;
        }
    }
    return ERR_OK;
}

void show_help()
{
    printf("Usage: mem_scan_linux <pid_of_target>\n");
}

void show_types()
{
    printf("\n1) 8-Bit Integer     (1 Byte)\n");
    printf("2) 16-Bit Integer    (2 Bytes)\n");
    printf("3) 32-Bit Integer    (4 Bytes)\n");
    printf("4) 64-Bit Integer    (8 Bytes)\n");
    printf("5) Float             (4 Bytes)\n");
    printf("6) Double            (8 Bytes)\n");
}

BOOL ask_for_more(INT option)
{
    BOOL correct_choice = false;
    INT keep_searching = 0;
    while(!correct_choice)
    {
        if(option == ASK_FILTER)
        {
            printf("Keep searching? (0: no; 1: yes): ");
        }
        else printf("Do you want to scan more values? (0: no; 1: yes): ");
        scanf("%i", &keep_searching);
        if(keep_searching < 0 || keep_searching > 1)
        {
            printf("\nPlease, select a valid choice (1 for YES, 0 for NO): ");
        } 
        else correct_choice = true;
    }
    return (keep_searching == 1) ? true : false;
}

INT ask_data(INT type_index, UCHAR **data)
{
    MU_TYPES value;
    INT ret_val;
    BOOL value_ok = false;
    while(!value_ok)
    {
        switch(type_index)
        {
            case OPT_1BYTE:
                scanf("%hhu", &value.byte);
                if(value.byte < CHAR_MIN || value.byte > CHAR_MAX)
                {
                    printf("Value not in 8-Bit Integer range. Provide a correct value:\n");
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
                scanf("%hu", &value.int16);
                if(value.int16 < SHRT_MIN || value.int16 > SHRT_MAX)
                {
                    printf("Value not in 16-Bit Integer range. Provide a correct value:\n");
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
                scanf("%d", &value.int32);
                if(value.int32 < INT_MIN || value.int32 > INT_MAX)
                {
                    printf("Value not in 32-Bit Integer range. Provide a correct value:\n");
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
                scanf("%ld", &value.int64);
                if(value.int64 < LONG_MIN || value.int64 > LONG_MAX)
                {
                    printf("Value not in 64-Bit Integer range. Provide a correct value:\n");
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
                scanf("%f", &value.real32);
                if(value.real32 < INT_MIN || value.real32 > INT_MAX)
                {
                    printf("Value not in Float range. Provide a correct value:\n");
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
                scanf("%le", &value.real64);
                if(value.real64 < LONG_MIN || value.real64 > LONG_MAX)
                {
                    printf("Value not in Float range. Provide a correct value:\n");
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
            /* END SWITCH */
        }
    }

    return ret_val;
}