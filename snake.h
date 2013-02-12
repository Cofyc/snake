#ifndef SNAKE_H
#define SNAKE_H

#include "compat.h"
#include <ncurses.h>

struct snake_t {
    int length;
    struct snake_node *head;
    struct snake_node *tail;
};

struct snake {
    unsigned int yard_x; /* width, defaults: 24 */
    unsigned int yard_y; /* height, defaults: 24 */
    unsigned char *yard_buffer; /* yard_x * yard_y */
    bool running;
    unsigned int food;
    struct snake_t *snake_inst;
    WINDOW *stdscr;
    WINDOW *sidebar;
};

struct snake snake;

#define SNAKE_OK      0
#define SNAKE_ERR     -1
#define SNAKE_ERR_LEN 256

int snake_init(char *err);
int snake_run(char *err);
int snake_end(void);

#endif
