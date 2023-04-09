//Guy Cohen
//206463606
//Virtual Memory Management
#include "sim_mem.h"
#include <cstring>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
using namespace std;
//only read permission ==0, read && write permission =1.
//no frame=-1
//D=0, is clean
//swap_index=-1 , no in swap file
//V=0
char *str1;


int free_space=-1;// first index of free space in memory.
int ind=0;// it will use for the first process that will swap out.

sim_mem:: sim_mem(char exe_file_name1[],char exe_file_name2[], char swap_file_name[], int text_size,
                  int data_size, int bss_size, int heap_stack_size, int num_of_pages, int page_size,
                  int num_of_process){
    this->text_size= text_size;
    this->data_size=data_size;
    this->bss_size=bss_size;
    this->heap_stack_size=heap_stack_size;
    this->num_of_pages=num_of_pages;
    this->page_size=page_size;
    this->num_of_proc=num_of_process;
    this->page_table=(page_descriptor**)malloc(sizeof(page_descriptor*)*(this->num_of_proc));// page table, size of number of process.
    if(this->page_table==NULL){
        perror("\nfailed with malloc of page_table! \n");
        free(this->page_table);
        exit(1);
    }
    this->page_table[0]=(page_descriptor*)malloc(this->num_of_pages*sizeof (page_descriptor));
    if(this->page_table[0]==NULL){
        free(this->page_table);
        perror("\nfailed with malloc of page_table[0]! \n");
        exit(1);
    }
    if (this->page_size==0){
        cout<< "error, page size=0";
        exit(1);
    }
    int only_read=this->text_size/this->page_size;

    for(int i=0; i<num_of_pages ; i++){
        this->page_table[0][i].V=0;
        this->page_table[0][i].D=0;
        this->page_table[0][i].frame=-1;
        this->page_table[0][i].swap_index=-1;
        this->page_table[0][i].P=1;
        if(i<only_read)
            this->page_table[0][i].P=0;
    }

    if((this->program_fd[0]=open(exe_file_name1,O_RDWR, 0))==-1){
        free(this->page_table[0]);
        free(this->page_table);
        perror("\nUnable to open file1 ");
        exit(1);
    }

    if (num_of_proc==2){
        this->page_table[1]=(page_descriptor*)malloc(sizeof (page_descriptor)*(num_of_pages));
        free(this->page_table[0]);
        free(this->page_table);
        if(this->page_table[1]==NULL){
            perror("\nfailed with malloc of page_table[1]! \n");
            exit(1);
        }
        for(int i=0; i< num_of_pages ; i++){
            this->page_table[1][i].V=0;
            this->page_table[1][i].D=0;
            this->page_table[1][i].P=1;
            this->page_table[1][i].frame=-1;
            this->page_table[1][i].swap_index=-1;
            if(i<only_read)
                this->page_table[1][i].P=0;
        }
        if((this->program_fd[1]= open(exe_file_name2, O_RDWR,0))==-1){
            free(this->page_table[1]);
            free(this->page_table[0]);
            free(this->page_table);
            perror("\nUnable to open file2 ");
            exit(1);
        }

    }
    memset(main_memory, '0', MEMORY_SIZE);// main memory is  initializes with zeros.

    if((this->swapfile_fd= open(swap_file_name, O_RDWR|O_CREAT, S_IWUSR |S_IRUSR))==-1){
        perror("\nUnable to open swap_file ");
        if(num_of_process == 2)
            free(this->page_table[1]);
        free(this->page_table[0]);
        free(this->page_table);
        exit(1);
    }

    int text_pages = 0;
    if(text_size <= page_size) {
        text_pages = 1;
    }
    else {
        text_pages = text_size / page_size;
        text_pages += (text_size) % page_size == 0 ? 0 : 1;
    }

    for(int i=0; i< page_size * (num_of_pages - text_pages);i++){
        write(this->swapfile_fd, "0", 1);
    }
}

sim_mem:: ~sim_mem(){
    if(this->num_of_proc==2){
        free(this->page_table[1]);
        close(this->program_fd[1]);

    }
    free(this->page_table[0]);
    free(this->page_table);
    close(this->program_fd[0]);
    close(swapfile_fd);
}


