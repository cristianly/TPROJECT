/*
 ============================================================================
 Name        : function_base.c
 Author      : Qui Nghia
 Version     :
 Copyright   :
 Description : define struct struct and function of file function_base.h
 ============================================================================
 */


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "function_base.h"

struct sensor{
	unsigned int pos_x;
	unsigned int pos_y;
};

struct object{
	unsigned int radius;
	unsigned short pos_x;
	unsigned short pos_y;
	unsigned int time_life;
	unsigned int time_period;
	unsigned short dist_moving;
};

struct room{
	struct sensor sensor[NUM_SENSOR_IN_ROOM];
	struct object objects[NUM_MAX_OBJECT];
	unsigned int num_object;
	unsigned int width;
	unsigned int height;
	unsigned int id;
};

void set_time_rand(){
	srand(time(NULL));
}

// random in segment from-to, example: random_segment(2,4) => {2, 3, 4}
int random_segment(unsigned int from,unsigned int to){
	return rand()%(to - from + 1) + from;
}

char* sensor_itoa(int n){
	char *s = (char*)malloc(20);
	strcpy(s,"");
	snprintf(s,10,"%d",n);
	return s;
}

char* genObject(struct room roo, unsigned int object_index){
	char *s = (char*)malloc(20);
	char *rs = (char*)malloc(60);
	unsigned int i, x1, x2, y1, y2, rad, pos_x, pos_y;

	pos_x = roo.objects[object_index].pos_x;
	pos_y = roo.objects[object_index].pos_y;

	strcpy(s,"");
	strcpy(rs,"");
	rad = roo.objects[object_index].radius;
	x1 = (pos_x - rad < 0)? 0 : pos_x - rad;
	x2 = (pos_x + rad > roo.width)? roo.width : pos_x + rad;
	y1 = (pos_y - rad < 0)? 0 : pos_y - rad;
	y2 = (pos_y + rad > roo.height)? roo.height : pos_y + rad;

	for(i = 0 ; i < NUM_SENSOR_IN_ROOM ; i ++){
		sprintf(s," %d %d ,",random_segment(x1, x2), random_segment(y1, y2));
		strcat(rs,s);
	}
	//puts("");
	if(strlen(rs) >= 2)
		rs[strlen(rs) - 2] = '\0';
	free(s);
	return rs;
}

char* genObjectPosition(struct room roo){

	int i;
	char *s = (char*)malloc(100);
	char *c_tmp;

	snprintf(s,100,"ROOM_ID %d NUM_OBJECT %d ;",roo.id, roo.num_object);
	for(i = 0 ; i < roo.num_object ; i ++){
		c_tmp = genObject(roo,0);
		strcat(s, c_tmp);
		free(c_tmp);
		strcat(s," ;");
		//puts(s);
	}
	strcat(s,"*");
	return s;
}

void addObject(struct room *roo,
		unsigned int radius,
		unsigned short pos_x,
		unsigned short pos_y,
		unsigned int time_life,
		unsigned int time_period,
		unsigned int dist_moving){

	int index = roo->num_object;
	roo->num_object++;

	roo->objects[index].radius = radius;
	roo->objects[index].pos_x = pos_x;
	roo->objects[index].pos_y = pos_y;
	roo->objects[index].time_life = time_life;
	roo->objects[index].time_period = time_period;
	roo->objects[index].dist_moving = dist_moving;
}



