// file name: main.cpp

#include <iostream>
#include <errno.h>
#include "joboply.h"
#include "test_jobs.h"
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{
   CTestJob1 tb1;

   
   CJoboply::GetInstance()->AddJob(&tb1, 2);
   
   
   while(1) {
      CJoboply::GetInstance()->Status();
      sleep(2);
   };
   
   return 0;
}
