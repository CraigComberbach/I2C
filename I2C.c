/**************************************************************************************************
Target Hardware:		PIC24F...
Chip resources used:	I2C modules
Code assumptions:		Error handling is done by external code
						Only one I2C module is used
Purpose:				Allow access to interrupt/polling driven I2C resources
Notes:

Version History:
v0.1.0	2016-05-16	Craig Comberbach	Compiler: XC16 v1.11	IDE: MPLABx 3.30	Tool: ICD3		Computer: Intel Core2 Quad CPU 2.40 GHz, 5 GB RAM, Windows 10 Home 64-bit
v0.0.0	2013-11-01	Craig Comberbach	Compiler: C30 v3.31		IDE: MPLABx 1.80	Tool: RealICE	Computer: Intel Xeon CPU 3.07 GHz, 6 GB RAM, Windows 7 64 bit Professional SP1
	First version
**************************************************************************************************/
/*************    Header Files    ***************/
#include "Config.h"
#include "I2C.h"

/************* Semantic Versioning***************/
#if I2C_MAJOR != 0
	#error "I2C.c has had a change that loses some previously supported functionality"
#elif I2C_MINOR != 1
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
	uint32_t delayUntil_mS;					//Delay the call for n milliseconds, maxes out at almost 8 years 2 months
	uint32_t delayUntilCounter_mS;			//Counter for delay
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

void I2C_Routine(uint16_t time_mS)
{
	int8_t loop;

	//Update Countdown Timers
	for(loop = 0; loop < NUMBER_OF_I2C_STATE_MACHINES; ++loop)
	{
		if(stateMachine[loop].currentState == I2C_DELAYED)
		{
			stateMachine[loop].delayUntilCounter_mS += time_mS;//Increment it
			if(stateMachine[loop].delayUntilCounter_mS >= stateMachine[loop].delayUntil_mS)
			{
				//Undelay it
				stateMachine[loop].delayUntilCounter_mS = 0;
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

void Setup_I2C_State_Machine(enum I2C_Module module, enum I2C_STATE_MACHINE_LIST stateMachineNumber, uint16_t speed_kHz, unsigned long delayUntil, int (*functionI2C)(enum I2C_Module))
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
	stateMachine[stateMachineNumber].delayUntil_mS = delayUntil;

	return;
}

void Initialize_I2C(enum I2C_Module module)
{
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
	moduleLock[module] = MODULE_IS_FREE;

	return;
}

int8_t I2C_Busy(enum I2C_Module module)
{
	int8_t status = 0;

	if(I2C1CONbits.ACKEN)
		status += 1;//Ack
	if(I2C1CONbits.SEN)
		status += 2;//Start
	if(I2C1CONbits.RSEN)
		status += 4;//Repeated Start
	if(I2C1CONbits.PEN)
		status += 8;//Stop
	if(I2C1CONbits.RCEN)
		status += 16;//Receive
	if(I2C1STATbits.TRSTAT)
		status += 32;//Transmit

	return status;//0 indicates that the module isn't busy
}

void I2C_Contention_Arbitration(enum I2C_Module module)
{
	//This is based off of section 3.1.16 Bus clear of the Rev.5 of UM10204 (I2C-bus specification and user manual)
	static int8_t state[NUMBER_OF_I2C_MODULES];
	static int8_t born = 1;
	int8_t loop;

	//Initialize the starting state of all of the modules on the first run through
	if(born)
	{
		for(loop = 0; loop < NUMBER_OF_I2C_MODULES; ++loop)
			state[loop] = 0;
		born = 0;
	}

	//Go through the contention arbitration of the module
	switch(state[module])
	{
		case 0:
			//Send 9 clock signals (8 data + 1 ACK) with the data line held high
			I2C_Write(0xFF, module);
			break;
		case 1:
			//TODO - Write the code that allows this to resume
			break;
		default:
			state[module] = 0;
	}

	//Advance the state for when we get back here
	state[module]++;

	return;
}

void I2C_Write(int8_t byte, enum I2C_Module module)
{
	I2C1TRN = byte;
	return;
}

void I2C_Start_Read(enum I2C_Module module)
{
	I2C1CONbits.RCEN = 1;//Enable Receive mode, this register is cleared in hardware when done
	return;
}

int8_t I2C_Finish_Read(enum I2C_Module module)
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

int8_t I2C_Slave_Acknowledge(enum I2C_Module module)
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

int8_t I2C_Status_RX_TX(enum I2C_Module module)
{
	int8_t status = 0;

	//Transmit buffer full
	if(I2C1STATbits.TBF)
		status += 1;

	//Receive buffer full
	if(I2C1STATbits.RBF)
		status += 2;

	return status;
}

int8_t I2C_Error_Detected(enum I2C_Module module)
{
	int8_t status = 0;

	if(I2C1STATbits.BCL)
	{
		I2C1STATbits.BCL = 0;
		status += 1;//Master bus collision detected
	}

	if(I2C1STATbits.IWCOL)
	{
		I2C1STATbits.IWCOL = 0;
		status += 2;//Write collision detected
	}

	if(I2C1STATbits.I2COV)
	{
		I2C1STATbits.I2COV = 0;
		status += 4;//Receive Overflow detected
	}

	return status;
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
