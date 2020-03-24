#include <string>
#include <map>
#include <vector>
#include "handler.h"

/* The mapping {name_of_the_target_func => pair of handlers} */
typedef std::map<std::string, HandlerInfo> HandlerMap;
static HandlerMap handler_map;


static void
set_handler_decl_properties(tree hdecl)
{
	DECL_ASSEMBLER_NAME(hdecl);
	TREE_PUBLIC(hdecl) = 1;
	DECL_EXTERNAL(hdecl) = 1;
	DECL_ARTIFICIAL(hdecl) = 1;
}

/* void hook_copy_from_user_pre(void * dest, void * src, unsigned long len)
 *
 */
//static void 
//build_handler_decls_copy_from_user(void)
//{
//	tree fntype;
//	struct HandlerInfo hi;
//	
//	/* Type sequence for the target function. */
//	hi.ts.push_back(ptr_type_node); /* arg1 */
//	hi.ts.push_back(ptr_type_node); /* arg2 */
//	hi.ts.push_back(long_unsigned_type_node); /* arg3 */
//	hi.ts.push_back(ptr_type_node); /* callee address */
//		
//	fntype = build_function_type_list(void_type_node /* return type */,
//		hi.ts[0], hi.ts[1], hi.ts[2], hi.ts[3], NULL_TREE);
//	hi.pre = build_fn_decl("hook_copy_from_user_pre", fntype);
//	set_handler_decl_properties(hi.pre);
//
//	handler_map["copy_from_user"] = hi;
//}
/* void hook_copy_to_user_pre(void * dest, void * src, unsigned long len)
 *
 */
/*static void 
build_handler_decls_copy_to_user(void)
{
	tree fntype;
	struct HandlerInfo hi;
	
	hi.ts.push_back(ptr_type_node); 
	hi.ts.push_back(ptr_type_node); 
	hi.ts.push_back(long_unsigned_type_node); 
	hi.ts.push_back(ptr_type_node);
		
	fntype = build_function_type_list(void_type_node ,
		hi.ts[0], hi.ts[1], hi.ts[2], hi.ts[3], NULL_TREE);
	hi.pre = build_fn_decl("hook_copy_to_user_pre", fntype);
	set_handler_decl_properties(hi.pre);

	handler_map["copy_to_user"] = hi;
}
*/
const HandlerInfo *
get_handlers_by_function_name(const char *name, tree fndecl)
{
	tree fntype = TREE_TYPE(fndecl);
	
	HandlerMap::const_iterator it;
	it = handler_map.find(name);
	if (it == handler_map.end())
		return NULL; 
	
	const HandlerInfo *hi = &it->second;

	return hi;
}

void
build_handler_decls(void)
{
	tree fntype;

	/* DECLs for the handlers of direct calls. */
	//build_handler_decls_copy_from_user();
	//build_handler_decls_copy_to_user();
	
	/* DECLs for the handlers of indirect calls. */
	
}
/* ====================================================================== */
