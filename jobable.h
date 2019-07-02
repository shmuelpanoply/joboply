/* file name: jobable.h */

#ifndef __JOBABLE__H__
#define __JOBABLE__H__

using namespace std;


/* This enum is used to get status of a job for CJoboply PoV*/
// TODO: move this to another file
typedef enum {
   JS_UNKNOWN  = 0x0,
   JS_SCHEDULE = 0x1,
   JS_WAITING  = 0x2,
   JS_RUNNING  = 0x4,
} job_status_t;


/* 
   Use this interface class to create a job object
   for CJoboply.
*/
class IJobable {
public:
   // Clone function for Prototype pattern
   virtual IJobable* Clone(void) = 0;
   
public:
   // Init method, will get called one time. before running
   virtual void Init(void) = 0;

   // For every run all the 3 function will get called.
   virtual void PreRun(void) = 0;
   virtual void Run(void) = 0;
   virtual void PostRun(void) = 0;


   // Return an int that should represnt the "internal job-wise" status.
   virtual int GetStatus(void) = 0;
};


#endif // __JOBABLE__H__
