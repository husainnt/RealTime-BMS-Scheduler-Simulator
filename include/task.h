#ifndef TASK_H
#define TASK_H
#include <string>
using namespace std;

struct Task
{
    string name;
    int arrival_time;
    int execution_time;
    int deadline;
    int period;
    int priority;
};

#endif