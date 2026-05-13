#ifndef TASK_H
#define TASK_H
#include <string>
#include <pthread.h>
using namespace std;

struct Task
{
    string name;
    int arrival_time;
    int execution_time;
    int deadline;
    int period;
    int priority;
    int hit_count;
    int miss_count;
    int pipe_fd[2];
};
#endif