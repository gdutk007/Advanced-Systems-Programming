#include <stdio.h>    
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <ctype.h>

struct event_t {
    char event[256];
    char time[256];
    char location[256];
};

struct item_t {
    struct event_t calendarEvent;
    struct item_t * next;
};

struct bucket_t {
    char date[256];
    struct item_t * items;
    struct bucket_t * next;
};

static struct bucket_t * head = NULL;

struct bucket_t * createBucket(){
    struct bucket_t * t = NULL;
    t = (struct bucket_t*)malloc(sizeof(struct bucket_t));
    t->items = NULL;
    t->next = NULL;
    return t;
}

struct item_t * createItem(){
    struct item_t * item = NULL;
    item = (struct item_t *) malloc(sizeof(struct item_t));
    item->next = NULL;
    return item;
}




struct bucket_t * searchCalenderForDate(const char * date, int size){
    struct bucket_t * bucketPtr = head;
    struct item_t * itemPtr = NULL;
    while(bucketPtr){
        if(bucketPtr->items){
            if( !strcmp(bucketPtr->date,date) ){
                return bucketPtr;
            }
        }
        bucketPtr = bucketPtr->next;
    }
    return NULL;
}

struct item_t * searchCalenderDateForItem( struct bucket_t * bucketPtr, struct event_t event  ){
    struct item_t * itemList = bucketPtr->items;
    while (bucketPtr && itemList)
    {   if( strcmp(itemList->calendarEvent.event,event.event) && 
            strcmp(itemList->calendarEvent.location,event.location) && 
            strcmp(itemList->calendarEvent.time,event.time) ){
            return itemList;
        }
        itemList = itemList->next;
    }
    return NULL;
}

void createCalenderEvent(const char * date, int size, struct event_t event){
    struct bucket_t * bucketPtr = searchCalenderForDate(date,size);
    if(!bucketPtr){
        // create date entry and add event
        bucketPtr = createBucket();
        if(!head) head = bucketPtr;
    }
    // once we have a bucket pointer with the date,
    // add an event to the bucket
    struct item_t * itemPtr = bucketPtr->items;
    if(!itemPtr){
        itemPtr = createItem();
        itemPtr->calendarEvent = event;
        itemPtr->next = NULL;
    }else{
        while(itemPtr->next){
            itemPtr = itemPtr->next;
        }
        itemPtr->next = createItem();
        itemPtr->next->calendarEvent = event;
        itemPtr->next->next = NULL;
    }
}

void printCalendar(){
    struct bucket_t * headPtr = head;
    while(headPtr){
        struct item_t * itemPtr = itemPtr = headPtr->items;
        while(itemPtr){
            printf("%s,%s,%s,%s",headPtr->date, 
                    itemPtr->calendarEvent.event, 
                    itemPtr->calendarEvent.location, 
                    itemPtr->calendarEvent.time);
            itemPtr = itemPtr->next;
        }
        headPtr = headPtr->next;
    }
}

int main(){
    head = createBucket();
    printf("hello world\n");
    return 0;
}