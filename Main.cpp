/*
 * To Exit program send EXIT to MainControl
 *
 */
#include <iostream>
#include <chrono>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include "mraa.hpp"
#include "C:\Users\Ricardo\eclipse-workspace\MQ Defines.h"
#include "MCP9808.hpp"
#include "TSL2591.hpp"
#include<fstream>

using namespace mraa;
using namespace std;
using namespace chrono;

//#define DEBUG
//#define MEASURING
#ifdef MEASURING
#define MEASURING_LENGHT 7200
uint times[MEASURING_LENGHT];
#endif

/*
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};
*/

enum ReadState {
	ST_READ, ST_CONFSENSOR, ST_ACTUATOR, ST_MAINCONTROL
};

enum ControlOptions {
	CO_Option_Invalid,
	MAINCONTROL, SENSOR, ACTUATOR
};

enum SensorType {
	S_Option_Invalid,
	TEMP, LUX
};

enum ActuatorType {
	A_Option_Invalid,
	LED, PWM
};

// structures for message queue
//	Struct to send
struct mesg_buffer QueueMessageSend;
//	Struct to serve as a buffer when a message is received to avoid overwrites
struct mesg_buffer QueueMessageBuffer;

char running=1;
uint8_t flag=0;
static int msgid_send, msgid_receive;

pthread_t thread_id;
pthread_mutex_t my_lock;

Gpio* user_button = NULL;
Gpio* ext_button = NULL;
Gpio* user_led = NULL;
Gpio* load = NULL;
Gpio* led = NULL;
Gpio* DIPSW =NULL;
Pwm* pwm = NULL;
Aio* a_pin = NULL;
MCP9808* sens_temp1 = NULL;
MCP9808* sens_temp2 = NULL;
TSL2591* sens_light = NULL;

void publishSensors(void);

void MQSetup(void);

void SensorInit (void);

void MQ_sub(string topic);

void MQ_pub(string topic, string content);

int sendMessage(void);

int receiveNMessage(int long n);

int receiveAnyMessage(void);

void check(int err);

/*
static int set_semvalue(void);

static void del_semvalue(void);

static int semaphore_v(void);

static int semaphore_p(void);
*/

void exitRoutine(void) {
	running=0;
	user_led->write(0);

#ifdef MEASURING
	/*Only used to measure quality and speed of code*/
	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
    char fileName [80];
    strftime (fileName,80,"Measurement_Results_%d_%m_%y_%H_%M.txt",now);
    cout << fileName << endl;
	ofstream MeasureFile;
	MeasureFile.open(fileName, std::ios::app); //app is append which means it will put the text at the end
	for(int i=0;i < MEASURING_LENGHT;i++) {
		MeasureFile << times[i] << endl;
	}
	MeasureFile.close();
	/*No Longer part of the measure system */
#endif

	MQ_sub("end");
	pthread_join(thread_id, NULL);
}

ControlOptions resolveOption(string input) {
    if( input == "MainControl" ) 	return MAINCONTROL;
    if( input == "Sensor" ) 		return SENSOR;
    if( input == "Actuator" ) 		return ACTUATOR;

    return CO_Option_Invalid;
 }

SensorType resolveSensor(string input) {
	if( input == "Temp" ) 			return TEMP;
	if( input == "Lux" ) 			return LUX;

	return S_Option_Invalid;
}

ActuatorType resolveActuator(string input) {
    if( input == "LED" ) 			return LED;
    if( input == "PWM" ) 			return PWM;

	return A_Option_Invalid;
}

string readNextSubTopic(string fullTopic, size_t *startIndex, size_t *endIndex) {
	string tmp;
	if((*endIndex=fullTopic.find('/', *startIndex+1))!=(std::string::npos)) {
		tmp = fullTopic.substr(*startIndex+1,(*endIndex-*startIndex-1));
		*startIndex = *endIndex;
		return tmp;
	}else return fullTopic.substr(*startIndex+1);
}

