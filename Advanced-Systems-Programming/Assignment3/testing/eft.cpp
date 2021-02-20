#include <iostream>
#include <bits/stdc++.h>
#include <cstdlib>
#include <string>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

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

sem_t SemTransfers;
vector<sem_t> mSem;
vector<Account_t> Accounts;
vector<Transfer_t> Transfers;

static int MaxThreadNumber = 0;

static void * ParseInput(void * arg);

static void * SendMoney(void * arg);

bool parseAccount(string accountstr);

bool parseTransfer(string Transfer);

void printAccountsAndTransfers();

int GetAccountIndex(int account);

bool InsertAccount(int id, int money=0);

void PrintAccounts();

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
    //printAccountsAndTransfers();
    vector<pthread_t> threads;
    threads.resize(MaxThreadNumber);
    mSem.resize(Accounts.size());
    // init buffer semaphores
    int counts = (MaxThreadNumber == 1) ? 1 : MaxThreadNumber-1;  
    sem_init(&SemTransfers,0,counts);
    // init account semaphores
    for(int i = 0; i < mSem.size(); ++i)
        sem_init(&mSem[i],0,1);
    
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
    for(int i = 0; i < threads.size(); ++i)
        pthread_join(threads[i],nullptr);
    
    PrintAccounts();
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
    if(!InsertAccount(Account.AccountId,Account.Money)){
        cout << "Account already exists, not insterting \n";
    }
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
    cout << "\n";
}

static void * SendMoney(void * arg){
    int tid = *((int*)arg);
    while(Transfers.size()){
        sem_wait(&SemTransfers);
        //cout << "tid: " << tid << " came into room \n";
        if(Transfers.size()){
            Transfer_t transfer = Transfers.back();
            Transfers.pop_back();
            int index1 = GetAccountIndex(transfer.To);
            int index2 = GetAccountIndex(transfer.From);
            if(index1 < 0 || index2 < 0){
                cout << "Account out of bounds, exiting\n";
                exit(0);
            }
            sem_wait(&mSem[index1]);
            sem_wait(&mSem[index2]);
            // cout << "tid: " << tid << " transfering from " << transfer.From 
            //                         << " to " << transfer.To  << "\n";
            Accounts[index1].Money += transfer.amount; 
            Accounts[index2].Money -= transfer.amount;
            sem_post(&mSem[index2]);
            sem_post(&mSem[index1]);
        }
        sem_post(&SemTransfers);
        usleep(100000);
        //cout << "tid: " << tid << "leaving \n";
    }
    pthread_exit(nullptr);
}

int GetAccountIndex(int account){
    for(int i = 0; i < Accounts.size(); ++i){
        if(Accounts[i].AccountId == account) return i;
    }
    return -1;
}

bool InsertAccount(int id, int money){
    for(auto acc : Accounts){
        if(id == acc.AccountId) return false;
    }
    Account_t newAccout;
    newAccout.AccountId = id;
    newAccout.Money = money;
    Accounts.push_back(newAccout);
    return true;
}

void PrintAccounts(){
    for(auto acc : Accounts)
        cout <<acc.AccountId<<" "<<acc.Money << "\n";

}