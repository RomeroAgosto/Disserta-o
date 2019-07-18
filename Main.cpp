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

/*
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};
*/

// structure for message queue
struct mesg_buffer QueueMessage;

uint8_t flag=0;
static int msgid1, msgid2;

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

int receiveNMessage(int long n);

int receiveAnyMessage(void);

void check(int err);

void sigalrm_handler(int sig);

/*
static int set_semvalue(void);

static void del_semvalue(void);

static int semaphore_v(void);

static int semaphore_p(void);
*/

int main(int argc, char *argv[]){
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
			alarm(1);
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
			MQ_pub("Sensor/Temp/1/Val", to_string(sens_temp -> readTempC()));
			MQ_pub("Sensor/Temp/2/Val", to_string(sens_temp2 -> readTempC()));
			MQ_pub("Sensor/Lux/1/Val", to_string(sens_light -> getLux()));
			cout << "-------------------------------------" << endl;
			flag=0;
		}
	}

	return 0;
}

void MQSetup(void){

	// msgget creates a message queue
	// and returns identifier
	// msgget used to try to join with the queue from the Handler
	msgid1 = msgget((key_t)MQ_KEY1, 0666 | IPC_CREAT);
	if (msgid1 == -1) {
		fprintf(stderr, "msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	msgid2 = msgget((key_t)MQ_KEY2, 0666 | IPC_CREAT);
	if (msgid2 == -1) {
		fprintf(stderr, "msgget failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	printf("Connection Established\n"
			"Main MQID: %d	&	%d\n", msgid1, msgid2);
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
	if (msgsnd(msgid1, (void *)&QueueMessage, MAX_TEXT, 0) == -1) {
		fprintf(stderr, "msgsnd failed\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}

int receiveNMessage(int long n) {
	if (msgrcv(msgid2, (void *)&QueueMessage, BUFSIZ, n, 0) == -1) {
		fprintf(stderr, "msgrcv failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	return 0;
}

int receiveAnyMessage(void) {
	if (msgrcv(msgid2, (void *)&QueueMessage, BUFSIZ, 0, 0) == -1) {
		fprintf(stderr, "msgrcv failed with error: %d\n", errno);
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

void sigalrm_handler(int sig)
{
	flag=1;
}

/*
 * static int set_semvalue(void){

	union semun sem_union;
	sem_union.val = 1;
	if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
	return(1);
}

static void del_semvalue(void){

	union semun sem_union;
	if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
		fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_v(void){

	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
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
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_p failed\n");
		return(0);
	}
	return(1);
}
*/
