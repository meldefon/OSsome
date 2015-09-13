//
// Created by meldefon on 9/13/15.
//


void doJob(){

    //Get next customer with a generic method
    clerkPullFromLine(passportLineLock[myLine]);

    //Enter into interaction monitor
    Lock workLock = passportClerkLock[myLine];
    CV workCV = passportClerkCV[myLine];
    /*workCV->Acquire()*/ //Could put this in here or in the PullFromLine function

    //Check and write to public data
    workDelay(workDelayTime);
    checked[currentCustomerSSN] = applicationSubmitted[currentCustomerSSN] && pictureTaken[currentCustomerSSN];

    //Tell customer you've finished your job
    workCV->Signal();
    workCV->Wait();

    //Now customer is gone
    workLock->Release();

}



void workDelay(int workDelayTime){
    for(int i = 0;i<workDelayTime;i++){
        CurrentThread->Yield();
    }
}