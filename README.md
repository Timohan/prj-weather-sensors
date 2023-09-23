# prj-weather-sensors
Weather sensor values send to own server

## Basic instructions

* ESP32 is chip
* BME280 reads temperature, humid, presure
* PSM5003 reads air quality
* All values from sensors are sent to private server via wifi (tcp/ip)

## Build and Installation

### Development Environment
https://github.com/espressif/esp-idf.git  
commit id: 3640dc86bb4b007da0c53500d90e318f0b7543ef

### Pre-build

On esp-idf folder after build/install:  
source ./export.sh

### Add own header "default_values.h
header file: "default_values.h" is missing because it contains private information

Add file default_values.h with this kind of source, but fix TCP_IP_ADDR, WIFI_SSID, WIFI_PASS:

#ifndef DEFAULT_VALUES_H  
#define DEFAULT_VALUES_H  

#define TCP_IP_ADDR     "192.168.1.xxx"  
#define TCP_IP_PORT     7000  

#define WIFI_SSID      "YourWifiSSID"  
#define WIFI_PASS      "YourwifiPassword"  

#endif // DEFAULT_VALUES_H

### Build
idf.py build

### Flash & Monitor
export PORT=/dev/ttyACM0  
idf.py -p $PORT flash  
idf.py -p $PORT monitor  

### To set this working without monitor on ESP32
You need to use RC Delay

3.3v -> 10k resistor -> EN pin (AND) 1uF capacitor to GND   


