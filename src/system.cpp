#include <iostream>
#include <fstream>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include "../include/task.h"
using namespace std;
// global mutex var
pthread_mutex_t mutex;

/*through this function I implement the RMS logic, where a shorter period=higher priority
   I first sort the tasks by their length and then assign priorities.
*/
void assignRMSPriorities(vector<Task> &taskList)
{
    // used built in sort func
    sort(taskList.begin(), taskList.end(), [](const Task &a, const Task &b)
         { return a.period < b.period; });

    // after sorting, the first task has the shortest period, so i give it priority 1.
    for (size_t i = 0; i < taskList.size(); i++)
    {
        taskList[i].priority = i + 1;
    }
}

/*through this func i simulate a real time periodic execution loop for 5 cycles. It handles
   arrival delays, pipe messaging, workload execution, and deadline tracking.
*/
void *taskWorker(void *arg)
{
    Task *task = (Task *)arg;

    // my logic to run each BMS task for exactly 5 cycles to observe its periodic timeline
    for (int cycle = 1; cycle <= 5; cycle++)
    {
        // i scale usleep with 1000 as usleep calc in mirco sec, but i/p is is milli sec
        usleep(task->arrival_time * 1000);
        // CS start
        pthread_mutex_lock(&mutex);
        cout << "\nTask " << task->name << " started | Cycle " << cycle << " | Priority " << task->priority << endl;
        pthread_mutex_unlock(&mutex);

        // used pipes to implement inter task comm
        if (task->name == "Voltage")
        {
            const char *alert = "High Voltage Warning!";
            write(task->pipe_fd[1], alert, strlen(alert) + 1);
        }
        else if (task->name == "Logger")
        {
            /*my logic to fix deadlocks at start and shutdown as earlier in my code, the voltage ran very fast and finished around 5 cycles early
            causing the output to freeze, so i limited reading to cyc 1 and 2
            */
            if (cycle <= 2)
            {
                char buffer[100];
                read(task->pipe_fd[0], buffer, sizeof(buffer));
                pthread_mutex_lock(&mutex);
                cout << "Logger Task received pipe message: " << buffer << endl;
                pthread_mutex_unlock(&mutex);
            }
        }

        // sim cpu processing time
        usleep(task->execution_time * 1000);

        pthread_mutex_lock(&mutex);
        // here i chk for hit and miss
        if (task->execution_time <= task->deadline)
        {
            cout << task->name << " completed within deadline" << endl;
            task->hit_count++;
        }
        else
        {
            cout << task->name << " missed deadline" << endl;
            task->miss_count++;
        }
        pthread_mutex_unlock(&mutex);
        // here i find the remaining time left so task restarts when next period arrives
        int remaining_time = task->period - task->execution_time;
        if (remaining_time > 0)
        {
            usleep(remaining_time * 1000);
        }
    }
    return NULL;
}

// through this func i load tasks from my i/p file "bms_default.txt"
vector<Task> loadTasks(const string &fileName)
{
    vector<Task> taskList;
    ifstream inputFile(fileName);
    if (!inputFile)
    {
        cerr << "Error opening file!" << endl;
        return taskList;
    }
    Task task;
    // reading col vec row wise
    while (inputFile >> task.name >> task.arrival_time >> task.execution_time >> task.deadline >> task.period)
    {
        task.priority = 0;
        task.hit_count = 0;
        task.miss_count = 0;
        taskList.push_back(task);
    }
    inputFile.close();
    return taskList;
}

// through this funct I display the req o/p
void displayTasks(const vector<Task> &taskList)
{
    cout << "\nTask Configuration\n";
    for (const auto &task : taskList)
    {
        cout << "--------------------------" << endl;
        cout << "Task: " << task.name << endl;
        cout << "Arrival Time: " << task.arrival_time << " ms" << endl;
        cout << "Execution Time: " << task.execution_time << " ms" << endl;
        cout << "Deadline: " << task.deadline << " ms" << endl;
        cout << "Period: " << task.period << " ms" << endl;
        cout << "Priority: " << task.priority << endl;
    }
}

int main()
{
    cout << "EV Battery Management Scheduler\n";

    // here I initialise mutex
    pthread_mutex_init(&mutex, NULL);
    // here i create pipe and call it
    int comm_pipe[2];
    if (pipe(comm_pipe) == -1)
    {
        cout << "Pipe failed!" << endl;
        return 1;
    }

    vector<Task> taskList = loadTasks("task_inputs/bms_default.txt");
    if (taskList.empty())
    {
        return 1;
    }

    // here i call function to setup priorities before handing them to threads
    assignRMSPriorities(taskList);
    displayTasks(taskList);

    // hold thread for each task
    vector<pthread_t> threads(taskList.size());
    for (size_t i = 0; i < taskList.size(); i++)
    {
        // passing shared pipeline endpoint handles down to every separate task data record
        taskList[i].pipe_fd[0] = comm_pipe[0];
        taskList[i].pipe_fd[1] = comm_pipe[1];

        // here i used pthread_create to spin up the independent Linux thread lines
        int status = pthread_create(&threads[i], NULL, taskWorker, &taskList[i]);
        if (status != 0)
        {
            cout << "Thread creation failed for " << taskList[i].name << endl;
        }
    }

    // barrier tracking synchronization:
    // the main thread waits here via pthread_join until all worker loops have completed entirely
    for (size_t i = 0; i < taskList.size(); i++)
    {
        pthread_join(threads[i], NULL);
    }

    cout << "\nTask Summary\n";
    for (const auto &task : taskList)
    {
        cout << task.name << " | Hits: " << task.hit_count << " | Misses: " << task.miss_count << endl;
    }

    close(comm_pipe[0]);
    close(comm_pipe[1]);
    pthread_mutex_destroy(&mutex);
    cout << endl;
    cout << "All tasks finished\n";
    return 0;
}