#ifndef I2C_H
#define	I2C_H

/* How to use this library
 * Defining I2C_USE_MASTER or I2C_USE_SLAVE will allow the compilation of one, the other, or both
 * This was done to save space on smaller PICs that might not need the functionality of both slave and master
 */

/************* Semantic Versioning***************/
#define I2C_LIBRARY

/*************   Magic  Numbers   ***************/
/*************    Enumeration     ***************/
/***********State Machine Definitions************/
/*************Function  Prototypes***************/
//Master
/***/
int I2C_Initialize_Master(int module, int speedkHz);

/***/
int I2C_Master_Write(char byte, int module);

/***/
int I2C_Master_Read(int module);

/***/
int I2C_Master_Start(int module);

/***/
int I2C_Master_Stop(int module);

/***/
int I2C_Master_Repeated_Start(int module);

/***/
int I2C_Master_Acknowledge(int ackNack, int module);


//Slave
int I2C_Initialize_Slave(int module, int speedkHz);
int I2C_Slave_Write(char byte, int module);
int I2C_Slave_Read(int module);
int I2C_Slave_Start(int module);
int I2C_Slave_Stop(int module);
int I2C_Slave_Repeated_Start(int module);
int I2C_Slave_Acknowledge(int ackNack, int module);

#endif	/* I2C_H */

