/*
 * DS1307RTC.h - library for DS1307 RTC
  
  Copyright (c) Michael Margolis 2009
  This library is intended to be uses with Arduino Time library functions

  The library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  30 Dec 2009 - Initial release
  5 Sep 2011 updated for Arduino 1.0
 */


#include "sdkconfig.h"
#define DS1307_CTRL_ID 0x68
#if defined(CONFIG_IDF_TARGET_ESP32)
#define  tmYearToY2k(Y)      ((Y) - 30)    // offset is from 2000
#define  y2kYearToTm(Y)      ((Y) + 30)
#define _I2C_NUMBER(num) I2C_NUM_##num
#define I2C_NUMBER(num) _I2C_NUMBER(num)

#define I2C_SLAVE_SCL_IO CONFIG_I2C_SLAVE_SCL               /*!< gpio number for i2c slave clock */
#define I2C_SLAVE_SDA_IO CONFIG_I2C_SLAVE_SDA               /*!< gpio number for i2c slave data */
#define I2C_SLAVE_NUM I2C_NUMBER(CONFIG_I2C_SLAVE_PORT_NUM) /*!< I2C port number for slave dev */
#define I2C_SLAVE_TX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave tx buffer size */
#define I2C_SLAVE_RX_BUF_LEN (2 * DATA_LENGTH)              /*!< I2C slave rx buffer size */

#define I2C_MASTER_SCL_IO CONFIG_I2C_MASTER_SCL               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO CONFIG_I2C_MASTER_SDA               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUMBER(CONFIG_I2C_MASTER_PORT_NUM) /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

#define DS1307RTC_CMD_START 0x00   /*!< Operation mode */
#define ESP_SLAVE_ADDR DS1307_CTRL_ID /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL I2C_MASTER_ACK                          /*!< I2C ack value */
#define NACK_VAL I2C_MASTER_NACK                            /*!< I2C nack value */
#endif


#if defined (__AVR_ATtiny84__) || defined(__AVR_ATtiny85__) || (__AVR_ATtiny2313__)
#include <TinyWireM.h>
#define Wire TinyWireM
#elif defined(CONFIG_IDF_TARGET_ESP32)
#include "driver/i2c.h"
#else
#include <Wire.h>
#endif
#include "DS1307RTC.h"



DS1307RTC::DS1307RTC()
{
#ifdef CONFIG_IDF_TARGET_ESP32
	i2c_master_init();
#else
  Wire.begin();
#endif
}
  
// PUBLIC FUNCTIONS
time_t DS1307RTC::get()   // Aquire data from buffer and convert to time_t
{
#ifdef CONFIG_IDF_TARGET_ESP32
  tm timeinfo;
  if (read(timeinfo) == false) return 0;
  return mktime(&timeinfo);
#else
  tmElements_t tm;
  if (read(tm) == false) return 0;
  return(makeTime(tm));
#endif

}

bool DS1307RTC::set(time_t t)
{
#ifdef CONFIG_IDF_TARGET_ESP32
  tm *timeinfo = localtime(&t);
  return write(*timeinfo);
#else
  tmElements_t tm;
  breakTime(t, tm);
  return write(tm); 
#endif
}

