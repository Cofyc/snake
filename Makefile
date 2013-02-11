# The default target
all::

CC = gcc
RM = rm -f

CFLAGS = -std=c99 -g -O2 -Wall
LDFLAGS = -lncurses -lm

LIB_H += compat.h
LIB_H += snake.h
LIB_H += logger.h
LIB_H += usage.h
LIB_H += wrapper.h

LIB_OBJS += main.o
LIB_OBJS += snake.o
LIB_OBJS += logger.o
LIB_OBJS += usage.o
LIB_OBJS += wrapper.o
LIB_OBJS += argparse/argparse.o

$(LIB_OBJS): $(LIB_H)

argparse/argparse.o: argparse/Makefile
	cd argparse && make

snake: $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $^

.PHONY: all install clean tags

all:: snake

install: all
	install -m 755 snake /usr/local/bin/

clean:
	$(RM) snake
	# objesnake files
	$(RM) *.o
	# indent backup files
	$(RM) *~

tags:
	snakeags -R --c-kinds=+p --fields=+S .
