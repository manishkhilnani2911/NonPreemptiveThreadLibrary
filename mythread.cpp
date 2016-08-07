#include <ucontext.h>
#include <queue>
#include <deque>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mythread.h"

//created my MANISH KHILNANI 
//unityid mkhilna

using namespace std;

#define STACKSIZE 8192

unsigned long tcount=0;

//structure of threads
struct node
{
    struct Thread *thread;
    struct node *next;
};

//structure to hold the info about a single thread
struct Thread{
	ucontext_t t_context;
	Thread* Parent;
	Thread* WaitingChild;
	unsigned long Child_Count,thread_id;
	//also need to create a list to store children of this thread
	deque<Thread*> ChildList;
};

struct Semaphore_st{
	int initial_value;
	deque<Thread*> Semdeque;
};

//queue to place threads ready to execute
deque<Thread*> ReadyQ;
//queue to place threads with result of join and join all
deque<Thread*> BlockedQ;


struct Thread *ParentThread;
struct Thread *RunningThread;

ucontext_t* par_context;

//functions declared in the header file
//function to start the program creating the main parent thread
void MyThreadInit(void (*start_funct)(void *), void *args) {
    //ut_Parent_Thread = (ucontext_t *) malloc(sizeof(ucontext_t));
    //cout<<"In MyThreadInit"<<endl;
	par_context = (ucontext_t*) malloc(sizeof(ucontext_t));
	getcontext(par_context);
   
	ParentThread =  new Thread;//(Thread*) malloc(sizeof(Thread));

    //memset(ParentThread, 0, sizeof (Thread));
        ParentThread->Parent = NULL;
        ParentThread->WaitingChild = NULL;

    RunningThread = new Thread;

    if (NULL == RunningThread) {
        //cout<<"\nMemory Allocation Failed at Line: %d\n", __LINE__";
        cout<<"\nMemory Allocation Failed at Line: "<< __LINE__<<endl;
        exit(0);
    }
    //memset(RunningThread, 0, sizeof(Thread));
    RunningThread->Parent = NULL;
        RunningThread->WaitingChild = NULL;

	//Calling Context
    getcontext(&RunningThread->t_context);

    RunningThread->t_context.uc_stack.ss_sp =  malloc(STACKSIZE*sizeof(char));
    if (NULL == RunningThread->t_context.uc_stack.ss_sp) {
            cout<<"\nMemory Allocation Failed at Line: "<< __LINE__<<endl;
            //cout<<"\nMemory Allocation Failed at Line: %d\n", __LINE__";
            exit(0);
    }
    RunningThread->t_context.uc_stack.ss_size = (STACKSIZE *sizeof(char));
    RunningThread->t_context.uc_link = NULL;


    RunningThread->Parent = ParentThread;
    
    //t_ParentThread->child = t_RunningThread;

    makecontext(&RunningThread->t_context,(void (*)(void))start_funct, 1, args);
//  printf("End mythreadinit");
    swapcontext(&ParentThread->t_context,&RunningThread->t_context);
}

//function to create a thread
void *MyThreadCreate(void (*start_funct)(void *), void *args) {
    /*Create New Thread
     *Make Parent of this thread as current running thread
     *Add to Ready queue
     * */

    Thread *TempRunningThread = NULL;
    Thread *TempParentThread = NULL;

    //cout<<"\nInside MyThreadCreate\n";

    TempRunningThread = new Thread;

            if (NULL == TempRunningThread) {
                    cout<<"\nMemory Allocation Failed at Line: "<< __LINE__<<endl;
                    exit(0);
            }
    //memset(TempRunningThread, 0, sizeof(Thread));
    TempRunningThread->Parent = NULL;
    TempRunningThread->WaitingChild = NULL;
    
    //Calling Context
    getcontext(&TempRunningThread->t_context);
//allocating stack space for the new thread
    TempRunningThread->t_context.uc_stack.ss_sp = (char *) malloc(STACKSIZE *sizeof(char));

    if (NULL == TempRunningThread->t_context.uc_stack.ss_sp) {
            printf("\nMemory Allocation Failed at Line: %d\n", __LINE__);
            exit(0);
    }
    TempRunningThread->t_context.uc_stack.ss_size = (STACKSIZE *sizeof(char));
	//saving context to jump after the current thread finishes
    TempRunningThread->t_context.uc_link = NULL;
    TempRunningThread->Parent = RunningThread;
    TempRunningThread->thread_id = tcount;
    tcount = tcount+1;
    makecontext(&TempRunningThread->t_context,(void (*)(void))start_funct, 1, args);

#if 0
    getcontext(&t_TempParentThread->uct_Context);

    swapcontext(&t_TempParentThread->uct_Context,&t_RunningThread->uct_Context);
#endif

    if(RunningThread->ChildList.empty())
    {
        //t_RunningThread->st_ChildrenList = malloc(sizeof(st_QUEUE));
        //memset(RunningThread->ChildrenList,0,sizeof(st_QUEUE));
        RunningThread->ChildList.clear();
       // cout<<"SADAD "<<RunningThread->ChildList.size()<<"\n";
        RunningThread->Child_Count = 0;
    }
//cout<<"SADAD "<<RunningThread->ChildList.size()<<"\n";
    
    /*Insert Newly created thread in the list of children of thread*/
    //this node will be added to the children queue of the current running thread
   // cout<<"SADAD "<<RunningThread->ChildList.size()<<"\n";
    RunningThread->ChildList.push_back(TempRunningThread);
   // cout<<"SADAD"<<RunningThread->ChildList.size()<<"\n";
    RunningThread->Child_Count = RunningThread->Child_Count + 1;
    
    if(RunningThread->ChildList.empty()){
        //cout<<"EHDFUIDF"<<endl;
    }
    //TempRunningThread->Parent = RunningThread;

    ReadyQ.push_back(TempRunningThread);
    //enque(&Q_ReadyQ,t_RunningThread);
    //OS_Printf("\nLeaving MTC\n");
    //makecontext(&t_RunningThread->uct_Context,(void (*)(void))start_funct, 1, args);
    /*Return Newly created thread*/
    return (void *)(TempRunningThread);
}


