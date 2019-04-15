#include <iostream>
#include <stdio.h>
#include <signal.h>
using namespace std;
#include "mraa.hpp"

using namespace mraa;

volatile int flag=0;

void sigalrm_handler(int sig) {
	flag=1;
}

void check(int err) {
	if (err!=0) {
		exit(2);
	}
}
int main(void) {
	cout << "Hello IOT2020." << endl;

	signal(SIGALRM, &sigalrm_handler);
	alarm(1);

	Gpio* user_button = NULL;
	user_button = new mraa::Gpio(63, true, true);
	user_button->dir(mraa::DIR_IN);

	Gpio* ext_button = NULL;
	ext_button = new mraa::Gpio(8);
	ext_button->dir(mraa::DIR_IN);

	Gpio* user_led = NULL;
	user_led = new mraa::Gpio(13, true, false);
	user_led->dir(mraa::DIR_OUT);

	Gpio* led = NULL;
	led = new mraa::Gpio(10);
	led->dir(mraa::DIR_OUT);

	Pwm* pwm = NULL;
	pwm = new mraa::Pwm(9);

	Aio* a_pin = NULL;
	a_pin = new mraa::Aio(0);

	float w;

/*
	mraa::I2c* temp;
	temp = new mraa::I2c(0);
	temp -> frequency(I2C_STD);
	temp -> address(0x4d);

	uint8_t temperature;
*/
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

			if(user_led->read()){
				check(user_led->write(0));
			} else {
				check(user_led->write(1));
			}
			w=a_pin->readFloat();
			led -> write(w/0.67);
			w=w*5;
			printf("%4.2f V\n", w);

/*
			temperature = temp -> readReg(0x00);
			cout << temperature <<endl;
*/
			alarm(1);
			flag=0;
		}

	}
	return 0;
}

