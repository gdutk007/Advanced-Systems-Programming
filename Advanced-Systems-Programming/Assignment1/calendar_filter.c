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

struct event_t { 
    char Action[2];
    char Title[11];
    char Date[11];
    char Time[6];
    char Location[11];
};

struct eventList_t { 
    struct event_t Event;
    struct eventList_t * nextEvent;
};

static struct eventList_t * calendarPtr = NULL;
static struct eventList_t * deletePtr = NULL;

struct event_t parseInput(char * buff);
struct eventList_t * SearchEventByTitle(const char * title);
struct eventList_t * SearchEventByDate(const char * date);
struct eventList_t * Search_EventByTitle_Date_Time(struct event_t Event);
struct eventList_t * SearchEarliestEventByDate(char * date);

struct eventList_t * deleteNode( struct eventList_t * curr ,struct eventList_t * node);
void CreateEvent(struct event_t Event);
void EditEvent(struct event_t Event);
void DeleteEvent(struct event_t Event);
void AppendEvent(struct event_t Event);
void printCalendar();
int isTimeEarlier(char * time1, char * time2);
int compareEvents(struct event_t event1, struct event_t event2);


void ProcessCalendar(struct event_t Event){
    switch (Event.Action[0])
    {
    case 'C':
        //Create Event
        CreateEvent(Event);
    break;
    case 'X':
        // Edit Event
        EditEvent(Event);
    break;
    case 'D':
        // Delete Event
        DeleteEvent(Event);
    break;
    default:
        //wtf
        printf("Default\n");
        break;
    }
}

int main(){
	char fileOutput[BUFFERSIZE];
	while( 1 ){
		char line[BUFFERSIZE];
		memset(line, 0x00, BUFFERSIZE);
		if ( fgets(line , BUFFERSIZE , stdin) == NULL ){
			break;
		}
		char str[BUFFERSIZE];
		memset(str, 0x00, BUFFERSIZE);
		strcpy(str, line);
		//printf("%s: %s",__func__,str);
        struct event_t Event = parseInput(str);
        if(strlen( Event.Action) != 0 && strlen( Event.Title) != 0 && strlen( Event.Time) != 0  ){
            ProcessCalendar(Event);
        }else{
            printf("Event could not be parsed \n");
        }
        
	}
    //printf("\n\n\n");
    //printCalendar();
    return 0;
}


struct event_t parseInput(char * buff){
	parse_state_t state = ACTION;
    struct event_t Event; 
	char * token = strtok(buff,",");	
	while(1){
		switch(state){
			case ACTION:
				if(token == NULL) return Event;
				strcpy(Event.Action, token);
                state = TITLE;
			break;
			case TITLE:
				if(token == NULL) return Event;
				token = strtok(NULL,",");
                strcpy(Event.Title, token);
				state = DATE;
			break;
			case DATE:
				if(token == NULL) return Event;
				token = strtok(NULL,",");
                strcpy(Event.Date, token);
				state = TIME;
			break;
			case TIME:
				if(token == NULL) return Event;
				token = strtok(NULL,",");
                strcpy(Event.Time, token);
				state = LOCATION;
			break;
			case LOCATION:
				if(token == NULL) return Event;
				token = strtok(NULL,",");
                memcpy(Event.Location, token, 8);
                Event.Location[9] = ' ';
                Event.Location[10] = '\0';
                return Event;
			break;	
			default:
				//printf("the line is mal-formed: %s ", token);
				return Event;
			break;
		}
	}
}

struct eventList_t * SearchEventByTitle(const char * title){
    struct eventList_t * temp = calendarPtr;
    while(temp){
        if(!strcmp(title,temp->Event.Title)){
            return temp;
        }
        temp = temp->nextEvent;
    }
    return NULL;
}

struct eventList_t * SearchEventByDate(const char * date){
    struct eventList_t * temp = calendarPtr;
    while(temp){
        //printf("%s: Date1:%s Date2:%s \n",__func__,temp->Event.Date,date);
        if(!strcmp(date,temp->Event.Date)){
            return temp;
        }
        temp = temp->nextEvent;
    }
    return NULL;
}

int isTimeEarlier(char * time1, char * time2){
    if( !strcmp(time1,"--:--")) return -1;
    if( !strcmp(time2,"--:--")) return -1;

    char t1[3];
    char t2[3];
    // tokenize
    t1[0] = time1[0];
    t1[1] = time1[1];
    t1[2] = '\0';
    t2[0] = time2[0];
    t2[1] = time2[1];
    t2[2] = '\0';
    // convert to int
    int num1 = (int) strtol(t1, (char **)NULL, 10);
    int num2 = (int) strtol(t2, (char **)NULL, 10);

    // if earlier hour return
    if(num1 < num2) return 0;

    // tokenize
    t1[0] = time1[3];
    t1[1] = time1[4];
    t1[2] = '\0';
    t2[0] = time2[3];
    t2[1] = time2[4];
    t2[2] = '\0';
    // convert to int
    num1 = (int) strtol(t1, (char **)NULL, 10);
    num2 = (int) strtol(t2, (char **)NULL, 10);
    // if earlier return
    if(num1 < num2) return 0;
    return -1;
}

struct eventList_t * SearchEarliestEventByDate(char * date){
    struct eventList_t * temp = calendarPtr;  
    struct eventList_t * earliestEvent = NULL;