int MyThreadJoin(void * childThread)
{
	Thread *thread = (Thread*) childThread;
    	//printf("\nMyThreadJoin\n");
    	if(NULL == childThread)
        	return 0; //Child Thread Does not Exist
    	if((thread->Parent) != RunningThread)
        	return -1;

	Thread* temp = RunningThread;
	Thread* Next_readyT = NULL;
	if(!ReadyQ.empty()){
		Next_readyT = (Thread*) ReadyQ.front();
                ReadyQ.pop_front();
		RunningThread = Next_readyT;
                temp->WaitingChild =(Thread*) childThread;
		BlockedQ.push_back(temp);
		swapcontext(&temp->t_context,&RunningThread->t_context);
		
	}
	return 0;
}

void MyThreadJoinAll(void)
{
    Thread *TempThread = RunningThread;
	//check to see if the childlist is empty or not
	//if not empty then put the current thread in blocked queue 
    if(!RunningThread->ChildList.empty())
    {   
            //enque(&Q_BlockedQ,t_RunningThread);
		BlockedQ.push_back(RunningThread);
            	RunningThread = ReadyQ.front();
		ReadyQ.pop_front();
                swapcontext(&TempThread->t_context,&RunningThread->t_context);
    }
}


void MyThreadYield(void) {
    Thread *Temp = RunningThread;
    Thread *Next_readyT = NULL;

    Next_readyT = (Thread *) ReadyQ.front();
    ReadyQ.pop_front();
    //getcontext(&RunningThread->t_context);
    ReadyQ.push_back(RunningThread);

    RunningThread = Next_readyT;
    swapcontext(&(Temp->t_context), &(RunningThread->t_context));
    //setcontext(&t_RunningThread->uct_Context);
	//RunningThread = Next_readyT;
}


