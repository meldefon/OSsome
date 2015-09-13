//
// Created by meldefon on 9/13/15.
//

#ifndef OSSOME_MONITOR_H
#define OSSOME_MONITOR_H


//global struct that contains all locks, monitor variables, and condition
//variables needed to properly implement synchronization
struct Monitor {
    Lock *lineLock;
    Condition *lineCV;
    Lock *clerkLock;
    Condition *clerkCV;
    int *lineCount;
    int *bribeLineCount;
    int *clerkState;
}


#endif //OSSOME_MONITOR_H
