
/*
 * main.h
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#include<stdio.h>

#define TRACE_DATA_LOAD 0
#define TRACE_DATA_STORE 1
#define TRACE_INST_LOAD 2

#define PRINT_INTERVAL 100000

void parse_args(int argc, char** argv);
void play_trace(FILE* inFile);
int read_trace_element(FILE* inFile, unsigned* access_type, unsigned* addr);


#endif