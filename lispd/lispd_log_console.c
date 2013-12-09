
#include "lispd_log_console.h"
#include "lispd_external.h"


/* styles */
typedef enum {
    style_plain = 0,
    style_bold = 1,
    style_faint = 2,
    style_italic = 3,
    style_underline = 4
} text_style_t;

typedef struct {
    int fg;
    int bg;
    int attr;
//    text_style_t attr;
} color_set_t;




typedef enum {
    lispd_color_black      = 0,
    lispd_color_red,
    lispd_color_green,
    lispd_color_yellow,
    lispd_color_blue,
    lispd_color_magenta,
    lispd_color_cyan,
    lispd_color_white
} lispd_color_t;


#define RESET   "\033[0m"


#define FG_COLOR( color ) ( 30 + lispd_color_ ## color )
#define BG_COLOR( color ) ( 40 + lispd_color_ ## color )



color_set_t color_sets[lispd_item_last] = {

/*fg/bg/attr/set_as_default*/
[lispd_item_default]       = { FG_COLOR(white), BG_COLOR(black), style_plain},
[lispd_item_eid]           = { FG_COLOR(black), BG_COLOR(yellow), style_underline},
[lispd_item_crit]          = { FG_COLOR(black), BG_COLOR(magenta),1},
[lispd_item_err]           = { FG_COLOR(blue), 1, style_plain},
[lispd_item_debug]         = { FG_COLOR(blue), 1, style_plain},
[lispd_item_warning]       = { 30, BG_COLOR(magenta),0},
[lispd_item_info]          = { FG_COLOR(blue), 1, style_plain},
[lispd_item_rloc]          = { FG_COLOR(magenta),1,0},
[lispd_item_port]          = { 1,32,0},
[lispd_item_filename]      = { 1,31,0}
};





static void lispd_log_console_close_entry(lispd_log_entry_t *entry)
{
    static char* eol[2] = { "\n", "\033[0m\n" };
    printf("%s%s", entry->str, eol[colorize] );
}

#define COLOR_SET_FORMAT "\e[%d;%dm\e[%dm"
#define EXPAND_COLOR_SET( color_set )   (color_set).attr , (color_set).fg , (color_set).bg

#define LISPD_APPEND_TO_ENTRY_FINAL(format, ... ) snprintf( startingPoint, MAX_STRING_LENGTH-usedLen, format, __VA_ARGS__ );


// TODO convert into inline function ?
#define LISPD_APPEND_TO_STR( format, value) do {if(colorize)  \
                                                    LISPD_APPEND_TO_ENTRY_FINAL( "\e[%d;%dm\e[%dm" format , EXPAND_COLOR_SET( *cs ) , value ) \
                                                 else  \
                                                    LISPD_APPEND_TO_ENTRY_FINAL( (format), value ) \
                                                } while(0)





static void lispd_log_console_append_to_entry(lispd_log_entry_t *entry, const lispd_log_item_type_t type, const char* str, const int integer, void *data)
{

    color_set_t *cs = &color_sets[type];

    size_t usedLen = strlen(entry->str);
    char *startingPoint = &(entry->str[usedLen]);

    switch (type) {
        case lispd_item_integer:
            LISPD_APPEND_TO_STR( "%d", integer );
            break;

        case lispd_item_warning:
        case lispd_item_info:
        case lispd_item_crit:
        case lispd_item_err:
        case lispd_item_debug:
            LISPD_APPEND_TO_STR( "%s: ", str );
            break;

        default:
            LISPD_APPEND_TO_STR( "%s", str );
    }


}


static lispd_log_entry_t* lispd_log_console_get_entry(const lispd_log_level_t log_level)
{

    static lispd_log_entry_t entry;

    if (! is_loggable(log_level))
        return 0;

    entry.str[0] = '\0';
    entry.log_descriptor = lispd_log_get_level_descriptor( log_level );
    entry.log_level = log_level;

    lispd_log_console_append_to_entry( &entry, entry.log_descriptor->type, entry.log_descriptor->log_name, 0,0  );

    return &entry;
}


static int lispd_log_console_close_logger(void *data)
{
    /* Do nothing */
    return 0;
}


lispd_log_ops_t log_console_ops = {
    .start_logger = 0,
    .close_logger = &lispd_log_console_close_logger,
    .new_entry = &lispd_log_console_get_entry,
    .close_entry = &lispd_log_console_close_entry,
    .append_to_entry = &lispd_log_console_append_to_entry
};



