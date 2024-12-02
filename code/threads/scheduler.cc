// scheduler.cc
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would
//	end up calling FindNextToRun(), and that would put us in an
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "scheduler.h"

#include "copyright.h"
#include "debug.h"
#include "main.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------


static int
L2Compare(Thread *x, Thread *y) {
    if (x->priority > y->priority) {
        return -1;
    } else if (x->priority < y->priority) {
        return 1;
    } else {
        // priority一樣，比ID
        if (x->getID() < y->getID()) {
            return -1;
        } 
        return 1;
    }
}

static int
L1Compare(Thread *x, Thread *y) {
    if (x->remainingBurstTime < y->remainingBurstTime) {
        return -1;
    } else if (x->remainingBurstTime > y->remainingBurstTime) {
        return 1;
    } else {
        // priority一樣，比ID
        if (x->getID() < y->getID()) {
            return -1;
        } 
        return 1;
    }
}


Scheduler::Scheduler() {
    L3 = new List<Thread *>;
    L2 = new SortedList<Thread *>(L2Compare);
    L1 = new SortedList<Thread *>(L1Compare);
    
    
    toBeDestroyed = NULL;
}

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler() {
    delete L3;
    delete L2;
    delete L1;
    
}

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void Scheduler::ReadyToRun(Thread *thread) {
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
    thread->setStatus(READY);

    thread->startWaitTime = kernel->stats->totalTicks;
    ASSERT(thread->priority >= 0 && thread->priority <= 149);
    if (thread->priority >= 0 && thread->priority <= 49 ) {
        DEBUG(dbgZ, "[A] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() << "] is inserted into queue L[3]");
        L3->Append(thread);
    }
    else if (thread->priority >= 50 && thread->priority <= 99 ) {
        DEBUG(dbgZ, "[A] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() << "] is inserted into queue L[2]");
        L2->Insert(thread);
    }
    else if (thread->priority >= 100 && thread->priority <= 149 ) {
        DEBUG(dbgZ, "[A] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() << "] is inserted into queue L[1]");
        L1->Insert(thread);
    } 
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun() {
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    if (!L1->IsEmpty()) {
        Thread* thread = L1->RemoveFront();
        DEBUG(dbgZ, "[B] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() << "] is removed from queue L[1]");
        return thread;
    }
    else if (!L2->IsEmpty()) {
        Thread* thread = L2->RemoveFront();
        DEBUG(dbgZ, "[B] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() << "] is removed from queue L[2]");
        return thread;
    } else if (!L3->IsEmpty()) {
        Thread* thread = L3->RemoveFront();
        DEBUG(dbgZ, "[B] Tick ["<< kernel->stats->totalTicks <<"]: Thread [" << thread->getID() << "] is removed from queue L[3]");
        return thread;
    } else {
        return NULL;
    }
}


//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void Scheduler::Run(Thread *nextThread, bool finishing) {
    Thread *oldThread = kernel->currentThread;

    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {  // mark that we need to delete current thread
        ASSERT(toBeDestroyed == NULL);
        toBeDestroyed = oldThread;
    }

    if (oldThread->space != NULL) {  // if this thread is a user program,
        oldThread->SaveUserState();  // save the user's CPU registers
        oldThread->space->SaveState();
    }

    oldThread->CheckOverflow();  // check if the old thread
                                 // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running

    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());

    // This is a machine-dependent assembly language routine defined
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".
    
    //double execTime = (oldThread->burstTime <= 0.00001) ? oldThread->lastBurstTime : oldThread->burstTime;
    double execTime = 0;
    if ((oldThread->burstTime-0.0) <= 0.00001) {
        execTime = oldThread->lastBurstTime;
        //std::cout << "-->sleep" << std::endl;
    } else {
        execTime = oldThread->burstTime;
        //std::cout << "<--yield" << std::endl;
    }
    DEBUG(dbgZ, "[E] Tick ["<< kernel->stats->totalTicks <<"]: Thread ["<< nextThread->getID() 
    <<"] is now selected for execution, thread ["<< oldThread->getID() 
    <<"] is replaced, and it has executed ["<< execTime << "] ticks");

    nextThread->initialTick = kernel->stats->totalTicks;
    nextThread->waitTime = 0.0;
    SWITCH(oldThread, nextThread);
    //oldThread->initialTick = kernel->stats->totalTicks;
    // we're back, running oldThread

    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    CheckToBeDestroyed();  // check if thread we were running
                           // before this one has finished
                           // and needs to be cleaned up

    if (oldThread->space != NULL) {     // if there is an address space
        oldThread->RestoreUserState();  // to restore, do it.
        oldThread->space->RestoreState();
    }
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void Scheduler::CheckToBeDestroyed() {
    if (toBeDestroyed != NULL) {
        delete toBeDestroyed;
        toBeDestroyed = NULL;
    }
}

//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void Scheduler::Print() {
    cout << "Ready list contents:\n";
    L3->Apply(ThreadPrint);
}
