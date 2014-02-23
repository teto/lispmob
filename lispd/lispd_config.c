/*
 * lispd_config.c
 *
 * This file is part of LISP Mobile Node Implementation.
 * Handle lispd command line and config file
 * Parse command line args using gengetopt.
 * Handle config file with libconfuse.
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
 *    Lorand Jakab      <ljakab@ac.upc.edu>
 *    Alberto Rodriguez Natal <arnatal@ac.upc.edu>
 *
 */

#include "cmdline.h"
#ifdef ANDROID
#include "../android/jni/confuse_android/src/confuse.h"
#else
#include "confuse.h"
#endif
#include "lispd_afi.h"
#include "lispd_config.h"
#include "lispd_external.h"
#include "lispd_iface_list.h"
#include "lispd_lib.h"
#include "lispd_local_db.h"
#include "lispd_map_cache.h"
#include "lispd_map_cache_db.h"
#include "lispd_mapping.h"
#include "lispd_referral_cache_db.h"
#include "lispd_rloc_probing.h"



#ifdef OPENWRT
#include <uci.h>
#include <libgen.h>
#include <string.h>
#endif


/********************************** Function declaration ********************************/

int add_database_mapping(
        char   *eid,
        int    iid,
        char   *iface_name,
        char   *override_rloc,
        int    priority_v4,
        int    weight_v4,
        int    priority_v6,
        int    weight_v6);

int add_map_server(
        char       *map_server,
        int        key_type,
        char       *key,
        uint8_t    proxy_reply);

int add_proxy_etr_entry(
        char   *addr,
        int    priority,
        int    weight);

int add_server(
        char *server,
        lispd_addr_list_t  **list);

int add_static_map_cache_entry(
        char   *eid,
        int    iid,
        char   *rloc,
        int    priority,
        int    weight);

int add_ddt_root_entry(
        char                        *address,
        int                         priority,
        int                         weight);

void validate_rloc_probing_parameters (
        int probe_int,
        int probe_retries,
        int probe_retries_interval);

/*
 * Validates the information obtained from the configuration file
 */
int validate_configuration();

/****************************************************************************************/


/*
 *  handle_lispd_command_line --
 *
 *  Get command line args and set up whatever is needed
 *
 *  David Meyer
 *  dmm@1-4-5.net
 *  Wed Apr 21 13:31:00 2010
 *
 *  $Header: /usr/local/src/lispd/RCS/lispd_config.c,v 1.16 2010/04/21 23:32:08 root Exp $
 *
 */

void handle_lispd_command_line(
        int     argc,
        char    **argv)
{
    struct gengetopt_args_info args_info;

    if (cmdline_parser(argc, argv, &args_info) != 0){
        exit_cleanup();
    }

    if (args_info.daemonize_given) {
        daemonize = TRUE;
    }
    if (args_info.config_file_given) {
        config_file = strdup(args_info.config_file_arg);
    }
    if (args_info.debug_given) {
        debug_level = args_info.debug_arg;
    }else{
        debug_level = -1;
    }
    if (args_info.afi_given) {
        switch (args_info.afi_arg){
        case 0: /* afi given = 4 */
            default_rloc_afi = AF_INET;
            break;
        case 1: /* afi given = 6 */
            default_rloc_afi = AF_INET6;
            break;
        default:
            lispd_log_msg(LISP_LOG_INFO,"AFI must be IPv4 (-a 4) or IPv6 (-a 6)\n");
            break;
        }
    }else{
        default_rloc_afi = AF_UNSPEC;
    }
}


#ifdef OPENWRT
/* Compiling for OpenWRT */

/* UCI parsing function (for OpenWRT) */

