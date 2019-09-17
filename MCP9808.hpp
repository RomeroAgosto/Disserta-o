#pragma once

#include "stdint.h"

#define MCP9808_REG_CONFIG 				(0x01)      ///< MCP9808 config register

#define MCP9808_REG_CONFIG_HYS_UPPER	(0x0400)	///< Upper Hysteresis bit
#define MCP9808_REG_CONFIG_HYS_LOWER	(0x0200)	///< Lowwer Hysteresis bit
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


typedef enum
{
	MCP9808_Resolution_Half             = 0x00, 	/// Sensor has a resolution of one half of a degree Celsius
	MCP9808_Resolution_Quarter          = 0x01, 	/// Sensor has a resolution of one quarter of a degree Celsius
	MCP9808_Resolution_Eighth           = 0x02, 	/// Sensor has a resolution of one eighth of a degree Celsius
	MCP9808_Resolution_Sixteenth        = 0x03, 	/// Sensor has a resolution of one sixteenth of a degree Celsius
}
MCP9808_Resolution_t;

class MCP9808 {
public:
	MCP9808(uint8_t address = MCP9808_I2CADDR_DEFAULT);

	float readTempC();
	float readTempF();

	void setResolution(MCP9808_Resolution_t value);
	MCP9808_Resolution_t getResolution();

	void shutdown_wake(bool sw);
	void shutdown();
	void wake();

private:
	uint8_t myAddress;
	MCP9808_Resolution_t myResolution;
	uint8_t read8 (uint8_t reg);
	uint16_t read16 (uint8_t reg);
	uint8_t getRealResolution();
};
