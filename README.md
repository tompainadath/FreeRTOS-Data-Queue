# FreeRTOS-Data-Queue
# Summary
The project analyses the data transmission between tasks using a Data Queue. For this, basic IP integrator was explored. A block design with two GPIO interfaces was created in the Vivado software. An SDK FreeRTOS template was imported and was modified to input switches and buttons, and output LEDs. Programmed the system to let the LEDs behave certain way according to inputs from switches and buttons by reading from the Data Queue. The motivation for this analysis was to understand concept of task management and implement Data Queue for transmission in Embedded Systems. The results were as expected. Successfully synthesized the block design and the system behaved the way desired.
# Introduction
The objectives of the project were to control the behavior of LEDs on the zybo board using the switches and buttons using FeeRTOS tasks. The tasks to perform are described below:
1.	Create a FreeRTOS main() application that creates three tasks and a single queue. The queue has a handle xQueueBtnSw which can hold 10 entries large enough to encode which new button is depressed or new total switch settings is available. 
2.	Create a FreeRTOS task TaskLED at priority 2 that receives a button value and the total current switch settings (see below) from the queue xQueueBtnSw every 10 seconds using an appropriate loop delay, a block time of 30 seconds for the receive queue.
3.	The BTN value is used to light an LED only if the matching SW is ON. The matching BTNs and SWs would be BTN0 and SW0, BTN1 and SW1, BTN2 and SW2 and BTN3 and SW3. Thus LEDn corresponding to a depressed BTNn would only light is SWn is ON. 
4.	Create a FreeRTOS task TaskBTN that reads the BTNs at priority 2 for data transmission on the common queue. Encode BTN0 as 1, BTN1 as 2, BTN2 as 4 and BTN3 as 8 or the lower 4 bits of a byte. The upper 4 bits of this queue data is 0. The block time for the transmit queue is 30 seconds. The BTN value is only sent to the queue if a new BTN depression is sensed. 
5.	Because of BTN bounce you should make sure that a BTN is no longer depressed before executing the FreeRTOS queue function. This is a required task for this Lab. 
6.	Create a FreeRTOS task TaskSW that reads the SWs at priority 2 for data

transmission on the common queue. Encode SW0 as 16, SW1 as 32, SW2 as 64 and SW3 as 128 or the upper 4 bits of a byte. The lower 4 bits of the queue data is 0. The block time for the transmit queue is 30 seconds. The current total SW value is only sent to the queue if a new total SW value is sensed. 
7.	 An error condition of empty or full receive queue should flash all the LEDs for 1 second on and 1 second off until the error is resolved. 

# Discussion
Created a block design consisting of ZYNQ7 processing system, processor system reset, AXI Interconnect and two AXI GPIOs. The diagram of the design is shown below:
 
Figure 1. Diagram of the Hardware design
It can be observed that two GPIO blocks are created to control LEDs and get inputs from buttons and switches. HDL wrapper file was created and generated bitstream. The design was then exported to SDK. Imported the template ‘Hello world’ file and modified it to perform the specified tasks. 
First step was to initialize the inputs and outputs and setting the direction. The code below shows how this was done:
 
Figure 2. Code to create tasks
Three tasks were created using the xTaskCreate function. They were TaskLED, TaskBTN and TaskSW. All tasks were given the same priority, tskIDLE_PRIORITY+2. The code snippet below shows how this was accomplished
 
Figure 3. Code to create tasks
Also created a Queue called xQueueBtnSw as shown below:
 
Figure 4. Code to create Queue
The Queue was created such that the maximum items it can contain is 10 and each item can have a maximum size of int.
Next step was to create and define the tasks for LEDs, Switches and Buttons. The code below shows the LED task:
 
Figure 5. Task LED (a)
The above code deals with the error conditions. It uses goto and label statements to go in and out of conditions. The code checks if the Queue is empty or full. If either case is met, the code goes to the error condition protocol which is inside a while loop. Within this loop, it checks the update of the Queue. The section of code below shows the no error condition protocols. The program waits 30 seconds to receive new data from the Queue. Once it receives data from the Queue it looks for a matching condition to output appropriate LED count. It also keeps track of the previously received item from the Queue. 
 
Figure 6. Task LED (b)
The section of code above receives the data from the queue every 10 seconds with a block time of 30 seconds. The if else statements check if the same button and switch numbers are transmitted. If the Queue is full the program skips the 10 second delay to quickly read the next item in the Queue to resolve the error. 
The code below is the Button task. This section of code is responsible for sending the button values into the Queue. It also takes care of Button bounce.
 
Figure 7. Button Task 
The above section of code first takes care of button bounce by reading from the button twice with a small delay in between. It then proceeds to compare the two value to check if the button is still depressed. If they are different the program take the first read value and sends to Queue.
The image below shows the Switch task. The switch task takes care of the switch inputs to be sent to Queue. It makes sure not to sent 0s to Queue when no switches are ON as well as not to sent one value per switch turned ON. 

 
Figure 8. Switch Task 
The code above converts the switch value to upper bits of a byte. This is then checked for repeating or 0 values before sending into the Queue. 
The images below show the program in action:
 
Figure 9. Error Condition (a)

 
Figure 10. Error Condition (b)
The above two images shows the status of the LEDs when the Queue is empty or full.
 
Figure 11. SW0 and BTN0 received
The image above shows when the SW0 is turned ON and BTN0 was pressed.
 
Figure 12. SW1 and BTN1 received
The image above shows when the SW1 is turned ON and BTN1 was pressed.

 
Figure 13. SW2 and BTN2 received
The image above shows when the SW2 is turned ON and BTN2 was pressed.
 
Figure 14. SW3 and BTN3 received
The image above shows when the SW3 is turned ON and BTN3 was pressed.
# Conclusions
Successfully generated a hardware design using Vivado software and implemented the tasks on Zybo board by programming in C using SDK. The main objective of the lab was to create tasks and a queue and manage them based on the specifications. Successfully added data to Queue from switches and button inputs and was transmitted to LED task to interpret it and make decisions. Overall the Lab was a success. 
