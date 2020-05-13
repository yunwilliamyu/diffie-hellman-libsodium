IDIR=.
CC=gcc
CFLAGS=-I${IDIR} -lsodium

OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

all: ${OBJS} main
	echo "All made."

main:
	${CC} -o $@ $@.o ${CFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	rm -f ${OBJS} main
	@echo "All cleaned up!"
