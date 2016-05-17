#ifndef I2C_H
#define	I2C_H
/***********Add to config file header************/
/*
//I2C Library
#define I2C_MAJOR	1
#define I2C_MINOR	0
#define I2C_PATCH	0
*/

/***************Add to config file***************/
/*
//I2C Modules
enum I2C_Module
{
	I2C1_MODULE,
	NUMBER_OF_I2C_MODULES
};

//I2C Statemachines
enum I2C_STATE_MACHINE_LIST
{
	NUMBER_OF_I2C_STATE_MACHINES
};

#ifndef I2C_LIBRARY
	#error "You need to include the I2C library for this code to compile"
#endif
 */

/************* Semantic Versioning***************/
#define I2C_LIBRARY

/*************   Magic  Numbers   ***************/
#define ACK							1
#define NACK						0
#define RELEASE_I2C_MODULE			0
#define DO_NOT_RELEASE_I2C_MODULE	1

/*************    Enumeration     ***************/
enum I2C_STATES
{
	I2C_RUNNING,	//The state machine is currently free to run if the associated module is free
	I2C_STOPPED,	//The state machine is stopped indefinitely and will not run or use the associated module (Default state for all state machines)
	I2C_DELAYED		//The state machine is temporarily delayed, but will resume at a predetermined time in the future (Maximum of 8 years 2 months)
};

/***********State Machine Definitions************/
/*************Function  Prototypes***************/
/**
 * Setup and runs the statemachines for the modules involved, including the baudrate generator
 * @param time_mS The number of mS since it was last called
 */
void I2C_Handler(uint16_t time_mS);

/**
 * Sets up the I2C modules and initializes the I2C handler
 * Does not setup the Baud Rate generator
 */
void Initialize_I2C(void);

/**
 * Setup an externally specified I2C Statemachine
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 * @param stateMachineNumber The name of the statemachine that is to be setup, use enum I2C_STATE_MACHINE_LIST
 * @param speed_kHz The speed the module is to operate in kHz, the code will calcualte the values for the baudrate generator
 * @param callInterval_mS The initial delay, and the delay between the statemachine running
 * @param functionI2C The external function that contains the statemachine
 */
void Setup_I2C_State_Machine(enum I2C_Module module, enum I2C_STATE_MACHINE_LIST stateMachineNumber, uint16_t speed_kHz, unsigned long callInterval_mS, int (*functionI2C)(enum I2C_Module));

/**
 * A value that is to be transmitted for a specified module
 * @param byte The 8-bit byte that is to be transmitted
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 */
void I2C_Transmit(int8_t byte, enum I2C_Module module);

/**
 * Initiate a receive for a specified module
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 */
void I2C_Start_Receive(enum I2C_Module module);

/**
 * Read the received 8-bit message from the specified modules register
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 * @return 
 */
int8_t I2C_Finish_Receive(enum I2C_Module module);

/**
 * Intitate a start condition on a specified module
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 */
void I2C_Start(enum I2C_Module module);

/**
 * Intitate a repeated start condition on a specified module
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 */
void I2C_Repeated_Start(enum I2C_Module module);

/**
 * Intitate a stop condition on a specified module
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 */
void I2C_Stop(enum I2C_Module module);

/**
 * Check to see if a module has acknowledged a transmission
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 * @return Binary response:\n
 * 1 = Yes, message was acknowledged by the slave\n
 * 0 = No, message was not acknowledged by the slave
 */
int8_t I2C_Check_If_Slave_Acknowledged(enum I2C_Module module);

/**
 * Intitate an ACK condition on a specified module
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 */
void I2C_Ack(enum I2C_Module module);

/**
 * Intitate a NAK condition on a specified module
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 */
void I2C_Nack(enum I2C_Module module);

/**
 * Disable a specified module from use
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 */
void I2C_Disable_Module(enum I2C_Module module);

/**
 * Enable a specified module from use
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 */
void I2C_Enable_Module(enum I2C_Module module);

/**
 * In the event of a lockup (multi-master, unresponsive slave, etc) this will force all lines to be released
 * @param module The name of the module that is to be used for this statemachine, use enum I2C_Module
 */
void I2C_Contention_Arbitration(enum I2C_Module module);

#endif	/* I2C_H */

