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

/* TODO move to a lispd_conf.h */
#define LISPD_ENABLE_COLORS

#define MAX_STRING_LENGTH 500
#define POOL_SIZE 5
#define LISPD_DEBUG 1


typedef enum {
lispd_item_default,    /* used for basic string */
lispd_item_eid,
lispd_item_rloc,
lispd_item_port,
lispd_item_mask,
lispd_item_filename,
lispd_item_interface,
lispd_item_crit,
lispd_item_warning,
lispd_item_debug,
lispd_item_integer,
//lispd_item_LISP_LOG_DEBUG_2,
//lispd_item_LISP_LOG_DEBUG_3,
lispd_item_info,
lispd_item_err,
lispd_item_mapserver,
lispd_item_mapresolver,
lispd_item_last     /* always keep it last, used as counter */
} lispd_log_item_type_t ;


/* http://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/  */


#define LISPD_ENTRY_EXPAND( arg1 ) LISPD_ENTRY_EXPAND_( STRIP_PARENS(arg1))

#define LISPD_ENTRY_EXPAND_(...) LISPD_ENTRY_EXPAND_I( __VA_ARGS__, lispd_item_default,0,0)
#define LISPD_ENTRY_EXPAND_I( str, item_type , int, data, ...)  item_type, str, int , data


// TODO if there is an empty parameter, then the next one is already expanded
#define LISPD_INTEGER( integer )  ( 0,lispd_item_integer, integer, 0)
#define LISPD_EID( ip )  ( (ip),lispd_item_eid, 0, 0)
#define LISPD_EIDA( ip )  ( get_char_from_lisp_addr_t(ip),lispd_item_eid, 0, 0)
// convert into 3 parameters ? set mask __VA_ARGS__
#define LISPD_MAPPING( mapping )  LISPD_EID(get_char_from_lisp_addr_t( (mapping)->eid_prefix) ),"/", LISPD_INTEGER( (mapping)->eid_prefix_length  )
//#define LISPD_MAPPING( mapping )  ( get_char_from_lisp_addr_t( (mapping)->eid_prefix),lispd_item_eid, 0, 0),"/",( 0,lispd_item_integer, integer, 0)
#define LISPD_RLOC( ip ) ( (ip), lispd_item_rloc, 0,0)
#define LISPD_RLOC_A( locator ) ( (get_char_from_lisp_addr_t(*(locator->locator_addr))), lispd_item_rloc, 0,0)
//    #define LISPD_PORT( port )  lispd_color_output( 0, port,  col_port)
//    #define LISPD_MASK( mask)  lispd_color_output( 0, mask,  col_port)
#define LISPD_PORT( port )  (port)
#define LISPD_MASK( mask)   (mask)
#define LISPD_FILENAME( filename )  lispd_color_output(filename , 0, col_filename)
#define LISPD_IFNAME( if_name)  (if_name)
#define LISPD_MS( ip ) LISPD_RLOC( ip)
#define LISPD_MR( ip ) LISPD_RLOC( ip)
#define LISPD_AFI( afi ) LISPD_INTEGER( afi)
#define LISPD_PETR( host ) LISPD_RLOC(host)
#define LISPD_PITR( host ) LISPD_RLOC(host)
#define LISPD_ERRNO( errno ) strerror(errno)
#define LISPD_TIMER( errno ) LISPD_INTEGER(errno)
//#define LISPD_LOG_DESCRIPTOR( descriptor ) lispd_color_output(descriptor.log_name , 0, descriptor.color )




#define APPLY(macro, args) APPLY_I(macro, args)
#define APPLY_I(macro, args) macro args

// Here is the key : if x has no parenthesis then the STRIP_PARENS_I is a string, and no macro is called
#define STRIP_PARENS(x) EVAL( (REPLACE_IF_MULTIPLE_ARGS x), x)
#define REPLACE_IF_MULTIPLE_ARGS(...) 1,1

#define EVAL(args, x) EVAL_I( args, x)
#define EVAL_I(args, x) MAYBE_STRIP_PARENS( VA_NUM_1_OR_SEVERAL args, x)


#define MAYBE_STRIP_PARENS( one_or_several, x) MAYBE_STRIP_PARENS_I(one_or_several, x)
#define MAYBE_STRIP_PARENS_I(one_or_several, x) CONCAT(MAYBE_STRIP_PARENS, one_or_several)(x)

#define MAYBE_STRIP_PARENS1(x) x
#define MAYBE_STRIP_PARENS_SEVERAL(x) MAYBE_STRIP_PARENS_SEVERAL_I x
#define MAYBE_STRIP_PARENS_SEVERAL_I(...) __VA_ARGS__



/* Used to count the number of arguments (up to 10 arguments, update the macros if you need more) */
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, 20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
#define VA_NUM_1_OR_SEVERAL(...) VA_NUM_ARGS_IMPL(__VA_ARGS__, \
                            _SEVERAL,_SEVERAL,_SEVERAL,_SEVERAL,_SEVERAL, \
                            _SEVERAL,_SEVERAL,_SEVERAL,_SEVERAL,_SEVERAL, \
                            _SEVERAL,_SEVERAL,_SEVERAL,_SEVERAL,_SEVERAL, \
                            _SEVERAL,_SEVERAL,_SEVERAL,_SEVERAL, \
                            1)
#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,N,...) N



#define CONCAT(a, b) a##b

