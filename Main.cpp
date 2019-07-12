#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

using namespace std;

// structure for message queue
struct mesg_buffer {
    long mesg_type;
    char mesg_text[100];
} message;

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{	
	key_t key;
    int msgid;

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

    cout << "Main" << msgid << endl;
    message.mesg_type = 2;
    strncpy(message.mesg_text, "Hello World", sizeof(message.mesg_text));

    // msgsnd to send message
	msgsnd(msgid, &message, sizeof(message), 0);

	return 0;
}

