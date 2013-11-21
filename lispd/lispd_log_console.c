
#include "lispd_log_console.h"

inline void lispd_log(
        int         log_level,
        char        *log_name,
        const char  *format,
        va_list     args);


static void lispd_log_to_console(
        const lispd_log_level_t lisp_log_level, const char *format, ...)
{
    va_list args;
    char *log_name; /* To store the log level in string format for printf output */
    int log_level;


    va_start (args, format);

    switch (lisp_log_level){
    case LISP_LOG_CRIT:
        log_name = "CRIT";
        log_level = LOG_CRIT;
        lispd_log(log_level, log_name, format, args);
        break;
    case LISP_LOG_ERR:
        log_name = "ERR";
        log_level = LOG_ERR;
        lispd_log(log_level, log_name, format, args);
        break;
    case LISP_LOG_WARNING:
        log_name = "WARNING";
        log_level = LOG_WARNING;
        lispd_log(log_level, log_name, format, args);
        break;
    case LISP_LOG_INFO:
        log_name = "INFO";
        log_level = LOG_INFO;
        lispd_log(log_level, log_name, format, args);
        break;
    case LISP_LOG_DEBUG_1:
        if (debug_level > 0){
            log_name = "DEBUG";
            log_level = LOG_DEBUG;
            lispd_log(log_level, log_name, format, args);
        }
        break;
    case LISP_LOG_DEBUG_2:
        if (debug_level > 1){
            log_name = "DEBUG-2";
            log_level = LOG_DEBUG;
            lispd_log(log_level, log_name, format, args);
        }
        break;
    case LISP_LOG_DEBUG_3:
        if (debug_level > 2){
            log_name = "DEBUG-3";
            log_level = LOG_DEBUG;
            lispd_log(log_level, log_name, format, args);
        }
        break;
    default:
        log_name = "LOG";
        log_level = LOG_INFO;
        lispd_log(log_level, log_name, format, args);
        break;
    }

    va_end (args);
}

inline void lispd_log(
        int         log_level,
        char        *log_name,
        const char  *format,
        va_list     args)
{
    if (daemonize){
        vsyslog(log_level,format,args);
    }else{
        printf("%s: ",log_name);
        vfprintf(stdout,format,args);
        printf("\n");
    }
}

lispd_log_ops_t log_console_ops = {
    .start_log = NULL,
    .close_log = NULL,
    .log = &lispd_log_to_console
};



