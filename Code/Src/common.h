#ifndef __COMMON_DEFINES_H__
#define __COMMON_DEFINES_H__
// Definition des adresses
#include <mrfi.h> 
#include "fifo.h"

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

#define BROADCAST  0xFF		//define the adresse of broadcast
#define IS_CREATER 0x1F		//define if it is the creater of network
#define IS_NOT_CREATER 0x0	//define if it is not the creater of network

#define WAIT_SCAN	0x0
#define WAIT_BEACON	0x1F
#define WAIT_SYNCHRONE	0x2F
#define WAIT_MESSAGE	0x3F
#define WAIT_SLEEP	0x4F

#define BEACON_SIZE     0x10			 //define Beacon size	include the length(1B)
#define PAYLOAD_MAX_SIZE    30			 

#define NO_NETWORK		0x0


// Definition des flags
#define FBEACON 0x0F
#define FDATA 	0x1F
#define FRIP 	0x2F

// en ms avec SMCLK = 8MHZ
#define N_1MS 8000

// en ms
#define DUREE_SLOT 2
// 32 slots ?
#define N_SLOT 32
// en ms
#define DUREE_CYCLE 	500
// en ms
#define DUREE_SCAN (2*DUREE_CYCLE)	//1000
// en ms
#define DUREE_ACTIVE 350
// en ms
#define DUREE_SLEEP (DUREE_CYCLE -DUREE_SLOT*(N_SLOT+1) - DUREE_ACTIVE)
// en ms
#define DUREE_SURVEILLE 5000

typedef struct
{
	uint8_t ID_Network;
	uint8_t ID_Slot;
	uint32_t Voisin;
} mBeacon;

typedef struct {
	uint8_t Next_hop;
	uint8_t data[MRFI_MAX_FRAME_SIZE-11];
} mData;

typedef struct {
	uint8_t Dst;
	uint8_t Next_hop;
	uint8_t Metric;
} mRip;


typedef struct
{
	uint8_t length;
	uint8_t src;
	uint8_t dst;
	uint8_t flag;
	union{
		mBeacon beacon;
		mRip route[N_SLOT];
		mData data;
	} payload;
} mPacket;


typedef struct
{
	uint8_t state;				 
	uint8_t ID_Network;			 
	uint8_t MAC;				 
	uint8_t synchrone;	
	uint8_t synchrone_old;			 		 
	uint8_t HOST;
	uint8_t ID_Beacon;			 
	uint8_t Dst;
	uint16_t Counter; 			 	
	uint32_t Voisin;	
	//to check if the voisin is down or gone
	uint16_t check[N_SLOT-2];
	uint16_t check_old[N_SLOT-2];
	//to check if the network is down or not
	uint16_t Surveille_Cnt; 			 
	uint16_t Surveille_Cnt_Old; 
	mRip Route_table[N_SLOT];	 
	uint8_t AddrFiltered[N_SLOT];	 
	QList FIFO_Send;
	QList FIFO_Recieve;	
} Status;

#endif

