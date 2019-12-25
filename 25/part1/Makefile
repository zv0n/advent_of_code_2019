CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
PROJECT = zork

all: ${PROJECT}

${PROJECT}: main.c
	${CC} ${CFLAGS} -o $@ $^

test: ${PROJECT}
	cat speedrun | ./${PROJECT}

clean:
	${RM} *.o ${PROJECT}

.PHONY: all clean test
