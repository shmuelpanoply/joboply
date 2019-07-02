/* file name: joboply.cpp */

#include "joboply.h"
#include <pthread.h>
#include <iostream>
#include <unistd.h>

using namespace std;

void job_done(CActiveJob *pAJob)
{
   // tell joboply a job was done
   CJoboply::GetInstance()->JobDone(pAJob);
}

static void *__TickerThreadMain(void *arg)
{
   while (1) {
      CJoboply::GetInstance()->Tick();
      sleep(TICK_EVERY);

   }
}


/* Cool marcos for easy read and maintain mutex lock */
#define LOCK_SLOTS pthread_mutex_lock(&_slots_mutex);
#define UNLOCK_SLOTS pthread_mutex_unlock(&_slots_mutex);
#define LOCK_WAIT pthread_mutex_lock(&_waiting_mutex);
#define UNLOCK_WAIT pthread_mutex_unlock(&_waiting_mutex);
#define LOCK_RUNNER pthread_mutex_lock(&_runner_mutex);
#define UNLOCK_RUNNER pthread_mutex_unlock(&_runner_mutex);


CJoboply::CJoboply(void):
    _slots(JOBOPLY_MAX_SLOTS), _jobRunner(JOBOPLY_MAX_RUNNING_JOBS, job_done)
{
   // init all mutex
   pthread_mutex_init(&_slots_mutex, NULL);
   pthread_mutex_init(&_waiting_mutex, NULL);
   pthread_mutex_init(&_runner_mutex, NULL);
   
   StartTicking();
}

CJoboply::~CJoboply(void)
{
   // free all mutex
   pthread_mutex_destroy(&_slots_mutex);
   pthread_mutex_destroy(&_waiting_mutex);
   pthread_mutex_destroy(&_runner_mutex);
}

void
CJoboply::StartTicking(void)
{
   pthread_create(&_tickerThread, NULL, __TickerThreadMain, this);
}


unsigned int 
CJoboply::AddJob(IJobable* pJob, unsigned int nReschedule)
{
   CScheduleJob *pSJob;
   unsigned int nJobId;

   nJobId = _jobIdMaker++;
   
   // Create a new Schedule job
   pSJob = new CScheduleJob(pJob, nReschedule, nJobId);
   
   

LOCK_WAIT
   // Add job to the waitng job list
   _waitingJobs.push(pSJob);
   pSJob->SetStatusBit(JS_WAITING);
UNLOCK_WAIT

LOCK_SLOTS
   // Reschedule job for it next run
   RescheduleJob(pSJob);
UNLOCK_SLOTS

   // Run jobs from Queue
   ExecuteJobFromQueue();
   return nJobId;
}


void
CJoboply::RescheduleJob(CScheduleJob *pSJob)
{
// TODO: add assert that make sure slots lock was taken
   if (pSJob->GetReschedule() == 0) {
       // no need to Reschedule, bye bye
       return;
   }
   _slots.AddJob(pSJob);
}

void
CJoboply::ExecuteJobFromQueue(void)
{
   CScheduleJob   *pSJob;
   bool isStarted;

LOCK_WAIT

   while (!_waitingJobs.empty()) {

      // Get the first job from the queue
      pSJob = _waitingJobs.front();

LOCK_RUNNER
      // add the job to the job runner
      _jobRunner.RunJob(pSJob, &isStarted);
UNLOCK_RUNNER

      // if the runner wasn't able to start the job, don't add more jobs
      if (!isStarted) {
         break;
      }

      // remove the job from the waiting queue
      _waitingJobs.pop();
      pSJob->ClearStatusBit(JS_WAITING);
   }

UNLOCK_WAIT
}

void 
CJoboply::Tick(void)
{
   CJobList *pSlot;
   CScheduleJob *pSJob;
   
LOCK_SLOTS
LOCK_WAIT
   // move to the next slot
   _slots.Tick();
   // get the current slot
   pSlot = _slots.GetZeroSlot();
   
   // go over all the jobs from the slots
   while (!pSlot->empty()) {
      // get a job from the slot
      pSJob = pSlot->front();
      // remove the job from the slot
      pSlot->pop_front();      
      pSJob->ClearStatusBit(JS_SCHEDULE);
      
      // add the job to the waiting queue
      _waitingJobs.push(pSJob);
      pSJob->SetStatusBit(JS_WAITING);
      
      // reschule the job for the next run
      RescheduleJob(pSJob);
   }
UNLOCK_WAIT
UNLOCK_SLOTS

   // try executing jobs from the waiting queue
   ExecuteJobFromQueue();
}

void
CJoboply::JobDone(CActiveJob *pAJob)
{
LOCK_RUNNER
   // tell job runner that a job is done, it will know how to handle it
   _jobRunner.JobDone(pAJob);
UNLOCK_RUNNER

   // if a job is done, try to execture more jobs from the waiting queue
   ExecuteJobFromQueue();
}

void
CJoboply::WaitingJobStatus()
{
   CScheduleJob *pSJob;
   int nSize;
   int i;

   nSize = _waitingJobs.size();
   
   cout << "Waiting Job Queue:\n";
   cout << "\tQueue size: " << nSize << endl;

   for (i = 0; i < nSize; ++i) {
      pSJob = _waitingJobs.front();
      pSJob->Status();
      _waitingJobs.pop();
      _waitingJobs.push(pSJob);
   }
}


void 
CJoboply::Status()
{
LOCK_RUNNER
   _jobRunner.Status();
UNLOCK_RUNNER

LOCK_WAIT
   WaitingJobStatus();
UNLOCK_WAIT

LOCK_SLOTS
   _slots.Status();
UNLOCK_SLOTS

}