int handle_uci_lispd_config_file(char *uci_conf_file_path) {


    struct uci_context  *ctx                            = NULL;
    struct uci_package  *pck                            = NULL;
    struct uci_section  *s                              = NULL;
    struct uci_element  *e                              = NULL;
    int                 uci_debug                       = 0;
    int                 uci_retries                     = 0;
    int                 uci_rloc_probe_int              = 0;
    int                 uci_rloc_probe_retries          = 0;
    int                 uci_rloc_probe_retries_interval = 0;
    const char*         uci_site_id                     = NULL;
    const char*         uci_xtr_id                      = NULL;
    const char*         uci_address                     = NULL;
    int                 uci_key_type                    = 0;
    const char*         uci_key                         = NULL;
    int                 uci_proxy_reply                 = 0;
    int                 uci_priority_v4                 = 0;
    int                 uci_weigth_v4                   = 0;
    int                 uci_priority_v6                 = 0;
    int                 uci_weigth_v6                   = 0;
    int                 uci_priority                    = 0;
    int                 uci_weigth                      = 0;
    const char*         uci_interface                   = NULL;
    int                 uci_iid                         = 0;
    const char*         uci_rloc                        = NULL;
    const char*         uci_eid_prefix                  = NULL;
    int                 uci_ddt_enabled                 = 0;

    char                *uci_conf_dir                   = NULL;
    char                *uci_conf_file                  = NULL;

    //arnatal TODO XXX: check errors for the whole function



    ctx = uci_alloc_context();

    if (ctx == NULL) {
        lispd_log_msg(LISP_LOG_CRIT, "Could not create UCI context. Exiting ...");
        exit_cleanup();
    }

    uci_conf_dir = dirname(strdup(uci_conf_file_path));
    uci_conf_file = basename(strdup(uci_conf_file_path));


    uci_set_confdir(ctx, uci_conf_dir);

    lispd_log_msg(LISP_LOG_DEBUG_1,"Conf dir: %s\n",ctx->confdir);

    uci_load(ctx,uci_conf_file,&pck);

    if (pck == NULL) {
        lispd_log_msg(LISP_LOG_CRIT, "Could not load conf file: %s. Exiting ...",uci_conf_file);
        uci_perror(ctx,"Error while loading packet ");
        uci_free_context(ctx);
        exit_cleanup();
    }


    lispd_log_msg(LISP_LOG_DEBUG_3,"package uci: %s\n",pck->ctx->confdir);


    uci_foreach_element(&pck->sections, e) {
        uci_debug = 0;
        uci_retries = 0;

        uci_address = NULL;
        uci_key_type = 0;
        uci_key = NULL;
        uci_proxy_reply = 0;
        uci_priority_v4 = 0;
        uci_weigth_v4 = 0;
        uci_priority_v6 = 0;
        uci_weigth_v6 = 0;
        uci_priority = 0;
        uci_weigth = 0;
        uci_iid = 0;
        uci_interface = NULL;
        uci_rloc = NULL;
        uci_eid_prefix = NULL;

        s = uci_to_section(e);

        if (strcmp(s->type, "daemon") == 0){

            if (strcmp(uci_lookup_option_string(ctx, s, "router_mode"), "on") == 0){
                router_mode = TRUE;
            }else{
                router_mode = FALSE;
            }

            uci_debug = strtol(uci_lookup_option_string(ctx, s, "debug"),NULL,10);


            if (debug_level == -1){//Used to not overwrite debug level passed by console
                if (uci_debug > 0)
                    debug_level = uci_debug;
                else
                    debug_level = 0;
                if (debug_level > 3)
                    debug_level = 3;
            }

            uci_retries = strtol(uci_lookup_option_string(ctx, s, "map_request_retries"),NULL,10);

            if (uci_retries >= 0 && uci_retries <= LISPD_MAX_RETRANSMITS){
                map_request_retries = uci_retries;
            }else if (uci_retries > LISPD_MAX_RETRANSMITS){
                map_request_retries = LISPD_MAX_RETRANSMITS;
                lispd_log_msg(LISP_LOG_WARNING, "Map-Request retries should be between 0 and %d. Using default value: %d",
                        LISPD_MAX_RETRANSMITS, LISPD_MAX_RETRANSMITS);
            }



            continue;
        }

        if (strcmp(s->type, "ddt-client") == 0){
            if (strcmp(uci_lookup_option_string(ctx, s, "enabled"), "on") == 0){
                ddt_client = TRUE;
            }else{
                ddt_client = FALSE;
            }
            continue;
        }

        if (strcmp(s->type, "ddt-root-node") == 0){
            uci_address = uci_lookup_option_string(ctx, s, "address");
            uci_priority = strtol(uci_lookup_option_string(ctx, s, "priority"),NULL,10);
            uci_weigth = strtol(uci_lookup_option_string(ctx, s, "weight"),NULL,10);

            if (add_ddt_root_entry(uci_address, uci_priority, uci_weigth) != GOOD ){
                lispd_log_msg(LISP_LOG_ERR, "Can't add ddt root node %s", uci_address);
            }else{
                lispd_log_msg(LISP_LOG_DEBUG_1, "Added %s as a ddt root node", uci_address);
            }
            continue;
        }


        if (strcmp(s->type, "rloc-probing") == 0){
            uci_rloc_probe_int = strtol(uci_lookup_option_string(ctx, s, "rloc_probe_interval"),NULL,10);
            uci_rloc_probe_retries = strtol(uci_lookup_option_string(ctx, s, "rloc_probe_retries"),NULL,10);
            uci_rloc_probe_retries_interval = strtol(uci_lookup_option_string(ctx, s, "rloc_probe_retries_interval"),NULL,10);
            continue;
        }

        if (strcmp(s->type, "nat-traversal") == 0){
            if (strcmp(uci_lookup_option_string(ctx, s, "nat_aware"), "on") == 0){
                nat_aware = TRUE;
            }else{
                nat_aware = FALSE;
            }
            uci_site_id = uci_lookup_option_string(ctx, s, "site_ID");
            uci_xtr_id = uci_lookup_option_string(ctx, s, "xTR_ID");

            if (nat_aware == TRUE){
                if ((convert_hex_string_to_bytes(uci_site_id,site_ID.byte,8)) != GOOD){
                    lispd_log_msg(LISP_LOG_CRIT, "Configuration file: Wrong Site-ID format");
                    exit_cleanup();
                }
                if ((convert_hex_string_to_bytes(uci_xtr_id,xTR_ID.byte,16)) != GOOD){
                    lispd_log_msg(LISP_LOG_CRIT, "Configuration file: Wrong xTR-ID format");
                    exit_cleanup();
                }
            }

            continue;
        }



        if (strcmp(s->type, "map-resolver") == 0){
            uci_address = uci_lookup_option_string(ctx, s, "address");

            if (add_server((char *)uci_address, &map_resolvers) != GOOD){
                lispd_log_msg(LISP_LOG_CRIT,"Can't add %s Map Resolver.",uci_address);
            }else{
                lispd_log_msg(LISP_LOG_DEBUG_1, "Added %s to map-resolver list", uci_address);
            }
            continue;
        }



        if (strcmp(s->type, "map-server") == 0){

            uci_address = uci_lookup_option_string(ctx, s, "address");
            uci_key_type = strtol(uci_lookup_option_string(ctx, s, "key_type"),NULL,10);
            uci_key = uci_lookup_option_string(ctx, s, "key");

            if (strcmp(uci_lookup_option_string(ctx, s, "proxy_reply"), "on") == 0){
                uci_proxy_reply = TRUE;
            }else{
                uci_proxy_reply = FALSE;
            }

            if (add_map_server((char *)uci_address,
                    uci_key_type,
                    (char *)uci_key,
                    uci_proxy_reply) != GOOD ){
                lispd_log_msg(LISP_LOG_CRIT, "Can't add %s Map Server.", uci_address);
            }else{
                lispd_log_msg(LISP_LOG_DEBUG_1, "Added %s to map-server list", uci_address);
            }
            continue;
        }


        if (strcmp(s->type, "proxy-etr") == 0){
            uci_address = uci_lookup_option_string(ctx, s, "address");
            uci_priority = strtol(uci_lookup_option_string(ctx, s, "priority"),NULL,10);
            uci_weigth = strtol(uci_lookup_option_string(ctx, s, "weight"),NULL,10);

            if (add_proxy_etr_entry((char *)uci_address,
                    uci_priority,
                    uci_weigth) != GOOD ){
                lispd_log_msg(LISP_LOG_ERR, "Can't add proxy-etr %s", uci_address);
            }else{
                lispd_log_msg(LISP_LOG_DEBUG_1, "Added %s to proxy-etr list", uci_address);
            }
            continue;
        }


        if (strcmp(s->type, "database-mapping") == 0){
            uci_eid_prefix = uci_lookup_option_string(ctx, s, "eid_prefix");
            uci_interface = uci_lookup_option_string(ctx, s, "interface");
            uci_priority_v4 = strtol(uci_lookup_option_string(ctx, s, "priority_v4"),NULL,10);
            uci_weigth_v4 = strtol(uci_lookup_option_string(ctx, s, "weight_v4"),NULL,10);
            uci_priority_v6 = strtol(uci_lookup_option_string(ctx, s, "priority_v6"),NULL,10);
            uci_weigth_v6 = strtol(uci_lookup_option_string(ctx, s, "weight_v6"),NULL,10);

            if (add_database_mapping((char *)uci_eid_prefix,
                    uci_iid,
                    (char *)uci_interface,
                    uci_priority_v4,
                    uci_weigth_v4,
                    uci_priority_v6,
                    uci_weigth_v6) != GOOD ){
                lispd_log_msg(LISP_LOG_ERR, "Can't add EID prefix %s. Discarded ...",
                        uci_eid_prefix);
            }else{
                lispd_log_msg(LISP_LOG_DEBUG_1, "Added EID prefix %s in the database.",
                        uci_eid_prefix);
            }
            continue;
        }


        if (strcmp(s->type, "static-map-cache") == 0){
            uci_eid_prefix = uci_lookup_option_string(ctx, s, "eid_prefix");
            uci_rloc = uci_lookup_option_string(ctx, s, "rloc");
            uci_priority = strtol(uci_lookup_option_string(ctx, s, "priority"),NULL,10);
            uci_weigth = strtol(uci_lookup_option_string(ctx, s, "weight"),NULL,10);

            if (add_static_map_cache_entry((char *)uci_eid_prefix,
                    uci_iid,
                    (char *)uci_rloc,
                    uci_priority,
                    uci_weigth) != GOOD ){
                lispd_log_msg(LISP_LOG_WARNING,"Can't add static-map-cache (EID:%s -> RLOC:%s). Discarded ...",
                        uci_eid_prefix,
                        uci_rloc);

            }else{
                lispd_log_msg(LISP_LOG_DEBUG_1,"Added static-map-cache (EID:%s -> RLOC:%s)",
                        uci_eid_prefix,
                        uci_rloc);
            }
            continue;
        }


        if (strcmp(s->type, "proxy-itr") == 0){
            uci_address = uci_lookup_option_string(ctx, s, "address");

            if (add_server((char *)uci_address, &proxy_itrs) != GOOD){
                lispd_log_msg(LISP_LOG_ERR, "Can't add %s to proxy-itr list. Discarded ...", uci_address);
            }else{
                lispd_log_msg(LISP_LOG_DEBUG_1, "Added %s to proxy-itr list", uci_address);
            }
            continue;
        }

    }

    validate_rloc_probing_parameters (uci_rloc_probe_int, uci_rloc_probe_retries, uci_rloc_probe_retries_interval);

    if (!proxy_etrs){
        lispd_log_msg(LISP_LOG_WARNING, "No Proxy-ETR defined. Packets to non-LISP destinations will be "
                "forwarded natively (no LISP encapsulation). This may prevent mobility in some scenarios.");
        sleep(3);
    }

    if (debug_level == 1){
        lispd_log_msg (LISP_LOG_INFO, "Log levet: Low debug");
    }else if (debug_level == 2){
        lispd_log_msg (LISP_LOG_INFO, "Log levet: Medium debug");
    }else if (debug_level == 3){
        lispd_log_msg (LISP_LOG_INFO, "Log levet: High Debug ");
    }

    lispd_log_msg (LISP_LOG_DEBUG_1, "****** Summary of the configuration ******");
    dump_local_db(LISP_LOG_DEBUG_1);
    if (is_loggable(LISP_LOG_DEBUG_1)){
        dump_map_cache_db(LISP_LOG_DEBUG_1);
    }
    dump_map_servers(LISP_LOG_DEBUG_1);
    dump_servers(map_resolvers, "Map-Resolvers", LISP_LOG_DEBUG_1);
    dump_proxy_etrs(LISP_LOG_DEBUG_1);
    dump_servers(proxy_itrs, "Proxy-ITRs", LISP_LOG_DEBUG_1);

    uci_free_context(ctx);

    err = validate_configuration();

    return(err);
}

