# Scheduler

Simulation of scheduling using Discrete Event Simulation. Following scheduling algorithms have been included - 
1. FCFS
2. LCFS
3. SJF
4. Round Robin
5. Priority 

The input is of the form <arrival time> <total cpu time> <cpu burst> <io burst>
  
To run the program 
1. make
2. main [-s<schedspec>] inputfile randfile
  
  <schedspec> can be F, S, L, R<quantum>, P<quantum>
