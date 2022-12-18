//     MTE241 F21 Lab 4 - Communication
//             November 6th, 2021
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 1. uWaterloo User ID: chaight@uwaterloo.ca
// 2. uWaterloo User ID: s265sun@uwaterloo.ca

#include <cmsis_os2.h>
#include <LPC17xx.h>
#include <stdio.h>
#include "src/random.h"

// Create n queues using osMessageQueue
// Create a client thread for each queue - sends a message to the queue

// Needs to display:
// 1. Number of messages sent to each queue
// 2. The number received from each queue
// 3. Number of discarded for each queue
// 4. Elapsed time (in seconds)
// 5. Average ratio of discarded messages:processed messages (queue blocking prob.)
// 6. Average message arrival rate (arrival rate of Markovian process)
// 7. Average service rate of server queue
// 8. Relative errors of these approximated values

// Displaying this will require a seperate thread

// Define program constants
const int arrivalRate = 9;
const int serviceRate = 10;
const int queueSize = 10;
const int numQueues = 2;

// Initialize queues
osMessageQueueId_t queueId[numQueues];

// Total time elaspsed
float timeElapsed = 0;

// Initialize messages
int msgIn = 0; //counter to act as outgoing message
int msgOut = 0; //counter to hold received message

//Keep track of servers/clients
int serverNum[numQueues];
int clientNum[numQueues];


//message statistic counts
int messageTot[numQueues];
int sent[numQueues];
int failed[numQueues];
int received[numQueues];

float avgLossRatio[numQueues];
float avgArrivalRate[numQueues];
float avgServiceRate[numQueues];

__NO_RETURN void server(void *arg) {
	for(;;) {
		//timeElapsed = osKernelGetSysTimerCount()/100000000.0;
		timeElapsed = osKernelGetTickCount() / osKernelGetTickFreq();
		
		int queueNum = *(int*)arg;
		
		osDelay(((next_event() * osKernelGetTickFreq() / serviceRate) >> 16)); //convert seconds to ticks, divide by average service rate, bit shift 16
		
		osMessageQueueGet(queueId[queueNum], &msgOut, NULL, osWaitForever); //get message
		received[queueNum]++; //increase received count for this specific queue
		
		avgServiceRate[queueNum]=received[queueNum]/timeElapsed;
		//printf("Message %d received",queueNum);
	}
}
__NO_RETURN void client(void *arg) {
	for(;;) {
		//timeElapsed = osKernelGetSysTimerCount()/100000000.0;
		timeElapsed = osKernelGetTickCount() / osKernelGetTickFreq();
		//delay random time
		osDelay((next_event() * osKernelGetTickFreq() / arrivalRate) >> 16); //convert seconds to ticks, divide by average arrival rate, bit shift 16
		
		for(int i = 0; i < numQueues; i++) {
			msgIn++; //change message everytime
			messageTot[i]++;
			if(osMessageQueuePut(queueId[i], &msgIn, NULL, osWaitForever) != osErrorResource) {
				sent[i]++; //increase sucessful sent into this specific queue
				//printf("Message %d sent",i);
			}
			else {
				failed[i]++; //increase fails sent into this specific queue
				//printf("Message %d failed",i);
			}
			avgLossRatio[i]=failed[i]/messageTot[i];
			avgArrivalRate[i]=messageTot[i]/timeElapsed;
		}
	osThreadYield();
	}
}

__NO_RETURN void output(void *arg) {
	// Initialize data variables
	int numPrintout = 0;
	float amlr = 0;
	float amar = 0;
	float asr = 0;
	float epblk = 0;
	float earrv = 0;
	float eserv = 0;

	for(;;) {
		// Delay 1 second
		osDelay(osKernelGetTickFreq());		
		
		// Print header
		if(numPrintout % 20 == 0) {
			printf("%5s, %5s, %5s, %5s, %5s, %5s, %6s, %6s, %6s, %6s, %6s, %6s\n",
			"Qid", "Time", "Sent", "Recv", "Over", "Wait", "P_blk", "Arrv", "Serv", "Epblk", "Earrv", "Eserv");
			numPrintout = 0;
		}
		
		for(int i = 0; i < numQueues; i++) {
			// Calculate time elapsed
			//timeElapsed = osKernelGetSysTimerCount()/100000000.0;
			timeElapsed = osKernelGetTickCount() / osKernelGetTickFreq();
			
			// Calculate average message loss ratio
			amlr = failed[i]/sent[i];
			// calculate average message arrival rate
			amar = sent[i]/timeElapsed;
			// Calculate average service rate
			asr = received[i]/((1 - (arrivalRate/serviceRate))*timeElapsed);
			
			// Calculate relative errors:
			epblk = (amlr - avgLossRatio[i])/avgLossRatio[i];
			earrv = (amar - avgArrivalRate[i])/avgArrivalRate[i];
			eserv = (asr - avgServiceRate[i])/avgServiceRate[i];
			
			// Print data
			printf("%5d, %6f, %5d, %5d, %5d, %5d, %6.4f, %6.4f, %6.4f, %6.4f, %6.4f, %6.4f\n",
				i,																		// Queue number
				timeElapsed,													// Elapsed time in seconds
				sent[i],															// Total number of messages sent
				received[i],													// Total number of messages received
				failed[i],														// Total number of overflows
				osMessageQueueGetCount(queueId[i]),		// Current number of messages in queue
				amlr,																	// Average message loss ratio
				amar,																	// Average message arrival rate
				asr,																	// Average service rate
				epblk,																// Relative blocking error rate
				earrv,																// Relative arrival error rate
				eserv);																// Relative service error rate
		}
	
		numPrintout += 1;
		osThreadYield();
	}
}

// Start OS
int main (void) {
	SystemInit();
	SystemCoreClockUpdate();
	
	// Start printout thread
	osThreadNew(output, NULL, NULL);
	
	// Start server/client threads
	for(int i = 0; i < numQueues; i++) {
		serverNum[i] = i;
		clientNum[i] = i;
		osThreadNew(server, &serverNum[i], NULL);
		osThreadNew(client, &clientNum[i], NULL);
		queueId[i] = osMessageQueueNew(queueSize, NULL, NULL); // Change 2nd parameter
	}
	
	// Start up RTOS
	osKernelInitialize();
	osKernelStart();
	for(;;) {}
}
