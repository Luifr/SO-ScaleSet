#include <stdio.h>
#include <time.h>
#include "inputGenerator.c"

#ifdef _WIN32 // if its windows
    #include <windows.h>
    #define  sleep(x) Sleep(1000*x)
#else // if its linux
    #include<unistd.h> 
#endif

typedef struct vm{
    int id;
    char isActive;
    char autoShutdown;
    int shutdownTime; // hour to auto shutdown (from 0 to 23)
    char autoTurnOn;
    int turnOnTime; // if this machinie is set to turn on automatically,
                    // the sacleset will turn on this machine, if it is off
    float costPerHour;
    float cpuUsage; // cpu use measured in a arbitrary unit
    float processPower; // process power measured in a arbitrary unit
} VM;

enum policy{
    meanCpuUsage, // vms will be set or shutdown based on mean cpu usage
                  // (vms will be based on a image, that is, will all be equal)
    fixed,  // set a specific number of pre configured vms (each vm will be 
            //configured manually and can be different from each other)
};

typedef struct scaleset{
    int id;
    int nextId; // id of the next instancied vm
    VM** vms;
    int numberOfActiveVms;
    int numberOfInactiveVms;
    int totalVms;
    float totalProcessPower;
    float activeProcessPower;
    float meanCpuUsage; /// mean os cpuUSage of current vms
    int ruleId;
    float lowerLimit;
    float upperLimit;
    int lowerLimitVMs; //how many VMs will be shuted down
    int upperLimitVMs; //how many VMs will be added

}SCALESET;

void addVm(SCALESET* ss);
void removeVm(SCALESET* ss);

VM* createVM(int nextId){
    VM* vm = malloc(sizeof(VM));
    vm->isActive = 1;
    vm->id = nextId++;
    vm->autoShutdown = 0;
    vm->autoTurnOn = 0;
    vm->cpuUsage = 0;
    vm->processPower = 100;
    return vm;
}

VM** createVMS(int number, int nextId, float costPerHour, int processPower){
    VM** vm = malloc(sizeof(VM*)*number);
    for(int i=0;i<number;i++){
        vm[i] = malloc(sizeof(VM));
        vm[i]->id = nextId++;
        vm[i]->isActive = 1;
        vm[i]->autoShutdown = 0;
        vm[i]->autoTurnOn = 0;
        vm[i]->costPerHour = costPerHour;
        vm[i]->cpuUsage = 0;
        vm[i]->processPower = processPower;
    }
    return vm;
}

SCALESET* createScaleSet(int ruleId, int numberVMs, int processPower, float costPerHour, 
    float lowerLimit, float upperLimit, int lowerLimitVMs, int upperLimitVMs){
    SCALESET* ss = malloc(sizeof(SCALESET));
    ss->vms = createVMS(numberVMs, 0, costPerHour, processPower);
    ss->id = 0;
    ss->meanCpuUsage = 0;
    ss->nextId = 0;
    ss->numberOfActiveVms = numberVMs;
    ss->numberOfInactiveVms = 0;
    ss->totalVms = numberVMs;
    ss->ruleId = ruleId;
    ss->lowerLimit = lowerLimit;
    ss->upperLimit = upperLimit;
    ss->lowerLimitVMs = lowerLimitVMs;
    ss->upperLimitVMs = upperLimitVMs;
    return ss;
}

void calculateMeanCpuUsage(SCALESET* ss){
    int len = ss->totalVms;
    int sumUsage=0,sumTotal=0;
    for(int i=0;i<len;i++){
        sumUsage += ss->vms[i]->cpuUsage;
        sumTotal += ss->vms[i]->processPower;
    }
    ss->meanCpuUsage = sumUsage / (float)sumTotal;
}

void execFixed(SCALESET* ss){ //TODO

}

void execMeanCpuUsage(SCALESET* ss){
    calculateMeanCpuUsage(ss);
    if(ss->meanCpuUsage > ss->upperLimit){
        for(int i=0;i<ss->upperLimitVMs;i++)
            addVm(ss);
    }
    if(ss->meanCpuUsage < ss->lowerLimit && ss->totalVms > 1){
        for(int i=0;i<ss->lowerLimitVMs && ss->totalVms>=0;i++)
            removeVm(ss);
    }
}

void addVm(SCALESET* ss){
    ss->vms = realloc(ss->vms,(ss->totalVms+1) * sizeof(VM*));
    ss->vms[ss->totalVms++] = createVM(ss->nextId);
    ss->numberOfActiveVms++;
}

void removeVm(SCALESET* ss){
    if(ss->vms[ss->totalVms-1]->isActive){
        ss->numberOfActiveVms--;
    }
    else{
        ss->numberOfInactiveVms--;
    }
    ss->totalVms--;
    ss->vms = realloc(ss->vms,ss->totalVms * sizeof(VM*));
}
    

void distributeProcessing(SCALESET* ss, int processing) {
    if (ss == NULL)
        return;
    int procinicial = processing;
    int mid = processing / ss->numberOfActiveVms;
    for (int i = 0; i < ss->numberOfActiveVms; ++i) {
        if(ss->vms[i]->isActive)
            processing-=mid;
            ss->vms[i]->cpuUsage = mid;
    }
}

int calculateMoneySpent(SCALESET* ss){
    int totalCost=0;
    for(int i=0;i<ss->totalVms;i++){
        if(ss->vms[i]->isActive){
            totalCost += ss->vms[i]->costPerHour;
        }
    }
    return totalCost;
}

SCALESET* createScaleSetFromInput(){
    int ruleId;
    int numberVMs; //inicial number of virtual machines
    int processPower;
    float costPerHour;
    float lowerLimit, upperLimit;
    int lowerLimitVMs, upperLimitVMs;
    scanf("%d %d %d %f %f %f %d %d", &ruleId, &numberVMs, &processPower, 
        &costPerHour, &lowerLimit, &upperLimit, &lowerLimitVMs, &upperLimitVMs);
    SCALESET *ss = createScaleSet(ruleId, numberVMs, processPower,
        costPerHour, lowerLimit, upperLimit, lowerLimitVMs, upperLimitVMs);
    return ss;
}


int main(int argc, char* argv[]){

    SCALESET* ss = createScaleSetFromInput(); 

    clock_t programBegin, programEnd;
    clock_t loopBegin, loopEnd;
    clock_t readBegin, readEnd;
    clock_t timer = 0;
    
    programBegin = clock();

    FILE* fp = fopen(FILE_NAME, "r");
 
    int processRequired;
    int moneySpent = 0;
    int localMoneySpent;
    int hour = 0;

    while(fscanf(fp, "%d", &processRequired) != EOF){ 
        loopBegin = clock();
        timer += loopBegin;

        distributeProcessing(ss, processRequired);

        localMoneySpent = calculateMoneySpent(ss);
        moneySpent += localMoneySpent;

        printf("Money Spent: %d\n", localMoneySpent);

        if(ss->ruleId == meanCpuUsage){
            execMeanCpuUsage(ss);
        }
        else if(ss->ruleId == fixed){
            execFixed(ss);
        }
        
        printf("Active VMs: %d\n",ss->totalVms);

        loopEnd = clock();
        hour = (hour+1)%24;
        
        printf("current time: %d\n", hour);
        printf("------------------------------------------------------------------------\n");
    }

    printf("Money spent: %d\n", moneySpent);

    return 0;
}
