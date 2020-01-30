/*
Copyright (c) 2019 Jakub Mandula

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the â€œSoftwareâ€�), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED â€œAS ISâ€�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


/*
 * PZEM-004Tv30.h
 *
 * Interface library for the upgraded version of PZEM-004T v3.0
 * Based on the PZEM004T library by @olehs https://github.com/olehs/PZEM004T
 *
 * Author: Jakub Mandula https://github.com/mandulaj
 *
 *
*/


#ifndef PZEM004TV30_H
#define PZEM004TV30_H

#include "sdkconfig.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#elif defined(CONFIG_IDF_TARGET_ESP32)
#include "string.h"
#include <stdint.h>
#include "driver/uart.h"

typedef struct
{
	uart_port_t uart_port;
	int tx_io_num;
	int rx_io_num;
} uart_data_t;

static const int RX_BUF_SIZE = 1024;

#else
#include "WProgram.h"
#endif

typedef struct {
         float voltage;
         float current;
         float power;
         float energy;
         float frequency;
         float pf;
         uint16_t alarms;
     }  power_meansuare_t; // Measured values

// #define PZEM004_NO_SWSERIAL
#if (not defined(PZEM004_NO_SWSERIAL)) && (defined(__AVR__) || defined(ESP8266) && (not defined(CONFIG_IDF_TARGET_ESP32)))
#define PZEM004_SOFTSERIAL
#endif

#ifdef PZEM004_SOFTSERIAL
#include <SoftwareSerial.h>
#endif


#define PZEM_DEFAULT_ADDR    0xF8


class PZEM004Tv30
{

public:
#if defined(PZEM004_SOFTSERIAL)
    PZEM004Tv30(uint8_t receivePin, uint8_t transmitPin, uint8_t addr=PZEM_DEFAULT_ADDR);
#elif defined(CONFIG_IDF_TARGET_ESP32)
    PZEM004Tv30(uart_data_t *uart_data, uint8_t addr=PZEM_DEFAULT_ADDR);
#else
    PZEM004Tv30(HardwareSerial* port, uint8_t addr=PZEM_DEFAULT_ADDR);
#endif
    ~PZEM004Tv30();

    power_meansuare_t* meansures();
    float voltage();
    float current();
    float power();
    float energy();
    float frequency();
    float pf();


    bool setAddress(uint8_t addr);
    uint8_t getAddress();

    bool setPowerAlarm(uint16_t watts);
    bool getPowerAlarm();

    bool resetEnergy();

    void search();

private:

#ifndef CONFIG_IDF_TARGET_ESP32
    Stream* _serial; // Serial interface
#endif
    bool _isSoft;    // Is serial interface software

    uint8_t _addr;   // Device address
    uint64_t _lastRead; // Last time values were updated
    power_meansuare_t _currentValues;

    void init(uint8_t addr); // Init common to all constructors

    bool updateValues();    // Get most up to date values from device registers and cache them
    uint16_t recieve(uint8_t *resp, uint16_t len); // Receive len bytes into a buffer

    bool sendCmd8(uint8_t cmd, uint16_t rAddr, uint16_t val, bool check=false, uint16_t slave_addr=0xFFFF); // Send 8 byte command

    void setCRC(uint8_t *buf, uint16_t len);           // Set the CRC for a buffer
    bool checkCRC(const uint8_t *buf, uint16_t len);   // Check CRC of buffer

    uint16_t CRC16(const uint8_t *data, uint16_t len); // Calculate CRC of buffer

#ifdef CONFIG_IDF_TARGET_ESP32
    uart_data_t * _uart_data;

    char* copiaUint8Array(uint8_t * array);

#endif
};

#endif // PZEM004T_H
