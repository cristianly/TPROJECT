#include "simulator.h"

//Global variable
struct SimConfig simconfig;
static int hPIDF 			= -1;
bool bIsListenerStop 		= false;
int nSimulatorStatus 		= STOPPED;
int main_sockfd;

int nRoomCounter 	= 0;
int msg_status_room  = NULL_MSG_STATUS;
sem_t semaphore_file;
sem_t semaphore_room;
int RoomStatus[MAX_ROOM] = {STATUS_STOP};

int random_segment(int from, int to){
	//srand(time(NULL));
	return (rand()%(to - from + 1) + from);
}

struct object genObject(int room_max_x, int room_max_y, short idroom){

	struct object ob;
	ob.radius = random_segment(RADIUS_MIN, RADIUS_MAX);
	ob.pos_x = random_segment(ob.radius, room_max_x - ob.radius);
	ob.pos_y = random_segment((ob.radius), room_max_y - ob.radius);
	ob.lifetime = random_segment(LIFETIME_MIN, LIFETIME_MAX);
	ob.interval = random_segment(OJB_MININTERVAL, OJB_MAXINTERVAL);
	ob.moving = random_segment(DIST_MIN, DIST_MAX);
	ob.room_id = idroom;
	return ob;
}

void* liveObject(void* arg){
	int x, y;
	struct object* oj = (struct object*)arg;
	while (((int)oj->lifetime > 0) && (msg_stop_room != oj->room_id)){
		x = random_segment(oj->pos_x - oj->moving, oj->pos_x + oj->moving);
		y = random_segment(oj->pos_y - oj->moving, oj->pos_y + oj->moving);
		oj->pos_x = (x > oj->radius) ? x : oj->radius;
		oj->pos_y = (y > oj->radius) ? y : oj->radius;
		oj->lifetime -= oj->interval;
		sleep(oj->interval);
	}
	printf("\n\n-------------Object destroy-----------\n\n");
	oj->radius 	= NULL_RADIUS_OBJECT;
}

void sendObjectData(struct object oj, struct Room* room, int sockdf){
	char *s = (char*)malloc(20);
	char *rs = (char*)malloc(60);
	unsigned int i, x1, x2, y1, y2, rad, pos_x, pos_y;

	pos_x = oj.pos_x;
	pos_y = oj.pos_y;

	strcpy(s,"");
	strcpy(rs,"");
	rad = oj.radius;
	x1 = (pos_x - rad < 0)? 0 : pos_x - rad;
	x2 = (pos_x + rad > room->width) ? room->width : pos_x + rad;
	y1 = (pos_y - rad < 0)? 0 : pos_y - rad;
	y2 = (pos_y + rad > room->height)? room->height : pos_y + rad;

	sprintf(rs, "ROOM %d: ", room->roomID);
	for(i = 0 ; i < NUM_SENSOR_IN_ROOM ; i ++){
		sprintf(s," %d %d ,",random_segment(x1, x2), random_segment(y1, y2));
		strcat(rs,s);
	}
	rs[strlen(rs)] = '\0';
	if (send(sockdf, rs, sizeof(rs), 0) <= 0){
		vWriteErrlogInfo("Room  cannot get send data from sensor", room->roomID);
	}

	free(s);
	free(rs);

}

void* liveRoom(void* arg){
	 struct Room *thisroom = (struct Room*)arg;

	 unsigned long time_counter = 0; //using it as timer
	 unsigned int time_period = 0;  //using for count down  to generate new object in room
	 int pos = 0;
	 struct object objects[thisroom->max_object];
	 int num_object = 0;
	 struct sockaddr_in servaddr;
	 struct sockaddr_in6 servaddr6;

	 int sockdf = getSocket(simconfig.sv_ipAddr, simconfig.sv_nPort, &servaddr, &servaddr6); //connect to operation
	 printf("socket created %d", sockdf);
	 for (pos = 0; pos < thisroom->max_object; pos++){
		 objects[pos].radius = NULL_RADIUS_OBJECT;
	 }

	 while (msg_stop_room != thisroom->roomID){
		 //Check numbers of object.

		 num_object = 0;
		 for (pos = 0; pos < thisroom->max_object; pos++){
			 if (objects[pos].radius != NULL_RADIUS_OBJECT){
				 num_object++;
			 }
		 }
		 //send status of this room
		 if (msg_status_room == thisroom->roomID){
			 char msg[MAX_LINE];
			 sprintf(msg, "Width: %d\n Height: %d\n MaxOj = %d\n There are %d object(s) in this room", thisroom->width, thisroom->height, thisroom->max_object, num_object);
			 /*
			  * send this msg to server whenever receive msg_status_room
			  */
			 msg_status_room = NULL_MSG_STATUS;
		 }
		 //Generate object belong with time period
		 pthread_t toj = 1;
		 if (time_period == 0){
			 if (num_object < thisroom->max_object){
				 pos = 0;
				 while  ((objects[pos].radius != NULL_RADIUS_OBJECT) && (pos < thisroom->max_object -1)) pos++;
				 objects[pos] = genObject(thisroom->width, thisroom->height, thisroom->roomID);
				 pthread_create(&toj, NULL, liveObject, (void*)&objects[pos]);
			 }
			 time_period = random_segment(GENOBJECT_MINTIME, GENOBJECT_MAXTIME);
		 }

		 time_period--;
		 time_counter++;
		 if (time_counter % simconfig.nInterval == 0){
			 for (pos = 0; pos < NUM_MAX_OBJECT; pos++){
				 if (objects[pos].radius > 0){
					/*
					 	 *send data of object to operation program here
					 	 *send (socket, data, length of data);
					 */
					 struct object o1 = objects[pos];
					 printf("ROOMID = %d\n r = %d dm\n x = %d\n y = %d\n lifetime = %d s\n interval = %d s\n moving distant = %d dm number object = %d  \n\n",thisroom->roomID, o1.radius, o1.pos_x, o1.pos_y, o1.lifetime, o1.interval, o1.moving, num_object);
					 //sendObjectData(objects[pos], thisroom, sockdf);
				 }
			 }
		 }
		 sleep(1);
	 }
}

