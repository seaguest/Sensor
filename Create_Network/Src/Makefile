SIMPLICITI_COMPONENTS_PATH = /home/seaguest/timsp430gcc
BSP_PATH = ${SIMPLICITI_COMPONENTS_PATH}/bsp
MRFI_PATH = ${SIMPLICITI_COMPONENTS_PATH}/mrfi

BOARD = EZ430RF
CPU = msp430x2274

RF = -DMRFI_CC2500

INCLUDES = -I${MRFI_PATH}/ \
-I${BSP_PATH}/ \
-I${BSP_PATH}/drivers/ \
-I${BSP_PATH}/boards/${BOARD}/

OBJECTS  = bsp.o mrfi.o interrupt.o uart.o

BINARIES = network

CC = msp430-gcc

CFLAGS = -DMAX_HOPS=3 ${RF} -mmcu=${CPU} -O2 -Wall -g ${INCLUDES}

all: ${BINARIES}

clean:
	rm -f ${OBJECTS} ${BINARIES}

%.o: %.c
	$(CC) ${CFLAGS} -c -o $@ $<

bsp.o: ${BSP_PATH}/bsp.c
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

mrfi.o: ${MRFI_PATH}/mrfi.c
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

interrupt.o:interrupt.c 
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

uart.o:uart.c 
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<


network: create_network.c bsp.o mrfi.o interrupt.o uart.o
	$(CC) ${CFLAGS} bsp.o mrfi.o interrupt.o uart.o -o $@ $<
