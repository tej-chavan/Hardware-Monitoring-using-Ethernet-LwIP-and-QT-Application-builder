/***************************************************************************************************************
*File name 		: start_application.c
*
*Version   		: 1.0
* 	
*Date      		: March 22, 2018
* 
*Authors   		: Tejas Chavan
*		   		: Rithvik Ballal
*
*Description	: This is the major file wherein most of the operations takes place:
				1. The process starts from the start_application() function where in the server creates a new 
				   socket, binds the socket to the ip address and generates secure connection.
				2.  Upon listening for new connections, it acceots a connection generate communication protocol.
					A call back function "accept_callback" wherein the server receives the packet.
				3. The server receives the packet using the unction tcp_recv() and calls a function recv_callback()
				4. In the receive call back function, it indicates that it has received a packet. 
				   It performs further Bit manipulation to segregate the command and data bits.
				5. Converts the incomming ASCII data to integrer to perform apropriate actions.
				6. When the server is ready to send the data, it converts it into ASCII and writes the data into the 
				   send buffer. Here we send the packet immediately instead of waiting for more packets.
*****************************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "xstatus.h"
#include "xtmrctr.h"
#include "pwm_tmrctr.h"
#include "lwip/err.h"
#include "lwip/tcp.h"
#include "PmodHYGRO.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xparameters.h"
#include "xintc.h"
#if defined (__arm__) || defined (__aarch64__)

#endif

//================================= DEFINITIONS==================================================
#define TIMER_FREQ_HZ 				100000000									// Definition for frequency of the pmodHYGRO
#define HYGRO_BASE_ADDR 			XPAR_PMODHYGRO_0_AXI_LITE_IIC_BASEADDR		// HYGRO base address
#define HYGRO_TIMER_ADDR			XPAR_PMODHYGRO_0_AXI_LITE_TMR_BASEADDR      // HYGRO Timer Address
#define HYGRO_DEVICE_ID				XPAR_PMODHYGRO_0_DEVICE_ID					// HYGRO Device ID

// Timer 1 definitions (Sec timer)
#define SEC_TIMER_DEVICE_ID			XPAR_AXI_TIMER_2_DEVICE_ID					// SEC timer device ID
#define SEC_TIMER_BASE_ADDR			XPAR_AXI_TIMER_2_BASEADDR					// SEC timer Base address
#define SEC_TIMER_HIGH_ADDR			XPAR_AXI_TIMER_2_HIGHADDR					// SEC timer high address
#define TmrCtrNumber				0											// Timer Counter number

// Interrupt controller device ID
#define INTC_DEVICE_ID				XPAR_INTC_0_DEVICE_ID
#define SEC_TIMER_INTERUPT_ID		XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_2_INTERRUPT_INTR

//======================INSTANCES===================================================================================
static XTmrCtr InstancePtr;						// Timer Instance
PmodHYGRO myDevice;								// Pmod HYGRO instance
XIntc 	   IntrptCtlrInst;	         		    // Interrupt Controller instance
XTmrCtr    SecTimerInst;                 		// Timer Instances

//==================================functions and variables declaration=============================================
uint16_t sts;	
volatile u32 packet_received;					// variable for the packet received from the client
volatile u16 received_data;						// Variable for the data section of the packet

int Generate_pwm(uint32_t duty_cycle);			// Fucntion used to generate the PWM
int do_init(void); 								// Initialize function

//Functions For HYGRO
int HYGROinitialize();
void EnableCaches();
void HYGROrun();
void HYGROCleanup();
void DisableCaches();
void give_temp_deg(void);
void give_temp_fer(void);

//Second timer interrupt handler declaration
void Second_Timer_Handler(void);
int Sec_Timer_initialize(void);
//====================================================================================================
int transfer_data() {
	return 0;
}

void print_app_header()
{
	xil_printf("\n\r\n\r-----lwIP TCP echo server ------\n\r");
	xil_printf("TCP packets sent to port 6001 will be echoed back\n\r");
}

/****************************************************************************/
/**
 * Receive_callback.
 * - Call back function to receive the data
 *
 *****************************************************************************/

err_t recv_callback(void *arg, struct tcp_pcb *tpcb,
                               struct pbuf *p, err_t err)
{
	float temp_degf, temp_degc , hum_perrh;
	//xil_printf("2.\n");
	/* do not read the packet if we are not in ESTABLISHED state */
	if (!p) {
		tcp_close(tpcb);
		tcp_recv(tpcb, NULL);
		return ERR_OK;
	}

	/* indicate that the packet has been received */
	tcp_recved(tpcb, p->len);
	usleep(1000);

	xil_printf("received\n");
	packet_received  = atoi(p->payload);
//	xil_printf("Data Read : %d length :%d\n", packet_received, p->len);

