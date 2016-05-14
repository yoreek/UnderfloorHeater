# Arduino based underfloor heater [![Build Status](https://travis-ci.org/yoreek/UnderfloorHeater.svg?branch=master)](https://travis-ci.org/yoreek/UnderfloorHeater)

The project is designed to control an heating system in order to keep the temperature of the floor above a certain value.

* Version: 1.0.0
* Release Date: 2016-05-14

## Web interface

![Web interface](https://github.com/yoreek/UnderfloorHeater/blob/master/doc/main.png)

![Schedule](https://github.com/yoreek/UnderfloorHeater/blob/master/doc/schedule.png)

## File server

![File server](https://github.com/yoreek/UnderfloorHeater/blob/master/doc/fs.png)

## Features

 * Multi-zone
 * Four modes - Off/Scheduled/Manual/Away
 * Web interface
 * Schedule for each day of week
 * Automaticaly update time via NTP protocol
 * Logging temperature and power consumption on SD card
 * File server

## Hardware

 * 1 Arduino Mega 2560
 * 1 Ethernet shield W5100
 * 1 SD card
 * 1 Thermistor per room
 * 1 Relay per room

## How do I get set up? ##

 * Download and Install [Time](https://github.com/yoreek/Time) library.
 * Download and Install [SdFat](https://github.com/greiman/SdFat) library.
 * Download and Install [DebugUtil](https://github.com/yoreek/Arduino-DebugUtil) library.
 * Download and Install [StringUtil](https://github.com/yoreek/Arduino-StringUtil) library.
 * Download and Install [Webduino](https://github.com/yoreek/Webduino) library.
 * Download and Install [Yudino](https://github.com/yoreek/Yudino) library.
 * Restart the Arduino Software
 * [Download](https://github.com/yoreek/Arduino-MyHeater/archive/master.zip) the Latest release from gitHub.
 * Unzip to any folder.
 * Run make to compile.

## Version History ##

 * 1.0.0 (2016-05-14): Initial version.


## Who do I talk to? ##

 * [Yoreek](https://github.com/yoreek)
