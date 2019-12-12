CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
PROJECT = robot

all: ${PROJECT}

${PROJECT}: main.c bitmap.c
	${CC} ${CFLAGS} -o $@ $^

black: main.c bitmap.c
	${CC} ${CFLAGS} -o $@ $^ -DBLACK

test: ${PROJECT}
	./${PROJECT}

clean:
	${RM} *.o ${PROJECT} black

.PHONY: all clean test
