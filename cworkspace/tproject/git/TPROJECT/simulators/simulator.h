
#define RADIUS_MIN 2
#define RADIUS_MAX 10
//lifetime
#define LIFETIME_MAX 50
#define LIFETIME_MIN 5
//moving
#define DIST_MIN 1
#define DIST_MAX 10
//interval
#define OJB_MININTERVAL 5
#define OJB_MAXINTERVAL 10

#define RAND_SCALE 10
#define GENOBJECT_MINTIME 4
#define GENOBJECT_MAXTIME 50

#define SENDDT_INTERVAL 4
#define OBJECT_RELEASE 1
#define NUM_MAX_OBJECT 5
#define NUM_SENSOR_IN_ROOM 3
#define NULL_RADIUS_OBJECT -1
#define NULL_MSG_STATUS -1



struct object{
		int radius;
		int pos_x;
		int pos_y;
		int lifetime;
		int interval;
		int moving;           //distant of moving by decimeter
		int room_id;
};


 int msg_stop_room = -1;



#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>
#include <thread_db.h>
#include <semaphore.h>


#define REG_MAX_LEN				128
#define	FILE_MODE				(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define STOP_MESG				"stop"
#define STATUS_MESG				"status"
#define START_MESG				"start"
#define ADDROOM_MESG			"addroom"
#define RMROOM_MESG				"rmroom"
#define STOPROOM_MESG			"stoproom"
#define MSG_STATUS_ROOM			"roomstatus"
#define LOG_PATH				"logs/"
#define CONFIG_PATH				"config/SimConfig.cfg"
#define CONFIG_PATH_TEMP		"config/temp_SimConfig.cfg"
#define BUFFERSIZE				1024
#define MAX_LINE				1024
#define DEFAULT_PORT			7899
#define PROGRAME_NAME			"Simulator"
#define MAX_ROOM				256
#define INVALID_POSITION		-1
#define DIVIDE_MSG_ADDROOM		';'
#define STATUS_STOP 			0
#define STATUS_START 			1

//parse key
#define NUMBER_OF_ROOMS			"<NUMBER OF ROOMS>:"
#define	IPADDR_LC				"<LOCALIP>:"
#define PORT_LC					"<LOCAL PORT>:"
#define	IPADDR_SV				"<SERVERIP>:"
#define PORT_SV					"<SERVER PORT>:"
#define INTERVAL				"<INTERVAL>:"
#define ROOM_ID					"<ROOM ID>:"
#define SENSOR1					"<SENSOR1>:"
#define SENSOR2					"<SENSOR2>:"
#define SENSOR3					"<SENSOR3>:"
#define WIDTH					"<WIDTH>:"
#define HEIGHT					"<HEIGHT>:"
#define MAX_OJ					"<MAX OJ>:"
#define START_ROOM_LIST			"<START ROOM LIST>:"
#define END_ROOM_LIST			"<END ROOM LIST>:"
#define FIRSTCHAR_LABEL			'<'
#define NUMBER_ITEMS_OF_ROOM	7
#define DIVIDE_CHAR				':'
#define FIRST_CHAR


//status of simulator
#define RUNNING					0
#define STOPPED					1
#define RESP_RUNNING_MESG		"Simulator is running"
#define RESP_STOPPED_MESG		"Simulator is stopped"
#define RESP_UNKNOWN_MESG		"Unknown status"
#define MSG_CANN_STOP_ROOM		"This room was stopped or no room has this id"
#define MSG_STOP_ROOM_SUCCESS 	"This room was stopped successfully"
#define MSG_CANN_RM_ROOM		"There is no room has this id"
#define MSG_RM_ROOM_SUCCESS 	"This room was removed successfully"

#define LISTENER_THREAD_ID		0;

typedef struct Position
{
	int x;
	int y;
};

typedef struct Room
{
	unsigned int 	roomID;
	struct 			Position sensor1;
	struct 			Position sensor2;
	struct 			Position sensor3;
	unsigned int 	width; //default decimeter
	unsigned int 	height; //default decimeter
	unsigned int 	max_object;

};

typedef struct SimConfig
{
	unsigned int 	nRoomNumbers;
	unsigned int 	nInterval;
	char 			sv_ipAddr[128];
	unsigned int 	sv_nPort;
	char 			lc_ipAddr[128];
	unsigned int 	lc_nPort;
	struct Room 	roomList[MAX_ROOM];
};

#define MSG_STATUS_ROOM_NULL	"There is no room which has this ID!"
#define MSG_STATUS_ROOM_STOP	"This room was stopped!"

