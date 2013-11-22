
#include "lispd_log_console.h"

inline void lispd_log(
        int         log_level,
        char        *log_name,
        const char  *format,
        va_list     args);


/* expects nb of arguments as first argument */
static void lispd_log_to_console(
        const lispd_log_level_t lisp_log_level, ...)
{

//    char *log_name; /* To store the log level in string format for printf output */
//    int log_level;
    int nb = 0;
    int i = 0;
    va_list args;
// lispd_logger.log( LISP_LOG_INFO, 1 , "LISPmob compiled for mobile node\n");
    va_start (args, lisp_log_level);
    nb = va_arg(args,int);

    printf("Nb of args %d, log level %d\n", nb , lisp_log_level);
//    printf("1st arg %s\n", va_arg(args,char*));

    for ( i = 0; i < nb; ++i){

        //
        char* str= va_arg(args,char*);

        // check for length ?
        if( str[0] == 0 ){
            continue;
        }
        else if ( str[0] != '%') {
            printf("%s", str);
            continue;
        }

        switch( str[1] ){
            case 0:
                printf("Warning You shouldn't have a  on its own");
                continue;

            // RLOC
            case 'R':
                // TODO consume argument and increase ++i
                {
                    char* rloc = va_arg(args,char*);
                    ++i;
                    printf("[POC rloc:%s]", rloc);
                }
                continue;
//            case 'r':
//            case '':
            default:
                break;
                // TODO sprintf

        }
    }
/*
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
    */
    va_end (args);

}

//inline void lispd_log(
//        int         log_level,
//        char        *log_name,
//        const char  *format,
//        va_list     args)
//{
//    if (daemonize){
//        vsyslog(log_level,format,args);
//    }else{
//        printf("%s: ",log_name);
//        vfprintf(stdout,format,args);
//        printf("\n");
//    }
//}


void lispd_log_console_close_entry(lispd_log_entry_t entry)
{
    // Nothing to do
}

void lispd_log_console_append_to_entry(lispd_log_entry_t entry, const lispd_log_item_type_t type, char* str, int integer, void *data)
{
    if (type == col_default) {
        printf("Param [%s]", str);
        return;
    }

    printf("Unsupported type %d", type);
}



lispd_log_ops_t log_console_ops = {
    .start_logger = NULL,
    .close_logger = NULL,
//    .log = &lispd_log_to_console,
    .new_entry = &lispd_log_new_entry,
    .close_entry = &lispd_log_console_close_entry,
    .append_to_entry = &lispd_log_console_append_to_entry
//    .preprocess_item = NULL
};



