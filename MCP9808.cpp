#include "MCP9808.hpp"
#include "i2c.hpp"

mraa::I2c* sensor;

uint16_t read16 (uint8_t reg);

void MCP9808Init() {
	sensor = new mraa::I2c(0);
}

float readTempC() {
	float tempe;
	uint16_t temperature;
	sensor -> address(MCP9808_I2CADDR);

	temperature=read16(MCP9808_REG_AMBIENT_TEMP);

	temperature = temperature & 0x0FFF;
	  tempe= temperature / 16.0;
	  if (temperature & 0x1000)
	    tempe -= 256;
	  return tempe;
}

float readTempC(uint8_t address) {
	float tempe;
	uint16_t temperature;
	sensor -> address(address);

	temperature=read16(MCP9808_REG_AMBIENT_TEMP);

	temperature = temperature & 0x0FFF;
	  tempe= temperature / 16.0;
	  if (temperature & 0x1000)
	    tempe -= 256;
	  return tempe;
}

uint16_t read16 (uint8_t reg) {
	uint16_t value, tmp1, tmp2;
	value = sensor->readWordReg(reg);

	//Bytes need to be switched
	tmp1 = (value & 0x00ff)<<8;
	tmp2 = (value & 0xff00)>> 8;
	value = tmp1 | tmp2;
	return value;
}

