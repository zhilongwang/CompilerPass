#ifndef __LIB_CALL__
#define __LIB_CALL__
typedef enum fun_type{
	icopy_from_user = 0,
	icopy_to_user,
	ietm_handler, //This function is we insert for every basic block.
	iunknown,
}fun_type;
typedef enum access_type{
	awrite_out = 0,
	aread_in,
	aspread,
	amem2reg,
	aread2reg,
}access_type;
typedef struct call_site{
	unsigned int call_address;//call site address.
	fun_type callee_type; //callee type.
	section_type stype;	//which section this call site belonging to.	
}call_site;

/*This struct discript the the impact of library fuction.
*/
typedef	struct{
	access_type a_type;
	void * w_addr;
	void * r_addr;
	unsigned short length;
}para;
typedef struct{
	fun_type f_type;
	unsigned int call_site;
	unsigned short dataflow_num;
}fun_info;
/*
typedef struct{
	uint32_t call_site;
	fun_type f_type;	
}library_call;
*/
#endif
