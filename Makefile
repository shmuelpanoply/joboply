all:
	g++ -std=c++11 -c main.cpp
	g++ -std=c++11 -c job_is.cpp
	g++ -std=c++11 -c joboply.cpp
	g++ -std=c++11 -o demo main.o job_is.o joboply.o -lpthread