// Aquire data from the RTC chip in BCD format
#ifdef CONFIG_IDF_TARGET_ESP32
bool DS1307RTC::read(tm &timeinfo)
#else
bool DS1307RTC::read(tmElements_t &timeinfo)
#endif
{
	uint8_t sec;
#if CONFIG_IDF_TARGET_ESP32
	ESP_ERROR_CHECK(beginTransmission(DS1307_CTRL_ID));
#else
  Wire.beginTransmission(DS1307_CTRL_ID);
#endif

#if ARDUINO >= 100
  Wire.write((uint8_t)0x00);
#elif CONFIG_IDF_TARGET_ESP32
  ESP_ERROR_CHECK(i2c_master_write_slave(I2C_MASTER_NUM, 0x00, 1));
#else
  Wire.send(0x00);
  if (Wire.endTransmission() != 0) {
      exists = false;
      return false;
    }
#endif  
  exists = true;

#if CONFIG_IDF_TARGET_ESP32
  uint8_t data_rd[TM_NBR_FIELDS];
  ESP_ERROR_CHECK(i2c_master_read_slave(I2C_MASTER_NUM, data_rd, TM_NBR_FIELDS));
#else
  // request the 7 data fields   (secs, min, hr, dow, date, mth, yr)
  Wire.requestFrom(DS1307_CTRL_ID, tmNbrFields);
  if (Wire.available() < tmNbrFields) return false;
#endif

#if ARDUINO >= 100
  sec = Wire.read();
  timeinfo.Second = bcd2dec(sec & 0x7f);
  timeinfo.Minute = bcd2dec(Wire.read() );
  timeinfo.Hour =   bcd2dec(Wire.read() & 0x3f);  // mask assumes 24hr clock
  timeinfo.Wday = bcd2dec(Wire.read() );
  timeinfo.Day = bcd2dec(Wire.read() );
  timeinfo.Month = bcd2dec(Wire.read() );
  timeinfo.Year = y2kYearToTm((bcd2dec(Wire.read())));
#elif CONFIG_IDF_TARGET_ESP32
  sec = data_rd[0];
  timeinfo.tm_sec = bcd2dec(sec & 0x7f);
  timeinfo.tm_min = bcd2dec(data_rd[1]);
  timeinfo.tm_hour =   bcd2dec(data_rd[2] & 0x3f);  // mask assumes 24hr clock
  timeinfo.tm_wday = bcd2dec(data_rd[3] );
  timeinfo.tm_mday = bcd2dec(data_rd[4] );
  timeinfo.tm_mon = bcd2dec(data_rd[5]);
  timeinfo.tm_year = y2kYearToTm((bcd2dec(data_rd[6])));

#else
  sec = Wire.receive();
  timeinfo.Second = bcd2dec(sec & 0x7f);
  timeinfo.Minute = bcd2dec(Wire.receive() );
  timeinfo.Hour =   bcd2dec(Wire.receive() & 0x3f);  // mask assumes 24hr clock
  timeinfo.Wday = bcd2dec(Wire.receive() );
  timeinfo.Day = bcd2dec(Wire.receive() );
  timeinfo.Month = bcd2dec(Wire.receive() );
  timeinfo.Year = y2kYearToTm((bcd2dec(Wire.receive())));
#endif
  if (sec & 0x80) return false; // clock is halted
  return true;
}

#ifdef CONFIG_IDF_TARGET_ESP32
bool DS1307RTC::write(tm &tm)
#else
bool DS1307RTC::write(tmElements_t &tm)
#endif
{
  // To eliminate any potential race conditions,
  // stop the clock before writing the values,
  // then restart it after.
#ifdef CONFIG_IDF_TARGET_ESP32
	ESP_ERROR_CHECK(beginTransmission(DS1307_CTRL_ID));
#else
  Wire.beginTransmission(DS1307_CTRL_ID);
#endif
#if ARDUINO >= 100  
  Wire.write((uint8_t)0x00); // reset register pointer  
  Wire.write((uint8_t)0x80); // Stop the clock. The seconds will be written last
  Wire.write(dec2bcd(tm.Minute));
  Wire.write(dec2bcd(tm.Hour));      // sets 24 hour format
  Wire.write(dec2bcd(tm.Wday));   
  Wire.write(dec2bcd(tm.Day));
  Wire.write(dec2bcd(tm.Month));
  Wire.write(dec2bcd(tmYearToY2k(tm.Year))); 
#elif CONFIG_IDF_TARGET_ESP32
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN)); // reset register pointer
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x80, ACK_CHECK_EN)); // Stop the clock. The seconds will be written last
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(tm.tm_min), ACK_CHECK_EN));
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(tm.tm_hour), ACK_CHECK_EN));      // sets 24 hour format
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(tm.tm_wday), ACK_CHECK_EN));
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(tm.tm_mday), ACK_CHECK_EN));
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(tm.tm_mon), ACK_CHECK_EN));
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(tmYearToY2k(tm.tm_year)), ACK_CHECK_EN));
#else  
  Wire.send(0x00); // reset register pointer  
  Wire.send(0x80); // Stop the clock. The seconds will be written last
  Wire.send(dec2bcd(tm.Minute));
  Wire.send(dec2bcd(tm.Hour));      // sets 24 hour format
  Wire.send(dec2bcd(tm.Wday));   
  Wire.send(dec2bcd(tm.Day));
  Wire.send(dec2bcd(tm.Month));
  Wire.send(dec2bcd(tmYearToY2k(tm.Year)));   
#endif
#if CONFIG_IDF_TARGET_ESP32
  if (endTransmission(DS1307_CTRL_ID, cmd) != ESP_OK) {
	  return false;
  }
#else
  if (Wire.endTransmission() != 0) {
    exists = false;
    return false;
  }
#endif
  exists = true;

#ifdef CONFIG_IDF_TARGET_ESP32
	ESP_ERROR_CHECK(beginTransmission(DS1307_CTRL_ID));
#else
  // Now go back and set the seconds, starting the clock back up as a side effect
  Wire.beginTransmission(DS1307_CTRL_ID);
