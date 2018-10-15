#include <stdio.h>
#include <stdlib.h>

typedef struct vm{
    int id;
    char autoShutdown;
    int shutdownTime; // hour to auto shutdown (from 0 to 23)
    char autoTurnOn;
    int turnOnTime; // if this machinie is set to turn on automatically, the sacleset will turn on this machine, if it is off
    float cpuUsage; // cpu use measured in percent
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
    float meanCpuUsage; // mean os cpuUSage of current vms
    int ruleId;
}SCALESET;

VM* createVM(){
    VM* vm = malloc(sizeof(VM));
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

// SCALESET* createScaleset(){
//     SCALESET* ss = malloc(sizeof(SCALESET));
//     ss->vms = createVM();
//     ss->id = 0;
//     ss->meanCpuUsage = 0;
//     ss->nextId = 0;
//     ss->numberOfActiveVms = 0;
//     ss->numberOfInactiveVms = 0;
//     ss->ruleId = meanCpuUsage;
//     return ss;
// }

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


int main(int argc, char* argv[]){
    
    SCALESET* ss = createScaleset(1);

    int (*this)(int argc, char* argv[]) = main;
    printf("asd\n");
    if(argv[0][0])
    this(0,NULL);

    return 0;
}