	received_data = packet_received;
	xil_printf(" DATA: %d | Command : %d | length = %d\n",received_data,(packet_received >> 16), p->len);

//================================================ Compute packets ===============================================================
// Bit manipulation is done to segregate the command and data bits from the entir packet.	
	if((packet_received >> 16) == 5)						// Asking for temperaure in degree
	{
		temp_degc = HYGRO_getTemperature(&myDevice);
		uint8_t dc_value_deg_temp = temp_degc;
		uint16_t dc_value_deg = 512 + dc_value_deg_temp;	// Send temperature readings by masking it with the command bits
	      xil_printf(
	         "Temperature: %d.%02d deg C\n\r",
	         (int) temp_degc,
	         ((int) (temp_degc * 100)) % 100
			 );
		itoa(dc_value_deg, p->payload, 10);					// Convert Integer to ASCII before sending it to the client

	}
	else if((packet_received >> 16) == 1)					// Asking for temperature in ferhenheit
	{
		usleep(1000);
		temp_degc = HYGRO_getTemperature(&myDevice);		// Function call to get temperature in degree
		temp_degf = HYGRO_tempC2F(temp_degc);				// Function call to get temperature in ferhenheit
		uint8_t dc_value_sv_temp = temp_degf;
		uint16_t dc_value_sv = 512 + dc_value_sv_temp;
	      xil_printf(
	         "Temperature: %d.%02d deg F\n\r",
	         (int) temp_degf,
	         ((int) (temp_degf * 100)) % 100
			 );
		itoa(dc_value_sv, p->payload, 10);
	}
	else if((packet_received >> 16) == 3)					// Asking for humidity
	{
		hum_perrh = HYGRO_getHumidity(&myDevice);
		uint8_t dc_value_humid_temp = hum_perrh;			// Function to get humidity
		uint16_t dc_value_humid = 1024 + dc_value_humid_temp;		// Masking with command bits
		 xil_printf(
			         "Humidity: %d.%02d RH\n\r",
			         (int) hum_perrh,
			         ((int) (hum_perrh * 100)) % 100
					 );
		itoa(dc_value_humid, p->payload, 10);
	}
	else if((packet_received >> 16) == 0)					// Providing Duty cycle
	{	
		Generate_pwm(received_data);						// Call to the genrate pwm function

	}

//==================================================================================================================
// Write the payload generated in above computation into the send buffer 
// tcp_output sends the packet immediately instead of waiting for more packets
	if (tcp_sndbuf(tpcb) > p->len) {
		err = tcp_write(tpcb, p->payload, p->len, 1);
		err = tcp_output(tpcb);
		xil_printf("Data Written: %s\n", p->payload);
	} else
		xil_printf("no space in tcp_sndbuf\n\r");
		usleep(1000);
		/* free the received pbuf */
		pbuf_free(p);


	/* free the received pbuf */
	pbuf_free(p);

	return ERR_OK;
}

/****************************************************************************/
/**
 * Here the server receives the data from the client and send sontrol to the call back fn
 *****************************************************************************/

err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	static int connection = 1;

	xil_printf("1.\n");
	/* set the receive callback for this connection */

	tcp_recv(newpcb, recv_callback);
xil_printf("2\n");
	/* just use an integer number indicating the connection id as the
	   callback argument */
	tcp_arg(newpcb, (void*)(UINTPTR)connection);

	/* increment for subsequent accepted connections */
	connection++;

	return ERR_OK;
}

/****************************************************************************/
/**
 * Start Application
 * This is where the main functioning starts.
 * Socket programming flow is executed here.
 * new -> bind -> Listen -> Accept.
 *****************************************************************************/


int start_application()
{
	init_platform();

	uint32_t sts;

	sts = do_init();
	if (XST_SUCCESS != sts)
		{
			xil_printf("FATAL ERROR: Could not initialize the peripherals\n\r");
			xil_printf("Please power cycle or reset the system\n\r");
			exit(1);
		}

	// Enable Microblaze interrupts
	microblaze_enable_interrupts();


	struct tcp_pcb *pcb;
	err_t err;
	unsigned port = 7;

	/* create new TCP PCB structure */
	pcb = tcp_new();
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\n\r");
		return -1;
	}

	/* bind to specified @port */
	err = tcp_bind(pcb, IP_ADDR_ANY, port);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port %d: err = %d\n\r", port, err);
		return -2;
	}

	/* we do not need any arguments to callback functions */
	tcp_arg(pcb, NULL);

	/* listen for connections */
	pcb = tcp_listen(pcb);
	if (!pcb) {
		xil_printf("Out of memory while tcp_listen\n\r");
		return -3;
	}

	/* specify callback to use for incoming connections */
	tcp_accept(pcb, accept_callback);

	xil_printf("TCP echo server started @ port %d\n\r", port);

	return 0;
}

