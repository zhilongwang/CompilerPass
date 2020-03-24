#include<stdlib.h>
#include<cstdint>
#include <unistd.h>
#include<stdio.h>
#include<sys/syscall.h>
#include<unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include<string.h>
#include <stdint.h>
#include <stdbool.h>
#define threshold 390
//#define threshold 2
#define __NR_retrieve_etb 391
static uint16_t counter = 0;

extern "C"{
void SYS_etm(uint16_t ins_of_bb){
	counter+=ins_of_bb;
	if(counter >= threshold){
		counter	=0;	
		if(!syscall(__NR_retrieve_etb)){
			printf("ERROR: ETM buffer is full!\n");
			exit(0);
		}
	}
}
}
