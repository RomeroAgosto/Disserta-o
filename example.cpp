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
	if (err<0) {
		exit(2);
	}
}

uint16_t reverse_int16(uint16_t v) {

	uint16_t r = v; // r will be reversed bits of v; first get LSB of v
	int s = sizeof(v) * 8 - 1; // extra shift needed at end

	for (v >>= 1; v; v >>= 1)
	{
	  r <<= 1;
	  r |= v & 1;
	  s--;
	}
	r <<= s; // shift when v's highest bits are zero
	return r;
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
	user_led = new mraa::Gpio(1, true, false);
	user_led->dir(mraa::DIR_OUT);

	/*
	Spi* led_mat = NULL;
	led_mat = new mraa::Spi(0);
	led_mat -> frequency(50000);
	uint16_t spi_word;
	*/

	Gpio* load = NULL;
	load = new mraa::Gpio(9, true, false);
	load->dir(mraa::DIR_OUT);

	Gpio* led = NULL;
	led = new mraa::Gpio(0);
	led->dir(mraa::DIR_OUT);

	Pwm* pwm = NULL;
	pwm = new mraa::Pwm(3);

	Aio* a_pin = NULL;
	a_pin = new mraa::Aio(0);

	float w;

	mraa::I2c* temp;
	temp = new mraa::I2c(0);
	temp -> address(0x18);

	uint16_t temperature, temp1, temp2;
	float tempe;

	for (;;) {

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

			temperature = temp -> readWordReg(0b00000101);

			//temperature = reverse_int16(temperature);
			temp1 = (temperature & 0x00ff)<<8;
			temp2 = (temperature & 0xff00)>> 8;
			temperature = temp1 | temp2;

			temperature = temperature & 0x0FFF;
			  tempe= temperature / 16.0;
			  if (temperature & 0x1000)
			    tempe -= 256;
			  printf("Current Temperature is %4.2f C\n", tempe);

			alarm(1);
			flag=0;
		}

	}
	return 0;
}