#else
/* OPENWRT is not defined */


/*
 *  handle_lispd_config_file --
 *
 *  Parse config file and set up whatever is needed
 *
 *  David Meyer
 *  dmm@1-4-5.net
 *  Wed Apr 21 13:31:00 2010
 *
 *  $Header: /usr/local/src/lispd/RCS/lispd_config.c,v 1.16 2010/04/21 23:32:08 root Exp $
 *
 */

int handle_lispd_config_file(char * lispdconf_conf_file)
{

    cfg_t                   *cfg                    = 0;
    int                     i                       = 0;
    int                     n                       = 0;
    int                     ret                     = 0;
    char                    *map_resolver           = NULL;
    char                    *proxy_itr              = NULL;
    char                    *nat_site_ID            = NULL;
    char                    *nat_xTR_ID             = NULL;
    int                     probe_int               = 0;
    int                     probe_retries           = 0;
    int                     probe_retries_interval  = 0;
    int                     ctr                     = 0;

    static cfg_opt_t map_server_opts[] = {
            CFG_STR("address",              0, CFGF_NONE),
            CFG_INT("key-type",             0, CFGF_NONE),
            CFG_STR("key",                  0, CFGF_NONE),
            CFG_BOOL("proxy-reply", cfg_false, CFGF_NONE),
            CFG_END()
    };

    static cfg_opt_t db_mapping_opts[] = {
            CFG_STR("eid-prefix",           0, CFGF_NONE),
            CFG_INT("iid",                  0, CFGF_NONE),
            CFG_STR("interface",            0, CFGF_NONE),
            CFG_STR("advertise-rloc",       0, CFGF_NONE),
            CFG_INT("priority_v4",          0, CFGF_NONE),
            CFG_INT("weight_v4",            0, CFGF_NONE),
            CFG_INT("priority_v6",          0, CFGF_NONE),
            CFG_INT("weight_v6",            0, CFGF_NONE),
            CFG_END()
    };

    static cfg_opt_t mc_mapping_opts[] = {
            CFG_STR("eid-prefix",           0, CFGF_NONE),
            CFG_INT("iid",                  0, CFGF_NONE),
            CFG_STR("rloc",                 0, CFGF_NONE),
            CFG_INT("priority",             0, CFGF_NONE),
            CFG_INT("weight",               0, CFGF_NONE),
            CFG_END()
    };

    static cfg_opt_t petr_mapping_opts[] = {
            CFG_STR("address",              0, CFGF_NONE),
            CFG_INT("priority",           255, CFGF_NONE),
            CFG_INT("weight",               0, CFGF_NONE),
            CFG_END()
    };

    static cfg_opt_t ddt_root_node_opts[] = {
            CFG_STR("address",              0, CFGF_NONE),
            CFG_INT("priority",           255, CFGF_NONE),
            CFG_INT("weight",               0, CFGF_NONE),
            CFG_END()
    };

    static cfg_opt_t nat_traversal_opts[] = {
            CFG_BOOL("nat_aware",   cfg_false, CFGF_NONE),
            CFG_STR("site_ID",              0, CFGF_NONE),
            CFG_STR("xTR_ID",               0, CFGF_NONE),
            CFG_END()
    };

    static cfg_opt_t rloc_probing_opts[] = {
            CFG_INT("rloc-probe-interval",           0, CFGF_NONE),
            CFG_INT("rloc-probe-retries",            0, CFGF_NONE),
            CFG_INT("rloc-probe-retries-interval",   0, CFGF_NONE),
            CFG_END()
    };

    cfg_opt_t opts[] = {
            CFG_SEC("database-mapping",     db_mapping_opts, CFGF_MULTI),
            CFG_SEC("static-map-cache",     mc_mapping_opts, CFGF_MULTI),
            CFG_SEC("map-server",           map_server_opts, CFGF_MULTI),
            CFG_SEC("proxy-etr",            petr_mapping_opts, CFGF_MULTI),
            CFG_BOOL("ddt-client",          cfg_false, CFGF_NONE),
            CFG_SEC("ddt-root-node",        ddt_root_node_opts, CFGF_MULTI),
            CFG_SEC("nat-traversal",        nat_traversal_opts, CFGF_MULTI),
            CFG_SEC("rloc-probing",         rloc_probing_opts, CFGF_MULTI),
            CFG_INT("map-request-retries",  0, CFGF_NONE),
            CFG_INT("control-port",         0, CFGF_NONE),
            CFG_INT("debug",                0, CFGF_NONE),
            CFG_BOOL("router-mode",         cfg_false, CFGF_NONE),
            CFG_INT("rloc-probing-interval",0, CFGF_NONE),
            CFG_STR_LIST("map-resolver",    0, CFGF_NONE),
            CFG_STR_LIST("proxy-itrs",      0, CFGF_NONE),
#ifdef ANDROID
            CFG_BOOL("override-dns",   		    cfg_false, CFGF_NONE),
            CFG_STR("override-dns-primary",     0, CFGF_NONE),
            CFG_STR("override-dns-secondary",   0, CFGF_NONE),
#endif
            CFG_END()
    };

    /*
     *  parse config_file
     */

    cfg = cfg_init(opts, CFGF_NOCASE);
    ret = cfg_parse(cfg, lispdconf_conf_file);

    if (ret == CFG_FILE_ERROR) {
        lispd_log_msg(LISP_LOG_CRIT, "Couldn't find config file %s, exiting...", config_file);
        exit_cleanup();
    } else if(ret == CFG_PARSE_ERROR) {
        lispd_log_msg(LISP_LOG_CRIT, "Parse error in file %s, exiting. Check conf file (see lispd.conf.example)", config_file);
        exit_cleanup();
    }


    /*
     *  lispd config options
     */

    router_mode   = cfg_getbool(cfg, "router-mode") ? TRUE:FALSE;

    ret = cfg_getint(cfg, "map-request-retries");
    if (ret >= 0){
        if (ret > LISPD_MAX_RETRANSMITS){
            ret = LISPD_MAX_RETRANSMITS;
            lispd_log_msg(LISP_LOG_WARNING, "Map-Request retries should be between 0 and %d. Using default value: %d",
                    LISPD_MAX_RETRANSMITS, LISPD_MAX_RETRANSMITS);
        }
        map_request_retries = ret;
    }


    /*
     * Debug level
     */

    if (debug_level == -1){
        ret = cfg_getint(cfg, "debug");
        if (ret > 0)
            debug_level = ret;
        else
            debug_level = 0;
        if (debug_level > 3)
            debug_level = 3;
    }


    /*
     *  RLOC Probing options
     */

    cfg_t *dm = cfg_getnsec(cfg, "rloc-probing", 0);
    if (dm != NULL){
        probe_int = cfg_getint(dm, "rloc-probe-interval");
        probe_retries = cfg_getint(dm, "rloc-probe-retries");
        probe_retries_interval = cfg_getint(dm, "rloc-probe-retries-interval");

        validate_rloc_probing_parameters (probe_int, probe_retries, probe_retries_interval);
    }else{
        lispd_log_msg(LISP_LOG_DEBUG_1, "Configuration file: RLOC probing not defined. "
                "Setting default values: RLOC Probing Interval: %d sec.",RLOC_PROBING_INTERVAL);
    }


    /*
     * Nat Traversal options
     */
    cfg_t *nt = cfg_getnsec(cfg, "nat-traversal", 0);
    if (nt != NULL){
        nat_aware   = cfg_getbool(nt, "nat_aware") ? TRUE:FALSE;
        nat_site_ID = cfg_getstr(nt, "site_ID");
        nat_xTR_ID  = cfg_getstr(nt, "xTR_ID");
        if (nat_aware == TRUE){
            if ((convert_hex_string_to_bytes(nat_site_ID,site_ID.byte,8)) != GOOD){
                lispd_log_msg(LISP_LOG_CRIT, "Configuration file: Wrong Site-ID format");
                exit_cleanup();
            }
            if ((convert_hex_string_to_bytes(nat_xTR_ID,xTR_ID.byte,16)) != GOOD){
                lispd_log_msg(LISP_LOG_CRIT, "Configuration file: Wrong xTR-ID format");
                exit_cleanup();
            }
        }
    }else {
        nat_aware = FALSE;
    }

    /*
     *  LISP config options
     */

    /*
     *  handle map-resolver config
     */
    n = cfg_size(cfg, "map-resolver");

    for(i = 0; i < n; i++) {
        if ((map_resolver = cfg_getnstr(cfg, "map-resolver", i)) != NULL) {
            if (add_server(map_resolver, &map_resolvers) == GOOD){
                lispd_log_msg(LISP_LOG_DEBUG_1, "Added %s to map-resolver list", map_resolver);
            }else{
                lispd_log_msg(LISP_LOG_CRIT,"Can't add %s Map Resolver.",map_resolver);
            }
        }
    }

    /*
     *  handle ddt root node config
     */

    ddt_client   = cfg_getbool(cfg, "ddt-client") ? TRUE:FALSE;

    n = cfg_size(cfg, "ddt-root-node");
    for(i = 0; i < n; i++) {
        cfg_t *ddt_node = cfg_getnsec(cfg, "ddt-root-node", i);
        if (add_ddt_root_entry(cfg_getstr(ddt_node, "address"),
                cfg_getint(ddt_node, "priority"),
                cfg_getint(ddt_node, "weight")) == GOOD) {
            lispd_log_msg(LISP_LOG_DEBUG_1, "Added %s to ddt root node list", cfg_getstr(ddt_node, "address"));
        } else{
            lispd_log_msg(LISP_LOG_ERR, "Can't add ddt root node %s", cfg_getstr(ddt_node, "address"));
        }
    }

    /*
     *  handle proxy-etr config
     */

    n = cfg_size(cfg, "proxy-etr");
    for(i = 0; i < n; i++) {
        cfg_t *petr = cfg_getnsec(cfg, "proxy-etr", i);
        if (add_proxy_etr_entry(cfg_getstr(petr, "address"),
                cfg_getint(petr, "priority"),
                cfg_getint(petr, "weight")) == GOOD) {
            lispd_log_msg(LISP_LOG_DEBUG_1, "Added %s to proxy-etr list", cfg_getstr(petr, "address"));
        } else{
            lispd_log_msg(LISP_LOG_ERR, "Can't add proxy-etr %s", cfg_getstr(petr, "address"));
        }
    }

    /*
     *  handle proxy-itr config
     */

    n = cfg_size(cfg, "proxy-itrs");
    for(i = 0; i < n; i++) {
        if ((proxy_itr = cfg_getnstr(cfg, "proxy-itrs", i)) != NULL) {
            if (add_server(proxy_itr, &proxy_itrs)==GOOD){
                lispd_log_msg(LISP_LOG_DEBUG_1, "Added %s to proxy-itr list", proxy_itr);
            }else {
                lispd_log_msg(LISP_LOG_ERR, "Can't add %s to proxy-itr list. Discarded ...", proxy_itr);
            }
        }
    }

    /*
     *  handle database-mapping config
     */

    n = cfg_size(cfg, "database-mapping");
    for(i = 0; i < n; i++) {
        ctr ++;
        cfg_t *dm = cfg_getnsec(cfg, "database-mapping", i);
        if (add_database_mapping(cfg_getstr(dm, "eid-prefix"),
                cfg_getint(dm, "iid"),
                cfg_getstr(dm, "interface"),
                cfg_getstr(dm, "advertise-rloc"),
                cfg_getint(dm, "priority_v4"),
                cfg_getint(dm, "weight_v4"),
                cfg_getint(dm, "priority_v6"),
                cfg_getint(dm, "weight_v6")) == GOOD
        ) {
            lispd_log_msg(LISP_LOG_DEBUG_1, "Added EID %s in the database.",
                    cfg_getstr(dm, "eid-prefix"));
        }else{
            lispd_log_msg(LISP_LOG_ERR, "Can't add database-mapping %s. Discarded ...",
                    cfg_getstr(dm, "eid-prefix"));
        }

    }

    /*
     *  handle map-server config
     */

    n = cfg_size(cfg, "map-server");
    for(i = 0; i < n; i++) {
        cfg_t *ms = cfg_getnsec(cfg, "map-server", i);
        if (add_map_server(cfg_getstr(ms, "address"),
                cfg_getint(ms, "key-type"),
                cfg_getstr(ms, "key"),
                (cfg_getbool(ms, "proxy-reply") ? 1:0))== GOOD
        ){
            lispd_log_msg(LISP_LOG_DEBUG_1, "Added %s to map-server list",cfg_getstr(ms, "address"));
        }else {
            lispd_log_msg(LISP_LOG_WARNING, "Can't add %s Map Server.",cfg_getstr(ms, "address"));
        }
    }

    /*
     *  handle static-map-cache config
     */

    n = cfg_size(cfg, "static-map-cache");
    for(i = 0; i < n; i++) {
        cfg_t *smc = cfg_getnsec(cfg, "static-map-cache", i);

        if (!add_static_map_cache_entry(cfg_getstr(smc, "eid-prefix"),
                cfg_getint(smc, "iid"),
                cfg_getstr(smc, "rloc"),
                cfg_getint(smc, "priority"),
                cfg_getint(smc, "weight"))

        ) {
            lispd_log_msg(LISP_LOG_WARNING,"Can't add static-map-cache (EID:%s -> RLOC:%s). Discarded ...",
                    cfg_getstr(smc, "eid-prefix"),
                    cfg_getstr(smc, "rloc"));
        }else{
            lispd_log_msg(LISP_LOG_DEBUG_1,"Added static-map-cache (EID:%s -> RLOC:%s)",
                    cfg_getstr(smc, "eid-prefix"),
                    cfg_getstr(smc, "rloc"));
        }
    }

    /* Check configured parameters when NAT-T activated. These limitations will be removed in future release */
    if (nat_aware == TRUE){
        if (ctr > 1){
            lispd_log_msg(LISP_LOG_CRIT,"NAT aware on -> This version of LISPmob is limited to one EID prefix "
                    "and one interface when NAT-T is enabled");
            exit_cleanup();
        }

        if (map_servers->next != NULL || map_servers->address->afi != AF_INET){
            lispd_log_msg(LISP_LOG_INFO,"NAT aware on -> This version of LISPmob is limited to one IPv4 Map Server.");
            exit_cleanup();
        }

        if (map_resolvers->next != NULL || map_resolvers->address->afi != AF_INET){
            lispd_log_msg(LISP_LOG_INFO,"NAT aware on -> This version of LISPmob is limited to one IPv4 Map Resolver.");
            exit_cleanup();
        }

        if (rloc_probe_interval > 0){
            rloc_probe_interval = 0;
            lispd_log_msg(LISP_LOG_INFO,"NAT aware on -> disabling RLOC Probing");
        }
    }

    /* Check number of EID prefixes */
    if (router_mode == FALSE){
        if (num_entries_in_db(get_local_db(AF_INET)) > 1){
            lispd_log_msg (LISP_LOG_ERR, "LISPmob in mobile node mode only supports one IPv4 EID prefix and one IPv6 EID prefix");
            exit_cleanup();
        }
        if (num_entries_in_db(get_local_db(AF_INET6)) > 1){
            lispd_log_msg (LISP_LOG_ERR, "LISPmob in mobile node mode only supports one IPv4 EID prefix and one IPv6 EID prefix");
            exit_cleanup();
        }
    }


    if (debug_level == 1){
        lispd_log_msg (LISP_LOG_INFO, "Log level: Low debug");
    }else if (debug_level == 2){
        lispd_log_msg (LISP_LOG_INFO, "Log level: Medium debug");
    }else if (debug_level == 3){
        lispd_log_msg (LISP_LOG_INFO, "Log level: High Debug");
    }

    lispd_log_msg (LISP_LOG_DEBUG_1, "****** Summary of the configuration ******");
    dump_local_db(LISP_LOG_DEBUG_1);
    if (is_loggable(LISP_LOG_DEBUG_1)){
        dump_map_cache_db(LISP_LOG_DEBUG_1);
    }
    dump_map_servers(LISP_LOG_DEBUG_1);
    dump_servers(map_resolvers, "Map-Resolvers", LISP_LOG_DEBUG_1);
    dump_proxy_etrs(LISP_LOG_DEBUG_1);
    dump_servers(proxy_itrs, "Proxy-ITRs", LISP_LOG_DEBUG_1);
    if (ddt_client == TRUE){
        dump_referral_cache_db(LISP_LOG_INFO);
    }

    cfg_free(cfg);

    err = validate_configuration();

    return(err);
}



