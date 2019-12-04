CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
PROJECT = crossing

all: ${PROJECT}

${PROJECT}: main.c
	${CC} ${CFLAGS} -o $@ $^

test: ${PROJECT}
	./${PROJECT} 153517 630395

clean:
	${RM} *.o ${PROJECT}

.PHONY: all clean test
