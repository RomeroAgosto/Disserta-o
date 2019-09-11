#include "MCP9808.hpp"
#include "i2c.hpp"
#include "unistd.h"
#include "byteswap.h"

mraa::I2c* MCP9808Sensor;

MCP9808::MCP9808(uint8_t address) {
	MCP9808Sensor = new mraa::I2c(0);
	MCP9808Sensor -> address(address);
	myAddress = address;
	myResolution = MCP9808_Resolution_Sixteenth;
	if (read16(MCP9808_REG_MANUF_ID) != 0x0054)
		exit(-1);
	if (read16(MCP9808_REG_DEVICE_ID) != 0x0400)
		exit(-1);

	MCP9808Sensor -> writeWordReg(MCP9808_REG_CONFIG, 0x0);
}

float MCP9808::readTempC() {
	float tempe;
	uint16_t temperature;

	temperature=read16(MCP9808_REG_AMBIENT_TEMP);

	temperature = temperature & 0x0FFF;
	  tempe= temperature / 16.0;
	  if (temperature & 0x1000)
	    tempe -= 256;
	  return tempe;
}
float MCP9808::readTempF() {
	return MCP9808::readTempC() * 9.0 / 5.0 + 32;
}

void MCP9808::setResolution(MCP9808_Resolution_t value) {
	MCP9808Sensor -> address(myAddress);
	MCP9808Sensor -> writeReg(MCP9808_REG_RESOLUTION, value);
	myResolution = value;
}

MCP9808_Resolution_t MCP9808::getResolution() {
	if(getRealResolution()==myResolution) {
  		return myResolution;
	} else {
		exit(-1);	
	}
}

void MCP9808::shutdown_wake(bool sw) {
  uint16_t tmp_shutdown;
  uint16_t tmp_register = read16(MCP9808_REG_CONFIG);
  MCP9808Sensor -> address(myAddress);
  if (sw == true) {
    tmp_shutdown = tmp_register | MCP9808_REG_CONFIG_SHUTDOWN;
    MCP9808Sensor -> writeWordReg(MCP9808_REG_CONFIG, tmp_shutdown);
  }
  if (sw == false) {
    tmp_shutdown = tmp_register & ~MCP9808_REG_CONFIG_SHUTDOWN;
    MCP9808Sensor -> writeWordReg(MCP9808_REG_CONFIG, tmp_shutdown);
  }
}

void MCP9808::shutdown() { MCP9808::shutdown_wake(false); }

void MCP9808::wake() {
  MCP9808::shutdown_wake(true);
  usleep(250);
}

uint8_t MCP9808::read8 (uint8_t reg) {
	MCP9808Sensor -> address(myAddress);
	return MCP9808Sensor -> readReg(reg);
}

uint16_t MCP9808::read16 (uint8_t reg) {
	uint16_t value;
	MCP9808Sensor -> address(myAddress);
	value = MCP9808Sensor->readWordReg(reg);

	//Bytes need to be switched

	return bswap_16(value);
}

uint8_t MCP9808::getRealResolution() {
	return read8(MCP9808_REG_RESOLUTION);
}
