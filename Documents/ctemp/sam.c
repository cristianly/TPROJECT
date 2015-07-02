#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <unistd.h>
union {
	int val;
	struct semid_ds* buf;
	ushort *array;
} carg;

int seminit(int val) {
	int i, semid;
	semid=semget(IPC_PRIVATE, 1, 0666|IPC_CREAT);
	
	if (semid==-1) return(-1);
	
	carg.val=1;
	
	if (semctl(semid, val, SETVAL, carg)==-1) return(-1);
	return (semid);
}

void p(int sem)
{
	struct sembuf pbuf;
	pbuf.sem_num=0;
	pbuf.sem_op=-1;     
	pbuf.sem_flg=SEM_UNDO;
	if (semop(sem, &pbuf, 1)== -1) 
	{
		perror("semop");
		exit(1);
	}
}

void v(int sem)
{
	struct sembuf vbuf;
	vbuf.sem_num=0;
	vbuf.sem_op=1;
	vbuf.sem_flg=SEM_UNDO;
	if (semop(sem, &vbuf, 1)== -1)
	{
		perror("semop");
		exit(1);
	}
	
}
int	semrel(int semid)
{
	return (semctl(semid, 0,IPC_RMID, 0));
}

void func(int sem) 
{
	while(1) {
		p(sem);     /* enter section */
		printf("%d Do s.t. in CS \n",getpid());
		sleep(1);
		//v(sem);    /* exit section */
		printf("\n%d Out of	CS \n",getpid());
		sleep(1);
	}
}

void main() {
	int sem;
	sem=seminit(0);
	
	if(fork()==0)
	{
		func(sem);
	}
	else func(sem);
}
