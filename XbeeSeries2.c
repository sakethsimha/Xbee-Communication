/*
 * XbeeS2.c
 *
 * Created: 2/20/2014 1:29:02 PM
 */ 
#include "XbeeS2.h"
#include "XbeeHAL.h"

RxPacket rx_pkt;

// Global Variables
char RSSI;
int MY;
int cb;	// callback flag -- set if a callback function for new packets has been specified
void (*cbFunPtr)(void);	// callback function pointer

/* Routine to call USART initialization routine from the HAL
 *
 * This routine configures the USART that the XBee is attached to.
 * Configues for settings 8N1 and 9600 baud.
 */
void XbeeUSART_init()
{
	USART_INIT();
}

/* Routine to send a byte through USART
 *
 * This routine polls the USART data buffer register full flag until it indicates it is ready
 * for a new byte to be transmitted, then loads the new byte into the USART data buffer
 * register for transmission.
 */
void USART_vSendByte(char Data)
{
	// Wait if a byte is being transmitted
	while (TX_BUFFER_IS_FULL()) {}; // Do nothing until UDR is ready for more data to be written to it
	// Transmit data
	XBEE_UDR = Data;
}

/* USART Receive Interrupt
 * 
 * Hardware interrupt routine inside the HAL calls this function. When a new byte is received, it is
 * checked for the start delimeter of a new packet. If it is, then the receive message function is called.
 * Otherwise, nothing happens. After a packet is received, the callback function is called to handle the
 * packet, if one has been configured.
 */
void rxISR()
{
	cli();
	TOGGLE_LED();
	if(XBEE_UDR == 0x7E)
	{
		RxPacket pkt;
		receive_Msg(&pkt);
		rx_pkt = pkt;
		// Call function for responding to new packet
		if(cb)
		(*cbFunPtr)();
	}
	
	TOGGLE_LED();
	sei();
}
/* Function to return the 16 bit Address of the Xbee
 *
 * This function calls the MY AT command and waits until the response frame is 
 * received.
 */
int get16bitAddress()
{
	RxPacket pkt;	
	int temp;
	
	do 
	{
		AT_Command(0x01, 'M', 'Y', 0, 0);	// Send DB AT Command
		while (RX_BUFFER_IS_FULL()){};	
		temp = XBEE_UDR;
		receive_Msg(&pkt);
	} while (pkt.api_identifier != 0x88);
				
	#ifdef DEBUG			
		printf("MY is %d\n", (pkt.data[0]<<8)|pkt.data[1]);
	#endif
	
	return (pkt.data[0]<<8)|pkt.data[1];	// Return MY
}

/* Send Message Function
 *
 * This function is passed a character array of an XBee API Packet, along with the
 * length of the packet. The checksum is calculated, then the start delimeter is transmitted,
 * followed by the length of the packet, then the data, and finally the checksum.
 */
void send_Msg(char *data, int len)
{
	//Generate checksum
	char checksum;
	int counter = 0, sum = 0;
	for(counter = 0; counter <= len - 1; counter++)
		sum += data[counter];
	//Checksum is calculated by adding the data values together, and subtracting the 
	//last 8 bits from 0xFF.
	checksum = 0xFF - (sum & 0x00FF); 
	//Transmit data
	USART_vSendByte(0x7E);  //Start delimiter
	USART_vSendByte(8 >> len);  //Length MSB
	USART_vSendByte(len);	//Length LSB
	for(counter = 0; counter <= len - 1; counter++)  //Transmit data
		USART_vSendByte(data[counter]);
	USART_vSendByte(checksum);  //Transmit checksum
}

void receive_Msg(RxPacket *rx_data)
{
	int count;
	char temp;
	rx_data->data_len = 0;
	
	while (RX_BUFFER_IS_FULL()) {}; // Do nothing until data have been received and is ready to be read from UDR
	temp = XBEE_UDR;	//next incoming byte is the MSB of the data size
	while (RX_BUFFER_IS_FULL()) {}; // Do nothing until data have been received and is ready to be read from UDR

	rx_data->len = (temp << 8) | XBEE_UDR;	//merge LSB and MSB to obtain data length
	while (RX_BUFFER_IS_FULL()) {}; // Do nothing until data have been received and is ready to be read from UDR
	rx_data->api_identifier = XBEE_UDR;
	
	switch(rx_data->api_identifier)	// Select proper sequence for receiving various packet types
	{
		case 0x90:	// Zigbee Receive Packet
			for(count = 1; count < rx_data->len; count++)
			{
				while (RX_BUFFER_IS_FULL()) {}; // Do nothing until data have been received and is ready to be read from UDR
				if(count == 1)
					rx_data->DH = ((long)XBEE_UDR << 24);
				else if(count == 2)
					rx_data->DH |= ((long)XBEE_UDR << 16);
				else if(count == 3)
					rx_data->DH |= ((long)XBEE_UDR << 8);
				else if(count == 4)
					rx_data->DH |= (long)XBEE_UDR;
				else if(count == 5)
					rx_data->DL = ((long)XBEE_UDR << 24);
				else if(count == 6)
					rx_data->DL |= ((long)XBEE_UDR << 16);
				else if(count == 7)
					rx_data->DL |= ((long)XBEE_UDR << 8);
				else if(count == 8)
					rx_data->DL |= (long)XBEE_UDR;
				else if(count == 9)
					rx_data->source_addr_16bit = (XBEE_UDR << 8);
				else if(count == 10)
					rx_data->source_addr_16bit = XBEE_UDR;
				else if(count == 11)
					rx_data->options = XBEE_UDR;
				else
				{
					rx_data->data[count - 12] = XBEE_UDR;
					rx_data->data_len++;
				}				
			}
			while (RX_BUFFER_IS_FULL()) {}; // Do nothing until data have been received and is ready to be read from UDR
			rx_data->checksum = XBEE_UDR;	//store checksum
			break;
		case 0x88:	// AT Command
			for(count = 1; count < rx_data->len; count++)
			{
				while (RX_BUFFER_IS_FULL()) {}; // Do nothing until data have been received and is ready to be read from UDR
				if(count == 1)
					rx_data->frame_id = XBEE_UDR;
				else if(count == 2)
					rx_data->AT_com[0] = XBEE_UDR;
				else if(count == 3)
					rx_data->AT_com[1] = XBEE_UDR;
				else if(count == 4)
					rx_data->status = XBEE_UDR;
				else
				{
					rx_data->data[count - 5] = XBEE_UDR;
					rx_data->data_len++;
				}
			}
			while (RX_BUFFER_IS_FULL()) {}; // Do nothing until data have been received and is ready to be read from UDR
			rx_data->checksum = XBEE_UDR;	//store checksum			
			break;
		default:
			break;
	}
	
}

