 /*
 * MTE241 Lab 2 - Hardware Interfacing
 *  1. uWaterloo User ID: chaight@uwaterloo.ca
 *  2. uWaterloo User ID: s265sun@uwaterloo.ca
 */

#include <LPC17xx.h>
#include "stdio.h"
#include "uart.h"

// Section selection
#define section 4

// Turn specific LED on or off
void set_LED (int LED_num, int stat) {
	// LED_num is the LED being modified
	// val=0 is off, val=1 is on, val=2 is all LEDs off
	int port = 0;
	switch(LED_num) {
		case 1:
			LED_num = 28;
			port = 1;
			break;
		case 2:
			LED_num = 29;
			port = 1;
			break;
		case 3:
			LED_num = 31;
			port = 1;
			break;
		case 4:
			LED_num = 2;
			port = 2;
			break;
		case 5:
			LED_num = 3;
			port = 2;
			break;
		case 6:
			LED_num = 4;
			port = 2;
			break;
		case 7:
			LED_num = 5;
			port = 2;
			break;
		case 8:
			LED_num = 6;
			port = 2;
			break; 
	}
	
	// Set LED pins to outputs
	LPC_GPIO1->FIODIR |= ((1u << 28) | (1u << 29) | (1u << 31));
	LPC_GPIO2->FIODIR |= ((1u << 2) | (1u << 3) | (1u << 4) | (1u << 5) | (1u << 6));

	if(stat==2 )	{ // Turn all LEDs off
		LPC_GPIO1->FIOCLR |= ((1u << 28) | (1u << 29) | (1u << 31));
		LPC_GPIO2->FIOCLR |= ((1u << 2) | (1u << 3) | (1u << 4) | (1u << 5) | (1u << 6));
		return;
	}
	else if (stat==1 && port==1) {	// Turn specific LED on
		LPC_GPIO1->FIOCLR |= (1u << LED_num);
		LPC_GPIO1->FIOSET |= (1u << LED_num);
	}
	else if(stat==1 && port==2) {	// Turn specific LED on
		LPC_GPIO2->FIOCLR |= (1u << LED_num);
		LPC_GPIO2->FIOSET |= (1u << LED_num);
	}
	else if(stat==0 && port==1)	// Turn specific LED off
		LPC_GPIO1->FIOCLR = (1u << LED_num);
	else	//LED off
		LPC_GPIO2->FIOCLR = (1u << LED_num);
}

// Initialize ADC
void init_adc() {
	LPC_SC->PCONP |= 1u << 12; // PCADC on bit 12
	LPC_ADC->ADCR |= 1u << 21; // ADC power on pin 21
	
	LPC_SC->PCLKSEL0 |= (0u << 25) | (1u << 25); // Clock select dividers
	
	LPC_PINCON->PINSEL1 |= (0u << 19) | (1u << 18); // Pin select 19 & 18 for AD0.2 function
	
	LPC_ADC->ADCR &= 0xffffff00; // 
	LPC_ADC->ADCR |= 1u << 2; // Enable AD0.2 for potentiometer function
}

#if section == 1
int main(void) //1. Push button LED
{
	SystemInit();
	set_LED(0,2); // Reset all LEDs to off
	while(1){
		if((LPC_GPIO2->FIOPIN & (1u << 10))) { // Set LED "off"
			set_LED(1,0);
		}
		else { // Set LED "on"
			set_LED(1,1);
		}
	}
}

#elif section == 2
int main(void) //2. Joystick
{
	SystemInit();
	while(1){
		// Detect joystick direction
		if(!(LPC_GPIO1->FIOPIN & (1u << 23))) { 			// Check if joystick is north
			printf("North");
		}
		else if(!(LPC_GPIO1->FIOPIN & (1u << 26))) { 	// Check if joystick is west
			printf("West");
		}
		else if(!(LPC_GPIO1->FIOPIN & (1u << 25))) { 	// Check if joystick is south
			printf("South");
		}
		else if(!(LPC_GPIO1->FIOPIN & (1u << 24))) { 	// Check if joystick is east
			printf("East");
		}
		else {
			printf("Centered");
		}
		//Joystick button
		if(!(LPC_GPIO1->FIOPIN & (1u << 20))) {   		// Check if joystick is pressed
			printf(" and joystick is pressed\n");
		}
		else {
			printf(" and joystick is not pressed\n");
		}
	}
}

#elif section == 3
int main(void) //3. UART LED
{
	SystemInit();
	UARTInit(1, 115200);	// Initialize serial connection
	set_LED(0,2); 				// Turn off all LEDs
	while(1){
		int curr_input = 0;
		int input_sum = 0;
		while(curr_input != 10) {							// Look for input
			curr_input = UARTReceiveChar(1);		// Record input
			if(curr_input != 10)
				input_sum = 10*input_sum+(curr_input-48);
		}
		set_LED(0,2);													// Clear LEDs
		for(int i=0; i<8; i++) {				
			if((input_sum & (1u << i))) {				// Bitmask "i"th digit and if it is a "1",
				set_LED(8-i,1);										//  set the "i"th LED on
			}
		}
	}
}

#elif section == 4
int main(void) //4. ADC readout
{
	SystemInit();
	init_adc(); // Initialize ADC
	
	while(1){		
		LPC_ADC->ADCR |= (1 << 24); // Set bit 24 to "1" to start conversion (Table 531)
		
		while ((LPC_ADC->ADGDR & 1u << 31) == 0) {} // Wait until ADC is done (Table 532)
 		
		float voltage = 0;												// Initialize voltage reading
		voltage = (LPC_ADC->ADGDR & 0xfff << 4);	// Read voltage from ADGDR, mask off bits 15:4 for voltage
		voltage /= 19856;													// Map voltage to 0-3.3V
		printf("%.2f\n",voltage);									// Print voltage
	}
}

#endif
