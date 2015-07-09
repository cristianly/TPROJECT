#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>


#define FILE_PATH "server.conf"
#define CONFIG_IPADDR "<IP_ADDR>:"
#define CONFIG_PORT "<PORT_NUM>:"
#define CONFIG_MAXCONN "<MAX_CLI>:"

#define PROGRAM_NAME "operation"
#define START "start"
#define STOP "stop"
#define FLAG_STOP "fstop"

char ip_addr[16] = "";
int port = -1;
int max_conn = -1;

int sockfd;