void ZigBee_TX_Request(char Frame_ID, long DH, long DL, int _16bitAddr, char Hops, char Options, char *RF_Data, int len )
{
	int i; // counting variable
	char buff[14 + len];	//temporary buffer for transmitting
	// ZigBee Transmit Request API Identifier
	buff[0] = 0x10;
	// Identifies the UART data frame for the host to correlate with a 
	// subsequent ACK (acknowledgment). Setting Frame ID to ‘0' will disable response frame.
	buff[1] = Frame_ID;	
	// MSB first, LSB last. Broadcast = 0x000000000000FFFF
	buff[2] = (DH >> 24);
	buff[3] = (DH >> 16);
	buff[4] = (DH >> 8);
	buff[5] = DH;
	buff[6] = (DL >> 24);
	buff[7] = (DL >> 16);
	buff[8] = (DL >> 8);
	buff[9] = DL;
	// 16 bit address
	buff[10] = (_16bitAddr >> 8);
	buff[11] = _16bitAddr;
	// Number of hops for message to take
	buff[12] = Hops;
	// Options
	buff[13] = Options;
	for(i = 0; i < len; i++)
		buff[14+i] = RF_Data[i];
	send_Msg(buff, 14+len);	
}

void AT_Command(char frameid, char command1, char command2, char *options, int len)
{
	char buff[5]; //temporary buffer for transmitting
	int count;
	buff[0] = 0x08;	// API ID for AT Commands
	buff[1] = frameid;	// Frame ID; set to 0 for no response
	buff[2] = command1;
	buff[3] = command2;
	for(count = 1; count <= len; count++)
		buff[3 + count] = options[count - 1];
	send_Msg(buff, 4 + len);
}

/* Function to get RSSI from the PWM pin on the XBee
 *
 * This function returns the high time in microseconds from the PWM signal with corresponds to
 * the RSSI of the last received message. This is accomplished by starting and stopping an 8-bit
 * timer.
 */
float getRSSIPWM(void)
{
	//char RSSI;
	float time_us = 0.0;
	
	Init_Timer0();
	while(bit_is_set(PIND, PORTD3) && (TCNT0 != 50));
	killTimer();
	while(bit_is_clear(PIND, PORTD3));
	Init_Timer0();
	while(bit_is_set(PIND, PORTD3) && (TCNT0 != 50));
	#ifdef DEBUG
		printf("TCNT0 is %d\n", TCNT0);
	#endif
	killTimer();
	
	return time_us/200.0;
}

/* This function is under construction, and is not guaranteed to work for all uC's */
void Init_Timer0(void)
{
	TCNT0 = 0;	// clear count value
	TCCR0B = 0x03;	// Select clk/64
}

/* This function is under construction, and is not guaranteed to work for all uC's */
void killTimer(void)
{
	TCCR0B = 0x00; // disable clk
	TCNT0 = 0;	// clear count value
}

/* This function is under construction, and is not guaranteed to work for all uC's */
float getTime_us(void)
{
	return (float)TCNT0*(1.0/((float)F_CPU/64.0));
	
}

/* Function to get the RSSI value of the last received message in dB
 *
 * This function calls the DB AT command. It then waits for the returning packet.
 * if the packet is not the returned DB value, then it is discarded.
 * The returned value is the attenuation of the signal in -dB.
 */
char getRSSI(void)
{
	RxPacket pkt;
	int temp;

	AT_Command(0x01, 'D', 'B', 0, 0);	// Send DB AT Command
	while (RX_BUFFER_IS_FULL()){};	
	temp = XBEE_UDR;	// Probably should add some code to check for start delimeter
	receive_Msg(&pkt);

	if(pkt.api_identifier != 0x88)	
	{
		#ifdef DEBUG
			printf("Failed to retrieve RSSI\n");
		#endif
		// otherwise, handle the packet as usual
		if(cb)
			(*cbFunPtr)();
		return 0xFF;
	}	
	else
	{
		#ifdef DEBUG
			printf("RSSI is %d\n", pkt.data[0]);
		#endif
		return pkt.data[0];	// Return RSSI value		
	}	

}

/* Callback function for new packets
 *
 * The user passes the name of their designated callback function. The function's address is
 * then set to the callback fuction pointer, cbFunPtr. The callback flag, cb, is set.
 */
void setNewPacketCB(void (*funptr)())
{
	cb = 1;
	cbFunPtr = funptr;
}

/* Get Packet
 *
 * Returns the last received packet.
 */
RxPacket getPacket()
{
	return rx_pkt;
}