#endif
/* ifdef OPENWRT*/


/*
 *  add_database_mapping
 *
 *  Get a single database mapping
 *
 *  David Meyer <dmm@1-4-5.net>
 *  Preethi Natarajan <prenatar@cisco.com>
 *
 */

int add_database_mapping(
        char   *eid,
        int    iid,
        char   *iface_name,
        char   *override_rloc,
        int    priority_v4,
        int    weight_v4,
        int    priority_v6,
        int    weight_v6)

{
    lispd_mapping_elt           *mapping            = NULL;
    lispd_locator_elt           *locator            = NULL;
    lispd_iface_elt             *interface          = NULL;
    lisp_addr_t                 *advertised_addr    = NULL;
    lisp_addr_t                 eid_prefix;           /* save the eid_prefix here */
    int                         eid_prefix_length   = 0;
    uint8_t                     is_new_mapping      = FALSE;

    if (iface_name == NULL){
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: No interface specificated for database mapping. Ignoring mapping");
        return (BAD);
    }

    if (iid > MAX_IID || iid < 0) {
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: Instance ID %d out of range [0..%d], disabling...", iid, MAX_IID);
        iid = 0;
    }

    if (priority_v4 < (MAX_PRIORITY - 1) || priority_v4 > UNUSED_RLOC_PRIORITY) {
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: Priority %d out of range [%d..%d], set minimum priority...",
                priority_v4, MAX_PRIORITY, UNUSED_RLOC_PRIORITY);
        priority_v4 = MIN_PRIORITY;
    }

    if (priority_v6 < (MAX_PRIORITY - 1)|| priority_v6 > UNUSED_RLOC_PRIORITY) {
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: Priority %d out of range [%d..%d], set minimum priority...",
                priority_v6, MAX_PRIORITY, UNUSED_RLOC_PRIORITY);
        priority_v6 = MIN_PRIORITY;
    }

    if (weight_v4 < (MIN_WEIGHT) || weight_v4 > MAX_WEIGHT) {
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: Weight %d out of range [%d..%d], set weight to 100...",
                weight_v4, MIN_WEIGHT, MAX_WEIGHT);
        weight_v4 = 100;
    }

    if (weight_v6 < (MIN_WEIGHT) || weight_v6 > MAX_WEIGHT) {
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: Weight %d out of range [%d..%d], set weight to 100...",
                weight_v6, MIN_WEIGHT, MAX_WEIGHT);
        weight_v6 = 100;
    }



    if (get_lisp_addr_and_mask_from_char(eid,&eid_prefix,&eid_prefix_length)!=GOOD){
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: Error parsing EID address");
        return (BAD);
    }


    if (if_nametoindex(iface_name) == 0) {
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: INVALID INTERFACE or not initialized virtual interface: %s ", iface_name);
    }



    /*
     * Lookup if the mapping exists. If not, a new mapping is created.
     */
    mapping = lookup_eid_exact_in_db(eid_prefix,eid_prefix_length);
    if (mapping == NULL)
    {
        mapping = new_local_mapping(eid_prefix,eid_prefix_length,iid);
        if (mapping == NULL){
            lispd_log_msg(LISP_LOG_ERR,"Configuration file: mapping %s could not be added",eid);
            return (BAD);
        }
        /* Add the mapping to the local database */
        if (add_mapping_to_db(mapping)!=GOOD){
            free_mapping_elt(mapping);
            return (BAD);
        }
        total_mappings ++;
        is_new_mapping = TRUE;
    }else{
        if (mapping->iid != iid){
            lispd_log_msg(LISP_LOG_ERR,"Same EID prefix with different iid. This configuration is not supported..."
                    "Ignoring EID prefix.");
            return (BAD);
        }
        is_new_mapping = FALSE;
    }
    /*
     * Add the interface.
     */
    /* Check if the interface already exists. If not, add it*/
    if ((interface=get_interface(iface_name))==NULL){
        interface = add_interface (iface_name);
    }

    /* set advertised addr check if override_rloc is defined. Only defined for IPv4 */
    if( override_rloc ) {
        if ((advertised_addr = malloc(sizeof(lisp_addr_t))) == NULL) {
            lispd_log_msg(LISP_LOG_WARNING, "Unable to allocate memory for lisp_addr_t: %s", strerror(errno));
            return(ERR_MALLOC);
        }

        if(get_lisp_addr_from_char( override_rloc, advertised_addr) != GOOD){
            lispd_log_msg(LISP_LOG_WARNING, "Unable to create lisp_addr");
            free(advertised_addr);
            return (BAD);
        }

    }


    /* If we couldn't add the interface and the mapping is new, we remove it. */
    if (interface == NULL && is_new_mapping == TRUE){
        if (is_new_mapping){
            del_mapping_entry_from_db (mapping->eid_prefix, mapping->eid_prefix_length);
            lispd_log_msg(LISP_LOG_WARNING,"add_database_mapping: Couldn't add mapping -> Couldn't create interface");
        }else{
            lispd_log_msg(LISP_LOG_WARNING,"add_database_mapping: Couldn't add locator to the mapping -> Couldn't create interface");
        }
        return (BAD);
    }

    /* Assign the mapping to the v4 mappings of the interface. Create IPv4 locator and assign to the mapping  */
    if (priority_v4 >= 0){
        if ((err = add_mapping_to_interface (interface, mapping, AF_INET)) == GOOD){

            locator = new_local_locator (interface->ipv4_address, ((advertised_addr) ? advertised_addr : interface->ipv4_address ), &(interface->status),priority_v4,weight_v4,255,0,&(interface->out_socket_v4));

            if (locator != NULL){
                if ((err=add_locator_to_mapping (mapping,locator))!=GOOD){
                    free_locator(locator);
                    return (BAD);
                }
            }else{
                return (BAD);
            }
        }else {
            return (BAD);
        }
    }
    /* Assign the mapping to the v6 mappings of the interface. Create IPv6 locator and assign to the mapping  */
    if (priority_v6 >= 0){
        if ((err = add_mapping_to_interface (interface, mapping, AF_INET6)) == GOOD){
            locator = new_local_locator (interface->ipv6_address, ((advertised_addr) ? advertised_addr : interface->ipv6_address ),&(interface->status),priority_v6,weight_v6,255,0,&(interface->out_socket_v6));
            if (locator != NULL){
                if ((err=add_locator_to_mapping (mapping,locator))!=GOOD){
                    free_locator(locator);
                    return (BAD);
                }
            }else{
                return (BAD);
            }
        }else{
            return (BAD);
        }
    }
    /* Recalculate the outgoing rloc vectors */
    if (calculate_balancing_vectors (mapping,&((lcl_mapping_extended_info *)mapping->extended_info)->outgoing_balancing_locators_vecs) != GOOD){
        lispd_log_msg(LISP_LOG_WARNING,"add_database_mapping: Couldn't calculate outgoing rloc prefenernce");
    }


    return(GOOD);
}

