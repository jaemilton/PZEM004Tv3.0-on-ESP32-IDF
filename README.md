ESP-IDF power meter with pzem004Tv3.0
====================

This is an application to be used with [Espressif IoT Development Framework](https://github.com/espressif/esp-idf).
Power metter using Peacefair PZEM-004T-10A and PZEM-004T-100A v3.0 Energy monitor.

***

This module is an upgraded version of the PZEM-004T with frequency and power factor measurement features, available at the usual places. It communicates using a TTL interface over a Modbus-RTU like communication protocol but is incompatible with the older [@olehs](https://github.com/olehs) library found here: [https://github.com/olehs/PZEM004T](https://github.com/olehs/PZEM004T). I would like to thank [@olehs](https://github.com/olehs) for the great library which inspired me to write this one.

#### Common issue:
Make sure the device is connected to the AC power! The 5V only power the optocouplers, not the actual chip. Also please be safe, AC is dangerous. It can cause more serious issues, such as *death*. You are responsible for your own stupidity. **So don't be stupid.**

### Manufacturer (optimistic) specifications

| Function      | Measuring range    | Resolution      | Accuracy | TODO: Realistic specifications |
|---------------|--------------------|-----------------|----------|--------------------------------|
| Voltage       | 80~260V            | 0.1V            | 0.5%     |                                |
| Current       | 0\~10A or 0\~100A*   | 0.01A or 0.02A* | 0.5%     |                                |
| Active power  | 0\~2.3kW or 0\~23kW* | 0.1W            | 0.5%     |                                |
| Active energy | 0~9999.99kWh       | 1Wh             | 0.5%     |                                |
| Frequency     | 45~65Hz            | 0.1Hz           | 0.5%     |                                |
| Power factor  | 0.00~1.00          | 0.01            | 1%       |                                |

\* Using the external current transformer instead of the built in shunt

#### Other features
  * 247 unique programmable slave addresses
    * Enables multiple slaves to use the same Serial interface
  * Over power alarm
  * Energy counter reset
  * CRC16 checksum
  * Better, but not perfect mains isolation