void * WorkerThread(void * a) {

	//Initial Read Machine State is START. Update upon state change.
	ReadState currentState = ST_READ;

	string m_topic;
	size_t index2cut, lastIndex;
	string subTopic;
	int sensorID;

	while(running){
		switch (currentState) {
			case ST_READ :
				index2cut=0, lastIndex=-1;
				receiveAnyMessage();
				m_topic = QueueMessageBuffer.topic;
#ifdef DEBUG
				printf("Type: %ld\nContent: %s\nTopic: %s\n", QueueMessageBuffer.mesg_type, QueueMessageBuffer.content, QueueMessageBuffer.topic);
#endif
				subTopic = readNextSubTopic(m_topic, &lastIndex, &index2cut);

				switch (resolveOption(subTopic)) {
					case SENSOR :
						currentState = ST_CONFSENSOR;
#ifdef DEBUG
						printf("Current State: SENSOR\n");
#endif
						break;

					case ACTUATOR :
						currentState = ST_ACTUATOR;
#ifdef DEBUG
						printf("Current State: ACTUATOR\n");
#endif
						break;

					case MAINCONTROL :
						currentState = ST_MAINCONTROL;
#ifdef DEBUG
						printf("Current State: MAINCONTROL\n");
#endif
						break;

					default :
#ifdef DEBUG
						printf("Entered Invalid State \"Option\"\n");
#endif
						currentState = ST_READ;
						break;
				}

				break;

			case ST_CONFSENSOR :
#ifdef DEBUG
				printf("Current State: ST_CONFSENSOR\n");
#endif
				subTopic = readNextSubTopic(m_topic, &lastIndex, &index2cut);
				switch(resolveSensor(subTopic)) {
					case TEMP :
					{
						sensorID = stoi(readNextSubTopic(m_topic, &lastIndex, &index2cut));
						subTopic = readNextSubTopic(m_topic, &lastIndex, &index2cut);
						if(subTopic=="Config") {
							subTopic = readNextSubTopic(m_topic, &lastIndex, &index2cut);
							if(subTopic=="Res") {
								if(sensorID==1) {
#ifdef DEBUG
									cout << "Changing Resolution of Sensor 1" << endl;
#endif
									sens_temp1 -> setResolution(static_cast<MCP9808_Resolution_t>(stoi(QueueMessageBuffer.content)));
									}
								if(sensorID==2) {
#ifdef DEBUG
									cout << "Changing Resolution of Sensor 2" << endl;
#endif
									sens_temp1 -> setResolution(static_cast<MCP9808_Resolution_t>(stoi(QueueMessageBuffer.content)));
									}
							}
							if(subTopic=="ShutWake") {
								if(sensorID==1) sens_temp1 -> shutdown_wake(stoi(QueueMessageBuffer.content));
								if(sensorID==2) sens_temp2 -> shutdown_wake(stoi(QueueMessageBuffer.content));
							}
						}
					}
						currentState = ST_READ;
						break;

					case LUX :
					{
						sensorID = stoi(readNextSubTopic(m_topic, &lastIndex, &index2cut));
						subTopic = readNextSubTopic(m_topic, &lastIndex, &index2cut);
						if(subTopic=="Config") {
							subTopic = readNextSubTopic(m_topic, &lastIndex, &index2cut);
							if(subTopic=="Gain") {

							}
							if(subTopic=="Integration") {

							}
						}

					}
						currentState = ST_READ;
						break;

					default :
#ifdef DEBUG
						printf("Entered Invalid State \"Type of Sensor\"\n");
#endif
						currentState = ST_READ;
						break;
				}
				break;

			case ST_ACTUATOR :
#ifdef DEBUG
				printf("Current State: ST_ACTUATOR\n");
#endif
				subTopic = readNextSubTopic(m_topic, &lastIndex, &index2cut);
				switch(resolveActuator(subTopic)) {
					case LED :
						if(stoi(QueueMessageBuffer.content)==1) {
							led -> write(1);
							break;
						}
						if(stoi(QueueMessageBuffer.content)==0) {
							led -> write(0);
							break;
						}
#ifdef DEBUG
						printf("Invalid value for LED state, please use 1 or 0\n");
#endif
						break;

					case PWM :
						pwm -> write(stof(QueueMessageBuffer.content)/100);
						break;

					default :
#ifdef DEBUG
						printf("Entered Invalid State \"Type of Actuator\"\n");
#endif
						currentState = ST_READ;
						break;

				}
				currentState = ST_READ;
				break;

			case ST_MAINCONTROL:
#ifdef DEBUG
				printf("Current State: ST_MAINCONTROL\n");
#endif
				if(strncmp(QueueMessageBuffer.content, "EXIT", 4)==0) exitRoutine();

				currentState = ST_READ;
				break;

			default :
#ifdef DEBUG
				printf("Entered Invalid State\n");
#endif
				currentState = ST_READ;
				break;
		}

	}

	return NULL;
}

int main(int argc, char *argv[]){
	time_point<steady_clock> reference;
	reference = chrono::steady_clock::now();
	float w;
	steady_clock::time_point t1, t2;

	int tickduration = 1000; // 1000 ms or 1 s per tick
	int timeduration;

#ifdef MEASURING
	/*Only used to measure quality and speed of code*/
	int countTimes=0;
	time_point<steady_clock> start, end;
	/*No Longer part of the measure system */
#endif

	MQSetup();
	SensorInit();
	MQ_sub("MainControl");

	pthread_create(&thread_id, NULL, WorkerThread, NULL);

	t1 = steady_clock::now();
	while (running) {
		if(!(user_button ->read())){
			exitRoutine();
			continue;
		}
		t2 = steady_clock::now();
		timeduration = duration_cast<duration<int, milli>>(t2 - t1).count();

		if (timeduration >= tickduration)
		{
			t1 = steady_clock::now();
			flag=1;
		}

		if (flag) {
#ifdef MEASURING
			/*Only used to measure quality and speed of code*/
			start = steady_clock::now();
			/*No Longer part of the measure system */
#endif

			/*
			if(DIPSW ->read()){
				sens_temp1 -> wake();
				sens_temp2 -> wake();
			}else {
				sens_temp1 -> shutdown();
				sens_temp2 -> shutdown();
			}
			*/

			if(user_led->read()){
				check(user_led->write(0));
			} else {
				check(user_led->write(1));
			}
			w=a_pin->readFloat();
			w=w*5;
			MQ_pub("Sensor/Analog/1/Val", to_string(w));
			publishSensors();

#ifdef MEASURING
			/*Only used to measure quality and speed of code*/
			end = steady_clock::now();
			times [countTimes] = duration_cast<microseconds>(start - reference).count();
			times [countTimes+1] = duration_cast<microseconds>(end - reference).count();
			countTimes+=2;
			if(countTimes>=MEASURING_LENGHT-1) {
				printf("Count= %d\n"
						"DONE!", countTimes);
				exitRoutine();
			}
			/*No Longer part of the measure system */
#endif

			flag=0;
		}
	}

	return 0;
}

