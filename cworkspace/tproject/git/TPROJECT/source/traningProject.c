/*
 ============================================================================
 Name        : traningProject.c
 Author      : Qui Nghia
 Version     :
 Copyright   : Your copyright notice
 Description : proccess main in project
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "function_base.c"

void demo_gen_object_position();


int main(void) {
	demo_gen_object_position();
	return EXIT_SUCCESS;
}


void demo_gen_object_position(){
	set_time_rand();
	struct room roo;

	roo.height = 100;
	roo.width = 100;
	roo.id = 1;
	roo.num_object = 0;

	addObject(&roo, 50, 50, 50, 100, 1, 1);
	addObject(&roo, 50, 50, 50, 100, 1, 1);

	puts(genObjectPosition(roo));
}