/****************************************************************************/
/**
 * Generates the PWm from AXI Timer and internally feeds it to the HB3
 *****************************************************************************/

int Generate_pwm(uint32_t duty_cycle)
{
	uint32_t status;
// Provide parameters to the PWM generator
	status = PWM_SetParams(&InstancePtr, 4*(10^3), duty_cycle);
	if (status != 0)
     {
	 xil_printf("Could not load duty cycle.  Function: Generate_pwm");
	 return XST_FAILURE;
	 }
	xil_printf("Duty cycle loaded");
	 return XST_SUCCESS;
}

/****************************************************************************/
/**
 * Initialize peripherals
 *
 *****************************************************************************/

int do_init(void)
{
	uint32_t status;							// status from xilinx lib calls

// Initialize HYGRO
status = HYGROinitialize();
if (XST_SUCCESS != status)
		{
			xil_printf("Could not initialize HYGRO");
			exit(1);
		}

// Initialize the AXI PWM timer
status = PWM_Initialize(&InstancePtr, XPAR_TMRCTR_1_DEVICE_ID, true, 100000000);
	if (status != 0)
		{
		xil_printf("Could not initialize the PWM generator\n");
		return XST_FAILURE;
		}

//Start the PWM timer
status =  PWM_Start(&InstancePtr);
	     if (status != 0)
	     {
	     xil_printf("Could not start PWM");
	     return XST_FAILURE;
	     }

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 * Initialize pmodHYGRO
 *
 * Use function HYGRO_begin()
 * Device ID, Base address, chip address, timer address, timer device id, timer freq
 *****************************************************************************/

int HYGROinitialize()
{
	EnableCaches();
	xil_printf("HYGRO Initialization started\n");
	HYGRO_begin(
	      &myDevice,
		  HYGRO_BASE_ADDR,
	      0x40, // Chip address of PmodHYGRO IIC
		  HYGRO_TIMER_ADDR,
		  HYGRO_DEVICE_ID,
	      TIMER_FREQ_HZ // Clock frequency of AXI bus, used to convert timer data
	   );
	xil_printf("HYGRO Initialization Done\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 *Enable Cache for proper HYGRO functioning
 *****************************************************************************/

void EnableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheEnable();
#endif
#endif
}

/****************************************************************************/
/**
 *
 *****************************************************************************/


void DemoCleanup() {
   DisableCaches();
}

void DisableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheDisable();
#endif
#endif
}



/****************************************************************************
 * Sec_Timer_initialization  is an AXI Timer which count down for 1 second
 * DO NOT MODIFY
 *
 *
 ***************************************************************************/
int Sec_Timer_initialize(void){

	uint32_t status;				// status from Xilinx Lib calls
	u32		ctlsts;		// control/status register or mask

	status = XTmrCtr_Initialize(&SecTimerInst,SEC_TIMER_DEVICE_ID);
	if (status != XST_SUCCESS) {
		xil_printf("Failed to initialize timer counter");
		return XST_FAILURE;
	}
	status = XTmrCtr_SelfTest(&SecTimerInst, TmrCtrNumber);
	if (status != XST_SUCCESS) {
		xil_printf("Timer Counter self test failed");
		return XST_FAILURE;
	}

	ctlsts = XTC_CSR_AUTO_RELOAD_MASK | XTC_CSR_LOAD_MASK |XTC_CSR_DOWN_COUNT_MASK |XTC_CSR_ENABLE_INT_MASK ;
	XTmrCtr_SetControlStatusReg(SEC_TIMER_BASE_ADDR, TmrCtrNumber,ctlsts);

	//Set the value that is loaded into the timer counter and cause it to be loaded into the timer counter
	XTmrCtr_SetLoadReg(SEC_TIMER_BASE_ADDR, TmrCtrNumber,199999998);
	XTmrCtr_LoadTimerCounterReg(SEC_TIMER_BASE_ADDR, TmrCtrNumber);
	ctlsts = XTmrCtr_GetControlStatusReg(SEC_TIMER_BASE_ADDR, TmrCtrNumber);
	ctlsts &= (~XTC_CSR_LOAD_MASK);
	XTmrCtr_SetControlStatusReg(SEC_TIMER_BASE_ADDR, TmrCtrNumber, ctlsts);

	ctlsts = XTmrCtr_GetControlStatusReg(SEC_TIMER_BASE_ADDR, TmrCtrNumber);
	ctlsts |= XTC_CSR_ENABLE_TMR_MASK;
	XTmrCtr_SetControlStatusReg(SEC_TIMER_BASE_ADDR, TmrCtrNumber, ctlsts);
	XTmrCtr_Enable(SEC_TIMER_BASE_ADDR, TmrCtrNumber);

	return XST_SUCCESS;

}






