CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic -O2
PROJECT = picture

all: ${PROJECT}

${PROJECT}: main.c bitmap.c
	${CC} ${CFLAGS} -o $@ $^

test: ${PROJECT}
	./${PROJECT}

clean:
	${RM} *.o ${PROJECT}

.PHONY: all clean test
