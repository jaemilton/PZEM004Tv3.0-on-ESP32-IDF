/*
 * DS1307RTC.h - library for DS1307 RTC
 * This library is intended to be uses with Arduino Time library functions
 */

#ifndef DS1307RTC_h
#define DS1307RTC_h
#include "sdkconfig.h"
#ifdef CONFIG_IDF_TARGET_ESP32
#include <time.h>
#else
#include <TimeLib.h>
#endif

// library interface description
class DS1307RTC
{
  // user-accessible "public" interface
  public:
    DS1307RTC();
    static time_t get();
    static bool set(time_t t);
#ifdef CONFIG_IDF_TARGET_ESP32
    static bool read(tm &timeinfo);
    static bool write(tm &timeinfo);
#else
    static bool read(tmElements_t &tm);
    static bool write(tmElements_t &tm);
#endif
    static bool chipPresent() { return exists; }
    static unsigned char isRunning();
    static void setCalibration(char calValue);
    static char getCalibration();

  private:
    static bool exists;
    static uint8_t dec2bcd(uint8_t num);
    static uint8_t bcd2dec(uint8_t num);

#ifdef CONFIG_IDF_TARGET_ESP32

#endif
};

#ifdef RTC
#undef RTC // workaround for Arduino Due, which defines "RTC"...
#endif

extern DS1307RTC RTC;

#endif
 