void vWriteErrlogInfo (const char *fmt, ...)
{
    char        achFilename [80];
    struct tm   *tm;
    FILE        *sysfd;
    time_t      timestamp;
    va_list     args;
	int			year;
 	char		pathname[1024];
	pathname[0] = '\0';
	sprintf(pathname,LOG_PATH);

    /* write data out to file */
    timestamp = time(0);
    tm = localtime(&timestamp);
	year = tm->tm_year;
	year += 1900;	// make it 4 digits
	sprintf (achFilename, "%s%.4d%.2d%.2d.log", 
			PROGRAME_NAME, year, tm->tm_mon+1, tm->tm_mday);
	
	strcat(pathname,achFilename);
    sysfd = fopen (pathname, "a");
    if (sysfd != NULL) {
		chmod (pathname, S_IWRITE | S_IREAD | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        fprintf(sysfd, "%.2d/%.2d/%.2d %.2d:%.2d:%.2d ", 
				tm->tm_mon+1, tm->tm_mday,
				year, tm->tm_hour, tm->tm_min, tm->tm_sec);
        va_start (args, fmt);
        vfprintf (sysfd,fmt, args);
        va_end (args);
        fprintf (sysfd, "\n");
        fclose (sysfd);
    }
}
int isLocked(const char* programName)
{
	struct flock oLock;

	oLock.l_type = F_WRLCK;
	oLock.l_start = 0;
	oLock.l_whence = SEEK_SET;
	oLock.l_len = 0;


	hPIDF = open(programName, O_CREAT|O_RDWR, FILE_MODE);

	if (hPIDF < 0)
	{
		fprintf(stdout, "Fatal Error - name-conflict: %s is used by another mudule! Please contact administrators\n", programName);
		return -1; 
	}
	else	if (fcntl(hPIDF, F_SETLK, &oLock) < 0) 
	{
		if (errno == EACCES || errno == EAGAIN)
		{
			fprintf(stdout, "The command is canceled;only one instance of %s can run on the server!\n",programName);
			close(hPIDF);
			return 1; 
		}
		else
		{
			fprintf(stdout, "Fatal Error: System failed! - Please contact administrators!\n");
			return -1; 
		}		
	}
	return 0; 
}
//Check IPv6
bool isIPv6(const char* ipstr)
{
	if (!ipstr)
		return false;
	size_t i=0,m=0,n=0,k=0;
	bool res=true;
	if (ipstr[0]==':' && ipstr[1]!=':') //begin with only one :
		return false;
	for (i=0;ipstr[i]!='\0';i++)
	{
		res=(ipstr[i]>='0' && ipstr[i]<='9')||
			(ipstr[i]>='A' && ipstr[i]<='F')||
			(ipstr[i]>='a' && ipstr[i]<='f')||
			ipstr[i]==':'||ipstr[i]=='*'||ipstr[i]=='?';
		if (!res)
			return false;
		++k;
		if (ipstr[i]==':') {
			k=0;n++;
			if (ipstr[i+1]==':')
				m++;
		}
		else if (k>4) return false;
	}
	if (i>39||i<2)
		return false;
	if (ipstr[i-1]==':' && ipstr[i-2]!=':') // end with only one :
		return false;
	if (n>7 || n<2)
		return false;
	if (m==0 && n!=7)
		return false;
	if (m>1)
		return false;
	return true;
}
int getSocket(char *szAddress, int iPort, struct sockaddr_in *servaddr, struct sockaddr_in6 *servaddr6)
{
	vWriteErrlogInfo("Start of function getSocket");
	in_addr_t	addr;
	struct	hostent	*he;
	int handleSocket;

	memset((char *)servaddr, 0, sizeof(servaddr));
	memset((char *)servaddr6, 0, sizeof(servaddr6));
	servaddr->sin_family = AF_INET;
	servaddr->sin_port = htons(iPort);	//port number
	
	servaddr6->sin6_family = AF_INET6;
	servaddr6->sin6_port = htons(iPort);	//port number
	if(!isIPv6(szAddress)) 
	{
		if ((int)(addr = inet_addr(szAddress)) != INADDR_NONE)
		{
			if ((he = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET)) == (struct hostent *)0) 
			{
				vWriteErrlogInfo("Cannot get host by address %s port:%d\n", szAddress,iPort);				
				memcpy((char *)&(servaddr->sin_addr),(char *)&addr, sizeof(addr));
			} 
			else
			{
				memcpy((char *)&(servaddr->sin_addr), he->h_addr, he->h_length);
			}
		} 
		else 
		{
			vWriteErrlogInfo("invalid IPAddress: %s\n",szAddress);
		}
		memset(servaddr->sin_zero, '\0', sizeof(servaddr->sin_zero));	
		handleSocket = socket(AF_INET, SOCK_STREAM,0);
	} 
	else 
	{
		memset(servaddr6, 0, sizeof(servaddr6));
		servaddr6->sin6_family = AF_INET6;		 // host byte order		
		int myAlen = sizeof(servaddr6);
		if(inet_pton(AF_INET6, szAddress, (void*) &servaddr6->sin6_addr) <=0)
		{
			vWriteErrlogInfo("invalid IPAddress: %s\n",szAddress);
		}
		servaddr6->sin6_port = htons(iPort);	 // short, network byte order
		handleSocket = socket(AF_INET6, SOCK_STREAM,0);
	}
	if (handleSocket < 0) 
	{
		vWriteErrlogInfo("Failed to create socket\n");
	}
	vWriteErrlogInfo("Connected to IP address: %s", szAddress);
	return handleSocket;
}