void publishSensors(){
	MQ_pub("Sensor/Temp/1/Val", to_string(sens_temp1 -> readTempC()));
	MQ_pub("Sensor/Temp/2/Val", to_string(sens_temp2 -> readTempC()));
	MQ_pub("Sensor/Lux/1/Val", to_string(sens_light -> getLux()));
}

void MQSetup(void){

	// msgget creates a message queue
	// and returns identifier
	// msgget used to try to join with the queue from the Handler
	msgid_send = msgget((key_t)MQKEY_TOHANDLER, 0666 | IPC_CREAT);
	if (msgid_send == -1) {
#ifdef DEBUG
		fprintf(stderr, "msgget failed with error: %d\n", errno);
#endif
		exit(EXIT_FAILURE);
	}
	msgid_receive = msgget((key_t)MQKEY_FROMHANDLER, 0666 | IPC_CREAT);
	if (msgid_receive == -1) {
#ifdef DEBUG
		fprintf(stderr, "msgget failed with error: %d\n", errno);
#endif
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG
	printf("Connection Established\n"
			"Main MQID: %d	&	%d\n", msgid_send, msgid_receive);
#endif
}

void SensorInit (void){

	user_button = new Gpio(63, true, true);
	user_button->dir(DIR_IN);

	ext_button = new Gpio(9);
	ext_button->dir(DIR_IN);

	DIPSW = new Gpio(8);
	DIPSW -> dir(DIR_IN);

	user_led = new Gpio(13);
	user_led->dir(DIR_OUT);

	led = new Gpio(1);
	led->dir(DIR_OUT);
	MQ_sub("Actuator/LED/1/#");

	pwm = new Pwm(3);
	MQ_sub("Actuator/PWM/1/#");

	a_pin = new Aio(0);

	sens_temp1 = new MCP9808();
	MQ_sub("Sensor/Temp/1/Config/#");
	sens_temp1 -> wake();
	sens_temp1 -> setResolution(MCP9808_Resolution_Sixteenth);

	sens_temp2 = new MCP9808(0x19);
	MQ_sub("Sensor/Temp/2/Config/#");
	sens_temp2 -> wake();
	sens_temp2 -> setResolution(MCP9808_Resolution_Sixteenth);

	sens_light = new TSL2591();
	MQ_sub("Sensor/Lux/1/Config/#");

}

void MQ_sub(string topic)
{
	//Inform that it is a subscribe and to what topic
	QueueMessageSend.mesg_type=TYPE_SUB;

    strcpy(QueueMessageSend.content, "");
    strcpy(QueueMessageSend.topic, topic.c_str());
	sendMessage();
}

void MQ_pub(string topic, string content)
{
	//Inform that it is a publication and to what topic
	QueueMessageSend.mesg_type=TYPE_PUB;

	strcpy(QueueMessageSend.content, content.c_str());
	strcpy(QueueMessageSend.topic, topic.c_str());
	sendMessage();

}

int sendMessage(void) {
	if (msgsnd(msgid_send, (void *)&QueueMessageSend, MAX_TEXT, 0) == -1) {
#ifdef DEBUG
		fprintf(stderr, "msgsnd failed\n");
#endif
		exit(EXIT_FAILURE);
	}
	return 0;
}

int receiveNMessage(int long n) {
	if (msgrcv(msgid_receive, (void *)&QueueMessageBuffer, BUFSIZ, n, 0) == -1) {
#ifdef DEBUG
		fprintf(stderr, "msgrcv failed with error: %d\n", errno);
#endif
		exit(EXIT_FAILURE);
	}
	return 0;
}

int receiveAnyMessage(void) {
	if (msgrcv(msgid_receive, (void *)&QueueMessageBuffer, BUFSIZ, 0, 0) == -1) {
#ifdef DEBUG
		fprintf(stderr, "msgrcv failed with error: %d\n", errno);
#endif
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
#ifdef DEBUG
		fprintf(stderr, "Failed to delete semaphore\n");
#endif
}

static int semaphore_v(void){

	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1) {
#ifdef DEBUG
		fprintf(stderr, "semaphore_v failed\n");
#endif
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
#ifdef DEBUG
		fprintf(stderr, "semaphore_p failed\n");
#endif
		return(0);
	}
	return(1);
}
*/
