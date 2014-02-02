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
	#warning "I2C.c has had a change that loses some previously supported functionality"
#elif I2C_MINOR != 1
	#warning "I2C.c has new features that this code may benefit from"
#elif I2C_PATCH != 0
	#warning "I2C.c has had a bug fix, you should check to see that we weren't relying on a bug for functionality"
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

int Initialize_I2C(enum I2C_Module module)
{

}
