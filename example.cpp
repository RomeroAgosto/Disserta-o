#include <iostream>
#include <stdio.h>
#include <signal.h>
using namespace std;
#include "mraa.hpp"
#include "MCP9808.hpp"
#include "TSL2591.hpp"

using namespace mraa;

uint8_t flag=0;


Gpio* user_button = NULL;
Gpio* ext_button = NULL;
Gpio* user_led = NULL;
Gpio* load = NULL;
Gpio* led = NULL;
Pwm* pwm = NULL;
Aio* a_pin = NULL;
MCP9808* sens_temp = NULL;
MCP9808* sens_temp2 = NULL;
//Spi* led_mat = NULL;

void sigalrm_handler(int sig) {
	flag=1;
}

void check(int err) {
	if (err<0) {
		exit(2);
	}
}

void setup () {

	signal(SIGALRM, &sigalrm_handler);
	alarm(1);

	user_button = new mraa::Gpio(63, true, true);
	user_button->dir(mraa::DIR_IN);

	ext_button = new mraa::Gpio(8);
	ext_button->dir(mraa::DIR_IN);

	user_led = new mraa::Gpio(1, true, false);
	user_led->dir(mraa::DIR_OUT);

	/*
	led_mat = new mraa::Spi(0);
	led_mat -> frequency(50000);
	uint16_t spi_word;
	*/

	load = new mraa::Gpio(9, true, false);
	load->dir(mraa::DIR_OUT);

	led = new mraa::Gpio(0);
	led->dir(mraa::DIR_OUT);

	pwm = new mraa::Pwm(3);

	a_pin = new mraa::Aio(0);

	TSL2591Init();

	sens_temp = new MCP9808();
	sens_temp -> setResolution(0);
	sens_temp2 = new MCP9808(0x19);

}
int main(void) {
	setup();

	float w;
	for (;;) {

		if(ext_button ->read()){
			sens_temp -> wake();
			sens_temp2 -> wake();
		}else {
			sens_temp -> shutdown();
			sens_temp2 -> shutdown();
		}
		if(!(user_button ->read())){
			alarm(0);
			check(user_led -> write(0));
			exit(1);
		}
		/*
		spi_word=0x10F0;
		load -> write(0);
		led_mat -> transfer_word(&spi_word, NULL, 2);
		load -> write(1);
		*/
		w=a_pin->readFloat();
		pwm -> write(w/0.67);
		check(led -> write(ext_button -> read()));


		if (flag) {

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
			cout << "-------------------------------------" << endl;
			alarm(1);
			flag=0;
		}

	}
	return 0;
}

