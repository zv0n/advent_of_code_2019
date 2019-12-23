CC ?= cc
CFLAGS ?= -std=c99 -O3
PROJECT = shuffle

all: ${PROJECT}

${PROJECT}: main.c deck.c
	${CC} ${CFLAGS} -o $@ $^

test: ${PROJECT}
	./${PROJECT}

clean:
	${RM} *.o ${PROJECT}

.PHONY: all clean test
