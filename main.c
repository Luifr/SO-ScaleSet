#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32 // if its windows
    #include <windows.h>
    #define  sleep(x) Sleep(1000*x)
#else // if its linux
    #include <unistd.h> 
#endif

#define FILE_NAME "inputRand.in"
#define OUT_SCALE_SET "scaleSet.out"
#define OUT_CONSTANT "constant.out"
#define LOG ".log"

FILE* logFile;  // File to store all the steps during the simulation =)

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
    ss->activeProcessPower = 0;
    ss->totalProcessPower = 0;
    ss->totalVms = numberVMs;
    ss->ruleId = ruleId;
    ss->lowerLimit = lowerLimit;
    ss->upperLimit = upperLimit;
    ss->lowerLimitVMs = lowerLimitVMs;
    ss->upperLimitVMs = upperLimitVMs;
    return ss;
}

SCALESET* createFixedScaleSet(int ruleId, int numberVMs, int processPower, float costPerHour){
    SCALESET* ss = malloc(sizeof(SCALESET));
    ss->vms = createVMS(numberVMs, 0, costPerHour, processPower);
    ss->id = 0;
    ss->meanCpuUsage = 0;
    ss->activeProcessPower = 0;
    ss->totalProcessPower = 0;
    ss->nextId = 0;
    ss->numberOfActiveVms = numberVMs;
    ss->numberOfInactiveVms = 0;
    ss->totalVms = numberVMs;
    ss->ruleId = ruleId;
    return ss;
}

int calculateMeanCpuUsage(SCALESET* ss){
    int len = ss->totalVms;
    int sumUsage = 0, sumTotal = 0, sumActive = 0;
    for(int i=0;i<len;i++){
        sumUsage += ss->vms[i]->cpuUsage;
        sumTotal += ss->vms[i]->processPower;
        if (ss->vms[i]->isActive)
            sumActive += ss->vms[i]->processPower;
    }
    ss->totalProcessPower = sumTotal;
    ss->activeProcessPower = sumActive;
    ss->meanCpuUsage = sumUsage / (float)sumTotal;

    return sumUsage;
}

void execFixed(SCALESET* ss){ //TODO
    calculateMeanCpuUsage(ss);
}

void execMeanCpuUsage(SCALESET* ss){
    int CpuUsage = calculateMeanCpuUsage(ss);
    fprintf(logFile, "Mean CPU usage: %.2f\n", ss->meanCpuUsage);

    if(ss->meanCpuUsage > ss->upperLimit){
        int numberVM = ss->numberOfActiveVms;
	int i = 0;
	while (CpuUsage / (float)ss->activeProcessPower > ss->upperLimit && i++ < ss->upperLimitVMs)
            addVm(ss);

        fprintf(logFile, "Mean CPU usage is above upper limit.\nVMs added: %d\n", ss->numberOfActiveVms - numberVM);
    }

    else if(ss->meanCpuUsage < ss->lowerLimit && ss->totalVms > 1){
       int numberVM = ss->numberOfActiveVms;
	int i = 0;
	while (CpuUsage / (float)ss->activeProcessPower < ss->lowerLimit && (i++ < ss->lowerLimitVMs) && (ss->totalVms > 1))
            removeVm(ss);

        fprintf(logFile, "Mean CPU usage is bellow lower limit.\nVMs removed: %d\n", numberVM - ss->numberOfActiveVms);
    }
}

void addVm(SCALESET* ss){
    ss->vms = realloc(ss->vms,(ss->totalVms+1) * sizeof(VM*));
    ss->vms[ss->totalVms++] = createVM(ss->nextId);
    ss->numberOfActiveVms++;
    ss->totalProcessPower += ss->vms[ss->totalVms-1]->processPower;
    ss->activeProcessPower += ss->vms[ss->totalVms-1]->processPower;
}

void removeVm(SCALESET* ss){
    if(ss->vms[ss->totalVms-1]->isActive){
        ss->numberOfActiveVms--;
    }
    else{
        ss->numberOfInactiveVms--;
    }
    ss->totalProcessPower -= ss->vms[ss->totalVms-1]->processPower;
    ss->activeProcessPower -= ss->vms[ss->totalVms-1]->processPower;
    ss->totalVms--;
    free(ss->vms[ss->totalVms]);
    ss->vms = realloc(ss->vms,ss->totalVms * sizeof(VM*));
}
    

