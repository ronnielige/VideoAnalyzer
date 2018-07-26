#ifndef _LOG_H_
#define _LOG_H_

#include "Form1.h"
#include <stdio.h>
using namespace VideoAnalyzer;

#define LOGLEVEL_NONE  0
#define LOGLEVEL_ERROR 1
#define LOGLEVEL_WARN  2
#define LOGLEVEL_INFO  3
#define LOGLEVEL_DEBUG 4
#define LOGLEVEL_FULL  5


void init_log();
void va_log(int level, char* format, ...);
void uninit_log();
#endif