/*
 *  add_static_map_cache_entry --
 *
 *  Get a single static mapping
 *
 *  David Meyer
 *  dmm@1-4-5.net
 *  Wed Apr 21 13:31:00 2010
 *
 *  $Header: /usr/local/src/lispd/RCS/lispd_config.c,v 1.16 2010/04/21 23:32:08 root Exp $
 *
 */

int add_static_map_cache_entry(
        char   *eid,
        int    iid,
        char   *rloc_addr,
        int    priority,
        int    weight)
{
    lispd_map_cache_entry    *map_cache_entry       = NULL;
    lispd_locator_elt        *locator               = NULL;
    lisp_addr_t              *locator_addr          = NULL;
    lisp_addr_t              eid_prefix             = {.afi=AF_UNSPEC};
    int                      eid_prefix_length      = 0;


    if (iid > MAX_IID || iid < 0) {
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: Instance ID %d out of range [0..%d], disabling...", iid, MAX_IID);
        iid = 0;
    }

    if (priority < MAX_PRIORITY || priority > UNUSED_RLOC_PRIORITY) {
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: Priority %d out of range [%d..%d], set minimum priority...",
                priority, MAX_PRIORITY, UNUSED_RLOC_PRIORITY);
        priority = MIN_PRIORITY;
    }

    if (get_lisp_addr_and_mask_from_char(eid,&eid_prefix,&eid_prefix_length)!=GOOD){
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: Error parsing RLOC address ...Ignoring static map cache entry");
        return (BAD);
    }

    if ((locator_addr = (lisp_addr_t *)malloc(sizeof(lisp_addr_t))) == NULL){
        lispd_log_msg(LISP_LOG_ERR, "add_static_map_cache_entry: Unable to allocate memory for lisp_addr_t: %s", strerror(errno));
        return (BAD);
    }

    if (get_lisp_addr_from_char(rloc_addr,locator_addr) != GOOD){
        lispd_log_msg(LISP_LOG_ERR, "add_static_map_cache_entry: Error parsing RLOC address ... Ignoring static map cache entry");
        free(locator_addr);
        return (BAD);
    }

    map_cache_entry = new_map_cache_entry(eid_prefix, eid_prefix_length, STATIC_MAP_CACHE_ENTRY,255);
    if (map_cache_entry == NULL){
        free(locator_addr);
        return (BAD);
    }

    map_cache_entry->mapping->iid = iid;

    locator = new_static_rmt_locator(locator_addr,UP,priority,weight,255,0);

    if (locator != NULL){
        if ((err=add_locator_to_mapping (map_cache_entry->mapping, locator)) != GOOD){
            free_locator(locator);
            free_map_cache_entry(map_cache_entry);
            return (BAD);
        }
    }else{
        free_map_cache_entry(map_cache_entry);
        return (BAD);
    }

    /*
     * Programming rloc probing timer
     */
    programming_rloc_probing(map_cache_entry);

    return (GOOD);
}

