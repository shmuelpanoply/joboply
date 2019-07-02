/* filename: joboply.h */

#ifndef __JOBOPLY__H__
#define __JOBOPLY__H__

#include "jobable.h"
#include "job_is.h"
#include <sstream>
#include <atomic>

#define JOBOPLY_MAX_SLOTS 12
#define JOBOPLY_MAX_RUNNING_JOBS 10
#define TICK_EVERY 6

/* THE CLASS TO USE */
class CJoboply 
{
private:
   /* private constructor, this one is a singleton :*/ 
   CJoboply(void);
   
public:
   /* Get instance singleton pattern's method */
   static inline CJoboply* GetInstance(void) {
      static CJoboply _instance; /*!< instance for signleton pattern */
      return &_instance;
   }

public:
   /* free all allocated memory */
   ~CJoboply(void);

   /* Add a job to the system
      nReschedule - is a value reschdule the job it wrap at JOBOPLY_MAX_SLOTS(12)
      for one time job use 0
      the method returns job id
   */
   unsigned int AddJob(IJobable* pJob, unsigned int nReschedule);

   /* Tick */
   void Tick(void);

   /* handle a job that is done */   
   void JobDone(CActiveJob *pAJob);

   /* log out status */
   void Status(void);

private:

   /* Reschedule a job */
   void RescheduleJob(CScheduleJob *pSJob);

   /* Execute job from waiting q */
   void ExecuteJobFromQueue(void);
   
   /* log out waiting q status*/
   void WaitingJobStatus(void);

   /* start a side thread to tick */
   void StartTicking(void);

private:
   /* data members */
   CJobSlots _slots;
   CJobQueue _waitingJobs;
   CJobRunner _jobRunner;
   atomic<int> _jobIdMaker;
   
   pthread_mutex_t _slots_mutex;
   pthread_mutex_t _waiting_mutex;
   pthread_mutex_t _runner_mutex;
   
   pthread_t _tickerThread;
};

#endif // __JOBOPLY__H__
