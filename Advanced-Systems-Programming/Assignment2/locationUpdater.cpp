#include <iostream>
#include <bits/stdc++.h>
#include <cstdlib>
#include <string>

using namespace std;


typedef enum {
	ACTION,
	TITLE,
	DATE,
	TIME,
	LOCATION,
	NONE
} parse_state_t;

typedef struct {
    string Action;
    string Title;
    string Date;
    string Time;
    string Location;
} Event_t ;

static int BufferLength;

map<string, Event_t> CalendarFilter;

vector<string> ValidEmails;

vector<Event_t> Event_Buffer;

bool parseArguments(int argc, char *argv[]);

bool ParseValidEmails();

bool CheckEmailString(string line);

bool CheckAction(string action);
bool CheckTitle(string title);
bool CheckDate(string date);
bool CheckTime(string time);
bool CheckLocation(string location);

int main(int argc, char *argv[]){

    bool validArg = parseArguments(argc,argv);
    if(!validArg && BufferLength != 0){
        cout << "Please Enter Valid Arguments! Program Terminating\n";
        exit(0);
    }
    Event_Buffer.resize(BufferLength);

    ParseValidEmails();

    return 0;
}


bool parseArguments(int argc, char *argv[]){
    if(argc != 2){
        cout << "You've entered an incorrect number of arguments\n";
        return false;
    }
    string bufferLen(argv[1]);
    const long len = strtol(bufferLen.c_str(),nullptr,10); 

    if( len < 0){
        cout << "please enter a valid number.\n";
        return false;
    }
    BufferLength = len;
    return true;
}

bool ParseValidEmails(){
    for (string line; getline(cin, line);) {
        if(strlen(line.c_str()) > 9){
            size_t pos = line.find("Subject: ");
            if(pos != string::npos){
                CheckEmailString(line.substr (pos+strlen("Subject: ")));
            }
        }
    }
    return true;
}

bool CheckEmailString(string line){
    cout << line << endl;
    istringstream is(line);
    string part;
    parse_state_t state = ACTION;
    while (getline(is, part, ',')){
        cout << part << endl;
        switch (state)
        {
        case ACTION:
            /* code */
        break;
        case TITLE:
            /* code */
        break;
        case DATE:
            /* code */
        break;
         case TIME:
            /* code */
        break;
        case LOCATION:
            /* code */
        break;       
        default:
            break;
        }
    }

    return true;
}

bool CheckAction(string action){
    return true;
}
bool CheckTitle(string title){
    return true;
}
bool CheckDate(string date){
    return true;
}
bool CheckTime(string time){
    return true;
}
bool CheckLocation(string location){
    return true;
}
