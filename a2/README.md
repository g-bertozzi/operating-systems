This program implements an MLFQ scheduler using a set of queues and simulates CPU execution over time. 
It handles process scheduling, queue transitions, and time slice management.

Input: text file with representation of a simulation case

For example, the following line contained within such a file:
```
13,3,0
```
indicates that at tick 13, task 3 has been created (i.e. the 0 means creation). A line in the file such as:
```
14,3,6
```
indicates that at tick 14, task 3 initiates an action that will require 6 ticks of CPU time. 
There may be many such CPU-tick lines for a task contained in the simulation case.

Lastly, there will appear a line in the file such as:

```
21,3,-1
```

here indicating that at tick 21, task 3 will terminate once its remaining burst’s CPU ticks are scheduled. 
(If no such CPU ticks are remaining, then the task would be said to terminate immediately.)

Output: displays CPU burst times of tasks as determined by MLFQ scheduler
