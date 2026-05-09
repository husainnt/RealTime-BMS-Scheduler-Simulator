#include <iostream>
#include <fstream>
#include <vector>
#include "../include/task.h"
using namespace std;

// func to load all tasks from file into a vector
vector<Task> loadTasks(const string &fileName)
{
    vector<Task> taskList;
    ifstream inputFile(fileName);
    if (!inputFile) // chk if file successfully opened
    {
        cout << "Error opening file!" << endl;
        return taskList;
    }
    Task currentTask;
    // reading all task data from file
    while (inputFile >> currentTask.name >> currentTask.arrival_time >> currentTask.execution_time >> currentTask.deadline >> currentTask.period)
    {
        currentTask.priority = 0;        // RMS will assign priority later
        taskList.push_back(currentTask); // store current task
    }
    inputFile.close();
    return taskList;
}
// func to display the tasks loaded
void displayTasks(const vector<Task> &taskList)
{
    cout << "\n TASK LIST \n";
    for (const auto &currentTask : taskList)
    {
        cout << "Task Name       : " << currentTask.name << endl;
        cout << "Arrival Time    : " << currentTask.arrival_time << endl;
        cout << "Execution Time  : " << currentTask.execution_time << endl;
        cout << "Deadline        : " << currentTask.deadline << endl;
        cout << "Period          : " << currentTask.period << endl;
        cout << "Priority        : " << currentTask.priority << endl;
        cout << "\n";
    }
}

int main()
{
    vector<Task> taskList;
    taskList = loadTasks("task_inputs/bms_default.txt");
    displayTasks(taskList);
    return 0;
}