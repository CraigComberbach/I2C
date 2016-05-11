#ifndef I2C_H
#define	I2C_H
/***********Add to config file header************/
/*
//I2C Library
#define I2C_MAJOR	0
#define I2C_MINOR	1
#define I2C_PATCH	0
*/

/***************Add to config file***************/
/*
#ifndef I2C_LIBRARY
	#error "You need to include the I2C library for this code to compile"
#endif
 */

/************* Semantic Versioning***************/
#define I2C_LIBRARY

/*************   Magic  Numbers   ***************/
#define ACK		1
#define NACK	0

/*************    Enumeration     ***************/
enum I2C_STATES
{
	I2C_RUNNING,	//The state machine is currently free to run if the associated module is free
	I2C_STOPPED,	//The state machine is stopped indefinitely and will not run or use the associated module (Default state for all state machines)
	I2C_DELAYED		//The state machine is temporarily delayed, but will resume at a predetermined time in the future (Maximum of 8 years 2 months)
};

/***********State Machine Definitions************/
/*************Function  Prototypes***************/
void I2C_Routine(uint16_t time_mS);
void Initialize_I2C(enum I2C_Module module);
void Setup_I2C_State_Machine(enum I2C_Module module, enum I2C_STATE_MACHINE_LIST stateMachineNumber, uint16_t speed_kHz, unsigned long delayUntil, int (*functionI2C)(enum I2C_Module));
int8_t I2C_Busy(enum I2C_Module module);
void I2C_Write(int8_t byte, enum I2C_Module module);
void I2C_Start_Read(enum I2C_Module module);
int8_t I2C_Finish_Read(enum I2C_Module module);
void I2C_Start(enum I2C_Module module);
void I2C_Repeated_Start(enum I2C_Module module);
void I2C_Stop(enum I2C_Module module);
int8_t I2C_Slave_Acknowledge(enum I2C_Module module);
void I2C_Ack(enum I2C_Module module);
void I2C_Nack(enum I2C_Module module);
int8_t I2C_Status_RX_TX(enum I2C_Module module);
int8_t I2C_Error_Detected(enum I2C_Module module);
void I2C_Disable_Module(enum I2C_Module module);
void I2C_Contention_Arbitration(enum I2C_Module module);

#endif	/* I2C_H */

