/* file name: test_jobs.h */

#ifndef __TEST_JOBS_H__
#define __TEST_JOBS_H__

#include "jobable.h"
#include <iostream>

using namespace std;

class CTestJob1: public IJobable
{
   virtual IJobable* Clone(void) { return new CTestJob1(); }
public:
   virtual void Init(void) { cout << "CTestJob1 init\n"; }
   virtual void PreRun(void) { cout << "CTestJob1 PreRun\n"; }
   virtual void Run(void) { cout << "CTestJob1 Run\n"; }
   virtual void PostRun(void) { cout << "CTestJob1 PostRun\n"; }
   virtual int GetStatus(void) { return 0; }
};

#endif /* __TEST_JOBS_H__ */
