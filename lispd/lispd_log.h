/*
 * lispd_log.h
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
 *    Alberto Rodriguez Natal <arnatal@ac.upc.edu>
 *
 */
#pragma once

#ifndef LISPD_LOG_H_
#define LISPD_LOG_H_

#define LISPD_ENABLE_PREPROCESSING

//int "%rloc" = 0;
//int "%eid" = 0;
//int "%  " = 0;

typedef enum {
col_default,    /* used to reset colors */
col_eid,
col_rloc,
col_port,
col_mask,
col_filename,
col_interface,
col_crit,
col_warning,
col_debug,
//col_LISP_LOG_DEBUG_2,
//col_LISP_LOG_DEBUG_3,
col_info,
col_err,
col_mapserver,
col_mapresolver,
col_last
} lispd_log_item_type_t ;


/* http://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/
replace 0 by empty or not */
#define _ARG16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define HAS_COMMA(...) _ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define _TRIGGER_PARENTHESIS_(...) ,

#define ISEMPTY(...)                                                    \
_ISEMPTY(                                                               \
          /* test if there is just one argument, eventually an empty    \
             one */                                                     \
          HAS_COMMA(__VA_ARGS__),                                       \
          /* test if _TRIGGER_PARENTHESIS_ together with the argument   \
             adds a comma */                                            \
          HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__),                 \
          /* test if the argument together with a parenthesis           \
             adds a comma */                                            \
          HAS_COMMA(__VA_ARGS__ (/*empty*/)),                           \
          /* test if placing it between _TRIGGER_PARENTHESIS_ and the   \
             parenthesis adds a comma */                                \
          HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/))      \
          )

#define PASTE5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define _ISEMPTY(_0, _1, _2, _3) HAS_COMMA(PASTE5(_IS_EMPTY_CASE_, _0, _1, _2, _3))
#define _IS_EMPTY_CASE_0001 ,



// TODO if there is an empty parameter, then the next one is already expanded
#ifdef LISPD_ENABLE_PREPROCESSING
    #define LISPD_EID( ip )  ,logger.append_to_entry(entry, col_eid, ip)
    #define LISPD_RLOC( ip ) ,lispd_logger.append_to_entry(entry, col_rloc, ip,0,0)
//    #define LISPD_PORT( port )  lispd_color_output( 0, port,  col_port)
//    #define LISPD_MASK( mask)  lispd_color_output( 0, mask,  col_port)
    #define LISPD_PORT( port )  (port)
    #define LISPD_MASK( mask)   (mask)
    #define LISPD_FILENAME( filename )  lispd_color_output(filename , 0, col_filename)
    #define LISPD_IFNAME( if_name) if_name
    #define LISPD_MS( ip ) LISPD_RLOC( ip)
    #define LISPD_MR( ip ) LISPD_RLOC( ip)
    #define LISPD_PETR( host ) LISPD_RLOC(host)
    #define LISPD_PITR( host ) LISPD_RLOC(host)
    #define LISPD_LOG_DESCRIPTOR( descriptor ) lispd_color_output(descriptor.log_name , 0, descriptor.color )
#else
    #define LISPD_EID( ip )  (ip)
    #define LISPD_RLOC( ip )  (ip)
    #define LISPD_PORT( port )  (port)
    #define LISPD_MASK( mask )  (mask)
    #define LISPD_FILENAME( filename )  (filename)
    #define LISPD_IFNAME( if_name) if_name
    #define LISPD_MAPSERVER( ip )  ( ip)
    #define LISPD_MAPRESOLVER( ip )  ( ip)
    #define LISPD_PETR( host ) (host)
    #define LISPD_PITR( host ) (host)
    #define LISPD_LOG_DESCRIPTOR( descriptor )  (descriptor.log_name )
#endif


/* Used to count the number of arguments (up to 10 arguments, update the macros if you need more) */
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 9,8,7,6,5,4,3,2,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5, _6, _7, _8, _9, N,...) N



#define CONCAT(a, b) a##b

/* Should not be called like this */
#define LISP_LOG0( level, ...)  "Needs format"


/* Add more if needed */


/* we should */
//#define LISP_LOG_SEVERAL(arg1,arg2,...) LISP_LOG2(arg1,arg2) LISP_LOG_SEVERAL(arg2,__VA_ARGS__)
//#define LISP_LOG (i, ) CONCAT(LISP_LOG)

