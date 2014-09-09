/*
 * XbeeS2.h
 *
 * Created: 6/20/2013 1:29:15 PM
 */ 


#ifndef XBEE_H_
#define XBEE_H_

#include <stdio.h>
#include <stdlib.h>

#define RSSI_PIN PIND
#define RSSI_DDR DDRD
#define RSSI_PIN_NO PIND3

typedef struct{
	int len;
	char api_identifier;
	long DH;
	long DL;
	int source_addr_16bit;
	char options;
	char frame_id;
	char AT_com[2];
	char status;
	char data[20];	// also stores the "value" for at command response
	int data_len;	// stores the length of the data
	char checksum;
	} RxPacket;

void rxISR();
void receive_Msg(RxPacket*);
void send_Msg(char*, int);
void USART_vSendByte(char);
void ZigBee_TX_Request(char, long, long, int, char, char, char*, int);
void AT_Command(char, char, char, char*, int);
void Init_Timer0(void);
float getRSSIPWM(void);
char getRSSI(void);
int get16bitAddress(void);
void killTimer(void);
float getTime_us(void);
void setNewPacketCB(void (*funptr)(void));
RxPacket getPacket(void);
void XbeeUSART_init(void);

#endif /* XBEE_H_ */
