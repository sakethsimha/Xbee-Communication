/*
 * Xbee_S2.c
 *
 */ 

#include "Motesquito.h"
#include <util/delay.h>

char test[] = {'T', 'E', 'S', 'T'};

void gas()
{
	//ZigBee_TX_Request(0x00, 0, 0x00FFFF, 0, 0x01, 0x01, test, 4);
	
	RxPacket rx_data = getPacket();
	// Echo packet back
	int i;
	char buff[rx_data.data_len];
	char temp;
	for(i=0;i<rx_data.data_len;i++)
	{
		temp = (rx_data.data[i]);
		buff[i] = temp;
	}
	ZigBee_TX_Request(0,0,0xFFFF,rx_data.source_addr_16bit, 1, 0, buff, rx_data.data_len);
	
}

int main(void)
{
	motesquitoInit();
	setNewPacketCB(gas);
	
	printf("Hello World\n");
	
	sei();
	
    while(1)
    { 
		_delay_ms(4000);
		ZigBee_TX_Request(0x00, 0, 0x00FFFF, 0, 0x01, 0x01, test, 4);
    }
}
