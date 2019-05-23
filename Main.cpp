#include <iostream>
#include <stdio.h>
#include <signal.h>
using namespace std;
#include "mraa.hpp"
#include "MCP9808.hpp"
#include "TSL2591.hpp"
#include <mosquittopp.h>

using namespace mraa;
using namespace mosqpp;

uint8_t flag=0;


Gpio* user_button = NULL;
Gpio* ext_button = NULL;
Gpio* user_led = NULL;
Gpio* load = NULL;
Gpio* led = NULL;
Gpio* DIPSW =NULL;
Pwm* pwm = NULL;
Aio* a_pin = NULL;
MCP9808* sens_temp = NULL;
MCP9808* sens_temp2 = NULL;
TSL2591* sens_light = NULL;
mosquittopp* mqtt = NULL;

void sigalrm_handler(int sig) {
	flag=1;
}

void check(int err) {
	if (err<0) {
		exit(-2);
	}
}

void setup () {

	signal(SIGALRM, &sigalrm_handler);
	alarm(1);

	user_button = new Gpio(63, true, true);
	user_button->dir(DIR_IN);

	ext_button = new Gpio(9);
	ext_button->dir(DIR_IN);

	DIPSW = new Gpio(8);
	DIPSW -> dir(DIR_IN);

	user_led = new Gpio(1, true, false);
	user_led->dir(DIR_OUT);

	load = new Gpio(9, true, false);
	load->dir(DIR_OUT);

	led = new Gpio(0);
	led->dir(DIR_OUT);

	pwm = new Pwm(3);

	a_pin = new Aio(0);

	sens_temp = new MCP9808();
	sens_temp2 = new MCP9808(0x19);
	sens_temp -> setResolution(MCP9808_Resolution_Half);

	sens_light = new TSL2591();

	mqtt = new mosquittopp();

	lib_init();											// Initialize libmosquitto

	mqtt -> connect("192.168.200.3", 1883, 120);		// Connect to MQTT Broker

}
int main(void) {
	setup();
	float w;
	for (;;) {
		if(!(user_button ->read())){
			alarm(0);
			check(user_led -> write(0));
			exit(1);
		}
		w=a_pin->readFloat();
		pwm -> write(w/0.67);
		check(led -> write(ext_button -> read()));


		if (flag) {
			if(DIPSW ->read()){
				sens_temp -> wake();
				sens_temp2 -> wake();
			}else {
				sens_temp -> shutdown();
				sens_temp2 -> shutdown();
			}

			if(user_led->read()){
				check(user_led->write(0));
			} else {
				check(user_led->write(1));
			}
			w=a_pin->readFloat();
			led -> write(w/0.67);
			w=w*5;
			printf("Analog Input = %4.2f V\n", w);
			printf("Sensor 1 :\nTemperature = %6.4f°C // %6.4f °F\n", sens_temp -> readTempC(), sens_temp -> readTempF());
			printf("Sensor 2 :\nTemperature = %6.4f°C // %6.4f °F\n", sens_temp2 -> readTempC(), sens_temp2 -> readTempF());
			printf("Lux = %f\n", sens_light -> getLux());
			cout << "-------------------------------------" << endl;
			alarm(1);
			flag=0;
		}

	}
	return 0;
}