int sendRequest(char* request, char *ip, int port)
{
	vWriteErrlogInfo("Sending request %s...",request);
    char response[2048];
    int resp_leng;
 
    char buffer[BUFFERSIZE];
    struct sockaddr_in servaddr;
	struct sockaddr_in6 servaddr6;
    int sock;
	struct hostent *temp;
	char *ipLocal;
	bool isipv6 = isIPv6(ip);
	
	sock = getSocket(ip, port, &servaddr, &servaddr6);
	if(sock < 0)
	{
			vWriteErrlogInfo("Failed to get socket:%d\n",sock);
	}
	if (getSocket(ip, port, &servaddr, &servaddr6)) {
		//Connect to socket
		if(!isipv6) {
			if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
				vWriteErrlogInfo("Failed to connect to socket [%s: %d]\n",ip, port);
				return 0;
			}
		} else {
			if (connect(sock, (struct sockaddr*)&servaddr6, sizeof(servaddr6)) == -1) {
				vWriteErrlogInfo("Failed to connect to socket [%s: %d]\n",ip, port);
				return 0;
			}
		}

		//send request
		if (send(sock, request, strlen(request), 0) != strlen(request)) {
			vWriteErrlogInfo("send() sent a different number of bytes than expected");
			return 0;
		}

		//shutdown the connection since no more data will be sent
		shutdown(sock, 1);

		//get response
		memset(response,0,sizeof(response));
		memset(buffer,0,sizeof(buffer));
		resp_leng= BUFFERSIZE;
		while (resp_leng == BUFFERSIZE)
		{
			resp_leng= recv(sock, (char*)&buffer, BUFFERSIZE, 0);
			if (resp_leng>0)
			strcat(response,buffer);
		}
		vWriteErrlogInfo("responsess:%s",response);
		printf(response);
		printf("\n");
	 
		//disconnect
		close(sock);
		vWriteErrlogInfo("Request is sent.");
	} 
	else 
	{
		vWriteErrlogInfo("Can't get socket from [IP,PORT]: [%s,%d]", ip, port);
	}
	return 1;
}

char* getValue(char* line,char* key)
{
	vWriteErrlogInfo("Getting value of \"%s\" in line:%s", key,line);
	char* retValue = line;
	int lenghtCutting = strlen(retValue) - strlen(key);
	memcpy(retValue, &retValue[strlen(key)],lenghtCutting);
	retValue[lenghtCutting] = '\0';
	vWriteErrlogInfo("result:%s",retValue);
	return retValue;
}

struct Position getPosition(char* line)
{
	vWriteErrlogInfo("Startting of funtion getPosition");
	struct Position position;
	position.x = INVALID_POSITION;
	position.y = INVALID_POSITION;
	if(strlen(line) <= 5)
	{
		vWriteErrlogInfo("No string to parse");
		return position;
	}
	
	char semicolon 			= ',';
	char openCharater 		= '(';
	char closeCharater 		= ')';
	int i = 0,
		j = 0;
	char stemp[50];

	while(line[i] != openCharater){
		i++;
	}
	
	i++;
	while(line[i] != semicolon){
		stemp[j++] = line[i++];
	}
	stemp[j] = '\0';
	position.x = atoi(stemp);

	j = 0;
	i++;
	while(line[i] != closeCharater){
		stemp[j++] = line[i++];
	}
	stemp[j] = '\0';
	position.y = atoi(stemp);

