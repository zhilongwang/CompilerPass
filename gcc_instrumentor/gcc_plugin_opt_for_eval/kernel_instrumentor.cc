#include <iostream>
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree-pass.h"
#include "context.h"
#include "function.h"
#include "tree.h"
#include "tree-ssa-alias.h"
#include "internal-fn.h"
#include "is-a.h"
#include "predict.h"
#include "basic-block.h"
#include "gimple-expr.h"
#include "gimple.h"
#include "gimple-pretty-print.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "tree-ssa-operands.h"
#include "tree-into-ssa.h"
#include <basic-block.h>
#include <coretypes.h>
#include <gimple.h>
#include <tree.h>
#include <tree-pass.h>
#include <cfgloop.h>
#include <iostream>
#include <list>
#include <map>

#define ETM_HANDLER	"etm_handler"
#define THRESHOLD	300
#define MAX_POINT_NON_LOOP	8			
// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;
static struct plugin_info my_gcc_plugin_info = { "1.0", "This is a gcc plugin for kernel module instrumentation" };

/*Used to save the index of bb in the inner most loop*/
typedef std::list<int> BB_LIST;
static BB_LIST bb_of_loop_list;
bool clearList(){
	BB_LIST::iterator it;
	for (it = bb_of_loop_list.begin(); it != bb_of_loop_list.end();)
	{
		
		printf("delete bb:%x\n", *it);
		bb_of_loop_list.erase(it++);
	}
	return true;
}

/*Used to save the inherited ins_num of bb */
//typedef std::map<unsigned int, unsigned int> INT_MAP;
//static INT_MAP bb_insnum_map;


static tree proto;
static tree decl;
static void 
my_start_unit(void * /*gcc_data*/, void * /*user_data*/)
{
	proto = build_function_type_list(
					void_type_node,      /* return type */
					integer_type_node,   /* first arg's type */
					NULL_TREE);
	decl = build_fn_decl(ETM_HANDLER, proto);
}

// create the function call to a `record_assignment' function and insert it
// before the given statement
/*
static void insert_instrumentation_fn(gimple curr_stmt, tree var_id)
{
    // and insert it before the statement that was passed as the first argument
    gimple call = gimple_build_call(decl, 1, var_id);
    gimple_stmt_iterator gsi = gsi_for_stmt(curr_stmt);
    //gsi_insert_before(&gsi, call, GSI_NEW_STMT);
}
*/

