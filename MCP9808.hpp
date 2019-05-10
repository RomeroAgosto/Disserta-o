#pragma once

#include "stdint.h"

#define MCP9808_REG_CONFIG 				(0x01)      ///< MCP9808 config register

#define MCP9808_REG_CONFIG_SHUTDOWN 	(0x0100)   	///< shutdown config
#define MCP9808_REG_CONFIG_CRITLOCKED 	(0x0080) 	///< critical trip lock
#define MCP9808_REG_CONFIG_WINLOCKED 	(0x0040)  	///< alarm window lock
#define MCP9808_REG_CONFIG_INTCLR 		(0x0020)    ///< interrupt clear
#define MCP9808_REG_CONFIG_ALERTSTAT 	(0x0010)  	///< alert output status
#define MCP9808_REG_CONFIG_ALERTCTRL 	(0x0008)  	///< alert output control
#define MCP9808_REG_CONFIG_ALERTSEL 	(0x0004)   	///< alert output select
#define MCP9808_REG_CONFIG_ALERTPOL 	(0x0002)   	///< alert output polarity
#define MCP9808_REG_CONFIG_ALERTMODE 	(0x0001)  	///< alert output mode

#define MCP9808_REG_UPPER_TEMP 			(0x02)   	///< upper alert boundary
#define MCP9808_REG_LOWER_TEMP 			(0x03)   	///< lower alert boundary
#define MCP9808_REG_CRIT_TEMP 			(0x04)    	///< critical temperature
#define MCP9808_REG_AMBIENT_TEMP 		(0x05)	 	///< ambient temperature
#define MCP9808_REG_MANUF_ID 			(0x06)     	///< manufacture ID
#define MCP9808_REG_DEVICE_ID 			(0x07)    	///< device ID
#define MCP9808_REG_RESOLUTION 			(0x08)   	///< resolution

#define MCP9808_I2CADDR_DEFAULT			(0x18)		///< I2C address

void MCP9808Init(uint8_t address = MCP9808_I2CADDR_DEFAULT);

float readTempC(uint8_t address = MCP9808_I2CADDR_DEFAULT);

float readTempF(uint8_t address = MCP9808_I2CADDR_DEFAULT);

void shutdown_wake(bool sw);

void shutdown();

void wake();