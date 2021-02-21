/*
    Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
    Copyright (C) 2012 - 2018 Xilinx, Inc. All Rights Reserved.

    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
    the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software. If you wish to use our Amazon
    FreeRTOS name, please do so in a fair use way that does not cause confusion.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    http://www.FreeRTOS.org
    http://aws.amazon.com/freertos


    1 tab == 4 spaces!
*/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"


#define DELAY_30_SECONDS    30000UL

#define LED_CHANNEL 1
#define BTN_CHANNEL 1
#define SW_CHANNEL 2

/*-----------------------------------------------------------*/
#define LED_DEVICE_ID XPAR_AXI_GPIO_1_DEVICE_ID
#define SW_DEVICE_ID XPAR_AXI_GPIO_0_DEVICE_ID
#define BTN_DEVICE_ID XPAR_AXI_GPIO_0_DEVICE_ID


/* The TaskLED, TaskBTN and TaskSW tasks */
static void TaskLED( void *pvParameters );
static void TaskBTN( void *pvParameters );
static void TaskSW( void *pvParameters );
/*-----------------------------------------------------------*/

/* The task handles  */
static TaskHandle_t xLEDTask;
static TaskHandle_t xBTNTask;
static TaskHandle_t xSWTask;
static QueueHandle_t xQueueBtnSw = NULL;

XGpio LEDInst, SWInst, BTNInst;

static int items = 0;
static int current_input = 0;
static int prev_input = 0;

int main( void )
{

	xil_printf( "Hello from Freertos example main\r\n" );

	int status;

	 // Initialize LEDs
	 status = XGpio_Initialize(&LEDInst, LED_DEVICE_ID);
	 if (status != XST_SUCCESS) return XST_FAILURE;

	 // Initialize buttons
	 status = XGpio_Initialize(&BTNInst, BTN_DEVICE_ID);
	 if (status != XST_SUCCESS) return XST_FAILURE;

	 // Initialize slide switches
	 status = XGpio_Initialize(&SWInst, SW_DEVICE_ID);
	 if (status != XST_SUCCESS) return XST_FAILURE;

	 // Set LEDs direction to outputs
	 XGpio_SetDataDirection(&LEDInst, 1, 0x00);

	 // Set button direction to inputs
	 XGpio_SetDataDirection(&BTNInst, 1, 0xFF);

	 // Set slide switch direction to inputs
	 XGpio_SetDataDirection(&SWInst, 1, 0xFF);

	/* Create the three tasks.
	/* Create LED task with priority 2*/
	xTaskCreate( 	TaskLED, /* The function that implements the task. */
				( const char * ) "TaskLED",  /* Text name for the task, provided to assist debugging only. */
				configMINIMAL_STACK_SIZE, 	/* The stack allocated to the task. */
				NULL, 						/* The task parameter is not used, so set to NULL. */
				tskIDLE_PRIORITY + 2,			/* The task runs at the idle priority. */
				&xLEDTask);

	/* Create BTN task with priority 2*/
	xTaskCreate( TaskBTN,
				( const char * ) "BTN",
				configMINIMAL_STACK_SIZE,
				NULL,
				tskIDLE_PRIORITY + 2,
				&xBTNTask );

	/* Create SW task with priority 2*/
	xTaskCreate( TaskSW,
				 ( const char * ) "TaskSW",
				 configMINIMAL_STACK_SIZE,
				 NULL,
				 tskIDLE_PRIORITY + 2,
				 &xSWTask );

	/* Create the queue used by the tasks.  */
	xQueueBtnSw = xQueueCreate( 	10,						/* There is only 10 space in the queue. */
							sizeof( int ) );	/* Each space in the queue is large enough to hold an integer. */

	/* Check the queue was created. */
	configASSERT( xQueueBtnSw );


	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for( ;; );
}