/*
 *  add a map-resolver to the list
 */

int add_server(
        char                *server,
        lispd_addr_list_t   **list)
{
    lispd_addr_list_t   *new_list   = NULL;
    lispd_addr_list_t   *aux_list   = NULL;

    new_list = lispd_get_address( server, default_rloc_afi );
    if (new_list == NULL){
        lispd_log_msg(LISP_LOG_WARNING, "add_server: Unable to convert addresses: %s", server);
        return (BAD);
    }

    if( *list == NULL ){
        *list = new_list;
    }else{
        aux_list = new_list;
        while(aux_list->next != NULL){
            aux_list = aux_list->next;
        }
        aux_list->next = *list;
        *list = new_list;
    }
    return(GOOD);
}

/*
 *  add_map_server to map_servers
 */

int add_map_server(
        char         *map_server,
        int          key_type,
        char         *key,
        uint8_t      proxy_reply)

{
    lispd_addr_list_t               *list        = NULL;
    lisp_addr_t                     *addr        = NULL;
    lispd_map_server_list_t         *list_elt    = NULL;
    int                             result       = GOOD;

    if (map_server == NULL || key_type == 0 || key == NULL){
        lispd_log_msg(LISP_LOG_ERR, "Configuraton file: Wrong Map Server configuration.  Check configuration file");
        exit_cleanup();
    }

    list = lispd_get_address (map_server,default_rloc_afi);

    if (list == NULL){
        result = BAD;
    }

    while (list != NULL){
        addr = list->address;
        if ((list_elt = (lispd_map_server_list_t *)calloc(1,sizeof(lispd_map_server_list_t))) == NULL) {
            lispd_log_msg(LISP_LOG_WARNING, "add_map_server: Unable to allocate memory for lispd_map_server_list_t: %s", strerror(errno));
            free(addr);
            result = BAD;
            list = list->next;
            continue;
        }

        list_elt->address     = addr;
        list_elt->key_type    = key_type;
        list_elt->key         = strdup(key);
        list_elt->proxy_reply = proxy_reply;

        if(map_servers != NULL){
            list_elt->next = map_servers;
            map_servers = list_elt;
        }else {
            map_servers = list_elt;
        }

        list = list->next;
    }

    return(result);
}

