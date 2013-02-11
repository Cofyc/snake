#include "snake.h"

#include <unistd.h>
#include <sys/time.h>
#include "logger.h"
#include "usage.h"

/* basic block cell dimension, '[]' */
#define CELL_X 2
#define CELL_Y 1

#define CELL(status, color) (status << 4 | color)
#define XCOLOR_OF(cell)     ((cell) & 0x0F)
#define XSTATUS_OF(cell)    ((cell) >> 4)

static void
draw_cell(WINDOW *win, int y, int x, unsigned char cell)
{
    // color
    wattrset(win, COLOR_PAIR(XCOLOR_OF(cell) % 8));

    // show
    if (XSTATUS_OF(cell) == 1) {
        mvwaddch(win, y * CELL_Y, x * CELL_X, '[');
        mvwaddch(win, y * CELL_Y, x * CELL_X + 1, ']');
    } else if (XSTATUS_OF(cell) == 2) {
        mvwaddch(win, y * CELL_Y, x * CELL_X, '^');
    } else if (XSTATUS_OF(cell) == 3) {
        mvwaddch(win, y * CELL_Y, x * CELL_X, 118);
    } else if (XSTATUS_OF(cell) == 4) {
        mvwaddch(win, y * CELL_Y, x * CELL_X, '<');
    } else if (XSTATUS_OF(cell) == 5) {
        mvwaddch(win, y * CELL_Y, x * CELL_X, '>');
    } else {
        mvwaddch(win, y * CELL_Y, x * CELL_X, ' ');
        mvwaddch(win, y * CELL_Y, x * CELL_X + 1, ' ');
    }
}

struct snake_node {
    int offset_x;
    int offset_y;
    unsigned char cell;
    struct snake_node *prev;
    struct snake_node *next;
};

static struct snake_t *
snake_create()
{
    struct snake_t *snake_inst = (struct snake_t *)malloc(sizeof(struct snake_t));
    if (snake_inst == NULL)
        return NULL;
    snake_inst->head = NULL;
    snake_inst->tail = NULL;
    snake_inst->length = 0;
    return snake_inst;
}