	return position;
}

int loadConfig(char* path){
	vWriteErrlogInfo("Start of funtion loadConfig with path:%s",path);
	char line[MAX_LINE];
	FILE* file = fopen(path, "r"); /* should check the result */
	// make sure the file opened properly
    if(NULL == file)
    {
        vWriteErrlogInfo("Cannot open file: %s\n", path);
        return 0;
    }
	struct Room room;
	int validateCounter = 0;
    while (fgets(line, sizeof(line), file)) 
	{
		if(strstr(line,NUMBER_OF_ROOMS) != NULL)
		{
			simconfig.nRoomNumbers = atoi(getValue(line,NUMBER_OF_ROOMS));
			
		}
		else if(strstr(line,INTERVAL) != NULL)
		{
			simconfig.nInterval = atoi(getValue(line,INTERVAL));
		}
		else if(strstr(line,IPADDR_LC) != NULL)
		{
			strncpy(simconfig.lc_ipAddr,getValue(line,IPADDR_LC),sizeof(simconfig.lc_ipAddr));
		}
		else if(strstr(line,PORT_LC) != NULL)
		{
			simconfig.lc_nPort = atoi(getValue(line,PORT_LC));
		}
		else if(strstr(line,IPADDR_SV) != NULL)
		{
			strncpy(simconfig.sv_ipAddr,getValue(line,IPADDR_SV),sizeof(simconfig.sv_ipAddr));
		}
		else if(strstr(line,PORT_SV) != NULL)
		{
			simconfig.sv_nPort = atoi(getValue(line,PORT_SV));
		}
		if(strstr(line,ROOM_ID) != NULL)
		{
			validateCounter++;
			room.roomID = atoi(getValue(line,ROOM_ID));
		}
		else if(strstr(line,SENSOR1) != NULL)
		{
			char* sensor1 = getValue(line,SENSOR1);
			room.sensor1 = getPosition(sensor1);
			validateCounter++;
		}
		else if(strstr(line,SENSOR2) != NULL)
		{
			char* sensor2 = getValue(line,SENSOR2);
			room.sensor2 = getPosition(sensor2);
			validateCounter++;
			vWriteErrlogInfo("room.sensor2: x:%d---y:%d\n", room.sensor2.x,room.sensor2.y);
		}
		else if(strstr(line,SENSOR3) != NULL)
		{
			char* sensor3 = getValue(line,SENSOR3);
			room.sensor3 = getPosition(sensor3);
			validateCounter++;
			vWriteErrlogInfo("room.sensor3: x:%d---y:%d\n", room.sensor3.x,room.sensor3.y);
		}
		else if(strstr(line,WIDTH) != NULL)
		{
			room.width = atoi(getValue(line,WIDTH));
			validateCounter++;
			vWriteErrlogInfo("width:%d\n", room.width);
		}
		else if(strstr(line,HEIGHT) != NULL)
		{
			room.height = atoi(getValue(line,HEIGHT));
			validateCounter++;
			vWriteErrlogInfo("height:%d\n", room.height);
		}
		else if(strstr(line,MAX_OJ) != NULL)
		{
			room.max_object = atoi(getValue(line, MAX_OJ));
			validateCounter++;
			vWriteErrlogInfo("Max object:%d\n", room.max_object);
		}
		if(validateCounter == NUMBER_ITEMS_OF_ROOM)
		{
			simconfig.roomList[nRoomCounter] = room;
			nRoomCounter++;
			validateCounter = 0;
		}
	}
	fclose(file);
	return 1;
}

char* checkStatus()
{
	char* repStatus;
	switch(nSimulatorStatus)
	{
		case RUNNING:
		{
			repStatus = RESP_RUNNING_MESG;
			break;
		}
		case STOPPED:
		{
			repStatus = RESP_STOPPED_MESG;
			break;
		}
		default:
		{
			repStatus = RESP_UNKNOWN_MESG;
			break;
		}
	}
	return repStatus;
}

char* stop()
{
	bIsListenerStop = true;
	char* stopMesg = RESP_STOPPED_MESG;
	
	return stopMesg;
}

int getMessageContentIdRoom(char* request, char* MSG){
	int i, j = 0;
	char *content;
		content = malloc(sizeof(char)*(strlen(request) - strlen(MSG) + 1));

	   for (i = strlen(MSG); i < strlen(request); i++){
				content[j++] = request[i];
		}
		content[j]='\0';
	return atoi(content);
}

int stopRunningRoom(int roomid){
	int i;
	sem_wait(&semaphore_room);
	msg_stop_room = roomid;

	i = 0;
	while ((i < simconfig.nRoomNumbers)&&(simconfig.roomList[i].roomID != roomid)) i++;

	if (i == simconfig.nRoomNumbers) {
		sem_post(&semaphore_room);
		return 0;
	}

	RoomStatus[i] = STATUS_STOP;
	sleep(1);
	msg_stop_room = NULL_MSG_STATUS;
	sem_post(&semaphore_room);
	return 1;
}