// Returns the remaining requests needed to be processed (if the processing required
// is higher than the available).
int distributeProcessing(SCALESET* ss, int processing) {
    if (ss == NULL)
        return 0;

    // Calculates the percentage of process power required over the power available.
    // Than we can set the processing of each active vm to this percent, so we get all
    // the power needed, on the most balanced way.
    float usagePercent = processing / (float) ss->activeProcessPower;
    if (usagePercent > 1.0) // The percent cannot be higher than 1.
        usagePercent = 1;

    // Set the process power to each machine =)
    for (int i = 0; i < ss->numberOfActiveVms; ++i)
        if(ss->vms[i]->isActive)
            ss->vms[i]->cpuUsage = ss->vms[i]->processPower * usagePercent;

    // Get the remaining process power (if exists) to be processed in the next cycle.
    int remaining = processing - ss->activeProcessPower;
    return (remaining < 0) ? 0 : remaining;
}

int calculateMoneySpent(SCALESET* ss){
    int totalCost = 0;
    int costPerHour = ss->vms[0]->costPerHour;
    totalCost = ss->numberOfActiveVms*costPerHour;
    return totalCost;
}

SCALESET* createScaleSetFromInput(){
    SCALESET *ss;
    int ruleId;
    int numberVMs; //inicial number of virtual machines
    int processPower;
    float costPerHour;
    float lowerLimit, upperLimit;
    int lowerLimitVMs, upperLimitVMs;
    printf("Input the following information:\n");
    printf("Rule ");
    scanf("%d", &ruleId);
    printf("\nInicial number of VMs ");
    scanf("%d", &numberVMs);
    printf("\nProcessing power of each VM ");
    scanf("%d", &processPower);
    printf("\nCost of one VM per hour ");
    scanf("%f", &costPerHour);
    if(ruleId==0){
        printf("\nLower limit (0;1] ");
        scanf("%f",&lowerLimit);
        printf("\nUpper limit (0;1] ");
        scanf("%f",&upperLimit);
        printf("\nNumber of VMs that will be shutted down, if CPU usage is bellow the lower limit ");
        scanf("%d",&lowerLimitVMs);
        printf("\nNumber of VMs that will be turned on, if CPU usage is above de upper limit ");
        scanf("%d",&upperLimitVMs);
        ss = createScaleSet(ruleId, numberVMs, processPower,
            costPerHour, lowerLimit, upperLimit, lowerLimitVMs, upperLimitVMs);
    }
    else{
        ss = createFixedScaleSet(ruleId, numberVMs, processPower,
            costPerHour);
    }
    return ss;
}


int main(int argc, char* argv[]){

    SCALESET* ss = createScaleSetFromInput(); 
    FILE* outfp;

    logFile = fopen(LOG, "w+");

    if (ss->ruleId == 0)
        outfp = fopen(OUT_SCALE_SET, "w+");
    else
        outfp = fopen(OUT_CONSTANT, "w+");

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
    int day = 0;
    int remaining = 0;

    while(fscanf(fp, "%d", &processRequired) != EOF || remaining > 0){ 
        fprintf(logFile, "Input = %d, remaining = %d\n", processRequired, remaining);
        loopBegin = clock();
        timer += loopBegin;

        calculateMeanCpuUsage(ss);

        processRequired += remaining;
        remaining = distributeProcessing(ss, processRequired);

        localMoneySpent = calculateMoneySpent(ss);
        moneySpent += localMoneySpent;
        fprintf(outfp, "%d\n", moneySpent);

        fprintf(logFile, "Money Spent: %d\n", localMoneySpent);
        fprintf(logFile, "Active VMs: %d\n",ss->totalVms);

        if(ss->ruleId == meanCpuUsage){
            execMeanCpuUsage(ss);
        }
        else if(ss->ruleId == fixed){
            execFixed(ss);
        }
        

        loopEnd = clock();
        processRequired = 0;
        day += (hour+1) / 24;
        hour = (hour+1)%24;
        
        fprintf(logFile, "current time: day %d, %02d:00\n", day, hour);
    }

    printf("\nSIMULATION FINISHED SUCCESSFULLY\nTotal money spent: %d\n", moneySpent);

    fclose(outfp);
    fclose(fp);
    fclose(logFile);

    for(int i=0; i<ss->totalVms; i++)
        if(ss->vms[i])
            free(ss->vms[i]);

    free(ss->vms);
    free(ss);

    return 0;
}
