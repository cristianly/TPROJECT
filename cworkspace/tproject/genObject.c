#include <time.h>
#include <malloc.h>
#include <pthread.h>

#define RADIUS_MIN 0.1
#define RADIUS_MAX 1
#define LIFETIME_MAX 20
#define LIFETIME_MIN 5
#define DIST_MIN 0.1
#define DIST_MAX 1
#define OJB_MININTERVAL 2
#define OJB_MAXINTERVAL 10
#define RAND_SCALE 10


 struct object{
		float radius;
		unsigned short pos_x;
		unsigned short pos_y;
		unsigned int lifetime;
		unsigned int interval;
		float moving;
};

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
}

int main(){
	pthread_t toj;
	struct object o1 = genObject(15, 10);
	pthread_create(&toj, NULL, liveObject, (void*)&o1);

	while (1){
		printf("r = %f m\n x = %d\n y = %d\n lifetime = %d s\n interval = %d s\n moving distant = %f m \n\n", o1.radius, o1.pos_x, o1.pos_y, o1.lifetime, o1.interval, o1.moving);
		sleep(o1.interval);
	}

}
