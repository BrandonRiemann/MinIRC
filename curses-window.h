#ifndef CURSES_WINDOW_H__
#define CURSES_WINDOW_H__

#include <curses.h> /* For WINDOW */

#define DEFAULT_WRAP    80
#define DEFAULT_INDENT  10

typedef struct {
    int sig;
    void ( *handler )( void* );
} SignalContext;

typedef struct {
    int x;
    int y;
} CursorContext;

typedef struct {
    WINDOW *window;
    CursorContext cur_curr;
    CursorContext cur_msg;
    CursorContext cur_last;
    SignalContext *sigs;
    int num_sigs;
    int rows;
    int cols;
    int colors;
} WindowContext;

void window_init_context( WindowContext* );
void window_destroy_context( WindowContext* );
void window_save_cursor( WindowContext* );
void window_retrieve_cursor( WindowContext*, int*, int* );
void window_move_cursor( WindowContext*, int, int );
void window_set_cursor( WindowContext*, int, int );
void window_get_cursor( WindowContext*, int*, int* );
void window_get_last_cursor( WindowContext*, int*, int* );
void window_set_signal( WindowContext*, int, void* );
void window_unset_signal( WindowContext*, int );
void window_wait( WindowContext*, void ( * )( char ) );
void window_printw( WindowContext*, const char*, ... );
void window_refresh( void );
void window_clear( void );
void window_bold_on( WindowContext* );
void window_bold_off( WindowContext* );
void window_destroy( WindowContext* );
void window_create( WindowContext* );
void window_create_colored( WindowContext* );
void window_set_color_pair( WindowContext*, short, short, short );
void window_set_color_on( WindowContext*, int );
void window_set_color_off( WindowContext*, int );
int print_wrap( WindowContext*, int, int, int, int, const char*, ... );

#endif