/*-----------------------------------------------------------*/
static void TaskLED( void *pvParameters )
{
const TickType_t x30seconds = pdMS_TO_TICKS( DELAY_30_SECONDS );  //Convert to tick time for delay
static int led = 0;  //define and initialize led variable
	for( ;; )
	{
		check:  // Label to check for errors
			items = uxQueueMessagesWaiting(xQueueBtnSw );  // Get number of items in the queue
			// If the Queue has no items or is full goto label error else go to receive label
			if((items == 0) | (items == 10)){
				goto error;
			}
			else{
				goto receive;
			}

		// Error condition protocol
		error:
		while ((items == 0) | (items == 10) ){
			/* Reset current and previous inputs */
			prev_input = 0;
			current_input = 0;

			led = 15;  // Assign led to 15
			XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, led);  // Light up all the LEDs
			for (int Delay = 0; Delay < 50000000; Delay++);  // Keep lit for approximately 1 sec

			XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, 0x0);  // Turn off all LEDs
			for (int Delay = 0; Delay < 50000000; Delay++);  // Keep it OFF for approximately 1 sec

			/* if no items in the Queue go to label check to check if more items are added or not */
			if(items == 0){
				goto check;
			}
			/* Else go to label receive to receive item from the queue */
			else{
				goto receive;
			}
		}

		/* Recieve items from the Queue */
		receive:
		xil_printf("items %d\n", items);

		/* Block to wait for data arriving on the queue. */
		xQueueReceive( 	xQueueBtnSw,				/* The queue being read. */
						&current_input,	/* Data is read into this address. */
						x30seconds);	/* Wait 30 seconds for data. */

		/* Delay for 1 second. */
		xil_printf("current_input = %d\n", current_input);

		/* if else conditions to check if previous or current inputs are from same switch and button numbers */
		if (((current_input == 16 ) & (prev_input == 1)) | ((current_input == 1 ) & (prev_input == 16))){
			XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, 1);
		}
		else if (((current_input == 32 ) & (prev_input == 2)) | ((current_input == 2 ) & (prev_input == 32))){
			XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, 2);
		}
		else if (((current_input == 64 ) & (prev_input == 4)) | ((current_input == 4 ) & (prev_input == 64))){
			XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, 4);
		}
		else if (((current_input == 128 ) & (prev_input == 8)) | ((current_input == 8 ) & (prev_input == 128))){
			XGpio_DiscreteWrite(&LEDInst, LED_CHANNEL, 8);
		}


		prev_input = current_input;  // Update previous input as current input

		/* If the Queue was full skip the 10 second delay to quickly resolve the error */
		if (items == 10){
			goto check;
		}
		else{
			for (int Delay = 0; Delay < 210000000; Delay++);  // loop delay of approximately 10 seconds before reading the next queue item
		}
	}
}

/*-----------------------------------------------------------*/
static void TaskBTN( void *pvParameters )
{
	const TickType_t x30seconds = pdMS_TO_TICKS( DELAY_30_SECONDS);  // Convert delay time to ticktype
	/* Define and Initialize variables to store btn read */
	static int btn_read1 = 0;
	static int btn_read2 = 0;

	for( ;; )
	{
		/* Takes care of Button bounce by including a delay between
		 * two button reads. The button reads are compared to
		 * make a decision whether to send the data to Queue
		 */
		btn_read1 = XGpio_DiscreteRead(&BTNInst, BTN_CHANNEL);
		for (int Delay = 0; Delay < 1000; Delay++);
		btn_read2 = XGpio_DiscreteRead(&BTNInst, BTN_CHANNEL);

		/* Checks if the first btn value is the same as second btn value */
		if ((btn_read1 != 0) & (btn_read2 == 0)){
			xil_printf("btn = %d\n", btn_read1);
			/* Block to wait for data sent to the queue. */
			xQueueSend( 	xQueueBtnSw,				/* The queue being read. */
							&btn_read1,	/* Data is read from this address. */
							x30seconds );	/* Wait 30 seconds to send data. */
		}

	}
}

/*-----------------------------------------------------------*/
static void TaskSW( void *pvParameters )
{
	const TickType_t x30seconds = pdMS_TO_TICKS( DELAY_30_SECONDS );  // Convert delay time to ticktype

	/* Define and Initialize variables to store switch read */
	static int sw_read = 0;
	static int sw_data = 0;
	static int pre_sw_data = 0;
	for( ;; )
	{
		/* Switch case statements to read from switches and
		 * encode it to the higher bits of a byte
		 */
		sw_read = XGpio_DiscreteRead(&SWInst, SW_CHANNEL);
		if (sw_read == 1){
			sw_data = 16;
		}
		else if (sw_read == 2){
			sw_data = 32;
		}
		else if (sw_read == 4){
			sw_data = 64;
		}
		else if (sw_read == 8){
			sw_data = 128;
		}
		else{
			sw_data = 0;
		}

		/* Checks if a switch was turned ON previously and switch value is 0 */
		if ((sw_data != 0) & (pre_sw_data != sw_data)){
			xQueueSend( 	xQueueBtnSw,				/* The queue being read. */
						&sw_data,	/* Data is read into this address. */
						x30seconds);	/* Wait without a timeout for data. */
			xil_printf("sw_data %d\n", sw_data);
		}
		pre_sw_data = sw_data;  // Update previous switch value read to the new switch value.

	}
}



