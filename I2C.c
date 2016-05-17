/**************************************************************************************************
Target Hardware:		PIC24F...
Chip resources used:	I2C modules
Code assumptions:		Error handling is done by external code
						Only one I2C module is used
						All statemachines are declared in the config file (enum I2C_STATE_MACHINE_LIST) and not created on the fly
Purpose:				Allow access to interrupt/polling driven I2C resources
Notes:					Multi-module support is planned for a future release
						Support to arbitrarily start/stop a statemachine instead of just delaying it is planned for a future release

  +-----------+                                        +-----------+                 +-----------------+
  |   Start   |                                        |    End    |                 |      Start      |
  |I2C_Handler|                                        |I2C_Handler<---------+       |    Interrupt    |
  +-----+-----+                                        +-----------+         |       +--------+--------+
        |                                                                    |                |
        |    +---------------+                 +-------------------+         |       +--------v--------+
             |               |                 |                   |         |       |    Run State    |
        |    V               |Yes              V                   |Yes      |       |     Machine     |
+-------+--------+     +----------+    +----------------+    +----------+    |       +--------+--------+
|  Is the State  | Yes |Next State| No |  Is the State  | No |Next State| No |                |
|Machine Stopped?+---->+ Machine? +--->+Machine Running?+--->+ Machine? +----+       +--------v--------+
+----------------+     +--^----^--+    +----------------+    +--^----^--+         No |  State Machine  |
        |No               |    |               |Yes             |    |          +----+    Finished?    |
        v                 |    |               v                |    |          |    +-----------------+
+-------+--------+        |    |       +-------+--------+       |    |          |             |Yes
|Increment Timer |        |    |       | Is the Module  |  Yes  |    |          |    +--------v--------+
+-------+--------+        |    |       |    Locked?     +-------+    |          |    |Set State Machine|
        |                 |    |       +-------+--------+            |          |    |   to Delayed    |
        V                 |    |               |                     |          |    +--------+--------+
+----------------+        |    |               v                     |          |             |
|Interval met or |  No    |    |       +-------+--------+    +-------+--+       |    +--------v--------+
|   exceeded?    +--------+    |       |   Lock Module  |    |   Run    |       |    | Release Module  |
+----------------+             |       +-------+--------+    | Function |       |    +--------+--------+
        |Yes                   |               |             +-----^----+       |             |
        V                      |               v                   |            |    +--------v--------+
+---------------+              |       +-------+--------+          |            +---->  End Interrupt  |
|Undelay Module +--------------+       |  Set Baudrate  |          |                 +-----------------+
+---------------+                      |    Generator   +----------+            
                                       +----------------+

 
Version History:
v1.0.0	2016-05-16	Craig Comberbach	Compiler: XC16 v1.11	IDE: MPLABx 3.30	Tool: ICD3		Computer: Intel Core2 Quad CPU 2.40 GHz, 5 GB RAM, Windows 10 Home 64-bit
	Initialize_I2C() function no longer takes an argument
	Fixed syncing issue with multiple statemachines
	Removed code that was only used in standalone independent mode (Current status, etc)
	Cleaned up Contention Arbitration function and included the exact text from the standard for reference
	Refactored code
	Finished documenting in detail all of the functions
v0.1.0	2016-05-16	Craig Comberbach	Compiler: XC16 v1.11	IDE: MPLABx 3.30	Tool: ICD3		Computer: Intel Core2 Quad CPU 2.40 GHz, 5 GB RAM, Windows 10 Home 64-bit
	Supports automatic control of a single module. Multiple scheduled statmachines can run at different frequencies as required
v0.0.0	2013-11-01	Craig Comberbach	Compiler: C30 v3.31		IDE: MPLABx 1.80	Tool: RealICE	Computer: Intel Xeon CPU 3.07 GHz, 6 GB RAM, Windows 7 64 bit Professional SP1
	First version
**************************************************************************************************/
/*************    Header Files    ***************/
#include "Config.h"
#include "I2C.h"

/************* Semantic Versioning***************/
#if I2C_MAJOR != 1
	#error "I2C.c has had a change that loses some previously supported functionality"
#elif I2C_MINOR != 0
	#error "I2C.c has new features that this code may benefit from"
#elif I2C_PATCH != 0
	#error "I2C.c has had a bug fix, you should check to see that we weren't relying on a bug for functionality"
