#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
using namespace std;
#include <string.h>
#include "mraa.hpp"
#include "C:\Users\Ricardo\eclipse-workspace\MQ Defines.h"
#include "MCP9808.hpp"
#include "TSL2591.hpp"

using namespace mraa;

union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

// structure for message queue
struct mesg_buffer QueueMessage;

uint8_t flag=0;
static int msgid;
static int sem_id;

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

void MQSetup(void);

void SensorInit (void);

void MQ_sub(string topic);

void MQ_pub(string topic, string content);

int sendMessage(void);

void check(int err);

static int semaphore_v(void);

static int semaphore_p(void);

void sigalrm_handler(int sig);

int main(int argc, char *argv[]){

	sem_id = semget((key_t)SEM_KEY, 1, 0666 | IPC_CREAT);

	MQSetup();
	SensorInit();

	float w;
	for (;;) {
		if(!(user_button ->read())){
			alarm(0);
			check(user_led -> write(0));
			MQ_sub("end");
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
			MQ_pub("Sensor/Temp/1/C", to_string(sens_temp -> readTempC()));
			MQ_pub("Sensor/Temp/2/C", to_string(sens_temp2 -> readTempC()));
			MQ_pub("Sensor/Lux/1/C", to_string(sens_light -> getLux()));
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

void MQSetup(void){

	// msgget creates a message queue
	// and returns identifier
	// msgget used to try to join with the queue from the Handler
	if (!semaphore_p()) exit(EXIT_FAILURE);
	msgid = msgget((key_t)MQ_KEY, 0666 | IPC_CREAT);
	if (msgid == -1) {
		fprintf(stderr, "msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	if (msgrcv(msgid, (void *)&QueueMessage, BUFSIZ, CONNECTION_OPEN, IPC_NOWAIT) == -1) {
		printf("Message Queue not opened yet\n"
			   "Opening Message Queue and Waiting for connection from Handler\n");
		QueueMessage.mesg_type=CONNECTION_OPEN;
		strcpy(QueueMessage.topic, "");
		strcpy(QueueMessage.content, "Welcome to Message Queue");

		if (msgsnd(msgid, (void *)&QueueMessage, MAX_TEXT, 0) == -1) {
			fprintf(stderr, "msgsnd failed\n");
			exit(EXIT_FAILURE);
		}
		if (!semaphore_v()) exit(EXIT_FAILURE);
		if (msgrcv(msgid, (void *)&QueueMessage, BUFSIZ, CONNECTION_ACK, 0) == -1) {
			fprintf(stderr, "msgrcv failed with error: %d\n", errno);
			exit(EXIT_FAILURE);
		}
	}else {
		printf("Message Queue already exists, joining it...\n");
		QueueMessage.mesg_type=CONNECTION_ACK;
		strcpy(QueueMessage.content, "Hi");

		if (msgsnd(msgid, (void *)&QueueMessage, MAX_TEXT, 0) == -1) {
			fprintf(stderr, "msgsnd failed\n");
			exit(EXIT_FAILURE);
		}
	}
	printf("Connection Established\n"
			"Main MQID: %d\n", msgid);
}

void SensorInit (void){

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
	MQ_sub("Sensor/Temp/1/Config");
	sens_temp2 = new MCP9808(0x19);
	MQ_sub("Sensor/Temp/2/Config");
	sens_temp -> setResolution(MCP9808_Resolution_Half);

	sens_light = new TSL2591();
	MQ_sub("Sensor/Lux/1/Config");

}

void MQ_sub(string topic)
{
	//Inform that it is a subscribe and to what topic
	QueueMessage.mesg_type=TYPE_SUB;

    strcpy(QueueMessage.content, "");
    strcpy(QueueMessage.topic, topic.c_str());
	sendMessage();
}

void MQ_pub(string topic, string content)
{
	//Inform that it is a publication and to what topic
	QueueMessage.mesg_type=TYPE_PUB;

	strcpy(QueueMessage.content, content.c_str());
	strcpy(QueueMessage.topic, topic.c_str());
	sendMessage();

}

int sendMessage(void) {
	if (msgsnd(msgid, (void *)&QueueMessage, MAX_TEXT, 0) == -1) {
		fprintf(stderr, "msgsnd failed\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}

void check(int err)
{
	if (err<0) {
		exit(-2);
	}
}


static int semaphore_v(void){

	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1; /* V() */
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_v failed\n");
		return(0);
	}
	return(1);
}

static int semaphore_p(void){

	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1; /* P() */
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_p failed\n");
		return(0);
	}
	return(1);
}

void sigalrm_handler(int sig)
{
	flag=1;
}
