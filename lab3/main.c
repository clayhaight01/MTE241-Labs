//     MTE241 F21 Lab 3 - Multitasking
//             October 27, 2021
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 1. uWaterloo User ID: chaight@uwaterloo.ca
// 2. uWaterloo User ID: s265sun@uwaterloo.ca

#include <cmsis_os2.h>
#include <LPC17xx.h>
#include <stdio.h>

// Initialize LEDs
void LEDInit() {
	// Set LED pins as outputs
	LPC_GPIO1->FIODIR |= ((1u << 28) | (1u << 29) | (1u << 31));
	LPC_GPIO2->FIODIR |= ((1u << 2) | (1u << 3) | (1u << 4) | (1u << 5) | (1u << 6));
}

// Turn LEDs on
void LEDOn(int LEDNum){
	// Cases 1-8: Turn on LEDs 1-8
	switch(LEDNum) {
		case 1:
			LPC_GPIO1->FIOSET |= (1u << 28);
			break;
		case 2:
			LPC_GPIO1->FIOSET |= (1u << 29);
			break;
		case 3:
			LPC_GPIO1->FIOSET |= (1u << 31);
			break;
		case 4:
			LPC_GPIO2->FIOSET |= (1u << 2);
			break;
		case 5:
			LPC_GPIO2->FIOSET |= (1u << 3);
			break;
		case 6:
			LPC_GPIO2->FIOSET |= (1u << 4);
			break;
		case 7:
			LPC_GPIO2->FIOSET |= (1u << 5);
			break;
		case 8:
			LPC_GPIO2->FIOSET |= (1u << 6);
			break;
	}
}

// Turn LEDs off
void LEDOff(int LEDNum) {
	// Cases 1-8: Turn off LEDs 1-8
	// Case 9: Turn off LEDs 7, 8, 9
	// Case 10: Turn off all LEDs
	
	switch(LEDNum) {
		case 1:
			LPC_GPIO1->FIOCLR |= (1u << 28);
			break;
		case 2:
			LPC_GPIO1->FIOCLR |= (1u << 29);
			break;
		case 3:
			LPC_GPIO1->FIOCLR |= (1u << 31);
			break;
		case 4:
			LPC_GPIO2->FIOCLR |= (1u << 2);
			break;
		case 5:
			LPC_GPIO2->FIOCLR |= (1u << 3);
			break;
		case 6:
			LPC_GPIO2->FIOCLR |= (1u << 4);
			break;
		case 7:
			LPC_GPIO2->FIOCLR |= (1u << 5);
			break;
		case 8:
			LPC_GPIO2->FIOCLR |= (1u << 6);
			break;
		case 9:
			// Turn off LEDs 6, 7, 8
			LPC_GPIO2->FIOCLR |= ((1u << 4) | (1u << 5) | (1u << 6));
			break;
		case 10:
			// Turn all LEDs off
			LPC_GPIO1->FIOCLR |= ((1u << 28) | (1u << 29) | (1u << 31));
			LPC_GPIO2->FIOCLR |= ((1u << 2) | (1u << 3) | (1u << 4) | (1u << 5) | (1u << 6));
			break;
	}
}


// Initialize ADC (From Lab 2)
void ADCInit() {
	LPC_SC->PCONP |= 1u << 12; // PCADC on bit 12
	LPC_ADC->ADCR |= 1u << 21; // ADC power on pin 21
	
	LPC_SC->PCLKSEL0 |= (0u << 25) | (1u << 25); // Clock select dividers
	
	LPC_PINCON->PINSEL1 |= (0u << 19) | (1u << 18); // Pin select 19 & 18 for AD0.2 function
	
	LPC_ADC->ADCR &= 0xffffff00; // 
	LPC_ADC->ADCR |= 1u << 2; // Enable AD0.2 for potentiometer function
}


