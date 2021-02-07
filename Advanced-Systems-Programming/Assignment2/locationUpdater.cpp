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
    DONE,
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

multimap<string, Event_t> CalendarFilter;

vector<string> ValidEmails;

vector<Event_t> Event_Buffer;

bool parseArguments(int argc, char *argv[]);

bool ParseValidEmailsThread();

bool processCalendarThread();

bool CheckEmailString(string line);

bool CheckAction(string action);

bool CheckTitle(string title);

bool CheckDate(string date);

bool CheckTime(string time);

bool CheckLocation(string location);

bool processCalendar(string email);

bool CreateEvent(Event_t Event);

bool EditEvent(Event_t Event);

bool DeleteEvent(Event_t Event);

Event_t searchEarliestEventByDate(string date);

multimap<string, Event_t>::iterator searchEventByDateTitleTime(Event_t Event);

bool CompareTime(string time1, string time2);

bool CompareEvents(Event_t event1, Event_t event2);

multimap<string,Event_t>::iterator SearchEventByTitle(string title);

int main(int argc, char *argv[]){
    bool validArg = parseArguments(argc,argv);
    if(!validArg && BufferLength != 0){
        cout << "Please Enter Valid Arguments! Program Terminating\n";
        exit(0);
    }
    // this will be thread 1
    ParseValidEmailsThread();
    for(auto email : ValidEmails){
        cout << email << endl;
    }
    cout << endl;
    // this will be thread 2
    processCalendarThread();
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

bool ParseValidEmailsThread(){
    for (string line; getline(cin, line);) {
        if(strlen(line.c_str()) > 9){
            size_t pos = line.find("Subject: ");
            if(pos != string::npos){
                string email = line.substr(pos+strlen("Subject: "));
                if(CheckEmailString(email)){
                    ValidEmails.push_back(email);
                    /* When multi-threading this application
                       we will also push emails into a buffer
                       up to the buffer limit. Then we will 
                       yield the cpu, process those emails in
                       the other thread until the buffer is empty.
                       We will repeat this until the application
                       is done outputting. */
                }
            }
        }
    }
    return true;
}

bool CheckEmailString(string line){
    istringstream is(line);
    string part;
    parse_state_t state = ACTION;
    while (getline(is, part, ',')){
        switch (state)
        {
        case ACTION:
            state = (CheckAction(part)) ? TITLE : NONE;
        break;
        case TITLE:
            state = (CheckTitle(part)) ? DATE : NONE;
        break;
        case DATE:
            state = (CheckDate(part)) ? TIME : NONE;
        break;
         case TIME:
            state = (CheckTime(part)) ? LOCATION : NONE;
        break;
        case LOCATION:
            state = (CheckLocation(part)) ? DONE : NONE;
        break;       
        default:
            return false;
            break;
        }
    }
    return state == DONE;
}

bool CheckAction(string action){
    string Create("C"), Edit("X"), Delete("D");
    return (action.length()==1 && (action == Create || action == Edit || action == Delete));
}
bool CheckTitle(string title){
    // can titles have numebers????
    if( title.length() != 10 &&
        title.find('0') != std::string::npos ||
        title.find('1') != std::string::npos ||
        title.find('2') != std::string::npos ||
        title.find('3') != std::string::npos ||
        title.find('4') != std::string::npos ||
        title.find('5') != std::string::npos ||
        title.find('6') != std::string::npos ||
        title.find('7') != std::string::npos ||
        title.find('8') != std::string::npos ||
        title.find('9') != std::string::npos)
    {
        return false;
    }
    return true;
}
bool CheckDate(string date){
    if(date.length() != 10) return false;
    istringstream is(date);
    string part;
    while(getline(is, part, '/')){
        bool isNum = !part.empty() && std::find_if(part.begin(),part.end(), 
                                     [](unsigned char c) { return !std::isdigit(c); }) == part.end();
        if(!isNum) return false; // if there's no digit return false
    }
    return true;
}
bool CheckTime(string time){
    if(time.length() != 5) return false;
    istringstream is(time);
    string part;
    while(getline(is, part, ':')){
        bool isNum = !part.empty() && std::find_if(part.begin(),part.end(), 
                                     [](unsigned char c) { return !std::isdigit(c); }) == part.end();
        if(!isNum) return false; // if there's no digit return false
    }
    return true;
}
bool CheckLocation(string location){
    return location.length() == 10;
}

bool processCalendarThread(){
    for(string email : ValidEmails){
        processCalendar(email);    
    }
    return true;
}

bool processCalendar(string email){
    istringstream is(email);
    string part;
    Event_t Event;

    getline(is, part, ',');
    Event.Action = part;
    getline(is, part, ',');
    Event.Title = part;
    getline(is, part, ',');
    Event.Date = part;
    getline(is, part, ',');
    Event.Time = part;
    getline(is, part, ',');
    Event.Location = part;
    
    if( Event.Action == "C"){
        CreateEvent(Event);   
    }else if(Event.Action == "X"){
        EditEvent(Event);
    }else if(Event.Action == "D"){
        DeleteEvent(Event);
    }else{
        // error we shouldn't be here
    }
    return true;
}

bool CreateEvent(Event_t Event){
    if(CalendarFilter.find(Event.Date) == CalendarFilter.end()){
        CalendarFilter.emplace(Event.Date,Event);
        cout << Event.Date << "," << Event.Time << "," << Event.Location << endl;  
    }
    else{
        CalendarFilter.emplace(Event.Date,Event);
        Event_t earliestEvent = searchEarliestEventByDate(Event.Date);
        if(  !earliestEvent.Date.empty() && CompareEvents(earliestEvent,Event) ){
            cout << Event.Date << "," << Event.Time << "," << Event.Location << endl;  
        }
    }
    return true;
}

bool EditEvent(Event_t Event){
    // need to work on edit event
    auto iter = SearchEventByTitle(Event.Title);
    
    if(iter == CalendarFilter.end()) return false;
    iter->second = Event;
    Event_t earliestEvent = searchEarliestEventByDate(Event.Date);
    
    if( !earliestEvent.Date.empty() &&  CompareEvents(iter->second,earliestEvent) ){
        cout << iter->second.Date << "," << iter->second.Time << "," << iter->second.Location << endl; 
    }
    return true;
}

bool DeleteEvent(Event_t Event){
    auto iter = searchEventByDateTitleTime(Event);
    Event_t earliestEvent = searchEarliestEventByDate(Event.Date);
    if(!earliestEvent.Date.empty() && CompareEvents(earliestEvent,iter->second)){
        CalendarFilter.erase(iter);
        earliestEvent = searchEarliestEventByDate(Event.Date);
        if(!earliestEvent.Date.empty()){
            cout << earliestEvent.Date << "," << earliestEvent.Time << "," << earliestEvent.Location << endl;  
        }
        if(earliestEvent.Date.empty()){
            cout << Event.Date <<",--:--,NA\n";
        }
    }
    else if(!earliestEvent.Date.empty()){
        CalendarFilter.erase(iter);
        if(CalendarFilter.find(Event.Date) == CalendarFilter.end()){
            cout << Event.Date <<",--:--,NA\n";
        }
    }else{
        cout << "Calendar Events with the date " << Event.Date << "do not exits\n";
    }
    return true;
}

Event_t searchEarliestEventByDate(string date){
    Event_t earliestEvent = CalendarFilter.find(date)->second;
    for(auto iter = CalendarFilter.begin(); iter != CalendarFilter.end(); ++iter){
        if( iter->second.Date == date ){
            if(!CompareTime(earliestEvent.Time,iter->second.Time)){
                earliestEvent = iter->second;
            }
        }
    }
    return earliestEvent;
}

bool CompareTime(string time1, string time2){
    if(!CheckTime(time1) || !CheckTime(time2)) return false;

    string num1str = time1.substr(0,2);
    long num1 = strtol(num1str.c_str(),nullptr,10);      

    string num2str = time2.substr(0,2);
    long num2 = strtol(num2str.c_str(),nullptr,10);      
    if(num1 < num2) return true;
    else{
        if(num1 > num2) return false;
        // the rest of this else is if hours are equal
        // so now we check for minutes
        num1str = time1.substr(3,2);
        num1 = strtol(num1str.c_str(),nullptr,10);      

        num2str = time2.substr(3,2);
        num2 = strtol(num2str.c_str(),nullptr,10);      
        if(num1 < num2) return true;
    }
    return false;
}

bool CompareEvents(Event_t event1, Event_t event2){
    if(event1.Action.empty() || event2.Action.empty()) return false;

    if( event1.Action    == event2.Action &&
        event1.Title     == event2.Title &&
        event1.Time      == event2.Time &&
        event1.Date      == event2.Date &&
        event1.Location  == event2.Location
       ){
           return true;
       }else{
           return false;
       }
}

multimap<string,Event_t>::iterator SearchEventByTitle(string title){
    for(auto iter = CalendarFilter.begin(); iter != CalendarFilter.end(); ++iter){
        if(iter->second.Title == title) return iter;
    }
    return CalendarFilter.end();
}

multimap<string,Event_t>::iterator searchEventByDateTitleTime(Event_t Event){
    for(auto iter = CalendarFilter.begin(); iter != CalendarFilter.end(); ++iter){
        if(iter->second.Time == Event.Time &&
           iter->second.Title == Event.Title &&
           iter->second.Date == Event.Date){
                return iter;    
           }
    }
    return CalendarFilter.end();
}