/*
 * Add a locator of a ddt node to the root referral cache entry (root of the ddt tree)
 */

int add_ddt_root_entry(
        char                        *address,
        int                         priority,
        int                         weight)
{
    lispd_addr_list_t               *list                   = NULL;
    lisp_addr_t                     *ddt_locator_address    = NULL;
    lisp_addr_t                     root_eid_prefix         = {.afi = AF_UNSPEC};
    int                             result                  = GOOD;


    /* Validate data introduced in the configuration file */

    if (address == NULL){
        lispd_log_msg(LISP_LOG_ERR, "Configuration file (add_ddt_root_entry): The address of the  DDT root entry has not been specified. Discarding the entry");
        return (BAD);
    }

    if ((list = lispd_get_address (address,default_rloc_afi)) == NULL){
        lispd_log_msg(LISP_LOG_WARNING, "add_ddt_root_entry: Unable to process ddt root node with locator %s", address);
        return (BAD);
    }

    /* Check the parameters */
    if (priority > 255 || priority < 0) {
        lispd_log_msg(LISP_LOG_ERR, "Configuration file (add_ddt_root_entry): Priority %d out of range [0..255]", priority);
        return (BAD);
    }

    if (weight > 100 || weight < 0) {
        lispd_log_msg(LISP_LOG_ERR, "Configuration file (add_ddt_root_entry): Weight %d out of range [0..100]", priority);
        return (BAD);
    }

    while (list != NULL){
        ddt_locator_address = list->address;

        get_lisp_addr_from_char ("0.0.0.0", &root_eid_prefix);
        if ((err = add_update_ddt_static_entry_to_db (root_eid_prefix, 0, 0, ddt_locator_address, 10, 100, 0)) != GOOD ){
            lispd_log_msg(LISP_LOG_WARNING, "Configuration file (add_ddt_root_entry): Error processing the root ddt node %s (IPv4 database)",
                    address);
            free(ddt_locator_address);
            result = BAD;
        }

        get_lisp_addr_from_char ("0::0", &root_eid_prefix);
        if ((err = add_update_ddt_static_entry_to_db (root_eid_prefix, 0, 0, ddt_locator_address, 10, 100, 0)) != GOOD ){
            lispd_log_msg(LISP_LOG_WARNING, "Configuration file (add_ddt_root_entry): Error processing the root ddt node %s (IPv6 database)",
                    address);
            free(ddt_locator_address);
            result = BAD;
        }

        list = list->next;
    }

    return (result);
}