static unsigned int
execute_instrument(function *fun){
	basic_block bb;
	gimple_stmt_iterator gsi;
	gimple curr_stmt;
	printf("Process function:%s\n", function_name(fun));
	/*For one function with out loop, only one etm_handler need to be inserted*/
	if(number_of_loops(fun) < 2){
		printf("in non loop function\n");
		int insnum = 0;
		gimple_stmt_iterator first_gsi_of_fun;
		int BB_index[MAX_POINT_NON_LOOP] = {0};
		int num_istm_point = 0; //instrumentation point number
		FOR_EACH_BB_FN(bb, fun){
			for (gsi = gsi_start_bb(bb) ; !gsi_end_p(gsi) ; gsi_next(&gsi)) {
				insnum++;
			}
			if(insnum > THRESHOLD){ // this is use to handle a much long function beyond the capicity of etb
				printf("WARNING: A function without loop has more than %d gimple\n", THRESHOLD);
				BB_index[num_istm_point++] = bb->index;//recorder a new instrumentation point.
				insnum = 0;
			}
		}
		if(num_istm_point > 0){//handle the other instrumentation point.
			printf("WARNING: Instrument call in long plain function\n");
			FOR_EACH_BB_FN(bb, fun){
				int iter = 0;
				for(iter = 0; iter < num_istm_point; iter++){
					if(bb->index == BB_index[iter])	{
						first_gsi_of_fun = gsi_start_bb(bb) ; 
						tree var_id = build_int_cst(NULL_TREE,  THRESHOLD);
						gimple call = gimple_build_call(decl, 1, var_id);
						gsi_insert_before(&first_gsi_of_fun, call, GSI_NEW_STMT);
					}
				}
			}
		}
		FOR_EACH_BB_FN(bb, fun){//handle the first and default instrumentation point.
				printf("WARNING: Function with %d gimples\n", insnum);
				first_gsi_of_fun = gsi_start_bb(bb) ; 
				tree var_id = build_int_cst(NULL_TREE, ( num_istm_point == 0 ? insnum : THRESHOLD ) );
				gimple call = gimple_build_call(decl, 1, var_id);
				gsi_insert_before(&first_gsi_of_fun, call, GSI_NEW_STMT);
				update_ssa(TODO_update_ssa);
				return 0;
		}
	}
	/*For the inner most loop, only a etm_handler need to be insert*/
	struct loop *loop;
	basic_block *bbs;
	unsigned int i;
	FOR_EACH_LOOP (loop, LI_ONLY_INNERMOST){
		bbs = get_loop_body (loop);
		int insnum = 0;
		for (i = 0; i < loop->num_nodes; i++){
			for (gsi = gsi_start_bb(bbs[i]) ; !gsi_end_p(gsi) ; gsi_next(&gsi)) {
				insnum++;
			}
			if(insnum > THRESHOLD){ // this is use to handle a much long function beyond the capicity of etb
				printf("WARNING: A inner most loop has more than %d gimple\n", THRESHOLD);
				insnum = 0;
			}else{
				printf("push bb:%x\n", bbs[i]->index);
				bb_of_loop_list.push_front(bbs[i]->index);
			}

		}
		gimple_stmt_iterator first_gsi_of_loop = gsi_start_bb(bbs[0]) ; 
		tree var_id = build_int_cst(NULL_TREE,  insnum);
		gimple call = gimple_build_call(decl, 1, var_id);
		gsi_insert_before(&first_gsi_of_loop, call, GSI_NEW_STMT);
		free (bbs);
	}
	/*Pre-process, if a basic block only has one gimple, we move to its successor*/

	/*Generaly, insert a etm call for every basic block*/
	FOR_EACH_BB_FN(bb, fun)
	{
		//gimple_bb_info *bb_info = &bb->il.gimple;
		//print_gimple_seq(stderr, bb_info->seq, 0, 0);
		//printf("bb_index:%x\n", bb->index);
		/*Skip the basic block of innter most loop*/
		BB_LIST::iterator it;
		for(it = bb_of_loop_list.begin(); it != bb_of_loop_list.end();it++){	
			if (bb->index == *it){	
				printf("skip bb:%x\n", bb->index);
				continue;
			}
		}
		/*Instrument each basicblock*/
		int insnum = 0;
		for (gsi = gsi_start_bb(bb) ; !gsi_end_p(gsi) ; gsi_next(&gsi)) {
			insnum++;
		}
		if(insnum >2){
			for (gsi = gsi_start_bb(bb) ; !gsi_end_p(gsi) ; gsi_next(&gsi)) {
				curr_stmt = gsi_stmt(gsi);
				//print_gimple_stmt(stderr, curr_stmt, 0, 0);
				if (gimple_code(curr_stmt) == GIMPLE_ASSIGN
						/*&& lhs != NULL_TREE && TREE_CODE(lhs) != SSA_NAME && DECL_P(lhs)*/){ 
					//loc = gimple_location(curr_stmt);
					tree var_id = build_int_cst(NULL_TREE,  insnum);

					gimple call = gimple_build_call(decl, 1, var_id);
					//gimple_set_location(call, loc);
					gsi_insert_before(&gsi, call, GSI_NEW_STMT);
					/*This is solution to the bug. After insert many call to the gimple IR. the buffer for 
					 instrumentation is full, which may result to a buffer overflow,
					 This bug is very difficult to find because it will not appear when the source is not long 
					 enough.
					*/
					break;
				}
			}
		}

	}
	/*Clear bb id of inner most loops*/
	clearList();
	//bb_insnum_map.clear();
	update_ssa(TODO_update_ssa);
	return 0;

}

namespace{
	const pass_data pass_data_instrument =
	{
		GIMPLE_PASS,      /* type */
		"kernel_pass",          /* name */
		OPTGROUP_NONE, /* optinfo_flags */
		TV_NONE,       /* tv_id */
		PROP_gimple_any,      /* properties_required */
		0,             /* properties_provided */
		0,             /* properties_destroyed */
		0,             /* todo_flags_start */
		0,             /* todo_flags_finish */
	};
	class pass_instrument : public gimple_opt_pass
	{
		public:
			pass_instrument(gcc::context *ctxt) : gimple_opt_pass(pass_data_instrument, ctxt)
			{}

			/* opt_pass methods: */
			bool gate () {}
			unsigned int execute (function *fun) { return execute_instrument(fun); }
	}; 
} 


int plugin_init (struct plugin_name_args *plugin_info,
	     struct plugin_gcc_version *version)
{
	// We check the current gcc loading this plugin against the gcc we used to
	// created this plugin
	if (!plugin_default_version_check (version, &gcc_version))
	{
		std::cerr << "This GCC plugin is for version " << GCCPLUGIN_VERSION_MAJOR
			<< "." << GCCPLUGIN_VERSION_MINOR << "\n";
		return 1;
	}

	register_callback(plugin_info->base_name,
			/* event */ PLUGIN_INFO,
			/* callback */ NULL, /* user_data */ &my_gcc_plugin_info);

	struct register_pass_info pass_info;

	pass_info.pass = new pass_instrument(g);
	pass_info.reference_pass_name = "ssa";
	pass_info.ref_pass_instance_number = 1;
	pass_info.pos_op = PASS_POS_INSERT_AFTER;

	/* We need the callback to execute at the start of each compilation
	 * unit to create declarations of the needed functions. The calls
	 * to these functions will be inserted in the code later, during the
	 * GIMPLE pass. */
	register_callback(plugin_info->base_name, PLUGIN_START_UNIT, 
			&my_start_unit, NULL);

	register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);
	return 0;
}










