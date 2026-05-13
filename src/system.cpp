#include <iostream>
#include <fstream>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include "../include/task.h"
using namespace std;

pthread_mutex_t mutex;

void assignRMSPriorities(vector<Task> &taskList)
{
    sort(taskList.begin(), taskList.end(), [](const Task &a, const Task &b)
         { return a.period < b.period; });

    for (size_t i = 0; i < taskList.size(); i++)
    {
        taskList[i].priority = i + 1;
    }
}

// my worker thread
void *taskWorker(void *arg)
{
    Task *task = (Task *)arg;
    for (int cycle = 1; cycle <= 5; cycle++)
    {
        usleep(task->arrival_time * 1000);
        pthread_mutex_lock(&mutex);
        cout << "\nTask " << task->name << " started | Cycle " << cycle << " | Priority " << task->priority << endl;
        pthread_mutex_unlock(&mutex);

        // Inter-Task-Comm
        if (task->name == "Voltage")
        {
            const char *alert = "High Voltage Warning!";
            write(task->pipe_fd[1], alert, strlen(alert) + 1);
        }
        else if (task->name == "Logger")
        {
            char buffer[100];
            read(task->pipe_fd[0], buffer, sizeof(buffer));
            pthread_mutex_lock(&mutex);
            cout << "Logger Task received pipe message: " << buffer << endl;
            pthread_mutex_unlock(&mutex);
        }

        usleep(task->execution_time * 1000);
        pthread_mutex_lock(&mutex);

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
        int remaining_time = task->period - task->execution_time;
        if (remaining_time > 0)
        {
            usleep(remaining_time * 1000);
        }
    }
    return NULL;
}

// loading tasks from file
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
    pthread_mutex_init(&mutex, NULL);

    // creating pipe
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

    assignRMSPriorities(taskList);
    displayTasks(taskList);

    vector<pthread_t> threads(taskList.size());
    for (size_t i = 0; i < taskList.size(); i++)
    {
        // passing pipe ends to all tasks
        taskList[i].pipe_fd[0] = comm_pipe[0];
        taskList[i].pipe_fd[1] = comm_pipe[1];

        int status = pthread_create(&threads[i], NULL, taskWorker, &taskList[i]);
        if (status != 0)
        {
            cout << "Thread creation failed for " << taskList[i].name << endl;
        }
    }

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

    cout << "\nAll tasks finished\n";
    return 0;
}