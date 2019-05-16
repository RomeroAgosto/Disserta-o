#include "TSL2591.hpp"
#include "i2c.hpp"
#include "unistd.h"

mraa::I2c* TSL2591Sensor;

TSL2591::TSL2591() {
  TSL2591Sensor = new mraa::I2c(0);
  TSL2591Sensor -> address(0x29);
  myIntegration = TSL2591_INTEGRATIONTIME_100MS;
  myGain = TSL2591_GAIN_MED;
  if (TSL2591Sensor -> readReg(TSL2591_COMMAND_BIT | TSL2591_REGISTER_DEVICE_ID) != 0x50)
    exit(-1);

  setTiming(myIntegration);
  setGain(myGain);

}

void TSL2591::setTiming(tsl2591IntegrationTime_t integration) {
  wake();
  myIntegration = integration;
  TSL2591Sensor -> address(0x29);
  TSL2591Sensor -> writeReg(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONTROL, myIntegration | myGain);
  shutdown();
}

tsl2591IntegrationTime_t TSL2591::getTiming() {
  return myIntegration;
}

void TSL2591::setGain(tsl2591Gain_t gain) {
  wake();
  myGain = gain;
  TSL2591Sensor -> address(0x29);
  TSL2591Sensor -> writeReg(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONTROL, myIntegration | myGain);
  shutdown();
}

tsl2591Gain_t TSL2591::getGain() {
  return myGain;
}


float TSL2591::getLux() {
  uint32_t lum = getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  return calculateLux(full, ir);
}

float TSL2591::calculateLux(uint16_t ch0, uint16_t ch1) {
  float    atime, again;
  float    cpl, lux;
  //float lux1, lux2;

  // Check for overflow conditions first
  if ((ch0 == 0xFFFF) | (ch1 == 0xFFFF))
  {
    // Signal an overflow
    return 0;
  }

  // Note: This algorithm is based on preliminary coefficients
  // provided by AMS and may need to be updated in the future

  switch (myIntegration) {
    case TSL2591_INTEGRATIONTIME_100MS :
      atime = 100.0F;
      break;
    case TSL2591_INTEGRATIONTIME_200MS :
      atime = 200.0F;
      break;
    case TSL2591_INTEGRATIONTIME_300MS :
      atime = 300.0F;
      break;
    case TSL2591_INTEGRATIONTIME_400MS :
      atime = 400.0F;
      break;
    case TSL2591_INTEGRATIONTIME_500MS :
      atime = 500.0F;
      break;
    case TSL2591_INTEGRATIONTIME_600MS :
      atime = 600.0F;
      break;
    default: // 100ms
      atime = 100.0F;
      break;
  }

  switch (myGain) {
    case TSL2591_GAIN_LOW :
      again = 1.0F;
      break;
    case TSL2591_GAIN_MED :
      again = 25.0F;
      break;
    case TSL2591_GAIN_HIGH :
      again = 428.0F;
      break;
    case TSL2591_GAIN_MAX :
      again = 9876.0F;
      break;
    default:
      again = 1.0F;
      break;
  }

  // cpl = (ATIME * AGAIN) / DF
  cpl = (atime * again) / TSL2591_LUX_DF;

  // Original lux calculation (for reference sake)
  //lux1 = ( (float)ch0 - (TSL2591_LUX_COEFB * (float)ch1) ) / cpl;
  //lux2 = ( ( TSL2591_LUX_COEFC * (float)ch0 ) - ( TSL2591_LUX_COEFD * (float)ch1 ) ) / cpl;
  //lux = lux1 > lux2 ? lux1 : lux2;

  // Alternate lux calculation 1
  // See: https://github.com/adafruit/Adafruit_TSL2591_Library/issues/14
  lux = ( ((float)ch0 - (float)ch1 )) * (1.0F - ((float)ch1/(float)ch0) ) / cpl;

  // Alternate lux calculation 2
  //lux = ( (float)ch0 - ( 1.7F * (float)ch1 ) ) / cpl;

  return lux;
}

uint32_t TSL2591::getFullLuminosity (void) {
  // Enable the device
  wake();

  // Wait x ms for ADC to complete
  for (uint8_t d=0; d<=myIntegration; d++)
  {
    usleep(120000);
  }

  // CHAN0 must be read before CHAN1
  // See: https://forums.adafruit.com/viewtopic.php?f=19&t=124176
  uint32_t x;
  uint16_t y;
  TSL2591Sensor -> address(0x29);
  y = TSL2591Sensor -> readWordReg(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW);
  TSL2591Sensor -> address(0x29);
  x = TSL2591Sensor -> readWordReg(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN1_LOW);
  x <<= 16;
  x |= y;

  shutdown();

  return x;
}

uint16_t TSL2591::getLuminosity (uint8_t channel){
  uint32_t x = getFullLuminosity();

  if (channel == TSL2591_FULLSPECTRUM)
  {
    // Reads two byte value from channel 0 (visible + infrared)
    return (x & 0xFFFF);
  }
  else if (channel == TSL2591_INFRARED)
  {
    // Reads two byte value from channel 1 (infrared)
    return (x >> 16);
  }
  else if (channel == TSL2591_VISIBLE)
  {
    // Reads all and subtracts out just the visible!
    return ( (x & 0xFFFF) - (x >> 16));
  }

  // unknown channel!
  return 0;
}

void TSL2591::wake() {
  // Enable the device by setting the control bit to 0x01
  TSL2591Sensor -> address(0x29);
  TSL2591Sensor -> writeReg(TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE,
   TSL2591_ENABLE_POWERON | TSL2591_ENABLE_AEN | TSL2591_ENABLE_AIEN | TSL2591_ENABLE_NPIEN);
}

void TSL2591::shutdown() {
  // Disable the device by setting the control bit to 0x00
  TSL2591Sensor -> address(0x29);
  TSL2591Sensor -> writeReg(TSL2591_COMMAND_BIT | TSL2591_REGISTER_ENABLE, TSL2591_ENABLE_POWEROFF);
}
