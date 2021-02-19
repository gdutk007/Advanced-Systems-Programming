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

pthread_mutex_t mLock;

vector<Account_t> Accounts;
vector<Transfer_t> Transfers;

static int MaxThreadNumber = 0;

static void * ParseInput(void * arg);

static void * SendMoney(void * arg);

bool parseAccount(string accountstr);

bool parseTransfer(string Transfer);

void printAccountsAndTransfers(){
    cout << "size: " << Transfers.size() << "\n"; 
    cout << "Accounts: \n";
    for(auto account : Accounts){
        cout << "Id: " << account.AccountId << " Balance: " << account.Money
         << "\n";
    }
    cout << "\nTransfers: \n";
    for(auto eft : Transfers){
        cout << "From: " << eft.From << " To: " << eft.To << " Amount: " << eft.amount
         << "\n";
    }
}

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
    printAccountsAndTransfers();
    vector<pthread_t> threads;
    threads.resize(MaxThreadNumber);
    for(int i = 0; i < threads.size(); ++i){
        int * arg = new int();
        *arg = i;
        int ret = pthread_create(&threads[i],nullptr, SendMoney,
                                                     (void*)arg);
        if(ret){
            cout << "Error creating threads, exiting. \n";
            exit(0);
        }
    }
    pthread_exit(nullptr);
}

bool parseAccount(string accountstr){
    if(accountstr.empty() || accountstr.find("Transfer ") != string::npos)
     return false;
    istringstream is(accountstr);
    string part;
    Account_t Account;
    getline(is, part, ' ');
    Account.AccountId = strtol(part.c_str(),nullptr,10);
    getline(is, part, ' ');
    Account.Money = strtol(part.c_str(),nullptr,10);
    Accounts.push_back(Account);
    return true;   
}

bool parseTransfer(string Transfer){
    if(Transfer.empty())
     return false;
    istringstream is(Transfer);
    string part;
    Transfer_t eft;
    getline(is, part, ' ');
    eft.From = strtol(part.c_str(),nullptr,10);
    getline(is, part, ' ');
    eft.To = strtol(part.c_str(),nullptr,10);
    getline(is, part, ' ');
    eft.amount = strtol(part.c_str(),nullptr,10);
    Transfers.push_back(eft);
    return true;   
}

static void * SendMoney(void * arg){
    int tid = *((int*)arg);
    while(Transfers.size()){
        pthread_mutex_lock(&mLock);
        cout << "tid: " << tid << " popping" << "\n";
        if(Transfers.size())
            Transfers.pop_back();
        cout << "tid: " << tid << " size: " << Transfers.size() << "\n";
        pthread_mutex_unlock(&mLock);
    }
    pthread_exit(nullptr);
}