void stopAllRunningRoom(){
	int i;
	sem_wait(&semaphore_room);
	for (i = 0; i < simconfig.nRoomNumbers; i++){
		RoomStatus[i] = STATUS_STOP;
		msg_stop_room = simconfig.roomList[i].roomID;
		usleep(100);
	}
	msg_stop_room = NULL_MSG_STATUS;
	sem_post(&semaphore_room);
}

int startRoom(int roomid){
	int i;
	sem_wait(&semaphore_room);
	i = 0;
	while ((i < simconfig.nRoomNumbers)&&(simconfig.roomList[i].roomID != roomid))   i++;

	if (i == simconfig.nRoomNumbers) {
		sem_post(&semaphore_room);
		return 0;
	}

	RoomStatus[i] = STATUS_START;
	pthread_t newRoomThread;
	pthread_create(&newRoomThread, NULL, liveRoom, (void*)&simconfig.roomList[i]);
	sem_post(&semaphore_room);
	return 1;
}
void startAllRoom(){
	int i;
	sem_wait(&semaphore_room);
	for (i = 0; i < simconfig.nRoomNumbers; i++){
		RoomStatus[i] = STATUS_START;
		pthread_t newRoomThread;
		pthread_create(&newRoomThread, NULL, liveRoom, (void*)&simconfig.roomList[i]);
		printf("\n\nLoaded room!\n"
								"roomID: \t%d\n"
								"width: \t%d\n"
								"height: \t%d\n"
								"sensor1: (%d, %d)\n"
								"sensor2: (%d, %d)\n"
								"sensor3: (%d, %d)\n"
								"max object: \t%d\n"
								,simconfig.roomList[i].roomID, simconfig.roomList[i].width, simconfig.roomList[i].height, simconfig.roomList[i].sensor1.x,
								 simconfig.roomList[i].sensor1.y,simconfig.roomList[i].sensor2.x, simconfig.roomList[i].sensor2.y,
								 simconfig.roomList[i].sensor3.x, simconfig.roomList[i].sensor3.y, simconfig.roomList[i].max_object);

	}
	sem_post(&semaphore_room);
}

int removeRoom(int roomid_remove){
	vWriteErrlogInfo("Change config file to remove room with path:%s", CONFIG_PATH);

	int i = 0, j;
	while ((i < simconfig.nRoomNumbers)&&(simconfig.roomList[i].roomID != roomid_remove))
	for (j = i; j < simconfig.nRoomNumbers -1 ; j++){
		simconfig.roomList[j] = simconfig.roomList[j+1];
	}
	simconfig.nRoomNumbers -= 1;

	char line[MAX_LINE];
	sem_wait(&semaphore_file);
	FILE* fconfig = fopen(CONFIG_PATH, "r");
	FILE* ftemp = fopen(CONFIG_PATH_TEMP, "w");

	if(NULL == fconfig)
	{
	        vWriteErrlogInfo("Cannot open file: %s for reading\n", CONFIG_PATH);
	        return 0;
	}
	if(NULL == ftemp)
	{
        vWriteErrlogInfo("Cannot open file: %s for writing\n", CONFIG_PATH_TEMP);
        return 0;
	}
	int fthrough = 0;
	int roomid, roomnumber;
	char data[MAX_LINE];
	while (fgets(line, MAX_LINE, fconfig)){
		strcpy(data, line);
		data[strlen(line)] = '\0';

		if (strstr(line,NUMBER_OF_ROOMS) != NULL)
		{
			roomnumber = atoi(getValue(line,NUMBER_OF_ROOMS));
			roomnumber--;
			sprintf(data, "%s%d\n", NUMBER_OF_ROOMS, roomnumber);
		}
		if (strstr(line, ROOM_ID) != NULL)
			{
				roomid = atoi(getValue(line,ROOM_ID));
				if (roomid == roomid_remove){ //meet the id needed
					fthrough = 1;             //through all data of this room
				}
				else{
					fthrough = 0;			//meet new room
				}
			}
		if (!fthrough){
			fputs(data, ftemp);
		}
	}

	int result= rename( CONFIG_PATH_TEMP, CONFIG_PATH);
	if ( result == 0 ){
		vWriteErrlogInfo("Cannot rename temporary file to configuration file\n");
	}
	fclose(fconfig);
	fclose(ftemp);
	sem_post(&semaphore_file);
}

