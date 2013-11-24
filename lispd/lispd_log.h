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


/* http://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/  */


#define LISPD_ENTRY_EXPAND( arg1 ) LISPD_ENTRY_EXPAND_( STRIP_PARENS(arg1))
// TODO rearranger l'ordre ici
#define LISPD_ENTRY_EXPAND_(...) LISPD_ENTRY_EXPAND_I( __VA_ARGS__, col_default,0,0)
#define LISPD_ENTRY_EXPAND_I( str, item_type , int, data, ...)  item_type, str, int , data
//lispd_logger.append_to_entry( entry, str, item_type , int, data );


// TODO if there is an empty parameter, then the next one is already expanded
#ifdef LISPD_ENABLE_PREPROCESSING
    #define LISPD_EID( ip )  (col_eid, ip) )
//    #define LISPD_RLOC( ip ) (,lispd_logger.append_to_entry(entry, col_rloc, ip,0,0))
    #define LISPD_RLOC( ip ) (ip, col_rloc, 0,0)
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


//#define CAT(x, y) CAT_I(x, y)
//#define CAT_I(x, y) x ## y

#define APPLY(macro, args) APPLY_I(macro, args)
#define APPLY_I(macro, args) macro args

// Here is the key : if x has no parenthesis then the STRIP_PARENS_I is a string, and no macro is called
#define STRIP_PARENS(x) EVAL( (REPLACE_IF_MULTIPLE_ARGS x), x)
#define REPLACE_IF_MULTIPLE_ARGS(...) 1,1

#define EVAL(args, x) EVAL_I( args, x)
//#define EVAL(args, x)  args, x
//#define EVAL_I(args, x) args
#define EVAL_I(args, x) MAYBE_STRIP_PARENS( VA_NUM_1_OR_SEVERAL args, x)

//#define TEST_ARITY(...) APPLY(TEST_ARITY_I, (__VA_ARGS__, 2, 1))
//#define TEST_ARITY_I(a,b,c,...) c

#define MAYBE_STRIP_PARENS( one_or_several, x) MAYBE_STRIP_PARENS_I(one_or_several, x)
//#define MAYBE_STRIP_PARENS( one_or_several, x) one_or_several, x
#define MAYBE_STRIP_PARENS_I(one_or_several, x) CONCAT(MAYBE_STRIP_PARENS, one_or_several)(x)

#define MAYBE_STRIP_PARENS1(x) x
#define MAYBE_STRIP_PARENS_SEVERAL(x) MAYBE_STRIP_PARENS_SEVERAL_I x
#define MAYBE_STRIP_PARENS_SEVERAL_I(...) __VA_ARGS__

//STRIP_PARENS(this is a test)
//STRIP_PARENS((a,b,c))


/* Used to count the number of arguments (up to 10 arguments, update the macros if you need more) */
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 9,8,7,6,5,4,3,2,1)
#define VA_NUM_1_OR_SEVERAL(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, _SEVERAL,_SEVERAL,_SEVERAL,_SEVERAL,_SEVERAL,_SEVERAL,_SEVERAL,_SEVERAL,1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5, _6, _7, _8, _9, N,...) N



#define CONCAT(a, b) a##b


/* Should not be called like this */
#define LISP_LOG0( level, ...)  "Needs format"


/* Add more if needed */


/* we should */
//#define LISP_LOG_SEVERAL(arg1,arg2,...) LISP_LOG2(arg1,arg2) LISP_LOG_SEVERAL(arg2,__VA_ARGS__)
//#define LISP_LOG (i, ) CONCAT(LISP_LOG)

#define LISP_LOG5( arg1, ... )  LISP_LOG1(arg1) LISP_LOG4( __VA_ARGS__)
#define LISP_LOG4( arg1, ... )  LISP_LOG1(arg1) LISP_LOG3( __VA_ARGS__)
#define LISP_LOG3( arg1, ... )  LISP_LOG1(arg1) LISP_LOG2(__VA_ARGS__)
#define LISP_LOG2( arg1, arg2 )  LISP_LOG1(arg1) LISP_LOG1(arg2)

/* if arg1 empty, then copy arg2 as is, else consider it as string
TODO strip parenthesis
append_entry( COMPLETE(strip parenthesis(arg1))( )
*/
#define LISP_LOG1( arg1 ) lispd_logger.append_to_entry(entry, LISPD_ENTRY_EXPAND( arg1) );

// appele avant
/* last argument is empty */
#define LISPD_LOG( lispd_log_level,  ... )  do { \
                                                               lispd_log_entry_t entry = lispd_logger.new_entry(lispd_log_level); \
                                                               if( !entry ) break; \
                                                              LISPD_CALL_ADEQUATE_LOG( VA_NUM_ARGS(__VA_ARGS__) , __VA_ARGS__) \
                                                            lispd_logger.close_entry(entry); \
                                                        } while(0)



#define LISPD_CALL_ADEQUATE_LOG( nargs , ... ) LISPD_CALL_ADEQUATE_LOG_( CONCAT(LISP_LOG , nargs) , __VA_ARGS__ )
#define LISPD_CALL_ADEQUATE_LOG_( name, ... ) name( __VA_ARGS__)



//#define LISPD_LOG( lispd_log_level, ...)   LISPD_CALL_ADEQUATE_LOG( VA_NUM_ARGS(__VA_ARGS__) , lispd_log_level, __VA_ARGS__)

/* to verify preprocessor output, run the preprocessor alone (g++ -E lispd.c for instance): */
//#define LISPD_LOG( lispd_log_level, ...)   lispd_logger.log( lispd_log_level, VA_NUM_ARGS(__VA_ARGS__) ,  __VA_ARGS__)

#define MAX_STRING_LENGTH 500
#define POOL_SIZE 5


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
