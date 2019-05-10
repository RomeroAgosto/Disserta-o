#include "TSL2591.hpp"
#include "i2c.hpp"

mraa::I2c* TSL2591Sensor;

void TSL2591Init() {
  TSL2591Sensor = new mraa::I2c(0);
  TSL2591Sensor -> address(0x29);
  if (TSL2591Sensor -> readReg(TSL2591_COMMAND_BIT | TSL2591_REGISTER_DEVICE_ID) != 0x50)
    exit(1);
}