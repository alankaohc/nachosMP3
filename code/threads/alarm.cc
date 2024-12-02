// alarm.cc
//	Routines to use a hardware timer device to provide a
//	software alarm clock.  For now, we just provide time-slicing.
//
//	Not completely implemented.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "alarm.h"

#include "copyright.h"
#include "main.h"

//----------------------------------------------------------------------
// Alarm::Alarm
//      Initialize a software alarm clock.  Start up a timer device
//
//      "doRandom" -- if true, arrange for the hardware interrupts to
//		occur at random, instead of fixed, intervals.
//----------------------------------------------------------------------

Alarm::Alarm(bool doRandom) {
    timer = new Timer(doRandom, this);
}

//----------------------------------------------------------------------
// Alarm::CallBack
//	Software interrupt handler for the timer device. The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as
//	if the interrupted thread called Yield at the point it is
//	was interrupted.
//
//	For now, just provide time-slicing.  Only need to time slice
//      if we're currently running something (in other words, not idle).
//----------------------------------------------------------------------

void Alarm::CallBack() {
    Interrupt *interrupt = kernel->interrupt;
    MachineStatus status = interrupt->getStatus();
    
    if (status != IdleMode) {
        // aging 
        #pragma region
        int aging = 10; int maxWaitTime = 1500;
        for (int i=0; i<10; i++) {
            Thread* thread = kernel->getThread(i);
            if (thread != NULL && thread->getStatus() == READY) {
                thread->waitTime = kernel->stats->totalTicks - thread->startWaitTime;
                if (thread->waitTime >= maxWaitTime) {
                    thread->startWaitTime = kernel->stats->totalTicks;
                    ASSERT(thread->priority >= 0 && thread->priority <= 149);
                    // L3
                    if (thread->priority >= 0 && thread->priority <= 49) {
                        //std::cout << "-->Thread: " << thread->getID() << ", waitTime: " << thread->waitTime << std::endl;
                        thread->priority += aging;
                        //std::cout << "-->prioity: " << thread->priority << std::endl;
                        DEBUG(dbgZ, "[C] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() 
                                << "] changes its priority from ["<< thread->priority-aging <<"] to ["<< thread->priority <<"]");
                        if (thread->priority >= 50 && thread->priority <= 99) {
                            // L3 -> L2
                            if (kernel->scheduler->L3->IsInList(thread)) {
                                kernel->scheduler->L3->Remove(thread);
                                DEBUG(dbgZ, "[B] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() << "] is removed from queue L[3]");
                                kernel->scheduler->L2->Insert(thread);
                                DEBUG(dbgZ, "[A] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() << "] is inserted into queue L[2]");
                                
                                //std::cout << "-->L3->L2" << std::endl;
                                
                            } else {
                                //std::cout << "-->list error" << std::endl;
                            }  
                        } 
                        // nothing happen
                    }
                    //L2
                    if (thread->priority >= 50 && thread->priority <= 99) {
                        //std::cout << "-->Thread: " << thread->getID() << ", waitTime: " << thread->waitTime << std::endl;
                        thread->priority += aging;
                        //std::cout << "-->prioity: " << thread->priority << std::endl;
                        DEBUG(dbgZ, "[C] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() 
                                << "] changes its priority from ["<< thread->priority-aging <<"] to ["<< thread->priority <<"]");
                        if (thread->priority >= 100 && thread->priority <= 149) {
                            // L2 -> L1
                            if (kernel->scheduler->L2->IsInList(thread)) {
                                kernel->scheduler->L2->Remove(thread);
                                DEBUG(dbgZ, "[B] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() << "] is removed from queue L[2]");
                                kernel->scheduler->L1->Insert(thread);
                                DEBUG(dbgZ, "[A] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() << "] is inserted into queue L[1]");
                                //std::cout << "-->L2->L1" << std::endl;
                            } else {
                                //std::cout << "-->list error" << std::endl;
                            }  
                        } 
                        // nothing happen
                    }
                    //L1
                    if (thread->priority >= 100 && thread->priority <= 149) {
                        //std::cout << "-->Thread: " << thread->getID() << ", waitTime: " << thread->waitTime << std::endl;
                        if ( (thread->priority+aging) <= 149 ) {
                            thread->priority += aging;
                            //std::cout << "-->prioity: " << thread->priority << std::endl;
                            DEBUG(dbgZ, "[C] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() 
                                << "] changes its priority from ["<< thread->priority-aging <<"] to ["<< thread->priority <<"]");
                        } 
                        // nothing happen
                    }

                } 
            }
        }
        #pragma endregion
       
        // L1: preemptive->YieldOnReturn(), L3: time expired->YieldOnReturn()
        ASSERT(kernel->currentThread->priority >= 0 && kernel->currentThread->priority <= 149);
        if (kernel->currentThread->priority >= 0 && kernel->currentThread->priority <= 49) {
            // L3
            interrupt->YieldOnReturn();
        } else if (kernel->currentThread->priority >= 50 && kernel->currentThread->priority <= 99) {
            // L2
            if (!kernel->scheduler->L1->IsEmpty()) {
                interrupt->YieldOnReturn();
            }
        } else if (kernel->currentThread->priority >= 100 && kernel->currentThread->priority <= 149) {
            // L1
            interrupt->YieldOnReturn();
        } 
    }
}