#endif

/************Arbitrary Functionality*************/
#ifndef FCY_kHz
	#error "The system instruction speed (FCY_kHz) needs to be defined for this code to work correctly"
#endif

/*************   Magic  Numbers   ***************/
#define MODULE_IS_FREE		-1

/*************    Enumeration     ***************/
/***********State Machine Definitions*************/
static struct STATE_MACHINE
{
	uint8_t moduleToUse;					//The module that should be used
	uint16_t brg;							//The value to set the Baud rate generator to
	uint32_t callInterval_mS;				//Delay the call for n milliseconds, maxes out at almost 8 years 2 months
	uint32_t callIntervalCounter_mS;		//Counter for delay
	enum I2C_STATES currentState;			//The current state
	int (*function)(enum I2C_Module);		//The function to call in the interrupt that contains the state machine in question, returns a 1 if it needs more time, a 0 if it is finished
} stateMachine[NUMBER_OF_I2C_STATE_MACHINES];

/*************  Global Variables  ***************/
int8_t moduleLock[NUMBER_OF_I2C_MODULES];

/*************Function  Prototypes***************/
void __attribute__((interrupt, auto_psv)) _MI2C1Interrupt(void);

/************* Device Definitions ***************/
/************* Module Definitions ***************/
/************* Other  Definitions ***************/

void I2C_Handler(uint16_t time_mS)
{
	int8_t loop;

	//Update Countdown Timers
	for(loop = 0; loop < NUMBER_OF_I2C_STATE_MACHINES; ++loop)
	{
		if(stateMachine[loop].currentState != I2C_STOPPED)
		{
			stateMachine[loop].callIntervalCounter_mS += time_mS;//Increment it
			if(stateMachine[loop].callIntervalCounter_mS >= stateMachine[loop].callInterval_mS)
			{
				//Undelay it
				stateMachine[loop].callIntervalCounter_mS = 0;
				stateMachine[loop].currentState = I2C_RUNNING;
			}
		}
	}

	//Check if any statemachines need to run
	for(loop = 0; loop < NUMBER_OF_I2C_STATE_MACHINES; ++loop)
	{
		if(stateMachine[loop].currentState == I2C_RUNNING)
		{
			//Is the required module locked?
			if(moduleLock[stateMachine[loop].moduleToUse] != MODULE_IS_FREE)
				continue;
			else
			{
				moduleLock[stateMachine[loop].moduleToUse] = loop;
				I2C1BRG = stateMachine[loop].brg;
				stateMachine[loop].function(stateMachine[loop].moduleToUse);
			}
		}
	}

	return;
}

void Setup_I2C_State_Machine(enum I2C_Module module, enum I2C_STATE_MACHINE_LIST stateMachineNumber, uint16_t speed_kHz, unsigned long callInterval_mS, int (*functionI2C)(enum I2C_Module))
{
	uint16_t baudRate;

	//Error check and set the clock speed
	if(speed_kHz > FCY_kHz)
		speed_kHz = 100;
	baudRate = FCY_kHz / speed_kHz;
	baudRate -= FCY_kHz / 10000;
	baudRate -= 1;

	//Assign the Baud rate generator setting
	stateMachine[stateMachineNumber].brg = baudRate;

	//Assign the I2C module to use
	stateMachine[stateMachineNumber].moduleToUse = module;

	//Initialize the current state to the begin in
	stateMachine[stateMachineNumber].currentState = I2C_DELAYED;

	//Set the function to be called during interrupts
	stateMachine[stateMachineNumber].function = functionI2C;

	//Zero the delay
	stateMachine[stateMachineNumber].callInterval_mS = callInterval_mS;

	return;
}

