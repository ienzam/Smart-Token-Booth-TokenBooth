#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "lcd.h"
#include <avr/interrupt.h>


//////////////////TOKEN//////BOOTH/////////////////////////////

char hasUser;
char my_category;
char selfID[10];

//01 23 4 56789 012345 6


void printLCD(char f[],char s[])
{
    LCDClear();
    LCDWriteString(f);
    if (strlen(s))
    {
        LCDWriteStringXY(0,1,s);
    }
}

void printLCDFree() { printLCD("Token Booth",selfID); }

void genPacket(char *c, char commandType) {
    int i;
    c[0] = STB_ALIAS_SERVER;
    c[1] = STB_SERVER_ID;
    c[2] = STB_ALIAS_TOKENBOOTH;
    c[3] = SELF_ID;
    c[4] = commandType;
    for(i=5; i < 17; i++) c[i] = '-';
    c[17] = 0;
}

////////////////////process//////////////////////
void processData(char data[])
{
    //NOISE//
    if (data[0]==STB_ALIAS_TOKENBOOTH && data[1]==SELF_ID)
    {
        //printf("YES i am that guy\n\r");
        //printData(data);
    }
    else return ;
    char s[20];
    switch(data[4])
    {
    case STB_SERVER_PING_TOKEN:
        genPacket(s, hasUser);
        if (hasUser != STB_TOKEN_NO_USER) s[5] = my_category;
        else printLCDFree();
        printf("%s",s);
        break;
    case STB_SERVER_DONE_USER:
        genPacket(s, STB_TOKEN_DONE_USER);
        strncpy(s+5, data+5, 5);
        printf("%s",s);
        data[10]=0;
        printLCD(data+5, "Printing...");
        hasUser=STB_TOKEN_NO_USER;
        my_category = 'C';
        break;
    }
}

//========================================================
int SendCmd(char cmd,FILE *stream)
{
    UCSRB |= (1<<TXEN);
    while ((UCSRA & (1 << UDRE)) == 0x00) ;
    UCSRB &= ~(1<<TXB8);
        if ( cmd & 0x0100 ) UCSRB |= (1<<TXB8);
        UDR = cmd;
    UCSRB &= ~(1<<TXEN);
    return 0;
}

int RecieveCmd(FILE *stream)
{
    unsigned char ReceivedByte;
    while ((UCSRA & (1 << RXC)) == 0x00) {} // Do nothing until data have been recieved and is ready to be read from UDR
    ReceivedByte = UDR; // Fetch the recieved byte value into the variable "ByteReceived"
    return ReceivedByte;
}
//*******************************************/


ISR(USART_RXC_vect){ // Handler for RXD interrupt
    uchar in = UDR;
    char m[20];
    scanf("%s",m+1);
    m[0]=in;
    if (strlen(m)<17) return ;
    processData(m);
}

/////////////////BUTTON/////INT////////////////////

char category[]={'A',STB_CATEGORY_WITHDRAW,STB_CATEGORY_DEPOSIT,'B',STB_CATEGORY_HELP};

ISR(INT0_vect)
{
    if(hasUser == STB_TOKEN_HAS_USER) return;
    int cat=PINC&0b00000111;
    my_category=category[cat];
    hasUser=STB_TOKEN_HAS_USER;
    printLCD("Please Wait...","......");
}
///////////////////////////////////////////////////


int main(){

    DDRB = 0xFF;
    DDRC = 0xFF;
    DDRD &= ~1;
    DDRD |= 2;

    LCDInit(LS_NONE);
    LCDClear();
    GICR 	|= 	(1<<INT0);
    MCUCR 	|= 	(1<<ISC00) | (1<<ISC01);

    //SERIAL PORT
    UCSRB |= (1 << RXEN) | (1 << TXEN) | (1<<RXCIE);   // Turn on the transmission and reception circuitry
    UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1)  ; 		 // Use 8-bit character sizes
    UBRRH = (BAUD_PRESCALE >> 8); 	// Load upper 8-bits of the baud rate value into the high byte of the UBRR register
    UBRRL = BAUD_PRESCALE; 			// Load lower 8-bits of the baud rate value into the low byte of the UBRR register

    stdout = fdevopen(SendCmd,NULL);
    stdin =  fdevopen(NULL,RecieveCmd);

    //printf("Welcome!\n\rThis is develop by  Nayeem, Enzam, Shafiul ,Hira& Dipal.\n\rMachineStarted\n\r 0-INIT\n\r 1-SETvalue\n\r 2-STAT\n\r");
    //printf("Welcome! This transmitter will transmit nothing\n\rZZZZZ\n\r");

    LCDClear();
    sei();

    UCSRB &= ~(1<<TXEN);


    SELF_ID	= ((PINC&0b00111000)>>3) + '0';
    hasUser=STB_TOKEN_NO_USER;

    sprintf(selfID, "T000%c", SELF_ID);
    printLCDFree();

    while(1){}

    return 0;
}

