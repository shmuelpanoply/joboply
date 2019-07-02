/* file name: job_is.cpp */


#include "job_is.h"
#include <iostream>

using namespace std;

   ////////////////////////
   // class CScheduleJob //
   ////////////////////////

CScheduleJob::CScheduleJob(
   IJobable* pJob, 
   unsigned int nReschedule,
   unsigned int nJobId):
   _nReschedule(nReschedule), 
   _bFirstRun(true),
   _nJobId(nJobId),
   _status(JS_UNKNOWN)
{
   // make a private copy of the job
   _pJob = pJob->Clone();
}

unsigned int
CScheduleJob::GetReschedule(void)
{ 
   return _nReschedule;
}

unsigned int
CScheduleJob::GetId(void)
{
   return _nJobId;
}

void
CScheduleJob::Execute(void)
{
   if (_bFirstRun) {
      _pJob->Init();
      _bFirstRun = false;
   }
   _pJob->PreRun();
   _pJob->Run();
   _pJob->PostRun();
}

void
CScheduleJob::Status(void)
{
   cout << "\t\tJob ID: " << _nJobId 
        << "\tStatus: " << _status
        << "\tNext run: " << _nReschedule << endl;
}

void 
CScheduleJob::SetStatusBit(job_status_t bit)
{
   _status = bit;
}

void
CScheduleJob::ClearStatusBit(job_status_t bit)
{
   int offbit = ~(int)bit;
   int dummy;
   dummy = _status & offbit;
   _status = (job_status_t)dummy;
}


   /////////////////////
   // class CJobSlots //
   /////////////////////

CJobSlots::CJobSlots(unsigned int nSize): 
	_nSize(nSize), _nZeroSlot(0), _slots(nSize) { }


CJobSlots::~CJobSlots(void)
{
   // free all schedule jobs
   FreeScheduleJob();
}
   
void
CJobSlots::AddJob(CScheduleJob *pSJob)
{
   int nSlot;
  
   // get the next slot for the job 
   nSlot = (_nZeroSlot + pSJob->GetReschedule()) % _nSize;
  
   // add the job to the slot
   _slots[nSlot].push_back(pSJob);

   // set status bit
   pSJob->SetStatusBit(JS_SCHEDULE);
}
   
   
void 
CJobSlots::Tick(void)
{
   ++_nZeroSlot;
   _nZeroSlot = _nZeroSlot % _nSize;
}


CJobList *
CJobSlots::GetZeroSlot(void)
{
   return &(_slots[_nZeroSlot]);
}

void
CJobSlots::FreeScheduleJob(void)
{
   vector<CJobList>::iterator it;
   
   for (it = _slots.begin(); it != _slots.end(); ++it) {
      FreeSlot(&(*it));
   }
}

void
CJobSlots::FreeSlot(CJobList *list)
{
   CJobList::iterator it;

   for (it = list->begin(); it != list->end(); ++it) {
      delete *it;
   }
  
}

void
CJobSlots::Status(void)
{
   vector<CJobList>::iterator it;
   unsigned int nSlotIndex;

   cout << "Slots:\n";
   cout << "\tNumber of slots: " << _nSize << endl;
   cout << "\tZero slot: " << _nZeroSlot << endl;      
   
   for (it = _slots.begin(), nSlotIndex = 0; it != _slots.end(); ++it, ++nSlotIndex) {
      cout << "\tslot id: " << nSlotIndex << " slot size: " << it->size() << endl;
      SlotStatus(&(*it));
   }
}

void
CJobSlots::SlotStatus(CJobList *list)
{
   CJobList::iterator it;

   for (it = list->begin(); it != list->end(); ++it) {
      (*it)->Status();
   }
}

   //////////////////////
   // class CActiveJob //
   //////////////////////

CActiveJob::CActiveJob(CScheduleJob *pSJob, job_done_fn job_done_cb):
   _pSJob(pSJob), _job_done_cb(job_done_cb) {};


CActiveJob::~CActiveJob(void)
{
   unsigned int nReschedule;
   nReschedule = _pSJob->GetReschedule();
   
   _pSJob->ClearStatusBit(JS_RUNNING);   
   
   /* if the job won't reschedule free the job */
   if (nReschedule == 0) {
      delete _pSJob;
   }
}

static void *__ThreadMain(void *arg)
{
   CActiveJob *pActiveJob = (CActiveJob *)arg;

   pActiveJob->Execute();
   
   pActiveJob->JobDone();

   // free the job as it isn't active anymore   
   delete pActiveJob;
}


void
CActiveJob::RunJob(void)
{
   // create a new thread for the job
   pthread_create(&_thread, NULL, __ThreadMain, this);

   // set the bit as running
   _pSJob->SetStatusBit(JS_RUNNING);
}

void
CActiveJob::Execute(void)
{
   _pSJob->Execute();
}

void
CActiveJob::JobDone(void)
{
   _job_done_cb(this);
}


void
CActiveJob::Status(void)
{
   _pSJob->Status();
}


   //////////////////////
   // class CJobRunner //
   //////////////////////

CJobRunner::CJobRunner(unsigned int nMaxRunningJobs, job_done_fn job_done_cb):
   _nMaxRunningJobs(nMaxRunningJobs), _job_done_cb(job_done_cb)
{
   _nRunningJobs = 0;
}

void 
CJobRunner::RunJob(CScheduleJob *pSJob, bool *isStarted)
{
   CActiveJob   *pAJob;

   // if there are too many runnign jobs, don't add one more
   if (_nRunningJobs == _nMaxRunningJobs) {
      // return started as false
      *isStarted = false;
      return;
   }
   
   // inc running job counter
   ++_nRunningJobs;
   
   // create a new running job object
   pAJob = new CActiveJob(pSJob, _job_done_cb);   

   // add the running job to list
   _runningJobs.push_back(pAJob);
   
   // runthe job
   pAJob->RunJob();

   // return started as true
   *isStarted = true;
}


void 
CJobRunner::JobDone(CActiveJob *pAJob)
{
   // remove the job from the running job list
   _runningJobs.remove(pAJob);

   // dec the running jobs counter
   --_nRunningJobs;
   
   // Note to reader:
   //    the active job can be free now,
   //    it is getting free from the __ThreadMain function
}

void 
CJobRunner::Status(void)
{
   list<CActiveJob *>::iterator it;

   cout << "Job Runner:\n";
   cout << "\tMax running jobs: " << _nMaxRunningJobs << endl;
   cout << "\tRunning jobs: " << _nRunningJobs << endl;
   cout << "\tJobs: " << endl;

   for (it = _runningJobs.begin(); it != _runningJobs.end(); ++it) {
      (*it)->Status();
   }
 
}
