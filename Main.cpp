#include <iostream>
#include <stdio.h>
#include <signal.h>
using namespace std;
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string.h>
#include "mraa.hpp"
#include "MCP9808.hpp"
#include "TSL2591.hpp"

#define TYPE_SUB 1
#define TYPE_PUB 2
#define TYPE_TOP 3
#define TYPE_MES 4

using namespace mraa;

uint8_t flag=0;
int msgid;

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

// structure for message queue
struct mesg_buffer {
    long mesg_type;
    char mesg_text[100];
} message;

void sub(string topic);

int sendMessage();

void check(int err);

void sigalrm_handler(int sig);

void setup ()
{
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

}

void MQTTSetup(int argc, char *argv[]){
	key_t key;

    if(argc==2)
    {
    	key = atoi(argv[1]);
    }else
	{
		// ftok to generate unique key
		key = ftok("progfile", 65);
    }

    // msgget creates a message queue
    // and returns identifier
    sleep(1);
    msgid = msgget(key, 0);

}

int main(int argc, char *argv[])
{
	setup();
	MQTTSetup(argc, argv);

	sub("Queue");
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

void sub(string topic) {
	//Inform that it is a subscribe and to what topic
	message.mesg_type=TYPE_SUB;

    uint i;
    for (i = 0; i < sizeof(topic); i++) {
        message.mesg_text[i] = topic[i];
    }
	sendMessage();
}

int sendMessage() {
	msgsnd(msgid, &message, sizeof(message), 0);
	return 0;
}

void check(int err)
{
	if (err<0) {
		exit(-2);
	}
}

void sigalrm_handler(int sig)
{
	flag=1;
}