#define LISP_LOG4( arg1, arg2, ... )  LISP_LOG2(arg1,arg2) LISP_LOG3(arg2, __VA_ARGS__)
#define LISP_LOG3( arg1, arg2, arg3 )  LISP_LOG2(arg1,arg2) LISP_LOG2(arg2,arg3)

/* if arg1 empty, then copy arg2 as is, else consider it as string */
#define LISP_LOG2( arg1, arg2 )  LISP_LOG2_( ISEMPTY(arg1) , arg1,arg2)
#define LISP_LOG2_( isempty, arg1, arg2 ) CONCAT(LISP_LOG2_, isempty ) (arg1,arg2)

#define LISP_LOG2_0( arg1, arg2 ) LISP_LOG1( arg1 )
/* if arg1 empty */
#define LISP_LOG2_1( arg1, arg2 ) arg2
//LISP_LOG2(arg2, )

/* might be a corner case if str empty ? */
#define LISP_LOG1( str )    lispd_logger.append_to_entry(entry,col_default, str, 0, 0);
//LISP_LOG_FINAL((lispd_log_level),  "%s: " format, __func__)

// appele avant
/* last argument is empty */
#define LISPD_LOG( lispd_log_level,  ... )  do { \
                                                               lispd_log_entry_t entry = lispd_logger.new_entry(lispd_log_level); \
                                                               if( !entry ) break; \
                                                              LISPD_CALL_ADEQUATE_LOG( VA_NUM_ARGS(__VA_ARGS__,) , __VA_ARGS__,) \
                                                            lispd_logger.close_entry(entry); \
                                                        } while(0)



#define LISPD_CALL_ADEQUATE_LOG( nargs , ... ) LISPD_CALL_ADEQUATE_LOG_( CONCAT(LISP_LOG , nargs) , __VA_ARGS__ )
#define LISPD_CALL_ADEQUATE_LOG_( name, ... ) name( __VA_ARGS__)



//#define LISPD_LOG( lispd_log_level, ...)   LISPD_CALL_ADEQUATE_LOG( VA_NUM_ARGS(__VA_ARGS__) , lispd_log_level, __VA_ARGS__)

/* to verify preprocessor output, run the preprocessor alone (g++ -E lispd.c for instance): */
//#define LISPD_LOG( lispd_log_level, ...)   lispd_logger.log( lispd_log_level, VA_NUM_ARGS(__VA_ARGS__) ,  __VA_ARGS__)




typedef enum {
LISP_LOG_CRIT     ,          /* critical conditions -> Exit program */
LISP_LOG_ERR      ,          /* error conditions -> Not exit but should be considered by user */
LISP_LOG_WARNING  ,          /* warning conditions -> Low level errors. Program doesn't finish */
LISP_LOG_INFO     ,          /* informational -> Initial configuration, SMRs, interface change status*/
LISP_LOG_DEBUG_1  ,          /* low debug-level messages -> Control message */
LISP_LOG_DEBUG_2  ,          /* medium debug-level messages -> Errors in received packets. Wrong AFI, ... */
LISP_LOG_DEBUG_3             /* high debug-level messages -> Log for each received or generated packet */
} lispd_log_level_t;


typedef char* lispd_log_entry_t;


typedef struct  {
int (*start_logger)(const char *data);     /* data passed on via cli or config file */
int (*close_logger)(void *data);
lispd_log_entry_t (*new_entry)(const lispd_log_level_t lispd_log_level);
void (*close_entry)(lispd_log_entry_t entry);
//void (*log)(const lispd_log_level_t lispd_log_level, ...);
void (*append_to_entry)(lispd_log_entry_t entry, const lispd_log_item_type_t type, char* str, int integer, void *data);
} lispd_log_ops_t;

/** KEPT for retrocompatibility, remove once the new logger is ok **/
void lispd_log_msg( const lispd_log_level_t lispd_log_level, const char *format, ...);


lispd_log_entry_t lispd_log_new_entry(const lispd_log_level_t lispd_log_level);

/*
 * True if log_level is enough to print results
 */

int is_loggable (int log_level);



#endif /*LISPD_LOG_H_*/
