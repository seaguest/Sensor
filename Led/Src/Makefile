CPU = msp430x2274
CC = msp430-gcc
CFLAGS = -DMAX_HOPS=3 -mmcu=${CPU} -O2 -Wall -g

all:	led

clean:
	rm -f *.o
led:
	$(CC) ${CFLAGS} -o led *.c