#endif
#if ARDUINO >= 100  
  Wire.write((uint8_t)0x00); // reset register pointer  
  Wire.write(dec2bcd(tm.Second)); // write the seconds, with the stop bit clear to restart
#elif CONFIG_IDF_TARGET_ESP32
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN)); // reset register pointer
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, dec2bcd(tm.tm_sec), ACK_CHECK_EN)); // write the seconds, with the stop bit clear to restart
#else
  Wire.send(0x00); // reset register pointer  
  Wire.send(dec2bcd(tm.Second)); // write the seconds, with the stop bit clear to restart
#endif

#if CONFIG_IDF_TARGET_ESP32
  if (endTransmission(DS1307_CTRL_ID, cmd) != ESP_OK) {
	  return false;
  }
#else
  if (Wire.endTransmission() != 0) {
    exists = false;
    return false;
  }
#endif
  exists = true;
  return true;
}

unsigned char DS1307RTC::isRunning()
{
#ifdef CONFIG_IDF_TARGET_ESP32
	beginTransmission(DS1307_CTRL_ID);
#else
	Wire.beginTransmission(DS1307_CTRL_ID);
#endif
#if ARDUINO >= 100  
  Wire.write((uint8_t)0x00);
#elif CONFIG_IDF_TARGET_ESP32
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, ACK_CHECK_EN)); // reset register pointer
#else
  Wire.send(0x00);
#endif  

#if CONFIG_IDF_TARGET_ESP32
  endTransmission(DS1307_CTRL_ID, cmd);
#else
  Wire.endTransmission();
#endif

  // Just fetch the seconds register and check the top bit
  Wire.requestFrom(DS1307_CTRL_ID, 1);
#if ARDUINO >= 100
  return !(Wire.read() & 0x80);
#else
  return !(Wire.receive() & 0x80);
#endif  
}

void DS1307RTC::setCalibration(char calValue)
{
  unsigned char calReg = abs(calValue) & 0x1f;
  if (calValue >= 0) calReg |= 0x20; // S bit is positive to speed up the clock
  Wire.beginTransmission(DS1307_CTRL_ID);
#if ARDUINO >= 100  
  Wire.write((uint8_t)0x07); // Point to calibration register
  Wire.write(calReg);
#else  
  Wire.send(0x07); // Point to calibration register
  Wire.send(calReg);
#endif
  Wire.endTransmission();  
}

char DS1307RTC::getCalibration()
{
  Wire.beginTransmission(DS1307_CTRL_ID);
#if ARDUINO >= 100  
  Wire.write((uint8_t)0x07); 
#else
  Wire.send(0x07);
#endif  
  Wire.endTransmission();

  Wire.requestFrom(DS1307_CTRL_ID, 1);
#if ARDUINO >= 100
  unsigned char calReg = Wire.read();
#else
  unsigned char calReg = Wire.receive();
#endif
  char out = calReg & 0x1f;
  if (!(calReg & 0x20)) out = -out; // S bit clear means a negative value
  return out;
}

// PRIVATE FUNCTIONS

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t DS1307RTC::dec2bcd(uint8_t num)
{
  return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t DS1307RTC::bcd2dec(uint8_t num)
{
  return ((num/16 * 10) + (num % 16));
}

bool DS1307RTC::exists = false;

DS1307RTC RTC = DS1307RTC(); // create an instance for the user

#if CONFIG_IDF_TARGET_ESP32
static esp_err_t beginTransmission(uint16_t address)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (address << 1) | READ_BIT, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, DS1307RTC_CMD_START, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret;
	ret = i2c_master_cmd_begin((i2c_port_t)I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return ret;
}

static esp_err_t endTransmission(uint16_t address, i2c_cmd_handle_t cmd)
{
	esp_err_t ret = i2c_master_cmd_begin(address, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return ret;
}


/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/**
 * @brief test code to read esp-i2c-slave
 *        We need to fill the buffer of esp slave device, then master can read them out.
 *
 * _______________________________________________________________________________________
 * | start | slave_addr + rd_bit +ack | read n-1 bytes + ack | read 1 byte + nack | stop |
 * --------|--------------------------|----------------------|--------------------|------|
 *
 */
static esp_err_t i2c_master_read_slave(i2c_port_t i2c_num, uint8_t *data_rd, size_t size)
{
    if (size == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    if (size > 1) {
        i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief Test code to write esp-i2c-slave
 *        Master device write data to slave(both esp32),
 *        the data will be stored in slave buffer.
 *        We can read them out from slave buffer.
 *
 * ___________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write n bytes + ack  | stop |
 * --------|---------------------------|----------------------|------|
 *
 */
static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (ESP_SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}
#endif
