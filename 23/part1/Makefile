CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
PROJECT = router

all: ${PROJECT}

${PROJECT}: main.c
	${CC} ${CFLAGS} -o $@ $^

test: ${PROJECT}
	./${PROJECT}

clean:
	${RM} *.o ${PROJECT}

.PHONY: all clean test