int handleRequest(char* request, char* response)
{
	vWriteErrlogInfo("receive a request: %s\n", request);
	memset(response,0,sizeof(response));
	char* respMesg;
	if(strcmp(request,STOP_MESG) == 0)
	{
		respMesg = stop();
	}
	else if (strcmp(request,STATUS_MESG) == 0)
	{
		respMesg = checkStatus();
	}
	else if (strstr(request,MSG_STATUS_ROOM) != 0)
	{
		int id = getMessageContentIdRoom(request, RMROOM_MESG);
		if (id){
			showRoomStatus(id);
			vWriteErrlogInfo("show status of room which has id: %d\n", id);
		}
		else{
			vWriteErrlogInfo("\nCan not show status of room because of error roomid");
		}
	}
	else if (strstr(request, ADDROOM_MESG) != 0)
	{
		receiveAddingRoomMessage(request);
	}
	else if (strstr(request, RMROOM_MESG) != 0)
	{
		int id = getMessageContentIdRoom(request, RMROOM_MESG);
		if ((id) && (removeRoom(id))){
			vWriteErrlogInfo("remove room which has id: %d\n", id);
		}
		else{

			vWriteErrlogInfo("\nCan not remove room because of error roomid");
		}
	}
	else if (strstr(request, STOPROOM_MESG) != 0)
	{
		int id = getMessageContentIdRoom(request, STOPROOM_MESG);
		if ((id) && (stopRunningRoom(id))){
			send(main_sockfd, MSG_RM_ROOM_SUCCESS, sizeof(MSG_RM_ROOM_SUCCESS));		}
		else{
			send(main_sockfd, MSG_CANN_RM_ROOM, sizeof(MSG_CANN_RM_ROOM));
			vWriteErrlogInfo("\nCan not remove room because of error roomid");
		}
	}

	memcpy(response,respMesg,strlen(respMesg));	
	return 1;
}

void addRoom(struct Room r){
	char line[MAX_LINE];
	int i;
	pthread_t newRoomThread;
	simconfig.roomList[simconfig.nRoomNumbers] = r;
	pthread_create(&newRoomThread, NULL, liveRoom, (void*)&simconfig.roomList[simconfig.nRoomNumbers]);

	simconfig.nRoomNumbers += 1;

	/*
	 * open file configuration to add room information
	 */
	sem_wait(&semaphore_file);
	FILE* fconfig = fopen(CONFIG_PATH, "w");
	if(NULL == fconfig){
		vWriteErrlogInfo("Cannot open file: %s for update\n", CONFIG_PATH);
	}
	sprintf(line,"%s%d\n", NUMBER_OF_ROOMS, simconfig.nRoomNumbers);
	fputs(line, fconfig);

	sprintf(line,"%s%d\n", INTERVAL, simconfig.nInterval);
	fputs(line, fconfig);
	sprintf(line,"%s%s", IPADDR_LC, simconfig.lc_ipAddr);
	fputs(line, fconfig);
	sprintf(line,"%s%d\n", PORT_LC, simconfig.lc_nPort);
	fputs(line, fconfig);
	sprintf(line,"%s%s", IPADDR_SV, simconfig.sv_ipAddr);
	fputs(line, fconfig);
	sprintf(line,"%s%d\n", PORT_SV, simconfig.sv_nPort);
	fputs(line, fconfig);
	fputs("\n", fconfig);

	for (i = 0; i < simconfig.nRoomNumbers; i++){
		sprintf(line,"%s%d\n", ROOM_ID, simconfig.roomList[i].roomID);
		fputs(line, fconfig);
		sprintf(line,"%s(%d,%d)\n", SENSOR1, simconfig.roomList[i].sensor1.x, simconfig.roomList[i].sensor1.y);
		fputs(line, fconfig);
		sprintf(line,"%s(%d,%d)\n", SENSOR2, simconfig.roomList[i].sensor2.x, simconfig.roomList[i].sensor2.y);
		fputs(line, fconfig);
		sprintf(line,"%s(%d,%d)\n", SENSOR3, simconfig.roomList[i].sensor3.x, simconfig.roomList[i].sensor3.y);
		fputs(line, fconfig);
		sprintf(line,"%s%d\n", WIDTH, simconfig.roomList[i].width);
		fputs(line, fconfig);
		sprintf(line,"%s%d\n", HEIGHT, simconfig.roomList[i].height);
		fputs(line, fconfig);
		sprintf(line,"%s%d\n", MAX_OJ, simconfig.roomList[i].max_object);
		fputs(line, fconfig);
		fputs("\n", fconfig);
	}
	fclose(fconfig);
	sem_post(&semaphore_file);
}

//int addObject(struct object oj, int roomid);
/*
Function: listener
Descript: Receive message from Poller Server
*/
void getRoomValueFromMessage(char* line, struct Room *room){
		if(strstr(line,ROOM_ID) != NULL){
			room->roomID = atoi(getValue(line,ROOM_ID));
		}
		else if(strstr(line,SENSOR1) != NULL){
			char* sensor1 = getValue(line,SENSOR1);
			room->sensor1 = getPosition(sensor1);
			vWriteErrlogInfo("room.sensor1: x:%d---y:%d\n", room->sensor1.x,room->sensor2.y);
		}
		else if(strstr(line,SENSOR2) != NULL){
			char* sensor2 = getValue(line,SENSOR2);
			room->sensor2 = getPosition(sensor2);
			vWriteErrlogInfo("room.sensor2: x:%d---y:%d\n", room->sensor2.x,room->sensor2.y);
		}
		else if(strstr(line,SENSOR3) != NULL)
		{
			char* sensor3 = getValue(line,SENSOR3);
			room->sensor3 = getPosition(sensor3);
			vWriteErrlogInfo("room.sensor3: x:%d---y:%d\n", room->sensor3.x,room->sensor3.y);
		}
		else if(strstr(line,WIDTH) != NULL)
		{
			room->width = atoi(getValue(line,WIDTH));
			vWriteErrlogInfo("width:%d\n", room->width);
		}
		else if(strstr(line,HEIGHT) != NULL)
		{
			room->height = atoi(getValue(line,HEIGHT));
			vWriteErrlogInfo("height:%d\n", room->height);
		}
		else if(strstr(line,MAX_OJ) != NULL)
		{
			room->max_object = atoi(getValue(line, MAX_OJ));
			vWriteErrlogInfo("Max object:%d\n", room->max_object);
		}
}

