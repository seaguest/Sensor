#ifndef __COMMON_DEFINES_H__
#define __COMMON_DEFINES_H__
// Definition des adresses
#include <mrfi.h> 

#define VB_1 0x1
#define VB_2 0x2
#define VB_3 0x3
#define VB_4 0x4
#define ET_1 0x5
#define ET_2 0x6
#define ET_3 0x7
#define ET_4 0x8
#define KH_1 0x9
#define KH_2 0xA
#define KH_3 0xB
#define KH_4 0xC

#define MAC KH_1	//choose the MAC
#define BROADCAST 255;		//define the adresse of broadcast
#define IS_CREATER 1;		//define if it is the creater of network
#define IS_NOT_CREATER 0;	//define if it is not the creater of network

#define WAIT_BEACON	0
#define WAIT_MESSAGE	11
#define WAIT_SLEEP	22
#define BEACON_SIZE     11			 //define Beacon size
#define PAYLOAD_SIZE    19 			//define Beacon size

#define NO_NETWORK		0
#define ID_NETWORK_CREATE	10

// define the message which will be send
//attention the length <= PAYLOAD_SIZE-1 = 18
volatile uint8_t  state ;			//indiquer l'etat ; initialisation 0
volatile uint8_t  ID_Network ,ID_Beacon;	//indiquer ID de reseaux,  initialisation 0 (0 means no network)


// Definition des flags
#define FDATA 0x0
#define FBEACON 0x1

// en ms
#define DUREE_SLOT 12*2
// 32 slots ?
#define N_SLOT 32
// en ms
#define DUREE_CYCLE 12*500
// en ms
#define DUREE_SCAN 12*1000
// en ms
#define DUREE_ACTIVE 12*200
// en ms
#define DUREE_SLEEP (DUREE_CYCLE -DUREE_SLOT*N_SLOT - DUREE_ACTIVE)


typedef struct
{
	uint8_t length;
	uint8_t src[4];
	uint8_t dst[4];
	uint8_t flag;
	uint8_t data[MRFI_MAX_FRAME_SIZE-10];
} mPacket;

typedef struct
{
	uint8_t network_num;
	uint8_t slot_num;
	uint8_t slot_total;
} mBeacon;

#endif

