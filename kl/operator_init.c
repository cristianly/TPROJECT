#include "operator.h"

/*
 * init_operator
 * return 0, command error
 * return 1, start program
 * return 2, stop program
 */


void create_socket_operator_listening(){
	 struct sockaddr_in serv_addr;
	 int  n;

	   sockfd = socket(AF_INET, SOCK_STREAM, 0);

	   if (sockfd < 0)
	      {
			  perror("ERROR opening socket");
			  exit(1);
	      }

	   bzero((char *) &serv_addr, sizeof(serv_addr));

	   serv_addr.sin_family = AF_INET;
	   serv_addr.sin_addr.s_addr = inet_addr(ip_addr);
	   serv_addr.sin_port = htons(port);


	   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	   {
	      perror("ERROR on binding");
	      exit(1);
	   }

	   listen(sockfd, 50);
}

void request_operator_stop(){

	   int sockfd, n;
	   struct sockaddr_in serv_addr;

	   /* Create a socket point */
	   sockfd = socket(AF_INET, SOCK_STREAM, 0);

	   if (sockfd < 0)
	   {
	      perror("ERROR opening socket");
	      exit(1);
	   }

	   bzero((char *) &serv_addr, sizeof(serv_addr));
	   serv_addr.sin_family = AF_INET;
	   serv_addr.sin_port = htons(port);

	   if(inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr)<=0)
	      {
	          printf("\n inet_pton error occured\n");
	          exit(1);
	      }

	   if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	      {
	         printf("\n Error : Connect Failed \n");
	         exit(1);
	      }

	   n = write(sockfd, FLAG_STOP, strlen(FLAG_STOP));
}

int init_operator(int argc, char* argv[]){
	char comm[10];

	if (argc != 2) {
		printf("Please type follow style: ./%s [%s/%s]\n", PROGRAM_NAME, START, STOP);
		exit(1);
	}

	strcpy(comm, argv[1]);

	if (strcmp(comm, START)){
		create_socket_operator_listening();
		return 1;
	}

	if (strcmp(comm, STOP)){
		request_operator_stop();
		return 2;
	}

	return 0;
}


int main(int argc, char* argv[]){
	init_operator(argc, argv);

	return 0;
}
