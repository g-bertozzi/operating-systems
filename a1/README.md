1. fetch-info.c

Description: prints stats about OS it is operating on and specific processes

Input: either single integer argument for process number or no argument

If run without arguments, the program prints to the console the following information, in order:

  The model name of the CPU
  
  The number of cores
  
  The version of Linux running in the environment
  
  The total memory available to the system, in kilobytes
  
  The 'uptime' (the amount of time the system has been running, expressed in terms of days, hours, minutes and seconds)

If run with a numerical argument, the program prints to the console information about the corresponding process:

  The name of the process
  
  The console command that started the process (if any)
  
  The number of threads running in the process
  
  The total number of context switches that have occurred during the running of the process

  2. pipe4.c
     
Description:
