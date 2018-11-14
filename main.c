#include <stdio.h>
#include<stdlib.h>
#include <time.h>

#ifdef _WIN32 // if its windows
    #include <windows.h>
    #define  sleep(x) Sleep(1000*x)
#else // if its linux
    #include<unistd.h> 
#endif

#define FILE_NAME "input2.in"
#define OUT_FILE_NAME "output.out"
FILE* outfp;

//Colors
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

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

void calculateMeanCpuUsage(SCALESET* ss){
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
}

void execFixed(SCALESET* ss){ //TODO
    calculateMeanCpuUsage(ss);
}

void execMeanCpuUsage(SCALESET* ss){
    calculateMeanCpuUsage(ss);
    printf("Mean CPU usage: %.2f\n", ss->meanCpuUsage);
    if(ss->meanCpuUsage > ss->upperLimit){
        int numberVM = ss->numberOfActiveVms;
        for(int i=0;i<ss->upperLimitVMs;i++)
            addVm(ss);
        printf("Mean CPU usage is above upper limit.\nVMs added: %d\n", ss->numberOfActiveVms - numberVM);
    }
    if(ss->meanCpuUsage < ss->lowerLimit && ss->totalVms > 1){
       int numberVM = ss->numberOfActiveVms;
        for(int i = 0; (i < ss->lowerLimitVMs) && (ss->totalVms > 1);i++)
            removeVm(ss);
        printf("Mean CPU usage is bellow lower limit.\nVMs removed: %d\n", numberVM - ss->numberOfActiveVms);
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

    /*int procinicial = processing;
    int mid = processing / ss->numberOfActiveVms;
    for (int i = 0; i < ss->numberOfActiveVms; ++i) {
        if(ss->vms[i]->isActive)
            processing-=mid;
            ss->vms[i]->cpuUsage = mid;
    }*/
}

int calculateMoneySpent(SCALESET* ss){
    int totalCost = 0;
    int costPerHour = ss->vms[0]->costPerHour;
    totalCost = ss->numberOfActiveVms*costPerHour;
    // for(int i=0;i<ss->totalVms;i++){
    //     if(ss->vms[i]->isActive){
    //         totalCost += ss->vms[i]->costPerHour;
    //     }
    // }
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
    printf("Inicial number of VMs ");
    scanf("%d", &numberVMs);
    printf("Processing power of each VM ");
    scanf("%d", &processPower);
    printf("Cost of one VM per hour ");
    scanf("%f", &costPerHour);
    if(ruleId==0){
        printf("Lower limit (0;1] ");
        scanf("%f",&lowerLimit);
        printf("Upper limit (0;1] ");
        scanf("%f",&upperLimit);
        printf("Number of VMs that will be shutted down, if CPU usage is bellow the lower limit ");
        scanf("%d",&lowerLimitVMs);
        printf("Number of VMs that will be turned on, if CPU usage is above de upper limit ");
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

    outfp = fopen(OUT_FILE_NAME,"w+");

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

    printf(BOLDYELLOW"\n                               Simulation\n");
    printf("------------------------------------------------------------------------\n"RESET);

    while(fscanf(fp, "%d", &processRequired) != EOF || remaining > 0){ 
        printf("Number of requests: %d, Remaining requests: %d\n", processRequired, remaining);
        loopBegin = clock();
        timer += loopBegin;

        processRequired += remaining;
        remaining = distributeProcessing(ss, processRequired);

        localMoneySpent = calculateMoneySpent(ss);
        moneySpent += localMoneySpent;
        fprintf(outfp, "%d\n", moneySpent);

        printf("Money Spent: %d\n", localMoneySpent);
        printf("Active VMs: %d\n",ss->totalVms);

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
        
        printf("current time: day %d, %02d:00\n", day, hour);
        printf(BOLDYELLOW"------------------------------------------------------------------------\n"RESET);
    }

    //fprintf(outfp, "%d\n", moneySpent);
    printf("Total money spent: %d\n", moneySpent);

    free(ss->vms);
    free(ss);

    return 0;
}
