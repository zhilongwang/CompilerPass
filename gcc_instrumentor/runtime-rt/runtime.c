#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cpu.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/gfp.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/kprobes.h>
#include <linux/ctype.h>
#include <linux/syscore_ops.h>
#include <trace/events/sched.h>
#include <asm/processor.h>
#include <asm/errno.h>
#include <linux/tracepoint.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <asm/processor.h>	
#include <asm/mman.h>
#include <linux/mman.h>
#include <linux/init.h>
#include <linux/thread_info.h>
#include <linux/device.h>
#include <linux/string.h>
#include "runtime.h"
#include "lib_call.h"
//#define __DEBUG
#ifdef __DEBUG
#define ALERT(format,...) printk(KERN_ALERT format,##__VA_ARGS__)
#else
#define ALERT(format,...)
#endif


/************************************************
* Symbolic Map					*
************************************************/

static unsigned long kallsyms_lookup_name_ptr=0xc00e6538;
unsigned long (*ksyms_function)(const char *name) = NULL; 
typedef unsigned long (*ksyms_function_ptr_ty)(const char *name);
bool (* begin_trace_ptr)(void) = NULL;
bool (* end_trace_ptr)(void) = NULL;
bool (* retrieve_ptr)(void) = NULL;
SHM ** poi_addr = NULL;
SHM * buf_poi = NULL;
unsigned int get_sym_addr(char * symname){
		ksyms_function = (ksyms_function_ptr_ty)kallsyms_lookup_name_ptr; 
		unsigned int sym_addr = (void *) ksyms_function(symname);	
		ALERT( "symble %s addr:%x\n", symname, sym_addr);
		return sym_addr;
}
bool add_call(SHM * share_info, unsigned int call_site, fun_type f_type, unsigned short dataflow_num){
	if((share_info)->write_poi >= (share_info)->buff_end ){
		(share_info)->buff_bound = (share_info)->write_poi;
		(share_info)->write_poi = (share_info)->buf;
	}
	fun_info *f_info =(fun_info *)(share_info)->write_poi;
	(share_info)->write_poi += sizeof(fun_info);
	f_info->call_site = call_site;
	f_info->f_type = f_type;
	f_info->dataflow_num = dataflow_num;
	return true;
}
int add_mem2mem(SHM * share_info, access_type a_type, void * w_addr, void * r_addr, unsigned  short length){
	if((share_info)->write_poi >= (share_info)->buff_end ){
		(share_info)->buff_bound = (share_info)->write_poi;
		(share_info)->write_poi = (share_info)->buf;
	}
	para * parameter = (para *)(share_info)->write_poi;
	(share_info)->write_poi += sizeof(para);
	parameter->a_type = a_type;
	parameter->w_addr = w_addr;
	parameter->r_addr = r_addr;
	parameter->length = length;
	return true;
}
int add_read(SHM * share_info, access_type a_type, void * w_addr, unsigned short length){
	if((share_info)->write_poi >= (share_info)->buff_end ){
		(share_info)->buff_bound = (share_info)->write_poi;
		(share_info)->write_poi = (share_info)->buf;
	}
	para * parameter = (para *)(share_info)->write_poi;
	(share_info)->write_poi += sizeof(para);
	parameter->a_type = aread_in;
	parameter->w_addr = w_addr;
	parameter->length = length;
	return true;
}
void hook_copy_from_user_pre(void * dest, void * src, unsigned int len)
{	
	if(buf_poi == NULL){
		poi_addr = (SHM **)get_sym_addr("runtime_buff_addr");
		if(poi_addr == NULL){
			ALERT( "fail to get runtime_buff_address\n");
			return 0;	
		}
		ALERT( "poi_addr:%x\n", poi_addr);
		buf_poi = *(poi_addr);
		ALERT( "buf_poi:%x\n", buf_poi);
	}
	add_call( buf_poi, 0, icopy_from_user, 1);
	add_read( buf_poi, aread_in, dest, len );
	ALERT( "copy_from_user is hooked:\ndest:%x, src:%x, len:%x \n", dest, src, len);
	ALERT( "buf_poi->write_poi:%x\n", buf_poi->write_poi);
	return ;
}

void hook_copy_to_user_pre(void * dest, void * src, unsigned int len)
{
	return ;
}
void init_handler(void){
	ALERT( "Set up etm\n");	
	if(begin_trace_ptr == NULL){
		begin_trace_ptr = get_sym_addr("begin_trace");
		if(begin_trace_ptr == NULL) {
        		ALERT( "unable to locate begin_trace_ptr\n");
        		return 0;
		}
		ALERT( "Successful get the begin_trace_ptr\n");
	}
	(*begin_trace_ptr)();
}

#define threshold 400
static uint32_t counter = 0;
void etm_handler(uint32_t ins_of_bb){
	if(retrieve_ptr == NULL){
		retrieve_ptr = get_sym_addr("retrieve_etb");
		if(retrieve_ptr == NULL) {
        		ALERT( "unable to locate retrieve_ptr\n");
        		return 0;
		}
		ALERT( "Successful get the retrieve_ptr\n");
		init_handler();
	}
	counter+=ins_of_bb;
	if(counter >= threshold){
		counter	=0;
		if(!(*retrieve_ptr)()){
        		printk(KERN_INFO "Buffer is full\n");
		}
	
	}
	return;
}
