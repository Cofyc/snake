/**
 * The Snake.
 */

#include "snake.h"
#include "logger.h"
#include "usage.h"
#include "argparse/argparse.h"

struct snake snake = {
    .yard_y = 25,
    .yard_x = 25,
    .yard_buffer = NULL,
    .score = 0,
    .speed = 10,
    .running = true,
    .snake_inst = NULL,
    .stdscr = NULL,
};

static const char *const config_usage[] = {
    "the snake [options]",
    NULL
};

static int
show_version_cb(struct argparse *this, const struct argparse_option *option)
{
    printf("snake version %s\n", "0.1");
    exit(0);
}

int
main(int argc, const char **argv)
{
    logger_logfile = "/tmp/snake.log";
    struct argparse argparse;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN('v', "version", NULL, "show version number",
                show_version_cb),
        OPT_STRING('l', "logfile", &logger_logfile, "specify log file (defaults: /tmp/snake.log)"),
        OPT_END(),
    };

    argparse_init(&argparse, options, config_usage, 0);
    argc = argparse_parse(&argparse, argc, argv);

    char errstr[SNAKE_ERR_LEN];
    int err = snake_init(errstr);
    if (err == SNAKE_ERR) {
        snake_end();
        die(errstr);
    }
    err = snake_run(errstr);
    if (err == SNAKE_ERR) {
        snake_end();
        die(errstr);
    }

    snake_end();

    return 0;
}
