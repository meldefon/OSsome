//
// Created by meldefon on 9/13/15.
//

#ifndef MONITOR_H
#define MONITOR_H


//global struct that contains all locks, monitor variables, and condition
//variables needed to properly implement synchronization
struct Monitor {

    Monitor(){
        numOfClerks=0;
    }
    void initialize(char* lockName,char* clerkType_, int size) {
        lineLock = new Lock(lockName);
        //lineLock = CreateLock();
        lineCV = new Condition[size]();
        //lineCV = (int*) malloc(size * sizeof(int));
        clerkLock = new Lock[size]();
        //clerkLock = (int*) malloc(size * sizeof(int));
        clerkCV = new Condition[size]();
        //clerkCV = (int*) malloc(size * sizeof(int));
        lineCount = new int[size];
        //lineCount = (int*) malloc(size * sizeof(int));
        limboLineCV = new Condition();
        //limboLineCV = CreateCondition();
        bribeLineCount = new int[size];
        //bribeLineCount = (int*) malloc(size * sizeof(int));
        bribeLineCV = new Condition[size]();
        //bribeLineCV = (int*) malloc(size * sizeof(int));
        senLineCV = new Condition[size]();
        //senLineCV = (int*) malloc(size * sizeof(int));
        senLineCount = new int[size];
        //senLineCount = (int*) malloc(size * sizeof(int));
        clerkState = new int[size];
        //clerkState = (int*) malloc(size * sizeof(int));
        numOfClerks = size;
        clerkType = clerkType_;
        breakCV = new Condition();
        //breakCV = CreateCondition();
        currentCustomer = new int[size];
        //currentCustomer = (int*) malloc(size * sizeof(int));
        numCustomersInLimbo = 0;
        cashReceived = 0;

        //int sizeOfInt = sizeof(int);

        for(int i = 0; i < size; i++) {
            //*(lineCV + (i * sizeOfInt)) = CreateCondition();    
            //*(clerkLock + (i * sizeOfInt)) = CreateLock(); 
            //*(clerkCV + (i * sizeOfInt)) = CreateCondition();    
            //*(lineCount + (i * sizeOfInt)) = 0;    
            //*(bribeLineCount + (i * sizeOfInt)) = 0;    
            //*(bribeLineCV + (i * sizeOfInt)) = CreateCondition();    
            //*(senLineCV + (i * sizeOfInt)) = CreateCondition();    
            //*(senLineCount + (i * sizeOfInt)) = 0;    
            //*(clerkState + (i * sizeOfInt)) = 0;    
            //*(currentCustomer + (i * sizeOfInt)) = -1;    

            lineCount[i] = 0;
            bribeLineCount[i] = 0;
            senLineCount[i] = 0;
            clerkState[i] = 0;
            currentCustomer[i] = -1;
        }
    }

    Lock *lineLock;
    //int lineLock;
    Condition *lineCV;
    //int *lineCV;
    Condition *bribeLineCV;
    //int *bribeLineCV;
    Condition *senLineCV;
    //int *senLineCV;
    Condition *limboLineCV;
    //int limboLineCV;
    Lock *clerkLock;
    //int *clerkLock;
    Condition *clerkCV;
    //int *clerkCV;
    Condition *breakCV;
    //int breakCV;
    int *lineCount;
    int *bribeLineCount;
    int *senLineCount;
    int *clerkState;
    int numOfClerks;
    char* clerkType;
    int cashReceived;
    int *currentCustomer;
    int numCustomersInLimbo;
};


#endif //MONITOR_H
