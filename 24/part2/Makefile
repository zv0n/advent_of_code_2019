CC ?= cc
CFLAGS ?= -std=c99 -O3
PROJECT = life

all: ${PROJECT}

${PROJECT}: main.c
	${CC} ${CFLAGS} -o $@ $^

test: ${PROJECT}
	./${PROJECT}

clean:
	${RM} *.o ${PROJECT}

.PHONY: all clean test
