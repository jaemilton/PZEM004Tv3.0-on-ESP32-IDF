/*
 * i2c.h
 *
 *  Created on: Feb 16, 2020
 *      Author: jaemi
 */

#ifndef MAIN_INCLUDE_I2C_H_
#define MAIN_INCLUDE_I2C_H_
#include "sdkconfig.h"

#define STICKBREAKER 'V1.1.0'
#define I2C_BUFFER_LENGTH 128
typedef void(*user_onRequest)(void);
typedef void(*user_onReceive)(uint8_t*, int);

class i2c {
protected:
    uint8_t num;
    int8_t sda;
    int8_t scl;
    i2c_t * i2c;

    uint8_t rxBuffer[I2C_BUFFER_LENGTH];
    uint16_t rxIndex;
    uint16_t rxLength;
    uint16_t rxQueued; //@stickBreaker

    uint8_t txBuffer[I2C_BUFFER_LENGTH];
    uint16_t txIndex;
    uint16_t txLength;
    uint16_t txAddress;
    uint16_t txQueued; //@stickbreaker

    uint8_t transmitting;
    /* slave Mode, not yet Stickbreaker
            static user_onRequest uReq[2];
            static user_onReceive uRcv[2];
        void onRequestService(void);
        void onReceiveService(uint8_t*, int);
    */
    i2c_err_t last_error; // @stickBreaker from esp32-hal-i2c.h
    uint16_t _timeOutMillis;

public:
	i2c(uint8_t bus_num);
	virtual ~i2c();
	bool begin(int sda=-1, int scl=-1, uint32_t frequency=0); // returns true, if successful init of i2c bus
	// calling will attemp to recover hung bus

	void setClock(uint32_t frequency); // change bus clock without initing hardware
	size_t getClock(); // current bus clock rate in hz

	void setTimeOut(uint16_t timeOutMillis); // default timeout of i2c transactions is 50ms
	uint16_t getTimeOut();

	uint8_t lastError();
	char * getErrorText(uint8_t err);

	//@stickBreaker for big blocks and ISR model
	i2c_err_t writeTransmission(uint16_t address, uint8_t* buff, uint16_t size, bool sendStop=true);
	i2c_err_t readTransmission(uint16_t address, uint8_t* buff, uint16_t size, bool sendStop=true, uint32_t *readCount=NULL);

	void beginTransmission(uint16_t address);
	void beginTransmission(uint8_t address);
	void beginTransmission(int address);

	uint8_t endTransmission(bool sendStop);
	uint8_t endTransmission(void);

	uint8_t requestFrom(uint16_t address, uint8_t size, bool sendStop);
	uint8_t requestFrom(uint16_t address, uint8_t size, uint8_t sendStop);
	uint8_t requestFrom(uint16_t address, uint8_t size);
	uint8_t requestFrom(uint8_t address, uint8_t size, uint8_t sendStop);
	uint8_t requestFrom(uint8_t address, uint8_t size);
	uint8_t requestFrom(int address, int size, int sendStop);
	uint8_t requestFrom(int address, int size);

	size_t write(uint8_t);
	size_t write(const uint8_t *, size_t);
	int available(void);
	int read(void);
	int peek(void);
	void flush(void);

	inline size_t write(const char * s)
	{
		return write((uint8_t*) s, strlen(s));
	}
	inline size_t write(unsigned long n)
	{
		return write((uint8_t)n);
	}
	inline size_t write(long n)
	{
		return write((uint8_t)n);
	}
	inline size_t write(unsigned int n)
	{
		return write((uint8_t)n);
	}
	inline size_t write(int n)
	{
		return write((uint8_t)n);
	}

	void onReceive( void (*)(int) );
	void onRequest( void (*)(void) );

	uint32_t setDebugFlags( uint32_t setBits, uint32_t resetBits);
	bool busy();
};

#endif /* MAIN_INCLUDE_I2C_H_ */
