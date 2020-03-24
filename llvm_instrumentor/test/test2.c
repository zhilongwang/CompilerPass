#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
extern void SYS_etb(int count);
int main(){
	SYS_etb(123);
	return 0;
}
