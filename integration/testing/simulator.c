/**
 * @file test.c
 * @author Mark Dervishaj
 * @brief Prints attrib values with associated addresses. Each time a newline
 * enters, values are changed. Other than newline finishes the process
 * @version 0.1
 * @date 2022-01-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){

    unsigned char byte = 29;
    int gold = 1000;
    int silver = 1000;
    int bronze = 1000;
    short health = 12500;
    float ratio = 1.23242;
    int text_counter = 2;
    char text[] = "Welcome to the simulator! 1";
    int n_newlines = 1;

    while(1){
        printf("Iteration: %i\n", n_newlines);
        printf("Byte Val (%-2li bytes): %-28hhu - %p\n", sizeof(byte), byte, (void *) &byte);
        printf("Gold     (%-2li bytes): %-28i - %p\n", sizeof(gold), gold, (void *) &gold);
        printf("Silver   (%-2li bytes): %-28i - %p\n", sizeof(silver), silver, (void *) &silver);
        printf("Bronze   (%-2li bytes): %-28i - %p\n", sizeof(bronze), bronze, (void *) &bronze);
        printf("Health   (%-2li bytes): %-28hu - %p\n", sizeof(health), health, (void *) &health);
        printf("Ratio    (%-2li bytes): %-28f - %p\n", sizeof(float), ratio, (void *) &ratio);
        printf("Text     (%-2li bytes): %-28s - %p\n", sizeof(text), text, (void *) &text);

        char *line = NULL;
        size_t size = 0;
        getline(&line, &size, stdin);

        if(*line == '\n'){
            /* Newline read */
            byte++;
            gold--;
            silver-=2;
            bronze-=3;
            health-=4;
            ratio += 0.5;
            text[sizeof(text) - 2] = text_counter++ + '0';
            /* Reset */
            if(text_counter >= 10) text_counter = 1;
            n_newlines++;
        } else{
            free(line); break;
        }

        free(line);
    }
    return 0;
}