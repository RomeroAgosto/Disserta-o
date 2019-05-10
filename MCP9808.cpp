#include "MCP9808.hpp"
#include "i2c.hpp"
#include "unistd.h"

mraa::I2c* MCP9808Sensor;

uint16_t read16 (uint8_t reg);

void MCP9808Init(uint8_t address) {
	MCP9808Sensor = new mraa::I2c(0);
	MCP9808Sensor -> address(address);
	if (read16(MCP9808_REG_MANUF_ID) != 0x0054)
		exit(1);
	if (read16(MCP9808_REG_DEVICE_ID) != 0x0400)
		exit(1);

	MCP9808Sensor -> writeWordReg(MCP9808_REG_CONFIG, 0x0);
}

float readTempC(uint8_t address) {
	float tempe;
	uint16_t temperature;
	MCP9808Sensor -> address(address);

	temperature=read16(MCP9808_REG_AMBIENT_TEMP);

	temperature = temperature & 0x0FFF;
	  tempe= temperature / 16.0;
	  if (temperature & 0x1000)
	    tempe -= 256;
	  return tempe;
}
float readTempF(uint8_t address) {
	return readTempC(address) * 9.0 / 5.0 + 32;
}

void shutdown_wake(bool sw) {
  uint16_t conf_shutdown;
  uint16_t conf_register = read16(MCP9808_REG_CONFIG);
  if (sw == false) {
    conf_shutdown = conf_register | MCP9808_REG_CONFIG_SHUTDOWN;
    MCP9808Sensor -> writeWordReg(MCP9808_REG_CONFIG, conf_shutdown);
  }
  if (sw == true) {
    conf_shutdown = conf_register & ~MCP9808_REG_CONFIG_SHUTDOWN;
    MCP9808Sensor -> writeWordReg(MCP9808_REG_CONFIG, conf_shutdown);
  }
}

/*!
 *   @brief  Shutdown MCP9808
 */
void shutdown() { shutdown_wake(false); }

/*!
 *   @brief  Wake up MCP9808
 */
void wake() {
  shutdown_wake(true);
  usleep(250);
}

uint16_t read16 (uint8_t reg) {
	uint16_t value, tmp1, tmp2;
	value = MCP9808Sensor->readWordReg(reg);

	//Bytes need to be switched
	tmp1 = (value & 0x00ff)<<8;
	tmp2 = (value & 0xff00)>> 8;
	value = tmp1 | tmp2;
	return value;
}
