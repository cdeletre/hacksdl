#include "debug.h"
#include "configuration.h"
#include <stdio.h>
#include <stdarg.h>


extern hacksdl_config_t config;

void sdterr_yellow () {
    fprintf(stderr,"\033[1;33m");
}

void sdterr_red () {
    fprintf(stderr,"\033[1;31m");
}

void sdterr_blue () {
    fprintf(stderr,"\033[1;34m");
}

void sdterr_reset () {
    fprintf(stderr,"\033[0m");
}

/*
    HACKSDL_error: print error to stderr
*/
void HACKSDL_error_func(const char* func_name, char* format, ...)
{
    va_list args;

    sdterr_red();
    fprintf(stderr,"[HACKSDL.error(%s)] ", func_name);
    va_start(args, format);
    vfprintf(stderr,format, args);
    va_end(args);
    fprintf(stderr,"\n");
    sdterr_reset();
    fflush(stderr);

}

/*
    HACKSDL_info: print to stderr when verbose > 0
*/
void HACKSDL_info(char* format, ...)
{
    va_list args;

    if(config.verbose > 0)
    {
        sdterr_blue();
        fprintf(stderr,"[HACKSDL.info] ");
        va_start(args, format);
        vfprintf(stderr,format, args);
        va_end(args);
        fprintf(stderr,"\n");
        sdterr_reset();
        fflush(stderr);
    }

}

/*
    HACKSDL_debug: print to stderr when verbose > 1
*/
void HACKSDL_debug_func(const char* func_name, char* format, ...)
{
    va_list args;

    if(config.verbose > 1)
    {
        sdterr_yellow();
        fprintf(stderr,"[HACKSDL.debug(%s)] ", func_name);
        va_start(args, format);
        vfprintf(stderr,format, args);
        va_end(args);
        fprintf(stderr,"\n");
        sdterr_reset();
        fflush(stderr);
    }

}