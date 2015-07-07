#include <stdio.h>


#define FILE_PATH "server.conf"
#define CONFIG_IPADDR "<IP_ADDR>:"
#define CONFIG_PORT "<PORT_NUM>:"
#define CONFIG_MAXCONN "<MAX_CLI>:"

char ip_addr[16] = "";
int port = -1;
int max_conn = -1;

	void load_operator_config_setvalue(char* sfield, char* svalue){
		if (strcmp(CONFIG_IPADDR, sfield) == 0){
			strcpy(ip_addr, svalue);
		}
		if (strcmp(CONFIG_PORT, sfield) == 0){
			port = atoi(svalue);
		}
		if (strcmp(CONFIG_MAXCONN, sfield) == 0){
			max_conn = atoi(svalue);
		}
	}

	/*
	 * load_operator_config load server configuration from FILE_PATH
	 * success return 1
	 * failure return 0
	 */
	int load_operator_config(){
		char sbuff[50],
			 slabel[34],
			 scontent[16];
		int pos,
			slen,
			clen;
		FILE *fp;

		fp = fopen(FILE_PATH, "r");
		if (!fp){
			printf("Can not open file at %s", FILE_PATH);
			exit(1);
		}

		while (fgets(sbuff, 50, fp) != NULL) {
			slen = strlen(sbuff) - 1;

			pos = 0;
			while ((sbuff[pos++] != ':') && (pos < slen)) ;
			strncpy(slabel, sbuff, pos);
			slabel[pos] = '\0';

			strcpy(scontent, "");
			clen = 0;
			while (pos < slen){
				scontent[clen++] = sbuff[pos++];
			}
			scontent[clen] = '\0';

			load_operator_config_setvalue(slabel, scontent);
		}
		if ((port < 0) || (max_conn < 0) || (strcmp(ip_addr, "") == 0)){
			return 0;
		}
		return 1;
	}

int main(){
		printf("init return %d\n", load_operator_config());
	return 0;
}
