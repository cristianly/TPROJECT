/*
 * function_base.h
 *
 *  Created on: Jul 6, 2015
 *      Author: Qui Nghia
 */


#ifndef _DEFINE_BASE_H
#define _DEFINE_BASE_H 1

#define DELTA_MIN_POSITION_X 2
#define DELTA_MIN_POSITION_Y 2
#define RADIO_MIN_OBJECT 2
#define RADIO_MAX_PERCENT_OBJECT 0.25
#define NUM_SENSOR_IN_ROOM 3
#define NUM_MAX_OBJECT 10

struct sensor;
struct object;
struct room;

void set_time_rand();
int random_segment(unsigned int ,unsigned int );
char* sensor_itoa(int );
char* genObject(struct room , unsigned int );
char* genObjectPosition(struct room );
void addObject(struct room *, unsigned int, unsigned short, unsigned short, unsigned int, unsigned int, unsigned int );

#endif
