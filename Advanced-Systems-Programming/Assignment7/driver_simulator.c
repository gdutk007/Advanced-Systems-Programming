#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "simulator.h"

// process two
void usb_kdb_open();
void input_report_key();
void usb_kbd_irq();
void usb_submit_urb();
void usb_kbd_led();
void usb_kbd_event();
static void * interrupt_thread(void* arg);
static void * control_thread(void* arg);

int main(){
    
    return 0;
}

void usb_kdb_open(){

}

void input_report_key(){

}

void usb_kbd_irq(){

}

void usb_submit_urb(){

}

void usb_kbd_led(){

}

void usb_kbd_event(){

}

static void * interrupt_thread(void * arg){

}

static void * control_thread(void * arg){

}