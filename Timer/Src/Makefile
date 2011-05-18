CPU = msp430x2274
CC = msp430-gcc
CFLAGS = -DMAX_HOPS=3 -mmcu=${CPU} -O2 -Wall -g

all:
	make clean
	$(CC) ${CFLAGS} -o timer *.c
clean:
	rm -f timer
	