static struct snake_node *
snake_node_create(int offset_y, int offset_x, unsigned char cell)
{
    struct snake_node *node = (struct snake_node *)malloc(sizeof(struct snake_node));
    node->offset_x = offset_x;
    node->offset_y = offset_y;
    node->cell = cell;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

static void
snake_add_node(struct snake_t *snake_inst, struct snake_node *node)
{
    assert(node->prev == NULL);
    assert(node->next == NULL);

    if (snake_inst->tail) {
        snake_inst->tail->next = node;
        node->prev = snake_inst->tail;
        snake_inst->tail = node;
    } else {
        snake_inst->tail = snake_inst->head = node;
    }
    snake_inst->length++;
}

static void
snake_eat_cell(struct snake_t *snake_inst, unsigned char cell)
{
    struct snake_node *node = snake_node_create(snake_inst->tail->offset_y, snake_inst->tail->offset_x, cell);
    snake_add_node(snake_inst, node);
}

static void
signal_handler(int sig)
{
    snake_end();
}

static int
snake_set_error(char *err, const char *fmt, ...)
{
    va_list ap;

    if (!err)
        return SNAKE_ERR;

    va_start(ap, fmt);
    vsnprintf(err, SNAKE_ERR_LEN, fmt, ap);
    va_end(ap);

    return SNAKE_ERR;
}

static void
snake_update()
{
    int i, j;

    // Update yard.
    for (i = 0; i < snake.yard_y; i++) {
        for (j = 0; j < snake.yard_x; j++) {
            draw_cell(snake.stdscr, i, j, snake.yard_buffer[i * snake.yard_x + j]);
        }
    }

    // Update snake.
    struct snake_node *node = snake.snake_inst->head;
    while (node) {
        draw_cell(snake.stdscr, node->offset_y, node->offset_x, node->cell);
        node = node->next;
    }
    wrefresh(snake.stdscr);
}

int
snake_init(char *err)
{
    /* arrange interrupts to terminate */
    (void)signal(SIGINT, signal_handler);  

    /* initialize the curses library, return pointer to stdscr */
    initscr();
    snake.stdscr = stdscr;
    /* take input chars one at a time, no wait for \n */
    cbreak();
    noecho();
    /* if false, then reads will block */
    nodelay(stdscr, false);
    /* tell curses not to do NL->CR/NL on output */
    nonl();
    /* enable keyboard mapping (function key -> single value) */
    keypad(stdscr, true);
    /*
     * set cursor invisible
     * @Note: mac os x terminal don't support this currently
     */
    curs_set(0);

    if (has_colors()) {
        start_color();
        /*
         * Simple color assignment, often all we need.  Color pair 0 cannot
         * be redefined.  This example uses the same value for the color
         * pair as for the foreground color, though of course that is not
         * necessary:
         */
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
        init_pair(4, COLOR_BLUE, COLOR_BLACK);
        init_pair(5, COLOR_CYAN, COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_WHITE, COLOR_BLACK);
    }

    /* Check y & x. */
    int x = 0;
    int y = 0;
    getmaxyx(stdscr, y, x);
    if (x < snake.yard_x*2 + 2 || y < snake.yard_y + 1)
        return snake_set_error(err, "terminal should have at least %d width and %d height, current (%d, %d) plz resize it", snake.yard_x * 2 + 2, snake.yard_y + 1, x, y);

    /* Init yard buffer. */
    snake.yard_buffer = malloc(snake.yard_y * snake.yard_x);
    memset(snake.yard_buffer, 0, snake.yard_y * snake.yard_x);

    /* Init snake. */
    snake.snake_inst = snake_create();
    struct snake_node *head = snake_node_create(12, 13, 0x52);
    snake_add_node(snake.snake_inst, head);
    snake_add_node(snake.snake_inst, snake_node_create(12, 12, 0x12));
    snake_add_node(snake.snake_inst, snake_node_create(12, 11, 0x12));

    // Draw bg border.
    for (int i = 0; i < snake.yard_x + 1; i++) {
        draw_cell(stdscr, snake.yard_y, i, 0x17);
    }
    for (int i = 0; i < snake.yard_y + 1; i++) {
        draw_cell(stdscr, i, snake.yard_x, 0x17);
    }

    // Update
    snake_update();

    return 0;
}

static int
snake_move()
{
    struct snake_node *head = snake.snake_inst->head;
    struct snake_node *tail = snake.snake_inst->tail;
    while (tail != head) {
        tail->offset_y = tail->prev->offset_y;
        tail->offset_x = tail->prev->offset_x;
        tail = tail->prev;
    }
    logger(LOG_INFO, "length: %d", snake.snake_inst->length);
    assert(XSTATUS_OF(head->cell));
    if (XSTATUS_OF(head->cell) == 2) {
        head->offset_y--;
        if (head->offset_y < 0) {
            return 1;
        }
    } else if (XSTATUS_OF(head->cell) == 3) {
        head->offset_y++;
        if (head->offset_y > snake.yard_y) {
            return 1;
        }
    } else if (XSTATUS_OF(head->cell) == 4) {
        head->offset_x--;
        if (head->offset_x < 0) {
            return 1;
        }
    } else if (XSTATUS_OF(head->cell) == 5) {
        head->offset_x++;
        if (head->offset_x > snake.yard_x) {
            return 1;
        }
    }

    // check food
    if (XSTATUS_OF(snake.yard_buffer[head->offset_y * snake.yard_x + head->offset_x])) {
        snake_eat_cell(snake.snake_inst, snake.yard_buffer[head->offset_y * snake.yard_x + head->offset_x]);
        snake.yard_buffer[head->offset_y * snake.yard_x + head->offset_x] = 0;
    }

    snake_update();
    return 0;
}

static void
snake_up()
{
    if (XSTATUS_OF(snake.snake_inst->head->cell) == 3) {
        return;
    }
    snake.snake_inst->head->cell = 0x22;
}

static void
snake_down()
{
    if (XSTATUS_OF(snake.snake_inst->head->cell) == 2) {
        return;
    }
    snake.snake_inst->head->cell = 0x32;
}

static void
snake_left()
{
    if (XSTATUS_OF(snake.snake_inst->head->cell) == 5) {
        return;
    }
    snake.snake_inst->head->cell = 0x42;
}

static void
snake_right()
{
    if (XSTATUS_OF(snake.snake_inst->head->cell) == 4) {
        return;
    }
    snake.snake_inst->head->cell = 0x52;
}

static int
snake_rand()
{
    static int is_seeded = false;
    if (!is_seeded) {
        srand(((long)(time(0) * getpid())));
        is_seeded = true;
    }
    return rand();
}

static void
snake_alarm()
{
    // random food
    if (snake_rand() % 10 == 0) {
        snake.yard_buffer[snake_rand() % snake.yard_y * snake.yard_x + snake_rand() % snake.yard_x] = CELL(1, snake_rand() % 7);
    }
    if (snake_move()) {
        snake_end();
        die("You are dead.");
    }
}

int
snake_run(char *err)
{
    // use alarm signal as timer
    struct itimerval tout_val, ovalue;
    tout_val.it_interval.tv_usec = 400 * 1000;
    tout_val.it_interval.tv_sec = 0;
    tout_val.it_value.tv_usec = 400 * 1000;
    tout_val.it_value.tv_sec = 0;
    if (setitimer(ITIMER_REAL, &tout_val, &ovalue) < 0)
        return snake_set_error(err, "timer error");

    if (signal(SIGALRM, snake_alarm) == SIG_ERR)
        return snake_set_error(err, "signal error");

    while (true) {
        int c = getch();
        switch (c) {
        case 'h': // left
        case 260: // left arrow
            logger(LOG_INFO, "left");
            snake_left();
            break;
        case 'j': // down
        case 258: // down arrow
            logger(LOG_INFO, "down");
            snake_down();
            break;
        case 'k': // up
        case 259: // up arrow
            logger(LOG_INFO, "up");
            snake_up();
            break;
        case 'l': // right
        case 261: // right arrow
            logger(LOG_INFO, "right");
            snake_right();
            break;
        case 'q':
        case 27: // <ESC>
            logger(LOG_INFO, "returned");
            return 0;
            break;
        default:
            logger(LOG_WARNING, "unhandled keystroke code: %d", c);
            break;
        }
    }
}

int
snake_end(void)
{
    endwin();
    return 0;
}
