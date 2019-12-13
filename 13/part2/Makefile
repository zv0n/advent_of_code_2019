CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
PROJECT = arcade

all: ${PROJECT}

${PROJECT}: main.c
	${CC} ${CFLAGS} -o $@ $^

test: ${PROJECT}
	./${PROJECT}

clean:
	${RM} *.o ${PROJECT} black

.PHONY: all clean test
