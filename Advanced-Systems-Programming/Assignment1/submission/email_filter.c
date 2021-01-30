#include <stdio.h>    
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFERSIZE 2000

typedef enum {
	ACTION,
	TITLE,
	DATE,
	TIME,
	LOCATION,
	NONE
} parse_state_t;

int checkAction(char * buff){
	if(buff == NULL) return -1;
	for(unsigned int i = 0; i < strlen(buff); ++i){
		if(buff[i] == 'C' || buff[i] == 'D' || buff[i] == 'X') {
			return 0;
		}	
	}
	//printf("%s: failed to find action: %s",__func__,buff);
	return 1;
}

int checkTitle_Location_Length(char * buff){
	if(buff == NULL) return -1;
	int length = strlen(buff);
	if(strlen(buff) == 10){
		return 0;
	}else{
		if(length == 11 && buff[length-1] == '\n' ){
			return 0;
		} else if(length == 12 && buff[length-2] == '\n' && buff[length-1] == '\r'){
			return 0;
		}
		//printf("Title or location is not 10 characters: size: %lu, string:'%s'", strlen(buff),buff);
	}
	return 1;
}

int checkDate(char * buff){
	if(!buff) return -1;
	if(strlen(buff) != 10) return -1;
	if(buff[2] != '/' && buff[5] != '/') return -1;
	for(int i = 0; i < strlen(buff); ++i){
		if(i != 2 && i != 5){
			if(!isdigit(buff[i])) return -1;
		}
	}
	return 0;
}

int checkTime(char * buff){
	if(!buff) return -1;
	if(strlen(buff) != 5) return -1;
	if(buff[2] != ':') return -1;

	for(int i = 0; i < strlen(buff); ++i){
		if( i != 2){
			if(!isdigit(buff[i])) return -1;
		}
	}
	return 0;
}

int isWellFormed(char * buff){
	parse_state_t state = ACTION; 
	char * token = strtok(buff,",");	
	// while(token){
	// printf("token: %s \n",token);
	// token = strtok(NULL,",");
	// }
	// return -1;	
	while(1){
		switch(state){
			case ACTION:
				if(token == NULL) return 1;
				state = (!checkAction(token)) ? TITLE : NONE;
			break;
			case TITLE:
				if(token == NULL) return 1;
				token = strtok(NULL,",");
				state = (!checkTitle_Location_Length(token)) ? DATE : NONE;
			break;
			case DATE:
				if(token == NULL) return 1;
				token = strtok(NULL,",");
				state = (!checkDate(token)) ? TIME : NONE;
			break;
			case TIME:
				if(token == NULL) return 1;
				token = strtok(NULL,",");
				state = (!checkTime(token)) ? LOCATION : NONE;
			break;
			case LOCATION:
				if(token == NULL) return 1;
				token = strtok(NULL,",");
				state = (!checkTitle_Location_Length(token)) ? TITLE : NONE;
				return (state == TITLE) ?  0 :  1 ;
			break;	
			default:
				//printf("the line is mal-formed: %s ", token);
				return 1;
			break;
		}
	}
}

void emailFilter(char * line){
	char result[BUFFERSIZE];
	
}

int main(){ 	
	setvbuf(stdin,NULL,_IONBF,0);
	setvbuf(stdout,NULL,_IONBF,0);
	char fileOutput[BUFFERSIZE];
	int eventNumber = 0; 	
	while( 1 ) {
		char line[BUFFERSIZE];
		memset(line, 0x00, BUFFERSIZE);
		if ( fgets(line , BUFFERSIZE , stdin) == NULL ){
			break;
		}
		//printf("%s: %s",__func__,line);
		char str[BUFFERSIZE];
		memset(str, 0x00, BUFFERSIZE);
		strcpy(str, line);
		int val = 0;
		val = isWellFormed(&str[9]);
		if(!val){
			printf("%s",&line[9] );
			++eventNumber;
		}
		// else{
		// 	printf("***%s: val: %i Line is MAL formed: %s", __func__, val , line );
		// }
	}
	//printf("total Number of events: %i \n",eventNumber);
	return 0;   

}