// TASK 1: Read joystick
__NO_RETURN void t1 (void *arg) {
	// Let North=1, East=2, South=3, West=4, Centre=5
	int prevDir = 0;
	
	for(;;) {
		// Detect Joystick Direction
		// NORTH
		if(!(LPC_GPIO1->FIOPIN & (1u << 23))) {
			if(prevDir != 1) {
				// Print "0d1 = 0b0000 0001" to LEDs
				LEDOff(9); // Clear LEDS 6,7,8
				LEDOn(8);  // Turn on LED 8
			}
			prevDir = 1;
		}
		
		// EAST
		else if(!(LPC_GPIO1->FIOPIN & (1u << 24))){
			if(prevDir != 2) {
				// Print "0d2 = 0b0000 0010" to LEDs
				LEDOff(9); // Clear LEDS 6,7,8
				LEDOn(7);  // Turn on LED 7
			}
			prevDir = 2;
		}
		
		// SOUTH
		else if(!(LPC_GPIO1->FIOPIN & (1u << 25))){
			if(prevDir != 3) {
				// Print "0d3 = 0b0000 0011" to LEDs
				LEDOff(9); // Clear LEDS 6,7,8
				LEDOn(7);  // Turn on LED 7
				LEDOn(8);  // Turn on LED 8
			}
			prevDir = 3;
		}
		
		// WEST
		else if(!(LPC_GPIO1->FIOPIN & (1u << 26))){
			if(prevDir != 4) {
				// Print "0d4 = 0b0000 0100" to LEDs
				LEDOff(9); // Clear LEDS 6,7,8
				LEDOn(6);  // Turn on LED 6
			}
			prevDir = 4;
		}
		
		// CENTRE
		else {
			if(prevDir != 5) {
				// print "0d0 = 0b0000 0000" to LEDs (aka turn LEDs off)
				LEDOff(9);
			}
			prevDir = 5;
		}
		
		// Detect joystick button state
		if(!(LPC_GPIO1->FIOPIN & (1u << 20))) {// PRESSED
			LEDOn(5); // Turn on LED 5
		}
		else { // NOT PRESSED
			LEDOff(5); // Turn off LED 5
		}
		osThreadYield();
	}
}



// TASK 2: Read ADC value
__NO_RETURN void t2 (void *arg) {
	for(;;) {
		LPC_ADC->ADCR |= (1 << 24); // Set bit 24 to "1" to start conversion (Table 531)
		
		while ((LPC_ADC->ADGDR & 1u << 31) == 0) {} // Wait until ADC is done (Table 532)
 		
		float voltage = 0;												// Initialize voltage reading
		voltage = (LPC_ADC->ADGDR & 0xfff << 4);	// Read voltage from ADGDR, mask off bits 15:4 for voltage
		voltage /= 19856;													// Map voltage to 0-3.3V
		printf("%.2fV\n",voltage);								// Print voltage
		osDelay(5);																// Limit polling rate
	}
}



// TASK 3: Toggle LED on and off
__NO_RETURN void t3 (void *arg) {	
	int currState = 0;
	int prevState = 1;
	int toggle = 0;
	
	for(;;) {
		currState = ((LPC_GPIO2->FIOPIN & 1u << 10) == 0) ? 1 : 0; // Read button state, 1 for pressed, 0 otherwise
		if ((currState != prevState) && (currState == 1)) {
			toggle = !toggle;
			if(toggle) {
				LEDOn(1); // Turn on LED 1
			} else {
				LEDOff(1); // Turn off LED 1
			}
		}
		prevState = currState; // Update state
		osThreadYield();
	}
}

// Start Threads/Tasks
__NO_RETURN void app_main (void *arg) {
	// Create other threads
	osThreadNew(t1, NULL, NULL); // Task 1: Button Press
	osThreadNew(t2, NULL, NULL); // Task 2: Joystick Location
	osThreadNew(t3, NULL, NULL); // Task 3: ADC Reading
	
	printf("app main...\n");
	for (;;) {	
		osDelay(50);
	}
}

// Main loop - Start OS
int main (void) {
	SystemInit();
	LEDInit(); // Initialize LEDs
	ADCInit(); // Initialize ADC
	SystemCoreClockUpdate();
	
	// Start up RTOS
	osKernelInitialize();
	osThreadNew(app_main, NULL, NULL);
	osKernelStart();
	for(;;) {}
}
