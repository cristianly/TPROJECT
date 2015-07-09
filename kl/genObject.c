#include <time.h>
#include <malloc.h>
#include <pthread.h>

#define RADIUS_MIN 0.1
#define RADIUS_MAX 1
#define LIFETIME_MAX 50
#define LIFETIME_MIN 5
#define DIST_MIN 0.1
#define DIST_MAX 1
#define OJB_MININTERVAL 5
#define OJB_MAXINTERVAL 10
#define RAND_SCALE 10
#define GENOBJECT_MINTIME 4
#define GENOBJECT_MAXTIME 6
#define SENDDT_INTERVAL 4
#define OBJECT_RELEASE 1
#define NUM_MAX_OBJECT 5
#define NUM_SENSOR_IN_ROOM 3
#define NULL_RADIUS_OBJECT -1

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


struct sensor{
	unsigned int pos_x;
	unsigned int pos_y;
};
struct object{
		float radius;
		unsigned short pos_x;
		unsigned short pos_y;
		unsigned int lifetime;
		unsigned int interval;
		float moving;
};

 struct room{
 	struct sensor sensor[NUM_SENSOR_IN_ROOM];
 	struct object objects[NUM_MAX_OBJECT];
 	unsigned int num_object;
 	unsigned int width;
 	unsigned int height;
 	unsigned int id;
 };

 int stop_flag = -1;
 pthread_t toj;


float random_segment(unsigned int from,unsigned int to){
	srand(time(NULL));
	return (float)(rand()%(to*RAND_SCALE - from*RAND_SCALE + 1) + from*RAND_SCALE)/RAND_SCALE;
}

 struct object genObject(int room_max_x, int room_max_y){
	struct object ob;// = malloc(sizeof(object));
	ob.radius = random_segment(RADIUS_MIN, RADIUS_MAX);
	ob.pos_x = (int)random_segment(0, room_max_x);
	ob.pos_y = (int)random_segment(0, room_max_y);
	ob.lifetime = (int)random_segment(LIFETIME_MIN, LIFETIME_MAX);
	ob.interval = (int)random_segment(OJB_MININTERVAL, OJB_MAXINTERVAL);
	ob.moving = random_segment(DIST_MIN, DIST_MAX);
	return ob;
}

void* liveObject(void* arg){
	int x, y;
	struct object* oj = (struct object*)arg;
	while ((int)oj->lifetime > 0){
		x = random_segment(oj->pos_x - oj->moving, oj->pos_x + oj->moving);
		y = random_segment(oj->pos_y - oj->moving, oj->pos_y + oj->moving);
		oj->pos_x = (x > oj->radius) ? x : oj->radius;
		oj->pos_y = (y > oj->radius) ? y : oj->radius;
		oj->lifetime -= oj->interval;
		sleep(oj->interval);
	}
	oj->radius 	= NULL_RADIUS_OBJECT;
}
void* liveRoom(void* arg){
	 unsigned long time_counter = 0; //using it as timer
	 unsigned int time_period = 0;  //using for count down  to generate new object in room
	 int pos = 0;
	 struct room *thisroom = (struct room*)arg;
	 for (pos = 0; pos < NUM_MAX_OBJECT; pos++){
		 thisroom->objects[pos].radius = NULL_RADIUS_OBJECT;
	 }

	 while (stop_flag != thisroom->id){
		 //Check numbers of object.
		 thisroom->num_object = 0;
		 for (pos = 0; pos < NUM_MAX_OBJECT; pos++){
			 if (thisroom->objects[pos].radius != NULL_RADIUS_OBJECT){
				 thisroom->num_object++;
			 }
		 }
		 //Generate object belong with time period
		 pthread_t toj;
		 if (time_period == 0){
			 if (thisroom->num_object < NUM_MAX_OBJECT){
				 pos = 0;
				 while  ((thisroom->objects[pos].radius != NULL_RADIUS_OBJECT) && (pos < NUM_MAX_OBJECT -1)) pos++;
				 thisroom->objects[pos] = genObject(thisroom->width, thisroom->height);
				 pthread_create(&toj, NULL, liveObject, (void*)&thisroom->objects[pos]);
			 }
			 time_period = random_segment(GENOBJECT_MINTIME, GENOBJECT_MAXTIME);
		 }

		 time_period--;
		 time_counter++;
		 if (time_counter % SENDDT_INTERVAL == 0){
			 for (pos = 0; pos < NUM_MAX_OBJECT; pos++){
				 if (thisroom->objects[pos].radius > 0){
					/*
					 	 *send data of object to operation program here
					 */
				 }
			 }
		 }
		 sleep(1);
	 }
 }


int main(){
	//test
	struct room  r;
	r.height = 20;
	r.id = 1;
	r.num_object = 0;
	r.width = 15;

	pthread_t trom;
	 pthread_create(&trom, NULL, liveRoom, (void*)&r);
	 pthread_join(trom, NULL);
	/*while (1){
		printf("r = %f m\n x = %d\n y = %d\n lifetime = %d s\n interval = %d s\n moving distant = %f m \n\n", o1.radius, o1.pos_x, o1.pos_y, o1.lifetime, o1.interval, o1.moving);
		sleep(o1.interval);
	}*/
	 return 0;

}
