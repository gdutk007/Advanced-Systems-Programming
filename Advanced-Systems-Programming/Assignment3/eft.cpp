#include <iostream>
#include <bits/stdc++.h>
#include <cstdlib>
#include <string>
#include <pthread.h>

using namespace std;

typedef struct {
    int AccountId;
    int Money;
} Account_t ;

typedef struct {
    int From;
    int To;
    int amount;
} Transfer_t;

typedef struct{
    int argc;
    char **argv;
}args_t;

vector<Account_t> Accounts;
vector<Transfer_t> Transfers;

static int MaxThreadNumber = 0;

static void * ParseInput(void * arg);

bool parseAccount(string accountstr);

bool parseTransfer(string Transfer);

int main(int argc, char *argv[]){

    if(argc != 2){
        cout << "Please enter at least and at most 1 argument\n";
        exit(0);
    }
    // this will be main thread
    pthread_t mainT;
    int mainTret;
    mainTret = pthread_create(&mainT,nullptr,ParseInput,(void*)argv);

    pthread_exit(nullptr);
    return 0;
}

static void * ParseInput(void * arg){
    char **argv = (char **) arg;
    string bufferLen(argv[1]);
    const long len = strtol(bufferLen.c_str(),nullptr,10); 
    if( len <= 0){
        cout << "please enter a valid number.\n";
        pthread_exit(nullptr);
    }
    MaxThreadNumber = len;
    for (string line; getline(cin, line);){
            size_t pos = line.find("Transfer ");
            if( pos == string::npos ){
                /* If not a transfer, parse Account creation */
                if(!parseAccount(line)){
                    /* handle error */
                }
            }else{
                string transfer = line.substr(pos+strlen("Transfer "));
                /* if a transfer, parse transfer */
                if(!parseTransfer(transfer)){
                    /* handle error */
                }
            }
    }
    pthread_exit(nullptr);
}

bool parseAccount(string accountstr){
    return false;   
}

bool parseTransfer(string Transfer){
    return false;
}