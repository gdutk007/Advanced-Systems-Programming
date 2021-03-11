#include <iostream>
#include <bits/stdc++.h>
#include <cstdlib>
#include <string>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

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


constexpr unsigned int defaultLength{256};

typedef struct {
    char ValidEmail[defaultLength];
} Shared_Valid_Email_t;


static int BufferLength;

bool * Shared_Done;

pthread_mutex_t mLock;

pthread_mutex_t * m_Lock;

pthread_cond_t cv;

pthread_cond_t * m_cv;

multimap<string, Event_t> CalendarFilter;

vector<string> ValidEmails;

vector<Event_t> Event_Buffer;

Shared_Valid_Email_t * Shared_Valid_Emails;

int * Shared_Email_count;

int * Shared_Buffer_Read;

int * Shared_Buffer_Write;

bool parseArguments(int argc, char *argv[]);

void ParseValidEmailsThread();

void processCalendarThread();

void DebuggingEmailThread();

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

bool WriteToBuffer();

Event_t searchEarliestEventByDate(string date);

multimap<string, Event_t>::iterator safeSearchEarliestEventByDate(string date);

multimap<string, Event_t>::iterator searchEventByDateTitleTime(Event_t Event);

bool CompareTime(string time1, string time2);

bool CompareEvents(Event_t event1, Event_t event2);

multimap<string,Event_t>::iterator SearchEventByTitle(string title);