void receiveAddingRoomMessage(char* msg){
	int i = 0,
		j = 0;
	struct Room roominfo;
	char data[REG_MAX_LEN];

	while ((msg[i] != FIRSTCHAR_LABEL) && (i<strlen(msg))) i++;

	while (i < strlen(msg)){
		if (msg[i] != DIVIDE_MSG_ADDROOM){
			data[j++] = msg[i];
		}
		else{
			data[j] = '\0';
			j = 0;
			getRoomValueFromMessage(data, &roominfo);
		}
		i++;
	}
	//one more case when i meet the length of string but the loop can not recognize
	data[j] = '\0';
	getRoomValueFromMessage(data, &roominfo);
	roominfo.roomID = simconfig.nRoomNumbers;

	addRoom(roominfo);
}

void showRoomStatus(int roomid){
	int i, status, meet = 0 ;

	for (i = 0; i < simconfig.nRoomNumbers; i++){
		if (roomid == simconfig.roomList[i].roomID) {
			meet = 1;
			status = RoomStatus[i];
			break;
		}
	}

	if (!meet){
		sendRequest(MSG_STATUS_ROOM_NULL, simconfig.sv_ipAddr, simconfig.sv_nPort);
	}
	else{
		if (status == STATUS_STOP) {
			sendRequest(MSG_STATUS_ROOM_NULL, simconfig.sv_ipAddr, simconfig.sv_nPort);
		}
		if (status == STATUS_START) {
			msg_status_room = roomid;
		}
	}
}

void *listener(void *lpParameter)
{
	vWriteErrlogInfo("Start listening...");
	int socket;
	int newSocket;
	int newSocket6 = 0;
	int newSocket4 = 0;
	int sin_size6;
	int sin_size4;
	struct sockaddr_in6 my_addr6;	// my address information
	struct sockaddr_in6 their_addr6; // address of connecting client
	struct sockaddr_in my_addr4;	// my address information
	struct sockaddr_in their_addr4; // address of connecting client
	int size = 1;
	int BACKLOG = 1; //the number of pending connections queue will hold
	int err;
	bool isipv6 = isIPv6(simconfig.lc_ipAddr);
	char buffer[256],mess[2048];
	memset(buffer,0,sizeof(buffer));
	struct sockaddr_in serv_addr, cli_addr;
	int  n;
	char response[256];

	socket = getSocket(simconfig.lc_ipAddr, simconfig.lc_nPort, &my_addr4, &my_addr6);
	if (socket >= 0)
	{
		if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&size, sizeof(size)) == -1)
		{
			close(socket);
			vWriteErrlogInfo("Can't set sockopt to socket %d", socket);
			return 0;
		}
		//bind and listen on the socket
		if (isipv6) 
		{
			if (bind(socket, (struct sockaddr *)&my_addr6, sizeof (my_addr6)) == -1) 
			{
				close(socket);
				vWriteErrlogInfo("Can't bind the socket %d", socket);
				return 0;
			}
			if (listen(socket, BACKLOG) == -1)
			{
				close(socket);
				vWriteErrlogInfo("Can't listen by socket %d", socket);
				return 0;
			}
			int sockbufsize6 = 0;
			size = sizeof(int); 
			err = getsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char *)&sockbufsize6, (unsigned*) &size);
		}
		else 
		{
			if (bind(socket, (struct sockaddr *)&my_addr4, sizeof (my_addr4)) == -1)
			{
				close(socket);
				vWriteErrlogInfo("Can't bind the socket %d", socket);
				return 0;
			}
			if (listen(socket, BACKLOG) == -1) {
				close(socket);		
				vWriteErrlogInfo("Can't listen by socket %d", socket);
				return 0;
			}
			int sockbufsize4 = 0; size = sizeof(int); 
			err = getsockopt(socket, SOL_SOCKET, SO_RCVBUF, (char *)&sockbufsize4, (unsigned*) &size);
		}
		//end bind and listen
		while (!bIsListenerStop) 
		{
			sin_size6 = sizeof their_addr6;
			sin_size4 = sizeof their_addr4;
			
			memset(mess,0,sizeof(mess));
			// Accept the connection from the client
			if (isipv6)
				newSocket6 = accept(socket, (struct sockaddr *)&their_addr6, (unsigned*) &sin_size6);
			else 
				newSocket4 = accept(socket, (struct sockaddr *)&their_addr4, (unsigned*) &sin_size4);			
			newSocket = (newSocket6 > 0) ? newSocket6 :newSocket4;
			char clientIP[80];	
			if (newSocket < 0) 
			{
				vWriteErrlogInfo("ERROR on accept");
				continue;
			}
			if (newSocket6 > 0) 
				inet_ntop(AF_INET6, &their_addr6.sin6_addr, clientIP, sizeof(clientIP));
			else 
				inet_ntop(AF_INET, &their_addr4.sin_addr, clientIP, sizeof(clientIP));
			bzero(buffer,256);

			while ( (n = recv(newSocket, buffer, sizeof(buffer), 0)) > 0) 
			{		
				buffer[n] = '\0';
				strcat(mess,buffer);
			}
			//do any to resolve message
			vWriteErrlogInfo("Handle socket request:%s",mess);
			handleRequest(mess,response);		
			vWriteErrlogInfo("response lis:%s",response);
			//code send response for client in here
			n = send(newSocket, response, strlen(response), 0);
			if (n < 0) 
			{
				vWriteErrlogInfo("ERROR writing to socket");
			}
			close(newSocket);
		}
		//disconnect
		close(socket);
	}
	else 
	{
		vWriteErrlogInfo("Can't get socket from [IP:PORT]: [%s,%d]",simconfig.lc_ipAddr, simconfig.lc_nPort);
	}
	vWriteErrlogInfo("End of listening");

	return 0;
}