void Initialize_I2C(void)
{
	int8_t loop;

	//Pad Configuration Control Register
	//PADCFG1			= ;

	//I2C Slave Mode Address Mask Register
	//I2C1MSK			= ;

	//I2C Status Register
	//I2C1STAT			= ;

	//Setup Interrupts
	IFS1bits.MI2C1IF = 0;		//0 = Interrupt request has not occurred
	IEC1bits.MI2C1IE = 1;		//1 = Interrupt request is enabled

	//I2C Control Register
	I2C1CONbits.SMEN	= 0;	//0 = Disables the SMBus input thresholds
	I2C1CONbits.DISSLW	= 0;	//0 = Slew rate control is enabled
	I2C1CONbits.A10M	= 0;	//0 = I2C1ADD is a 7-bit slave address
	I2C1CONbits.IPMIEN	= 0;	//0 = IPMI Support mode is disabled
	I2C1CONbits.SCLREL	= 1;	//1 = Releases SCL1 clock
	I2C1CONbits.I2CSIDL	= 0;	//0 = Continues module operation in Idle mode
	I2C1CONbits.I2CEN	= 1;	//1 = Enables the I2C1 module and configures the SDA1 and SCL1 pins as serial port pins

	//Initialize the lock
	for(loop = 0; loop < NUMBER_OF_I2C_MODULES; ++loop)
		moduleLock[loop] = MODULE_IS_FREE;

	return;
}

void I2C_Contention_Arbitration(enum I2C_Module module)
{
	//3.1.16 Bus clear of the Rev.5 of UM10204 (I2C-bus specification and user manual)
	//In the unlikely event where the clock (SCL) is stuck LOW, the preferential procedure is to
	//reset the bus using the HW reset signal if your I2C devices have HW reset inputs. If the
	//I2C devices do not have HW reset inputs, cycle power to the devices to activate the
	//mandatory internal Power-On Reset (POR) circuit.
	//If the data line (SDA) is stuck LOW, the master should send nine clock pulses. The device
	//that held the bus LOW should release it sometime within those nine clocks. If not, then
	//use the HW reset or cycle power to clear the bus

	//Send 9 clock signals (8 data + 1 ACK) with the data line held high
	I2C_Transmit(0xFF, module);

	return;
}

void I2C_Transmit(int8_t byte, enum I2C_Module module)
{
	I2C1TRN = byte;
	return;
}

void I2C_Start_Receive(enum I2C_Module module)
{
	I2C1CONbits.RCEN = 1;//Enable Receive mode, this register is cleared in hardware when done
	return;
}

int8_t I2C_Finish_Receive(enum I2C_Module module)
{
	return I2C1RCV;//Return Result
}

void I2C_Start(enum I2C_Module module)
{
	I2C1CONbits.SEN = 1;
	return;
}

void I2C_Repeated_Start(enum I2C_Module module)
{
	I2C1CONbits.RSEN = 1;
	return;
}

void I2C_Stop(enum I2C_Module module)
{
	I2C1CONbits.PEN = 1;
	return;
}

int8_t I2C_Check_If_Slave_Acknowledged(enum I2C_Module module)
{
	return (!I2C1STATbits.ACKSTAT);
}

void I2C_Ack(enum I2C_Module module)
{
	I2C1CONbits.ACKDT = 0;//Set for ACK
	I2C1CONbits.ACKEN = 1;//Perform Acknowledge Sequence
	return;
}

void I2C_Nack(enum I2C_Module module)
{
	I2C1CONbits.ACKDT = 1;//Set for NACK
	I2C1CONbits.ACKEN = 1;//Perform Acknowledge Sequence
	return;
}

void I2C_Disable_Module(enum I2C_Module module)
{
	I2C1CONbits.I2CEN = 0;
	return;
}

void I2C_Enable_Module(enum I2C_Module module)
{
	I2C1CONbits.I2CEN = 1;
	return;
}

void __attribute__((interrupt, auto_psv)) _MI2C1Interrupt(void)
{
	//The MI2CxIF interrupt is generated on completion of the following master message events:
	//Start condition
	//Stop condition
	//Data transfer byte transmitted/received
	//Acknowledge transmit
	//Repeated Start
	//Detection of a bus collision event
	IFS1bits.MI2C1IF = 0;	//Clear Interrupt Flag

	//TODO - I need some way of detecting which module just completed! Check if there is a flag set for each module
	if(*stateMachine[moduleLock[I2C1_MODULE]].function != (void*)0)
	{
		if(stateMachine[moduleLock[I2C1_MODULE]].function(I2C1_MODULE) == RELEASE_I2C_MODULE)
		{
			stateMachine[moduleLock[I2C1_MODULE]].currentState = I2C_DELAYED;
			moduleLock[I2C1_MODULE] = MODULE_IS_FREE;
		}
	}
	return;
}
