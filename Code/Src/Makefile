SIMPLICITI_COMPONENTS_PATH = /home/seaguest/timsp430gcc
BSP_PATH = ${SIMPLICITI_COMPONENTS_PATH}/bsp
MRFI_PATH = ${SIMPLICITI_COMPONENTS_PATH}/mrfi
EXTRA_PATH = /home/seaguest/Sensor/Create_Network/Network

BOARD = EZ430RF
CPU = msp430x2274

RF = -DMRFI_CC2500

INCLUDES = -I${MRFI_PATH}/ \
-I${BSP_PATH}/ \
-I${BSP_PATH}/drivers/ \
-I${BSP_PATH}/boards/${BOARD}/\
-I${EXTRA_PATH}/

OBJECTS  = bsp.o mrfi.o interrupt.o uart.o cycle.o synchrone.o fifo.o route.o main.o

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

cycle.o:cycle.c 
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

synchrone.o:synchrone.c 
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

fifo.o:fifo.c 
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

route.o:route.c
	${CC} ${CFLAGS} -c $< -o $@
	@echo CC $<

network:main.c bsp.o mrfi.o interrupt.o uart.o cycle.o synchrone.o fifo.o route.o
	$(CC) ${CFLAGS} bsp.o mrfi.o interrupt.o uart.o cycle.o synchrone.o fifo.o route.o -o $@ $<

