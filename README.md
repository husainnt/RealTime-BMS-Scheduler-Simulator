# Real-Time Task Scheduling Simulator for EV Battery Management System (BMS)

An implementation of a Real-Time Operating System (RTOS) task scheduling simulator in C++ under a Linux environment. This project simulates an Electric Vehicle (EV) Battery Management System (BMS) running concurrent, safety-critical periodic tasks managed via the **Rate Monotonic Scheduling (RMS)** algorithm.

---

## 🏎️ Scenario Overview: EV Battery Management System
In an Electric Vehicle, the Battery Management System (BMS) must monitor multiple operational arrays concurrently to prevent cells from overcharging, undercharging, or experiencing catastrophic thermal runaway. 

This simulator models 5 distinct periodic background loops running at varied frequencies:
1. **Cell Voltage Monitoring** (100ms period) -> **Priority 1 (Highest)**
2. **Temperature Sensor Reading** (200ms period) -> **Priority 2**
3. **State of Charge (SoC) Calculation** (500ms period) -> **Priority 3**
4. **Cell Balancing Control** (1000ms period) -> **Priority 4**
5. **Diagnostic Logging** (2000ms period) -> **Priority 5 (Lowest)**

---

## 🛠️ Key Architectural Features

* **Multithreading Concurrency:** Deploys native POSIX threads (`pthreads`) where each tracking component runs as an independent, concurrent lifecycle worker.
* **Rate Monotonic Scheduling:** Automatically calculates static priority ranks inversely proportional to task periods at startup (shorter periods grab higher priority ranks).
* **Race Condition Synchronization:** Uses a global `pthread_mutex_t` gate to isolate the console standard output stream (`cout`), preventing interleaved terminal prints.
* **Inter-Task Communication (UNIX Pipe):** Implements data passing where the high-priority `Voltage` producer thread writes warnings into a pipeline that the `Logger` consumer thread reads and processes. Includes deadlock prevention logic.
* **Preemption Strategy Simulation:** Implements a student-level spinning back-off preemption check using a global CPU tracking state so low-priority tasks yield execution pathways when high-priority tasks arrive.

---

## 📁 Project Directory Structure

```text
RealTime-BMS-Scheduler-Simulator/
├── include/
│   └── task.h            # Task struct metadata blueprint definitions
├── src/
│   └── system.cpp        # Main application logic, scheduler engine, and thread workers
├── task_inputs/
│   ├── bms_default.txt   # Standard operational tasks vector parameters
│   └── overload.txt      # Heavy workload configuration to force deadline misses
└── README.md             # Project documentation and user guide