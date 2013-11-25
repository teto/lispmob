/*
 * lispd_log.c
 *
 * This file is part of LISP Mobile Node Implementation.
 * Write log messages
 *
 * Copyright (C) 2011 Cisco Systems, Inc, 2011. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Please send any bug reports or fixes you make to the email address(es):
 *    LISP-MN developers <devel@lispmob.org>
 *
 * Written or modified by:
 *    David Meyer       <dmm@cisco.com>
 *    Preethi Natarajan <prenatar@cisco.com>
 *    Alberto Rodriguez Natal <arnatal@ac.ucp.edu>
 *
 */

#include "lispd_log.h"
#include "lispd_external.h"
#include <syslog.h>
#include <stdarg.h>


static lispd_log_descriptor_t lispd_log_descriptors[LISP_LOG_DEBUG_3 + 1] =
{
[LISP_LOG_CRIT] = { "CRIT", LOG_CRIT , lispd_item_crit },
[LISP_LOG_WARNING] = { "WARNING", LOG_WARNING, lispd_item_warning },
[LISP_LOG_INFO] = { "INFO", LOG_INFO, lispd_item_info },
[LISP_LOG_ERR] = { "ERR", LOG_ERR, lispd_item_err },
[LISP_LOG_DEBUG_1] = { "DEBUG", LOG_DEBUG, lispd_item_debug },
[LISP_LOG_DEBUG_2] = { "DEBUG-2", LOG_DEBUG, lispd_item_debug },
[LISP_LOG_DEBUG_3] = { "DEBUG-3", LOG_DEBUG, lispd_item_debug }
};

/*
 * True if log_level is enough to print results
 */
int is_loggable (int log_level){
    if (log_level < LISP_LOG_DEBUG_1)
        return (TRUE);
    else if (log_level <= LISP_LOG_INFO + debug_level)
        return (TRUE);
    return (FALSE);
}

const lispd_log_descriptor_t lispd_log_get_level_descriptor(const lispd_log_level_t log_level)
{
    return lispd_log_descriptors[ log_level ];
}


inline void lispd_log(
        const lispd_log_descriptor_t lispd_log_level,
        const char  *format,
        va_list     args);





void lispd_log_msg(
        const lispd_log_level_t lisp_log_level, const char *format, ...)
{
    va_list args;
//    char *log_name; /* To store the log level in string format for printf output */
//    int log_level;


    va_start (args, format);
    lispd_log( lispd_log_descriptors[lisp_log_level], format,  args);
//    switch (lisp_log_level){
//    case LISP_LOG_CRIT:
//        log_name = "CRIT";
//        log_level = LOG_CRIT;
//        lispd_log(log_level, log_name, format, args);
//        break;
//    case LISP_LOG_ERR:
//        log_name = "ERR";
//        log_level = LOG_ERR;
//        lispd_log(log_level, log_name, format, args);
//        break;
//    case LISP_LOG_WARNING:
//        log_name = "WARNING";
//        log_level = LOG_WARNING;
//        lispd_log(log_level, log_name, format, args);
//        break;
//    case LISP_LOG_INFO:
//        log_name = "INFO";
//        log_level = LOG_INFO;
//        lispd_log(log_level, log_name, format, args);
//        break;
//    case LISP_LOG_DEBUG_1:
//        if (debug_level > 0){
//            log_name = "DEBUG";
//            log_level = LOG_DEBUG;
//            lispd_log(log_level, log_name, format, args);
//        }
//        break;
//    case LISP_LOG_DEBUG_2:
//        if (debug_level > 1){
//            log_name = "DEBUG-2";
//            log_level = LOG_DEBUG;
//            lispd_log(log_level, log_name, format, args);
//        }
//        break;
//    case LISP_LOG_DEBUG_3:
//        if (debug_level > 2){
//            log_name = "DEBUG-3";
//            log_level = LOG_DEBUG;
//            lispd_log(log_level, log_name, format, args);
//        }
//        break;
//    default:
//        log_name = "LOG";
//        log_level = LOG_INFO;
//        lispd_log(log_level, log_name, format, args);
//        break;
//    }

    va_end (args);
}

inline void lispd_log(
        const lispd_log_descriptor_t log_descriptor,
        const char  *format,
        va_list     args
        )
{
    if (daemonize){
        vsyslog( log_descriptor.syslog_log_level , format, args);
    }else{

        // Reorder instruction

        printf("%s: ", log_descriptor.log_name );
        vfprintf(stdout, format, args);
        printf("\n");
    }
}

/*
 * Editor modelines
 *
 * vi: set shiftwidth=4 tabstop=4 expandtab:
 * :indentSize=4:tabSize=4:noTabs=true:
 */
