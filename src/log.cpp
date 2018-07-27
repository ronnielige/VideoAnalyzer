#include "StdAfx.h"
#include "log.h"

char logfile[500];  // file name
int  LogLevel = 5;  // 0 - none; 1 - error; 2 - warn; 3 - info; 4 - debug; 5 - full
FILE* logFp = NULL;

void init_log()
{
    sprintf(logfile, "%d%02d%02d_%02d%02d%02d.log", DateTime::Now.Year, DateTime::Now.Month, DateTime::Now.Day, DateTime::Now.Hour, DateTime::Now.Minute, DateTime::Now.Second);
    logFp = fopen(logfile, "w");
    setvbuf(logFp, NULL, _IOFBF, 10240); // write file every 10240 bytes
}

static void get_time_str(char* str)
{
    sprintf(str, "%s:%3d", DateTime::Now.ToLocalTime().ToString(), DateTime::Now.Millisecond);
}

static void va_log_internal(char* str)
{
    char new_str[200];
    sprintf(new_str, "%s:%03d - %s", DateTime::Now.ToLocalTime().ToString(), DateTime::Now.Millisecond, str);
    fprintf(logFp, new_str);
}

void va_log(int level, char* format, ...)
{
    char str[300];
    if(level <= LogLevel)
    {
        va_list args;
        va_start(args, format);
        _vsnprintf(str, 299, format, args);
        va_log_internal(str);
        va_end(args);
    }
}

void uninit_log()
{
    if(logFp)
        fclose(logFp);
}