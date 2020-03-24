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
#include <basic-block.h>
#include <coretypes.h>
#include <gimple.h>
#include <tree.h>
#include <tree-pass.h>
#include <rtl.h>
#include <stringpool.h>
#include <tree-ssanames.h>
#include "handler.h"
#define ETM_HANDLER "etm_handler"
#define FUNC_HOOKER "func_hooker"
#define EXIT_HANDLER "exit_handler"
// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;
static struct plugin_info my_gcc_plugin_info = { "1.0", "This is a gcc plugin for kernel module instrumentation" };

/* Before GCC starts processing a compilation unit, create the necessary 
 * declarations. 
 */

static void 
my_start_unit(void * /*gcc_data*/, void * /*user_data*/)
{
	build_handler_decls();
}

static tree
make_copy_get_addr(tree var, gimple_seq *seq, location_t loc)
{
	gimple g;
	
	tree copy_var = make_ssa_name(TREE_TYPE(var), NULL);
	g = gimple_build_assign(copy_var, var);
	gimple_set_location(g, loc);
	gimple_seq_add_stmt(seq, g);
	
	tree new_var  = make_ssa_name(ptr_type_node, NULL);
	g = gimple_build_assign(new_var, build_fold_addr_expr(copy_var));
	gimple_set_location(g, loc);
	gimple_seq_add_stmt(seq, g);
	
	return new_var;
}

static void
instrument_function_call(gimple_stmt_iterator *gsi, tree *ls_ptr)
{
	gimple stmt = gsi_stmt(*gsi);
	gimple_seq seq = NULL;
	gimple g;	
	const HandlerInfo* hi = NULL;
	tree ret_tmp;

	tree fntype;
	tree fndecl = gimple_call_fndecl(stmt);

	if (fndecl) { /* Direct call */
		const char *name = 
			IDENTIFIER_POINTER(DECL_NAME(fndecl));
		/* Needed when adding the handlers */
		fntype = TREE_TYPE(fndecl);

		fprintf(stderr, "[DBG] \"%s()\"\n",
				name);

		hi = get_handlers_by_function_name(name, fndecl);
		if (hi == NULL) {
			return;
		}else{
			fprintf(stderr, "[DBG] add handler to \"%s()\"\n",
					name);
		}
	}else{
		return;
	}

	location_t loc = gimple_location(stmt);

	/* Prepare the arguments, convert the values to the
	 * appropriate types if necessary. */
	tree arg_types = TYPE_ARG_TYPES(fntype);
	unsigned int i = 0; 
	vec<tree> args = vNULL;

	for (tree tp = arg_types; tp; tp = TREE_CHAIN(tp), ++i) {
		tree src_type = TREE_VALUE(tp);
		if (types_compatible_p(src_type, void_type_node))
			break;

		tree arg = gimple_call_arg(stmt, i);

		if (!POINTER_TYPE_P(src_type) && 
				!INTEGRAL_TYPE_P(src_type)) 
		{
			tree new_arg = make_copy_get_addr(
					arg, &seq, gimple_location(stmt));
			args.safe_push(new_arg);
			continue;
		}
		args.safe_push(arg);
	}
	/* Pre-handlers for the indirect calls also get the address
	 * of the target function ('callee') as an argument */
/*	if (fndecl) {
		tree callee = gimple_call_fn(stmt);
		if (!callee) {
			fprintf(stderr, 
					"Failed to obtain the address of the function.\n");
			return;
		}
		args.safe_push(callee);
	}
*/
//	args.safe_push(*ls_ptr);
	g = gimple_build_call_vec(hi->pre, args);
	gimple_set_location(g, gimple_location(stmt));
	gimple_seq_add_stmt(&seq, g);
	gsi_insert_seq_before(gsi, seq, GSI_SAME_STMT);

}



static void
instrument_gimple(gimple_stmt_iterator *gsi, tree *ls_ptr)
{
	gimple stmt;
	stmt = gsi_stmt (*gsi);
	
	if (is_gimple_call(stmt)) {
		instrument_function_call(gsi, ls_ptr);
	}
}

// create the function call to a `record_assignment' function and insert it
// before the given statement
static void insert_instrumentation_fn(gimple curr_stmt, tree var_id)
{
    // build function declaration
    tree proto = build_function_type_list(
            void_type_node,      /* return type */
            integer_type_node,   /* first arg's type */
            NULL_TREE);
    tree decl = build_fn_decl(ETM_HANDLER, proto);

    // build the function call with the new value tree and the variable id tree
    // and insert it before the statement that was passed as the first argument
    gimple call = gimple_build_call(decl, 1, var_id);
    gimple_stmt_iterator gsi = gsi_for_stmt(curr_stmt);
    gsi_insert_before(&gsi, call, GSI_NEW_STMT);
}


static unsigned int
execute_instrument(function *fun){
		
	tree ls_ptr; /* Pointer to the local storage struct. */	
	ls_ptr = create_tmp_var(ptr_type_node, "__local_storage_ptr");
	mark_addressable(ls_ptr);

	basic_block bb;
//	std::cerr << function_name(fun) << std::endl;
	if(strcmp(function_name(fun), ETM_HANDLER) == 0){	
		return 0;
	}
	FOR_EACH_BB_FN(bb, fun)
	{
		//	std::cerr << function_name(fun)<<std::endl;
		int insnum = 0;
		gimple_stmt_iterator gsi;
		for (gsi = gsi_start_bb(bb) ; !gsi_end_p(gsi) ; gsi_next(&gsi)) {
			instrument_gimple(&gsi, &ls_ptr);
			insnum++;
		}
		
		gsi = gsi_start_bb(bb);
		gimple curr_stmt = gsi_stmt(gsi);

		tree var_id = build_int_cst(NULL_TREE,  insnum);

		// insert our instrumentation function before the current
		insert_instrumentation_fn(curr_stmt, var_id);
	
	}

	// Nothing special todo
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