    //printf("*****%s: date:%s \n",__func__,date);
    while(temp){
            if(temp && !strcmp(date,temp->Event.Date)){
                //printf("time1:%s, time2:%s****\n",temp->Event.Time,earliestEvent->Event.Time);
                if(!earliestEvent) earliestEvent = temp;
                if(isTimeEarlier(temp->Event.Time,earliestEvent->Event.Time) == 0){
                    earliestEvent = temp;
                    //printf("comparison is true, time is earlier \n");
                }else{
                    //printf("comparison is false, time is NOT earlier \n");
                }
            }
        temp = temp->nextEvent;
    }
    //printf("we are returning\n");
    return earliestEvent;
}

void AppendEvent(struct event_t Event){
    struct eventList_t * temp = calendarPtr;
    if(!temp){
        calendarPtr = (struct eventList_t *)malloc(sizeof(struct eventList_t));
        calendarPtr->Event = Event;
        calendarPtr->nextEvent = NULL;
        return; 
    }else{
        while(temp->nextEvent) temp = temp->nextEvent;        
    }
    temp->nextEvent = (struct eventList_t *)malloc(sizeof(struct eventList_t));
    temp->nextEvent->Event = Event;
    temp->nextEvent->nextEvent = NULL;   
}

int compareEvents(struct event_t event1, struct event_t event2){
    if( !strcmp(event1.Action,event2.Action) && !strcmp(event1.Date,event2.Date)
        && !strcmp(event1.Location,event2.Location) && !strcmp(event1.Time,event2.Time)){
            return 0;
    }
    return -1;  
}

void printCalendar(){
    struct eventList_t * temp = calendarPtr;
    while (temp)
    {
        printf("%s,%s,%s,%s,%s \n",temp->Event.Action,temp->Event.Title,temp->Event.Date,temp->Event.Time,temp->Event.Location);
        temp = temp->nextEvent;
    }
}

void CreateEvent(struct event_t Event){
    struct eventList_t * calendarEvent = SearchEventByDate(Event.Date);
    if(!calendarEvent){
        AppendEvent(Event);
        printf("%s,%s,%s \n",Event.Date,Event.Time,Event.Location);
        return; // early return since calendar was empty and has one item
    }
    AppendEvent(Event);
    calendarEvent = SearchEarliestEventByDate(Event.Date);

    if( calendarEvent && !strcmp(calendarEvent->Event.Action,Event.Action) && !strcmp(calendarEvent->Event.Date,Event.Date)
        && !strcmp(calendarEvent->Event.Location,Event.Location) && !strcmp(calendarEvent->Event.Time,Event.Time)){
        printf("%s,%s,%s \n",calendarEvent->Event.Date,
                                calendarEvent->Event.Time,calendarEvent->Event.Location);
    }
}

void EditEvent(struct event_t Event){
   // find event by title
   struct eventList_t * calendarEvent = SearchEventByTitle(Event.Title);
   if(!calendarEvent){
       printf("%s: Title does not exist\n",__func__);
       return;
   }
   //printf("we made it this far\n");
    // see if event we're editing is the earliest and if location has changed, if yes, print event.
   int isLocationChanged = -1;
   if(strcmp(Event.Location,calendarEvent->Event.Location) != 0 ){
       isLocationChanged = 0;
   }
   calendarEvent->Event = Event;
   calendarEvent = SearchEarliestEventByDate(calendarEvent->Event.Date);
   if( !isLocationChanged && calendarEvent && !compareEvents(calendarEvent->Event,Event)){
        printf("%s,%s,%s \n",calendarEvent->Event.Date,
                                calendarEvent->Event.Time,calendarEvent->Event.Location);
   }
}

struct eventList_t * deleteNode( struct eventList_t * curr ,struct eventList_t * node){
    if(!curr) return NULL;

    if(curr == node){
        deletePtr = curr;
        return curr->nextEvent;
    }
    curr->nextEvent = deleteNode( curr->nextEvent ,node);
     if(deletePtr){
         deletePtr->nextEvent = NULL;
         free(deletePtr);
         deletePtr = NULL;
     }
     return curr;
}

struct eventList_t * Search_EventByTitle_Date_Time(struct event_t Event){
   struct eventList_t * temp = calendarPtr;
   while(temp){
       if(!strcmp(temp->Event.Date,Event.Date) && !strcmp(temp->Event.Time,Event.Time) && !strcmp(temp->Event.Title,Event.Title) ){
           return temp;
       }
       temp = temp->nextEvent;
   } 
   return NULL;
}

void DeleteEvent(struct event_t Event){
    struct eventList_t * temp = Search_EventByTitle_Date_Time(Event);
    if(temp){
        struct eventList_t * earliestEvent = SearchEarliestEventByDate(Event.Date);
        if( earliestEvent && !compareEvents(temp->Event,earliestEvent->Event)){
            calendarPtr = deleteNode(calendarPtr, temp);
            earliestEvent = SearchEarliestEventByDate(Event.Date);
            if(earliestEvent){
                printf("%s,%s,%s \n",earliestEvent->Event.Date,
                                        earliestEvent->Event.Time,earliestEvent->Event.Location);
            }
        }else{
            calendarPtr = deleteNode(calendarPtr, temp);
        }
        if(!SearchEventByDate(Event.Date)){
            printf("%s,--:--,NA\n",Event.Date);
        }
    }
}