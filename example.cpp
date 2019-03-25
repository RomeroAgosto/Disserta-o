#include <iostream>
using namespace std;
#include "mraa.hpp"


using namespace mraa;


int main(void) {
	cout << "Hello IOT2020." << endl;

	Gpio* d_pin = NULL;
	d_pin = new mraa::Gpio(13, true, false);
	d_pin->dir(mraa::DIR_OUT);

	Aio* a_pin = NULL;
	a_pin = new mraa::Aio(0);

	unsigned int w;
	for (;;) {
			d_pin->write(1);
			sleep(1);
			d_pin->write(0);
			sleep(1);
			w=a_pin->read();
			cout << w << endl;

	}
	return 0;
}

