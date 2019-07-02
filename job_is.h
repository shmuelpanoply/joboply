/* filename: job_is.h */

#ifndef __JOB_IS__H__
#define __JOB_IS__H__


#include "jobable.h"
#include <queue>
#include <list>
#include <pthread.h>


using namespace std;

/* 
   This class is a schudle job, it wrap about the IJobable
   and contian more meta data such as
      > Joboply status
      > When to reschedule
      > JobId
*/
class CScheduleJob
{
public:
   /* param constructor */
   CScheduleJob(IJobable* pJob, unsigned int nReschedule, unsigned int nJobId);

   /* get reschedule value */
   unsigned int GetReschedule(void);

   /* get job id */
   unsigned int GetId(void);

   /* Execute the job */   
   void Execute(void);

   /* log out status */
   void Status(void);

   /* set status bit */ 
   void SetStatusBit(job_status_t bit);

   /* clear status bit */
   void ClearStatusBit(job_status_t bit);

private:
   /* data member */

   IJobable *_pJob;
   unsigned int _nReschedule;
   bool _bFirstRun;
   unsigned int _nJobId;
   job_status_t _status;
};

/* Q of CScheduleJob */
typedef queue<CScheduleJob *> CJobQueue;

/* List of CScheduleJob */
typedef list<CScheduleJob *> CJobList;

/* 
  Jobslot contain a slots (in a vector)
  each slot is a list of CScheduleJob.
  the jobs in that slot should run at the same time
*/
class CJobSlots
{
public:
   /* param constructor */
   CJobSlots(unsigned int nSize);

   /* destructor */
   ~CJobSlots(void);
      
   /* add a job to the right slot */
   void AddJob(CScheduleJob *pSJob);
   
   /* shift to the next slot */
   void Tick(void);

   /* get the current slot */   
   CJobList *GetZeroSlot(void);

   /* log out status */
   void Status(void);

private:
   /* free all ScheduleJob from all the slots */
   void FreeScheduleJob(void);

   /* Free all ScheduleJob from a slot */
   void FreeSlot(CJobList *list);

   /* log out status on a slot */
   void SlotStatus(CJobList *list);


private:
   vector<CJobList>  _slots;
   unsigned int _nSize;
   unsigned int _nZeroSlot;
};


/* 
   This class define active job
*/
class CActiveJob;


// define functio pointer (callback) to ack when a job is done.
typedef void (*job_done_fn)(CActiveJob *pActiveJob);

class CActiveJob {
public:
   /* param constructor */
   CActiveJob(CScheduleJob *pSJob, job_done_fn job_done_cb);

   /* destructor */
   ~CActiveJob(void);

   /* Run the job mean to create a thread that will execute the job */
   void RunJob(void);
   
   /* Call IJobable run methods */
   void Execute(void);
   
   /* handle a job that is done */
   void JobDone(void);

   /* log out status */
   void Status(void);

private:
   CScheduleJob  *_pSJob;
   job_done_fn _job_done_cb;
   pthread_t _thread;
};

/* This class runs and maintains all active jobs */
class CJobRunner {
public:
   /* param constructor */
   CJobRunner(unsigned int nMaxRunningJobs, job_done_fn job_done_cb);

   /* Note to reader:
      no destructor, all CActiveJob in _runningJobs, will get free
      by the execture thread */

   /* start running a job */
   void RunJob(CScheduleJob *pSJob, bool *isStarted);

   /* handle a job that is done */
   void JobDone(CActiveJob *pAJob);
   
   /* log out status */
   void Status(void);
   
private:
   list<CActiveJob *> _runningJobs;
   unsigned int _nMaxRunningJobs;
   unsigned int _nRunningJobs;
   
   job_done_fn _job_done_cb;
};


#endif // __JOB_IS__H__ 
