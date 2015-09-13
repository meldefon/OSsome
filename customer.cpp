//
// Created by meldefon on 9/13/15.
//




void doPassportClerkStuff(){

    //First get in line with a generic method
    myLine = getInLine(passportClerkLineMonitor);

    //Enter interaction monitor with passport clerk
    Lock workLock = passportClerkLock[myLine];
    CV workCV = passportClerkCV[myLine];
    workLock->Acquire();

    //Tell Clerk CV, then wait
    tellPassportClerkSSN(mySSN);
    workCV->Signal(workLock);
    workCV->Wait(workLock);

    //Now leave
    workCV->Signal(workLock);
    workLock->Release();

    //Decide weather to self-punish
    if(!passportClerkChecked) {
        punish(punishTime);
    }
    return;

}

void doCashierStuff(){

    //First get in line with a generic method
    myLine = getInLine(cashierLineMonitor);

    //Enter interaction monitor with passport clerk
    Lock workLock = cashierLock[myLine];
    CV workCV = cashierCV[myLine];
    workLock->Acquire();

    //Identify self
    tellCashierSSN(mySSN);
    workCV->Signal(workLock);
    workCV->Wait(workLock);

    //If wrong, leave and punish, if right, continue
    if(!cashierChecked){
        workCV->Signal(workLock);
        workLock->Release();
        punish(punishTime);
        return;
    }
    //Now you can pay
    payCashier();
    workCV->Signal(workLock);
    workCV->Wait(workLock);

    //Now you have passport, so leave
    workCV->Signal();
    workLock->Release();
    return;

}



void punish(int punishTime){
    for (int i = 0; i < punishTime; i++) {
        CurrentThread->Yield();
    }
}