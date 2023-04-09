# VirtualMemoryManagement

Authored by Guy Cohen

==Description==

This exercise is a simulation of CPU approaches to memory. It uses the paging mechanism that allows programs to be run when only part of it is in memory.
The program memory is also called virtual memory which is divided into pages which are loaded into memory as needed. This exercise realizes a virtual memory of a computer with up to two programs.
The main function of the exercise consists of a sequence of load and store (random) commands, these functions simulate the reading / writing of the processor.

==Program DATABASE==
1. struct page_descriptor **page_table-->pointer to page table. The page table is implemented as an array of page_taple type size of num_of_pages.
                                         this struct has some useful parameters that will help to recognise kinds of addresses.
2. main_memory--> designed to simulate the ram. This is an array of size 200. 
3. 3 files--> swapfile in case of RAM is full . In addition,2 others files that will be given by the user, they will simulate the programs.

==functions==
1. main--> As is well known, CPU operations consist of read, write, and compute / process operations.
           In this exercise the main function will concentrate only on reading and writing crying to simulate the memory operations on the computer.
2. char load(int process_id, int address)--> Receives a logical address with which the function will access to read data. The function ensures that the relevant page of the     requested process is in the main memory.For each address that the function receives, a conversion will be made from a virtual address to a physical address. (A virtual address consists of the page + offset number).The page table will then return for the page what the appropriate frame is.
If the page is already in the main memory, the function will check in the specific page table of the process (so we can access the appropriate frame in the main memory and advance it according to the offset for reading or writing).
If the page is not in the main memory, the function will get the page from the appropriate place.For each call to this function a number of questions are asked: Does the address we received have read or write permissions (P)? , Have any changes been made to page (D)? valid? What is the frame in the main memory?
Based on the answers to these questions the function can make sure that the relevant page (if possible) appears in the main memory.

3. void store(int process_id, int address, char value)-->This function is almost identical to the load function.
But its purpose is different.
As mentioned, the load function ensures that the page that matches the address we received will be in the main memory. It will then access the physical address in the main memory and return the character found at that address.
In contrast, the store function works similarly. But its purpose is slightly different. It gets some value, after the page has been copied to the appropriate place in the main memory, and then it stores that value there.

4.  void RAM_is_full(int proc_id)--> Its function is to check whether there is a free space in the main memory. Its function is to check whether there is a free space in the main memory.

5. void free_place_in_RAM(char ram [])--> The function of this function is to empty an old address from the main memory to the swap file. The order of the addresses is maintained thanks to the "ind" variable which is updated throughout the run of the program.

==Program Files==
sim_mem.h , sim_mem.cpp, main.cpp

==How to compile?==
compile: g++ sim_mem.cpp main.cpp -o main 
run: ./main

==Output:==
The main memory contents, the contents of the swap file and the page table of the process. 