/* Add more if needed */
#define LISP_LOG19( arg1, ... )  LISP_LOG1(arg1) LISP_LOG18( __VA_ARGS__)
#define LISP_LOG18( arg1, ... )  LISP_LOG1(arg1) LISP_LOG17( __VA_ARGS__)
#define LISP_LOG17( arg1, ... )  LISP_LOG1(arg1) LISP_LOG16( __VA_ARGS__)
#define LISP_LOG16( arg1, ... )  LISP_LOG1(arg1) LISP_LOG15( __VA_ARGS__)
#define LISP_LOG15( arg1, ... )  LISP_LOG1(arg1) LISP_LOG14( __VA_ARGS__)
#define LISP_LOG14( arg1, ... )  LISP_LOG1(arg1) LISP_LOG13( __VA_ARGS__)
#define LISP_LOG13( arg1, ... )  LISP_LOG1(arg1) LISP_LOG12( __VA_ARGS__)
#define LISP_LOG12( arg1, ... )  LISP_LOG1(arg1) LISP_LOG11( __VA_ARGS__)
#define LISP_LOG11( arg1, ... )  LISP_LOG1(arg1) LISP_LOG10( __VA_ARGS__)
#define LISP_LOG10( arg1, ... )  LISP_LOG1(arg1) LISP_LOG9( __VA_ARGS__)
#define LISP_LOG9( arg1, ... )  LISP_LOG1(arg1) LISP_LOG8( __VA_ARGS__)
#define LISP_LOG8( arg1, ... )  LISP_LOG1(arg1) LISP_LOG7( __VA_ARGS__)
#define LISP_LOG7( arg1, ... )  LISP_LOG1(arg1) LISP_LOG6( __VA_ARGS__)
#define LISP_LOG6( arg1, ... )  LISP_LOG1(arg1) LISP_LOG5( __VA_ARGS__)
#define LISP_LOG5( arg1, ... )  LISP_LOG1(arg1) LISP_LOG4( __VA_ARGS__)
#define LISP_LOG4( arg1, ... )  LISP_LOG1(arg1) LISP_LOG3( __VA_ARGS__)
#define LISP_LOG3( arg1, ... )  LISP_LOG1(arg1) LISP_LOG2(__VA_ARGS__)
#define LISP_LOG2( arg1, arg2 )  LISP_LOG1(arg1) LISP_LOG1(arg2)
#define LISP_LOG1( arg1 ) lispd_logger.append_to_entry( lispd_log_entry_, LISPD_ENTRY_EXPAND( arg1) );

/* Should not be called like this */
#define LISP_LOG0( level, ...)  "Needs format"


/* last argument is empty __LINE__*/
#ifdef LISPD_DEBUG
    #define LISPD_LOG(level, ...) LISPD_LOG_FINAL(level, __func__, ":" , __VA_ARGS__ )
#else
    #define LISPD_LOG(level, ...) LISPD_LOG_FINAL(level, __VA_ARGS__ )
#endif

//#define LISPD_LOG_FINAL(lispd_log_level,  ...) LISPD_LOG_FINAL_(lispd_log_level, __VA_ARGS__)
#define LISPD_LOG_FINAL( lispd_log_level,  ... )  do { \
                                               lispd_log_entry_t *lispd_log_entry_ = lispd_logger.new_entry(lispd_log_level); \
                                               if( !lispd_log_entry_ ) break; \
                                              LISPD_CALL_ADEQUATE_LOG( VA_NUM_ARGS(__VA_ARGS__) , __VA_ARGS__) \
                                            lispd_logger.close_entry(lispd_log_entry_); \
                                        } while(0)



#define LISPD_CALL_ADEQUATE_LOG( nargs , ... ) LISPD_CALL_ADEQUATE_LOG_( CONCAT(LISP_LOG , nargs) , __VA_ARGS__ )
#define LISPD_CALL_ADEQUATE_LOG_( name, ... ) name( __VA_ARGS__)



/* to verify preprocessor output, run the preprocessor alone (g++ -E lispd.c for instance): */

typedef enum {
LISP_LOG_CRIT     ,          /* critical conditions -> Exit program */
LISP_LOG_ERR      ,          /* error conditions -> Not exit but should be considered by user */
LISP_LOG_WARNING  ,          /* warning conditions -> Low level errors. Program doesn't finish */
LISP_LOG_INFO     ,          /* informational -> Initial configuration, SMRs, interface change status*/
LISP_LOG_DEBUG_1  ,          /* low debug-level messages -> Control message */
LISP_LOG_DEBUG_2  ,          /* medium debug-level messages -> Errors in received packets. Wrong AFI, ... */
LISP_LOG_DEBUG_3             /* high debug-level messages -> Log for each received or generated packet */
} lispd_log_level_t;

// TODO try adding const
typedef struct {
const char* log_name;
int syslog_log_level;
lispd_log_item_type_t type;
} lispd_log_descriptor_t;

//
//
//const

typedef struct {
char str[ MAX_STRING_LENGTH ];
lispd_log_level_t log_level;
lispd_log_descriptor_t log_descriptor;
} lispd_log_entry_t;



typedef struct  {
int (*start_logger)(const char *data);     /* data passed on via cli or config file */
int (*close_logger)(void *data);
lispd_log_entry_t* (*new_entry)(const lispd_log_level_t lispd_log_level);
void (*close_entry)(lispd_log_entry_t *entry);
void (*append_to_entry)(lispd_log_entry_t *entry, const lispd_log_item_type_t type, const char* str, const int integer, void *data);
} lispd_log_ops_t;

/** KEPT for retrocompatibility, remove once the new logger is ok **/
void lispd_log_msg( const lispd_log_level_t lispd_log_level, const char *format, ...);

const lispd_log_descriptor_t lispd_log_get_level_descriptor(const lispd_log_level_t log_level);

/*
 * True if log_level is enough to print results
 */

int is_loggable (const int log_level);



#endif /*LISPD_LOG_H_*/
