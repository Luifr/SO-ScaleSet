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
    int turnOnTime; // if this machinie is set to turn on automatically, the sacleset will turn on this machine, if it is off
    int cpuUsage; // cpu use measured in a arbitrary unit
    int processPower; // process power measured in a arbitrary unit
} VM;

enum policy{
    machinesPerHour, // set a specific number of pre configured vms to each hour in a day (each vm will be configured manually and can be different from each other)
    meanCpuUsage, // vms will be set or shutdown based on mean cpu usage (vms will be based on a image, that is, will all be equal)
    none // each vm is set and configured manually 
};

typedef struct scaleset{
    int id;
    int nextId; // id of the next instancied vm
    VM** vms;
    int numberOfActiveVms;
    int numberOfInactiveVms;
    int totalVms;
    float meanCpuUsage; /// mean os cpuUSage of current vms
    int ruleId;
    float upperLimit;
    float lowerLimit;
}SCALESET;

VM* createVM(){
    VM* vm = malloc(sizeof(VM));
    vm->isActive = 1;
    vm->autoShutdown = 0;
    vm->autoTurnOn = 0;
    vm->cpuUsage = 0;
    vm->processPower = 100;
    return vm;
}

VM** createVMS(int number){
    VM** vm = malloc(sizeof(VM*)*number);
    for(int i=0;i<number;i++){
        vm[i] = malloc(sizeof(VM));
        vm[i]->autoShutdown = 0;
        vm[i]->autoTurnOn = 0;
        vm[i]->cpuUsage = 0;
        vm[i]->processPower = 100;
    }
    return vm;
}

SCALESET* createScaleset(int number){
    SCALESET* ss = malloc(sizeof(SCALESET));
    ss->vms = createVMS(number);
    ss->id = 0;
    ss->meanCpuUsage = 0;
    ss->nextId = 0;
    ss->numberOfActiveVms = 0;
    ss->numberOfInactiveVms = 0;
    ss->ruleId = meanCpuUsage;

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

void execMeanCpuUsage(SCALESET* ss){
    calculateMeanCpuUsage(ss);
    if(ss->meanCpuUsage > ss->upperLimit){
        addVm(ss);
    }
    if(ss->meanCpuUsage < ss->lowerLimit && ss->totalVms > 1){
        removeVm(ss);
    }
}

void addVm(SCALESET* ss){
    ss->vms = realloc(ss->vms,ss->totalVms+1 * sizeof(VM*));
    ss->vms[ss->totalVms++] = createVM();
    ss->numberOfActiveVms++;
}

void removeVm(SCALESET* ss){
    if(ss->vms[ss->totalVms-1]->isActive){
        ss->numberOfActiveVms--;
    }
    else{
        ss->numberOfInactiveVms;
    }
    ss->vms = realloc(ss->vms,--ss->totalVms * sizeof(VM*));
}

void distributeProcessing(SCALESET* ss, int processing) {
    if (ss == NULL)
        return;

    int mid = processing / ss->numberOfActiveVms;

    for (int i = 0; i < ss->numberOfActiveVms; ++i) {
        ss->vms[i]->cpuUsage = mid;
    }
}

int main(int argc, char* argv[]){
    
    SCALESET* ss = createScaleset(1);
    clock_t programBegin,programEnd;
    clock_t loopBegin,loopEnd;
    clock_t readBegin, readEnd;
    clock_t timer=0;
    
    programBegin = clock();

    GenerateInput();
    FILE* fp = fopen(FILE_NAME, "r");

    int processRequired;

    while(1){
        fscanf(fp, "%d", &processRequired);

        loopBegin = clock();
        timer += clock();
        // if sclaeset ruleid == meanCpuUsage
        // execMeanCpuUsage()
        sleep(3);
        loopEnd = clock();
        printf("%ld %ld\n",loopBegin,loopEnd);
        printf("elapsed time: %lf\n",(double)(loopEnd-loopBegin)/(double)CLOCKS_PER_SEC);
        printf("total time: %lf\n",(double)(loopEnd-programBegin)/(double)CLOCKS_PER_SEC);
        printf("total time2: %lf\n",(double)(timer-programBegin)/(double)CLOCKS_PER_SEC);
    }



    return 0;
}