int main(int argc, char *argv[]){
    bool validArg = parseArguments(argc,argv);
    if(!validArg ){
        cout << "Please Enter Valid Arguments! Program Terminating\n";
        exit(0);
    }

    pthread_mutexattr_t psharedm;
    pthread_condattr_t psharedc;

    pthread_mutexattr_init(&psharedm);
    pthread_mutexattr_setpshared(&psharedm,PTHREAD_PROCESS_SHARED);
    
    pthread_condattr_init(&psharedc);
    pthread_condattr_setpshared(&psharedc,PTHREAD_PROCESS_SHARED);

    // create shared mutex
    m_Lock = (pthread_mutex_t * ) mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // create shared condition variable
    m_cv = (pthread_cond_t * ) mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // init mutex and cond variable
    pthread_mutex_init(m_Lock, &psharedm);
    pthread_cond_init(m_cv, &psharedc);

    // Shared Buffer Length
    Shared_Valid_Emails = (Shared_Valid_Email_t * )  mmap(NULL, BufferLength * sizeof(Shared_Valid_Email_t), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // Shared buffer count
    Shared_Email_count = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    // shared Done
    Shared_Done = (bool *) mmap(NULL, sizeof(bool), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // Shared buffer Read
    Shared_Buffer_Read = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // Shared buffer Write
    Shared_Buffer_Write = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *Shared_Buffer_Read = 0;
    *Shared_Buffer_Write = 0;
    *Shared_Done = false;
    *Shared_Email_count = 0; // make sure we start at count zero

    int pid;
    int status;
    if(  !(pid = fork())  ){
        // calling first process
        ParseValidEmailsThread();
        cout << "We should never get here... \n";
    }else{
        // here we fork into the second process
        if(!(pid = fork())){
            // calling second process
            processCalendarThread();
            cout << "We should never get here... \n";
            exit(0);
        }else{
            // try to wait for any children while there exists at least one
            while ((pid=waitpid(-1,&status,0))!=-1) {
                printf("Process %d terminated\n",pid);
            }
        }
    }
    cout << "For some reason we didn't wait until all the processes returned....\n";
    for(auto event : CalendarFilter){
        cout << event.second.Date << " " << event.second.Time << " " << event.second.Location << "\n";
    }
    return 0;
}


bool parseArguments(int argc, char *argv[]){
    if(argc != 2){
        cout << "You've entered an incorrect number of arguments\n";
        return false;
    }
    string bufferLen(argv[1]);
    const long len = strtol(bufferLen.c_str(),nullptr,10); 
    if( len <= 0){
        cout << "please enter a valid number.\n";
        return false;
    }
    BufferLength = len;
    return true;
}

void ParseValidEmailsThread(){
    for (string line; getline(cin, line);){
        pthread_mutex_lock(m_Lock);
        if(strlen(line.c_str()) > 9){
            size_t pos = line.find("Subject: ");
            if(pos != string::npos){
                string email = line.substr(pos+strlen("Subject: "));
                if(CheckEmailString(email)){
                    //cout << "From the ParseProcess the valid email is : " << email << "\n";
                    //sprintf(Shared_Valid_Emails[*Shared_Buffer_Write].ValidEmail, "%s", email.c_str());
                    for(int i = 0 ; i < email.size(); ++i){
                        Shared_Valid_Emails[*Shared_Buffer_Write].ValidEmail[i] = email[i];
                    }
                    *Shared_Buffer_Write = (*Shared_Buffer_Write+1)%BufferLength;
                    *Shared_Email_count += 1;
                    // cout << "ParseProcess Read Pointer: " << *Shared_Buffer_Read << " Shared Buffer Write: " << *Shared_Buffer_Write << 
                    //   " Email: " << email <<"\n";
                    //cout << "Count: " << *Shared_Email_count << " Write Count:  " << *Shared_Buffer_Write << "\n";
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
        
        pthread_mutex_unlock(m_Lock);
        /* After unlocking, signal cond if we have reached the required length */
        //cout << "write Buff values: " << *Shared_Buffer_Write << "\n";
        while( *Shared_Email_count >= BufferLength)
            pthread_cond_signal(m_cv);

    }
    *Shared_Done = true;
    /* signal again in case we are 
       still blocking in the other
       thread and we have stuff
       left to process */
    pthread_cond_signal(m_cv);
    exit(0);
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
    if( title.length() != 10 
        // &&
        // title.find('0') != std::string::npos ||
        // title.find('1') != std::string::npos ||
        // title.find('2') != std::string::npos ||
        // title.find('3') != std::string::npos ||
        // title.find('4') != std::string::npos ||
        // title.find('5') != std::string::npos ||
        // title.find('6') != std::string::npos ||
        // title.find('7') != std::string::npos ||
        // title.find('8') != std::string::npos ||
        // title.find('9') != std::string::npos
        )
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

bool processCalendar(string email){
    //cout << "Processing Email: " << email << " \n";
    if(email.empty()) return false;
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
    //cout << "Delete Event: " << Event.Date << "," << Event.Time << "," << Event.Location << endl;
    auto iter = searchEventByDateTitleTime(Event);
    Event_t earliestEvent = searchEarliestEventByDate(Event.Date);
    if(!earliestEvent.Date.empty() && CompareEvents(earliestEvent,iter->second)){
        CalendarFilter.erase(iter);
        auto newEarliestEvent = safeSearchEarliestEventByDate(earliestEvent.Date);
        if(newEarliestEvent != CalendarFilter.end()){
            cout << newEarliestEvent->second.Date << "," << newEarliestEvent->second.Time << "," << newEarliestEvent->second.Location << endl;  
        }
        if(newEarliestEvent == CalendarFilter.end()){
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


multimap<string, Event_t>::iterator safeSearchEarliestEventByDate(string date){
    if(CalendarFilter.find(date) == CalendarFilter.end()) return CalendarFilter.end();

    multimap<string, Event_t>::iterator earliestEvent = CalendarFilter.find(date);

    for(auto iter = CalendarFilter.begin(); iter != CalendarFilter.end(); ++iter){
        if( iter->second.Date == date ){
            if(!CompareTime(earliestEvent->second.Time,iter->second.Time)){
                earliestEvent = iter;
            }
        }
    }
    return earliestEvent;
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


void processCalendarThread(){
    while(true && !*Shared_Done){
        /* Lock mutex */
        pthread_mutex_lock(m_Lock);
        while( *Shared_Email_count <= 0 && !*Shared_Done ){
            pthread_cond_wait(m_cv,m_Lock);
        }
        /* Process the emails and then erase them.
           Ideally this should have been a dequeue
           but I got lazy */
        while( *Shared_Email_count > 0){
            *Shared_Email_count -= 1;
            //printf("Trying string : %s \n",&Shared_Valid_Emails[*Shared_Buffer_Read].ValidEmail[0]);
            string email(Shared_Valid_Emails[*Shared_Buffer_Read].ValidEmail);
            //cout << "Email: " << email << "\n";
            //cout << "ProcessCalProcess Read Pointer: " << *Shared_Buffer_Read << " Shared Buffer Write: " << *Shared_Buffer_Write << "\n";
            for(int i = 0; i < defaultLength; ++i){
                Shared_Valid_Emails[*Shared_Buffer_Read].ValidEmail[i] = 0x00;
            }
            
            if(processCalendar(email)){
                //cout << "Returned good from: " << email << "\n";
            }
            else{cout << "For some reason the shared mem is empty\n";}
            *Shared_Buffer_Read = (*Shared_Buffer_Read+1)%BufferLength;
        }
        pthread_mutex_unlock(m_Lock);
    }
    exit(0);
}

void DebuggingEmailThread(){
    while(true && !*Shared_Done){
        pthread_mutex_lock(m_Lock);
        while(!ValidEmails.size() && !*Shared_Done ){
            cout << "Cond_wait Blocking \n";
            pthread_cond_wait(m_cv,m_Lock);
        }

        while(ValidEmails.size()){
            cout << "The Valid Email is: " << ValidEmails.front() << endl;
            ValidEmails.erase(ValidEmails.begin());
        }
        pthread_mutex_unlock(m_Lock);
    }
    exit(0);
}