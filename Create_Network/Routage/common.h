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

#define BROADCAST  0xFF		//define the adresse of broadcast
#define IS_CREATER 0x1F		//define if it is the creater of network
#define IS_NOT_CREATER 0x0	//define if it is not the creater of network

#define WAIT_SCAN	0x0
#define WAIT_BEACON	0x1F
#define WAIT_SYNCHRONE	0x2F
#define WAIT_MESSAGE	0x3F
#define WAIT_SLEEP	0x4F

#define BEACON_SIZE     0x10			 //define Beacon size	include the length(1B)
#define PAYLOAD_MAX_SIZE    20			 

#define NO_NETWORK		0x0


// Definition des flags
#define FDATA 	0x0F
#define FBEACON 0x1F
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
#define DUREE_ACTIVE 200
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
	//uint16_t SEQ;
	//uint16_t ACK; 
	uint8_t Next_hop[4];
	uint8_t data[MRFI_MAX_FRAME_SIZE-14];
} mData;

typedef struct {
	uint8_t Dst[4];
	uint8_t Next_hop[4];
	uint8_t Metric;
} mRip;


typedef struct
{
	uint8_t length;
	uint8_t src[4];
	uint8_t dst[4];
	uint8_t flag;
	union{
		mBeacon beacon;
		mRip route[N_SLOT];
		mData data;
	} payload;
} mPacket;


typedef struct
{
	uint8_t state;				//first time ;initialisation
	uint8_t ID_Network;			//no network at first
	uint8_t MAC;				//MAC	
	uint8_t synchrone;			//if it had recieved beacon
	uint8_t HOST;
	uint8_t ID_Beacon;			//from who we get the beacon
	uint8_t Dst;
	uint16_t Counter; 			 
	uint16_t Surveille_Cnt; 			 
	uint16_t Surveille_Cnt_Old; 	
	uint32_t Voisin;	
	mRip Route_table[N_SLOT];	 
} Status;



#endif