void MyThreadExit(void)
{
	int i;
	cout<<"In Exit"<<endl;
	//cout<<"In Exit"<<endl;
	Thread* CurrentThread = NULL;
	Thread* temp = RunningThread;
	//if(RunningThread->thread_id == 0)
	//	ReadyQ.push_back((node*)RunningThread);
	Thread* Next_readyT = NULL;
	//get the next ready element from the readyQ
        //remove element from parents childlist
	//cout<<&RunningThread->Parent<<endl;
	//cout<<RunningThread->Parent->thread_id<<endl;
	cout<<"is"<<endl;
	if(RunningThread)
	RunningThread->Parent->Child_Count = RunningThread->Parent->Child_Count-1;
	cout<<"deleting"<<endl;
	//removing thread from the parents child list
	deque<Thread*>::iterator it;
        //if(RunningThread->Parent->ChildList.empty())
            //cout<<"EMPTPYY!"<<endl;
	if(RunningThread){
	for(i=0,it = RunningThread->Parent->ChildList.begin();it != RunningThread->Parent->ChildList.end();it++){
		if(*it == RunningThread)
			break;
		i++;
	}}
	cout<<"here"<<endl;
	
	if(RunningThread)
	RunningThread->Parent->ChildList.erase(RunningThread->Parent->ChildList.begin()+i);
	//checking for dependence in Blocked Queue and differentiate from join and join all
	//cout<<"here2"<<endl;
	if(!BlockedQ.empty()){
		for(i=0,it = BlockedQ.begin();it != BlockedQ.end();it++){
			if(*it == RunningThread->Parent && ( (RunningThread->Parent->WaitingChild == RunningThread) || (RunningThread->Parent->ChildList.empty()) )){
				CurrentThread = (Thread*)*it;
				break;
			}
			i++;	
		}
	}
	cout<<"Here1"<<endl;
	if(CurrentThread != NULL ){
		//remove element from the blocked Q
		ReadyQ.push_back(CurrentThread);
		BlockedQ.erase(BlockedQ.begin()+i);
		//Next_readyT = (Thread*) ReadyQ.front();
			Next_readyT = (Thread*) ReadyQ.front();
			RunningThread = Next_readyT;
	        //RunningThread = CurrentThread;

			//swapcontext( &(temp->t_context), &(RunningThread->t_context) );
		//swapcontext( &(temp->t_context), &(RunningThread->t_context) );
						ReadyQ.pop_front();
swapcontext( &(temp->t_context), &(RunningThread->t_context) );
	}
	else{
		//no thread in blockedQ check element from readyQ
		if(!ReadyQ.empty()){
		//cout<<"Q Not empty"<<endl;
			//cout<<"Not empty"<<endl;
			Next_readyT = (Thread*) ReadyQ.front();
			RunningThread = Next_readyT;
			ReadyQ.pop_front();
			//cout<<"swapping context from "<<temp->thread_id<<" to "<<RunningThread->thread_id<<endl;
			swapcontext( &(temp->t_context), &(RunningThread->t_context) );
			//ReadyQ.pop_front();
		}
		else{
			//cout<<"Empty Q"<<endl;
			setcontext(&ParentThread->t_context);
		}
	}
}


void * MySemaphoreInit(int initialValue)
{
    if(initialValue < 0)
    {
	cout<<"Less Than zero Value not allowed"<<endl;
        exit(1);        
    }
    Semaphore_st *newSemaphore = new Semaphore_st;
    newSemaphore->initial_value = initialValue;
    return newSemaphore;
}

void MySemaphoreSignal(void *sem)
{
    Semaphore_st *st_sem = (Semaphore_st*) sem;
    if(NULL == st_sem)
    {
        printf("\nError: Uninitialized Semaphore. Program Exiting\n");
        exit(1);
    }
    st_sem->initial_value++;
    if(st_sem->initial_value < 1)
    {
        /*Wiki:Definition http://en.wikipedia.org/wiki/Semaphore_%28programming%29
         * if the pre-increment value was negative (meaning there are
         *  processes waiting for a resource), it transfers a blocked
         *  process from the semaphore's waiting queue to the ready queue.*/
	Thread* TempThread = st_sem->Semdeque.front();
	st_sem->Semdeque.pop_front();
	ReadyQ.push_back(TempThread);
    }    
}


void MySemaphoreWait(void *Semaphore)
{
    Semaphore_st *st_Sem =(Semaphore_st*) Semaphore;
    if(NULL == st_Sem)
    {
        printf("\nError: Uninitialized Semaphore. Program Exiting\n");
        exit(1);
    }
    st_Sem->initial_value--;
    if(st_Sem->initial_value < 0)
    {
        /*From Wiki: http://en.wikipedia.org/wiki/Semaphore_%28programming%29
         If the value becomes negative, 
         *the process executing wait is blocked, i.e.,
         *  added to the semaphore's queue.*/
        Thread *TempThread = RunningThread;
        st_Sem->Semdeque.push_back(RunningThread);
        RunningThread =(Thread*) ReadyQ.front();
	ReadyQ.pop_front();
        if(NULL == RunningThread)
            setcontext(&ParentThread->t_context);
        swapcontext(&TempThread->t_context,&RunningThread->t_context);
    }
}

int MySemaphoreDestroy(void *sem)
{
	cout<<"In Destror"<<endl;
    Semaphore_st *st_Sem = (Semaphore_st*) sem;
	deque<Thread*>::iterator it;
    if(NULL == st_Sem)
    {
        printf("\nError: Uninitialized Semaphore. Program Exiting\n");
        exit(1);
    }
    if(!st_Sem->Semdeque.empty()){
        if(st_Sem->Semdeque.front())
        {
            return -1;
        }
        else
        {
	for(it = st_Sem->Semdeque.begin(); it != st_Sem->Semdeque.end(); it++)	
            delete *it;
		st_Sem->Semdeque.clear();
        }
    }
	cout<<"returning"<<endl;
    return 0;
}