void  sim_mem::store(int process_id, int address, char value){

    int page=address/this->page_size;
    int offset= address%this->page_size;

    if(process_id==1)
        process_id = 0;
    else
        process_id=1;

    if(address>this->page_size*this->num_of_pages){
        fprintf(stderr, "\nwrong address!\n");
        return;
    }
    if(this->page_table[process_id][page].P==0){// if there are only read permission, so it is a text zone.
        printf("\nThere is no permissions for address: [%d], cause it is a text area!\n", address );
        return;
    }

    if(page_table[process_id][page].V==1) {// the page is already in the main memory (RAM).
        main_memory[(this->page_table[process_id][page].frame)*this->page_size+offset]=value;
        this->page_table[process_id][page].D=1;
        return;
    }
    // page_table[process_id][page]: v=0, p=1)
    free_place_in_RAM(main_memory);

    if(free_space==-1) {//there is no place in the RAM
        //we want to find place for our process in the main memory, so we pop out the first process
        // first all we need to find free space in the swap file.
        RAM_is_full(process_id);
    }
    //if we are here, so we know that we have place in RAM
    if(page_table[process_id][page].D==1){// so we will find our page in the swap file.
        lseek(swapfile_fd, this->page_table[process_id][page].swap_index, SEEK_SET); //fd is now in the right place in swap file
        //we want to read page size chars into our RAM.
        free_place_in_RAM(main_memory);
        read(swapfile_fd, &main_memory[free_space], page_size);
        lseek(swapfile_fd, this->page_table[process_id][page].swap_index, SEEK_SET); //fd is now in the right place in swap file
        for(int i=0; i< page_size;i++){
            write(this->swapfile_fd, "0", 1);
        }
        main_memory[free_space+offset]=value;
        this->page_table[process_id][page].V=1;
        this->page_table[process_id][page].swap_index=-1;
        this->page_table[process_id][page].frame=free_space/this->page_size;
        this->page_table[process_id][page].D=0;
        return;
    }
    // if we are here
    if(page_table[process_id][page].D==0){// V=0, D=0, P=1, bss/heap/data.
        free_place_in_RAM(main_memory);
        if(address>=this->text_size && address< this->data_size+this->bss_size){// we are in data area
            // so, I am going to the exec file.
            lseek(this->program_fd[process_id], address-offset, SEEK_SET); //fd is now in the right place
            //we want to read page size chars into our RAM.
            read(this->program_fd[process_id], &main_memory[free_space], page_size);// now sure, we have the right address in the RAM.
            this->page_table[process_id][page].frame=free_space/page_size;
            main_memory[free_space+offset]=value;

        }
        else {
            int k = 0;
            for (int i = free_space; k < page_size; i++, k++) {
                main_memory[i] = '0';
            }
            free_place_in_RAM(main_memory);
            main_memory[free_space + offset] = value;
        }
        this->page_table[process_id][page].frame=free_space/page_size;
        this->page_table[process_id][page].V=1;
        this->page_table[process_id][page].D=1;
        this->page_table[process_id][page].swap_index=-1;
        return;
    }
}

char sim_mem::load(int process_id, int address){

    int page=address/this->page_size;
    int offset= address%this->page_size;

    if(process_id==1)
        process_id = 0;
    else
        process_id=1;

    if(address>this->page_size*this->num_of_pages){
        fprintf(stderr, "\nwrong address!\n");
        return 0;
    }

    if(this->page_table[process_id][page].V==1){// the page is already in the RAM.
        return main_memory[this->page_table[process_id][page].frame*this->page_size+offset];
    }
    // so v=0.
    // now we must know, that we have enough place in our RAM

    free_place_in_RAM(main_memory);
    if(free_space==-1) {//there is no place in the RAM
        //we want to find place for our process in the main memory, so we pop out the first process
        // first all we need to find free space in the swap file.
        RAM_is_full(process_id);

    }
    if(this->page_table[process_id][page].P==0){// if there are no permissions, so it is a text zone
        // so, I am going to the exec file.
        lseek(this->program_fd[process_id], address-offset, SEEK_SET); //fd is now in the right place
        //we want to read page size chars into our RAM.
        read(this->program_fd[process_id], &main_memory[free_space], page_size);// now sure, we have the right address in the RAM.
        this->page_table[process_id][page].frame=free_space/page_size;// or free_space
        this->page_table[process_id][page].V=1;
        return main_memory[free_space+offset];
    }
    // if we are here, so we have permissions ,P=1 V=0 and enough space in the main memory.
    if (this->page_table[process_id][page].D==1){//it is in the swap file. I will find it with swap_index.
        lseek(swapfile_fd, this->page_table[process_id][page].swap_index, SEEK_SET); //fd is now in the right place in swap file
        //we want to read page size chars into our RAM.
        free_place_in_RAM(main_memory);
        read(swapfile_fd, &main_memory[free_space], page_size);
        lseek(swapfile_fd, this->page_table[process_id][page].swap_index, SEEK_SET); //fd is now in the right place in swap file
        for(int i=0; i< page_size;i++){
            write(this->swapfile_fd, "0", 1);
        }

        this->page_table[process_id][page].V=1;
        this->page_table[process_id][page].swap_index=-1;
        this->page_table[process_id][page].frame=free_space/this->page_size;
        this->page_table[process_id][page].D=0;
        return main_memory[free_space+offset];
    }
    //if we are here, so P=1, V=0, D=0 and enough space in the main memory.

    if(this->page_table[process_id][page].D==0){
        if(address>=this->text_size && address<= data_size) {// it will be in the RAM.
            return main_memory[this->page_table[process_id][page].frame+offset];
        }
        else{
            free_place_in_RAM(main_memory);
            int k=0;
            for(int i=free_space;  k<page_size; i++, k++){
                main_memory[i]='0';
            }
        }
    }
    return '0';
}

