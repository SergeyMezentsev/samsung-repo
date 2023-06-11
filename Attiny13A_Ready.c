#define F_CPU 1200000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// define TX and button pins
#define RX_PIN	PB1
#define TX_PIN  PB2
#define LED_ABSENCE	PB3	

// set address of the module
#define ADDR 0x01

//settings for the radioprotocol
#define RF_ERR 500              //The shortest duration of the pulse
#define RF_START_TEACHER  5     // Number of start pulses in the teacher's message
#define RF_START_CHILD  3       //Number of start pulses in preambule of every children's message


// macros ASK/OOK
#define RF_on()   PORTB |=  (1<<TX_PIN)   // key-on
#define RF_off()  PORTB &= ~(1<<TX_PIN)   // key-off

// macros to modulate the signals with compensated timings
#define bit0Pulse()   {RF_on(); _delay_us(RF_ERR); RF_off(); _delay_us(RF_ERR);}
#define bit1Pulse()   {RF_on(); _delay_us(2*RF_ERR); RF_off(); _delay_us(2*RF_ERR);}
#define startPulse()  {RF_on(); _delay_us(3*RF_ERR); RF_off(); _delay_us(3*RF_ERR);}

// send a single byte via RF
void sendByte(uint8_t value) {
	for (uint8_t i=8; i; i--, value<<=1) {            // send 8 bits, MSB first
		(value & 0x80) ? (bit1Pulse()) : (bit0Pulse()); // send the bit
	}
}

// send complete telegram (3*startPulse + CMD)
void sendCode(uint8_t cmd) {
	uint8_t i;                         // counting variables
	for(i = RF_START_CHILD; i; i--) startPulse();
	sendByte(cmd);
}



//Functions that allow to check presence of the teacher's message
volatile uint8_t externalMessageFlag = 0;
ISR(INT0_vect)
{
	externalMessageFlag = 1;
}


uint8_t num = 0;
void checkStart()
{
	if (externalMessageFlag)
	{
		uint8_t counter = 0;
		while((PINB & (1<<RX_PIN)) != 0)
		{
			_delay_us(100);
			counter++;
		}
		
		if (counter > 12 && counter < 18) num++;
		externalMessageFlag = 0;
	}
}


//Checking for timeout
volatile uint8_t timerAbsence = 0;
volatile uint8_t absenceFlag = 0;

ISR(TIM0_OVF_vect)
{
	timerAbsence++;
	if(timerAbsence >= 25 && (!absenceFlag))
	{
		absenceFlag = 1;
	}
	
	if(absenceFlag)
	{
		PORTB ^= (1<<LED_ABSENCE);
	}
	
}




int main(void)
{
	// setup pins
	DDRB |= (1<<LED_ABSENCE);				// LED_PRESENCE pin as output
	DDRB |= (1<<TX_PIN);				    // TX_PIN pin as output
	DDRB &= ~(1<<RX_PIN);					// RX_PIN pin as input
	
	
	//setup interrupts
	GIMSK |= (1<<INT0);					   //
	MCUCR |= (1<<ISC01) | (1<<ISC00);	   //
	sei();
	
	
	//setup Timer
	TCCR0B |= (1<<CS00) | (1<<CS02);		//Divider of frequency by 1024
	TIMSK0 |= (1<<TOIE0);					//Enable overflow-interrupts
	
	
	while(1)
	{
		
		checkStart();
		
		
		if(num == 5) 
		{
			cli();
			
			//Send the Address
			_delay_ms(20);
			_delay_ms(50*(ADDR - 1));
			sendCode(ADDR);
			_delay_ms(25+50*(32-ADDR));

			 num = 0;
			 sei();
			 
			 //Reset Timeout counter and turn off the LED_ABSENCE
			 PORTB &= ~(1<<LED_ABSENCE);
			 timerAbsence = 0;
			 absenceFlag = 0;
		}
		
	}
}