void Usage(char* appName)
{
    fprintf(stdout, "%s - simulator \n",appName);
    fprintf(stdout, "usage: %s [options] \n",appName);
    fprintf(stdout, "Available options:\n");
    fprintf(stdout, "start                 New instance will be started\n");
    fprintf(stdout, "stop                  Simulator will be stopped\n");
	fprintf(stdout, "status                The status of simulator will be output on the screen\n");
}

int start()
{
	int i;
	printf("Start of function start()");
	vWriteErrlogInfo("Start of function start()");

	nSimulatorStatus = RUNNING;
	if(isLocked(PROGRAME_NAME) != 0)
	{ 
		vWriteErrlogInfo("Already exist an instance is running.");
		return 0;
	}
/*	struct Room r;
	r.height= 50;
	r.width = 14;
	r.roomID = 5;
	r.sensor1.x = 8;
	r.sensor1.y = 10;
	r.sensor2.x =14;
	r.sensor2.y = 6;
	r.sensor3.x = 1;
	r.sensor3.y = 1;
	r.max_object = 10;
	addRoom(r);*/
	char str[] = "addroom <SENSOR2>:(9,10);<SENSOR3>:  (7,5);<WIDTH>:   85;<HEIGHT>: 90;<SENSOR1>:(7,2);<MAX OJ>: 5";
	receiveAddingRoomMessage(str);
	startAllRoom();
	/*while(1){
		int id;
		printf("Enter id room you wana stop: "); scanf("%d", &id);
		removeRoom(id);
	}*/
	// create lisening thread
	thread_t listen_thread = 1;
	int hThread = pthread_create(&listen_thread, NULL,listener, NULL);
	if (hThread !=0)
	{
		vWriteErrlogInfo("Couldn't create thread" );
		return 0;
	}

	pthread_join(listen_thread, NULL );
	return 1;
}

int main(int argc, char **argv)
{
	sem_init(&semaphore_file, 0, 1);
	sem_init(&semaphore_room, 0, 1);
	vWriteErrlogInfo("Start simulator");
	char strRequest[REG_MAX_LEN];
	bool bIsNeedToSendReq;
	int nSentReturn;
	memset(strRequest,0,sizeof(strRequest));
	if(argc < 2){
		vWriteErrlogInfo("Invalid number of parameters, %d!",argc);
		Usage(argv[0]);
		exit(1);
	}
	if(!loadConfig(CONFIG_PATH))
	{
		printf("\ncan not load config!");
		exit(1);
	}

	strncpy(strRequest,argv[1],sizeof(strRequest));
	if(strcmp(strRequest,START_MESG) == 0)
	{
		start();
	}	
	else if(strcmp(strRequest,STOP_MESG) == 0){
		//re-confirm stop message
		char strConfirm[8];
		memset(strConfirm,0,sizeof(strConfirm));
		printf("Are you sure you want to stop Simulator? Y or N:");
		scanf("%s", &strConfirm);
		if(strcmp(strConfirm,"y") == 0)
		{
			bIsNeedToSendReq = true;
		}
	}
	else if(strcmp(strRequest,STATUS_MESG) == 0)
	{
		bIsNeedToSendReq = true;
	}
	else
	{
		Usage(argv[0]);
		exit(1);
	}
	if(bIsNeedToSendReq)
	{
		nSentReturn = sendRequest(strRequest,simconfig.lc_ipAddr,simconfig.lc_nPort);
			if(nSentReturn == 0)
				printf("Can't send soket message \"%s\" to Simulator \n",strRequest);
	}
	vWriteErrlogInfo("simulator will be exited!");
}
