CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic -O3
PROJECT = shuffle

all: ${PROJECT}

${PROJECT}: main.c deck.c
	${CC} ${CFLAGS} -o $@ $^

test: ${PROJECT}
	./${PROJECT}

clean:
	${RM} *.o ${PROJECT}

.PHONY: all clean test