void sim_mem::free_place_in_RAM(char ram []) {

    free_space=-1;
    for (int i=0 ; i<MEMORY_SIZE ; i++){
        if(main_memory[i]=='0'){
            for(int j=i; j<page_size+i; j++){
                if(main_memory[j]!='0'){
                    break;
                }
                if(main_memory[j]=='0' && j==page_size+i-1){
                    free_space=i;
                    if(free_space%page_size!=0){
                        continue;
                    }
                    return;
                }
            }
        }
    }
}

void sim_mem::RAM_is_full(int proc_id){
    str1 = (char *) malloc(this->page_size-1 * sizeof(char));
    if(str1==NULL){
        perror("malloc failed!!!");
        exit(1);
    }

    char *helper = (char *) malloc(this->page_size * sizeof(char));
    if(helper==NULL){
        free(str1);
        perror("\nmalloc failed! \n");
        exit(1);
    }
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file

    int free_place_in_swap = 0;// where you can storage your page.
    memset(helper, '0', this->page_size);//will help me find place with page size of zeros in the swap file.
    while ((read(swapfile_fd, str1, this->page_size)) == this->page_size) {
        str1[this->page_size]=0;
        if (strcmp(helper, str1) == 0) {// we found a place.
            break;
        }
        free_place_in_swap++;
    }// now I have the index of a free place in swap file

    lseek(swapfile_fd, this->page_size * free_place_in_swap,SEEK_SET);//jumping to the free place by the page size.
    for(int i=0; i<page_size ; i++){
        helper[i]=main_memory[ind];
        main_memory[ind]='0';
        ind++;
    }
    // if our address have permission 0 so it mustn't move to swap
    free_place_in_RAM(main_memory);
    int k=ind-this->page_size;
    for(int i=0; i<num_of_pages; i++ ){
        if(this->page_table[proc_id][i].frame==free_space/this->page_size) {
            if (this->page_table[proc_id][i].P == 0) {
                for(int j=0; j<this->page_size;j++){
                    helper[j]='0';
                    main_memory[k]='0';
                    k++;
                }
            }
        }
    }
    //
    write(swapfile_fd, helper , this->page_size);
    //we want now to change the swap index of the swapped page
    free_place_in_RAM(main_memory);
    for(int i=0; i<num_of_pages; i++){
        if(this->page_table[proc_id][i].frame==free_space/this->page_size){
            this->page_table[proc_id][i].V=0;
            this->page_table[proc_id][i].frame=-1;
            this->page_table[proc_id][i].swap_index=page_size * free_place_in_swap;
            if(this->page_table[proc_id][i].P==0){// it is a text area address.
                this->page_table[proc_id][i].swap_index=-1;
                this->page_table[proc_id][i].D=0;
            }
            else{
                this->page_table[proc_id][i].D=1;

            }
            break;
        }
    }
    if((ind+ this->page_size)>MEMORY_SIZE){
        ind=0;
    }


    free(helper);
    free(str1);


}

void sim_mem::print_memory(){
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", main_memory[i]);
    }
}

void sim_mem::print_swap() {
    char* str =(char*) malloc(this->page_size *sizeof(char));
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(swapfile_fd, str, this->page_size) == this->page_size) {
        for(i = 0; i <page_size ; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
    free(str);
    str=NULL;
}

void sim_mem::print_page_table() {
    int i;
    for (int j = 0; j < num_of_proc; j++) {
        printf("\n page table of process: %d \n", j+1);
        printf("Valid\t Dirty\t Permission \t Frame\t Swap index\n");
        for(i = 0; i < num_of_pages ; i++) {
            printf("[%d]\t[%d]\t[%d]\t[%d]\t[%d]\n",
                   page_table[j][i].V, page_table[j][i].D, page_table[j][i].P,
                   page_table[j][i].frame ,
                   page_table[j][i].swap_index);
        }
    }
}

