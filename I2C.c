/**************************************************************************************************
Target Hardware:		PIC24F
Chip resources used:	
Code assumptions:		
Purpose:				Allow access to interrupt/polling driven I2C resources
Notes:					

Version History:
v0.0.0	2013-11-1	Craig Comberbach	Compiler: C30 v3.31	IDE: MPLABx 1.80	Tool: RealICE	Computer: Intel Xeon CPU 3.07 GHz, 6 GB RAM, Windows 7 64 bit Professional SP1
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
/*************   Magic  Numbers   ***************/
/*************    Enumeration     ***************/
/***********State Machine Definitions*************/
/*************  Global Variables  ***************/
/*************Function  Prototypes***************/
/************* Device Definitions ***************/
/************* Module Definitions ***************/
/************* Other  Definitions ***************/

#ifdef I2C_USE_MASTER
	int I2C_Initialize_Master(int module, int speedkHz)
	{
		unsigned long BRG;
		int CON = 0;
		int I2CEN = 1;

		//Calculate Baud rate
		BRG = FOSC_HZ / 2;
		BRG /= speedkHz * 1000;
		BRG -= (FOSC_HZ / 2) / 10000000;
		BRG--;

		//Assign module settings
		switch(module)
		{
			default:
				return 0;//I2C module doesn't exist
		}

		return 1;
	}

	int I2C_Master_Write(char byte, int module)
	{
		//Choose a module
		switch(module)
		{
			default:
				return 0;//I2C module doesn't exist
		}

		return 1;
	}

	int I2C_Master_Read(int module)
	{
		//Choose a module
		switch(module)
		{
			default:
				return 0;//I2C module doesn't exist
		}
	}

	int I2C_Master_Start(int module)
	{}

	int I2C_Master_Stop(int module)
	{}

	int I2C_Master_Repeated_Start(int module)
	{}

	int I2C_Master_Acknowledge(int ackNack, int module)
	{
		//Range Check
		if((ackNack != 0) && (ackNack != 1))
			return 0;//Only Acks and Nack allowed

		//Choose a module
		switch(module)
		{
			default:
				return 0;//I2C module doesn't exist
		}

		return 1;
	}
#endif

#ifdef I2C_USE_SLAVE
	int I2C_Initialize_Slave(int module, int speedkHz)
	{
		unsigned long BRG;
		int CON = 0;
		int I2CEN = 1;

		//Calculate Baud rate
		BRG = FOSC_HZ / 2;
		BRG /= speedkHz * 1000;
		BRG -= (FOSC_HZ / 2) / 10000000;
		BRG--;

		//Assign module settings
		switch(module)
		{
			default:
				return 0;//I2C module doesn't exist
		}

		return 1;
	}

	int I2C_Slave_Write(char byte, int module)
	{
		//Choose a module
		switch(module)
		{
			default:
				return 0;//I2C module doesn't exist
		}

		return 1;
	}

	int I2C_Slave_Read(int module)
	{
		//Choose a module
		switch(module)
		{
			default:
				return 0;//I2C module doesn't exist
		}
	}

	int I2C_Slave_Start(int module)
	{}

	int I2C_Slave_Stop(int module)
	{}

	int I2C_Slave_Repeated_Start(int module)
	{}

	int I2C_Slave_Acknowledge(int ackNack, int module)
	{
		//Range Check
		if((ackNack != 0) && (ackNack != 1))
			return 0;//Only Acks and Nack allowed

		//Choose a module
		switch(module)
		{
			default:
				return 0;//I2C module doesn't exist
		}

		return 1;
	}
#endif