/*
 *  add_proxy_etr_entry --
 *
 *  Add a proxy-etr entry
 *
 */

int add_proxy_etr_entry(
        char                        *address,
        int                         priority,
        int                         weight)
{
    lispd_addr_list_t               *list        = NULL;
    lisp_addr_t                     petr_addr    = {.afi=AF_UNSPEC};
    lisp_addr_t                     *lisp_addr   = NULL;
    lispd_locator_elt               *locator     = NULL;
    int                             result       = GOOD;

    if (address == NULL){
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: The address of the Proxy ETR has not been specified. Discarding the entry");
        return (BAD);
    }

    /* Check the parameters */
    if (priority > 255 || priority < 0) {
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: Priority %d out of range [0..255]", priority);
        return (BAD);
    }

    if (weight > 100 || weight < 0) {
        lispd_log_msg(LISP_LOG_ERR, "Configuration file: Weight %d out of range [0..100]", priority);
        return (BAD);
    }

    if ((list = lispd_get_address (address,default_rloc_afi)) == NULL){
        lispd_log_msg(LISP_LOG_WARNING, "add_proxy_etr_entry: Unable to process proxy ETR  with locator %s", address);
        return (BAD);
    }

    /* Create the proxy-etrs map cache structure if it doesn't exist */
    if (proxy_etrs == NULL){
        if ((get_lisp_addr_from_char ("0.0.0.0", &petr_addr))!=GOOD){
            return (BAD);
        }
        proxy_etrs = new_map_cache_entry_no_db (petr_addr,0,STATIC_MAP_CACHE_ENTRY,0);
        if (proxy_etrs == NULL){
            return (BAD);
        }
    }

    while (list != NULL){
        lisp_addr = list->address;
        /* Create de locator representing the proxy-etr and add it to the mapping */
        locator = new_static_rmt_locator (lisp_addr,UP,priority,weight,255,0);
        if (locator != NULL){
            if ((err=add_locator_to_mapping (proxy_etrs->mapping, locator)) != GOOD){
                lispd_log_msg(LISP_LOG_DEBUG_1, "add_proxy_etr_entry: %s caouldn't be added to the proxy ETR list", get_char_from_lisp_addr_t(*lisp_addr));
                free_locator(locator);
                result = BAD;
            }
        }else{
            lispd_log_msg(LISP_LOG_DEBUG_1, "add_proxy_etr_entry: %s caouldn't be added to the proxy ETR list", get_char_from_lisp_addr_t(*lisp_addr));
            free(lisp_addr);
            result = BAD;
        }
        list = list->next;
    }

    return(result);
}

void validate_rloc_probing_parameters (
        int probe_int,
        int probe_retries,
        int probe_retries_interval){

    if (probe_int  < 0){
        rloc_probe_interval = 0;
    }else{
        rloc_probe_interval = probe_int;
    }
    if (rloc_probe_interval > 0){
        lispd_log_msg(LISP_LOG_DEBUG_1, "RLOC Probing Interval: %d", rloc_probe_interval);
    }else{
        lispd_log_msg(LISP_LOG_DEBUG_1, "RLOC Probing dissabled");
    }

    if (rloc_probe_interval != 0){

        if(probe_retries > LISPD_MAX_RETRANSMITS){
            rloc_probe_retries = LISPD_MAX_RETRANSMITS;
            lispd_log_msg(LISP_LOG_WARNING, "RLOC Probing retries should be between 0 and %d. Using %d retries",
                    LISPD_MAX_RETRANSMITS, LISPD_MAX_RETRANSMITS);
        }else if (probe_retries < 0){
            rloc_probe_retries = 0;
            lispd_log_msg(LISP_LOG_WARNING, "RLOC Probing retries should be between 0 and %d. Using 0 retries",
                    LISPD_MAX_RETRANSMITS);
        }else{
            rloc_probe_retries = probe_retries;
        }

        if (rloc_probe_retries > 0){
            if (probe_retries_interval < LISPD_MIN_RETRANSMIT_INTERVAL){
                rloc_probe_retries_interval = LISPD_MIN_RETRANSMIT_INTERVAL;
                lispd_log_msg(LISP_LOG_WARNING, "RLOC Probing interval retries should be between %d and RLOC Probing interval. Using %d seconds",
                        LISPD_MIN_RETRANSMIT_INTERVAL,LISPD_MIN_RETRANSMIT_INTERVAL);
            }else if(probe_retries_interval > rloc_probe_interval){
                rloc_probe_retries_interval = rloc_probe_interval;
                lispd_log_msg(LISP_LOG_WARNING, "RLOC Probing interval retries should be between %d and RLOC Probing interval. Using %d seconds",
                        LISPD_MIN_RETRANSMIT_INTERVAL,rloc_probe_interval);
            }else{
                rloc_probe_retries_interval = probe_retries_interval;
            }
        }
    }
}

/*
 * Validates the information obtained from the configuration file
 */
int validate_configuration()
{
    int result = GOOD;
    if (map_servers == NULL){
        lispd_log_msg(LISP_LOG_CRIT, "No Map Server configured.");
        result = BAD;
    }

    if (map_resolvers == NULL && ddt_client == FALSE){
        if (is_referral_db_empty() == FALSE){
            ddt_client = TRUE;
            lispd_log_msg(LISP_LOG_WARNING, "No Map Resolver configured. Enabling DDT client mode");
        }else{
            lispd_log_msg(LISP_LOG_CRIT, "No Map Resolver configured.");
            result = BAD;
        }
    }

    if(ddt_client == TRUE && is_referral_db_empty() == TRUE){
        lispd_log_msg(LISP_LOG_WARNING, "DDT client mode enabled but no ddt root node configured");
        result = BAD;
    }

    if (proxy_etrs == NULL){
        lispd_log_msg(LISP_LOG_WARNING, "No Proxy-ETR defined. Packets to non-LISP destinations will be "
                "forwarded natively (no LISP encapsulation). This may prevent mobility in some scenarios.");
        sleep(3);
    }else{
        calculate_balancing_vectors (
                proxy_etrs->mapping,
                &(((rmt_mapping_extended_info *)(proxy_etrs->mapping->extended_info))->rmt_balancing_locators_vecs));
    }

    return (result);
}

/*
 * Editor modelines
 *
 * vi: set shiftwidth=4 tabstop=4 expandtab:
 * :indentSize=4:tabSize=4:noTabs=true:
 */

