CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -pedantic
PROJECT = jumping

all: ${PROJECT}

${PROJECT}: main.c
	${CC} ${CFLAGS} -o $@ $^

test: ${PROJECT}
	cat assembly_program | ./${PROJECT}

clean:
	${RM} *.o ${PROJECT} black

.PHONY: all clean test
