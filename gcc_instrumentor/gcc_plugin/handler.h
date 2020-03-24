#ifndef __HANDER__
#define __HANDER__
#include "plugin.h"
#include "bversion.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "line-map.h"
#include "input.h"
#include "tree.h"

#include "tree-inline.h"
#include "version.h"
#include "rtl.h"
#include "tm_p.h"
#include "flags.h"
#include "hard-reg-set.h"
#include "output.h"
#include "except.h"
#include "function.h"
#include "toplev.h"
#include "basic-block.h"
#include "intl.h"
#include "ggc.h"
#include "timevar.h"

#include "params.h"
//#include "pointer-set.h"
#include "debug.h"
#include "target.h"
#include "langhooks.h"
#include "cfgloop.h"
#include "cgraph.h"
#include "opts.h"

#include "tree-pretty-print.h"
#include "gimple-pretty-print.h"
#include "c-tree.h"

#include "varasm.h"
#include "stor-layout.h"
#include "internal-fn.h"
#include "gimple-expr.h"
#include "context.h"
#include "tree-ssa-alias.h"
#include "stringpool.h"
#include "tree-ssanames.h"
#include "print-tree.h"
#include "tree-eh.h"
#include "tree-nested.h"
#include "gimplify.h"
#include "gimple.h"
#include "tree-ssa-operands.h"
#include "tree-phinodes.h"
#include "tree-cfg.h"
#include "gimple-iterator.h"
#include "gimple-ssa.h"
#include "ssa-iterators.h"

#include "vec.h"

/* Function hander patameter type.
 * 
 */
typedef std::vector<tree> TypeSeq;

/* A pair of handlers for the function calls along with the type sequence
 * for the handled function plus the replacement for the function (NULL 
 * means the replacement is not needed). 
 */
struct HandlerInfo {
	tree pre;	//insert before the call sit to get the parameter.
	tree post;	//insert after the call set to get the return address.
	TypeSeq ts;	//parameter type suqence.
	
	HandlerInfo() : pre(NULL_TREE), post(NULL_TREE){}
};



static void
set_handler_decl_properties(tree hdecl);
/* void hook_copy_from_user_pre(void * dest, void * src, unsigned long len)
 *
 */
static void 
build_handler_decls_copy_from_user(void);
/* void hook_copy_to_user_pre(void * dest, void * src, unsigned long len)
 *
 */
static void 
build_handler_decls_copy_to_user(void);

const HandlerInfo *
get_handlers_by_function_name(const char *name, tree fndecl);

void
build_handler_decls(void);
#endif

