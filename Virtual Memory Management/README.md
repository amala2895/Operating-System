
Load gcc version 6.2.0: module load gcc-6.2.0

# mmu.cpp 

1. Simulation of Virtual Memory Management (Paging) 
2. FIFO, Clock, Second Chance, NRU, Random and Aging Page replacement algorithms implemented. 
3. The input files will be in format - 
<number of processes>
  #for each process
  <number of vmas>
   #for each vma
   <start page no> <end page no> <write protected> <file mapped>
<instruction> <number>
 
 Instructions examples are - 
 c 0 (c means context switch, 0 is process number)
 r 5 (r means read operation on page 5)
 w 8 (w means write operation on page 8)
 
 The output is Summary of Page Table, Frame Table, Per Process statistics and total statistics.
 The Random File is used to generate random numbers for simulation
  
