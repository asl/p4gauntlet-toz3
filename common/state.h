#ifndef _TOZ3_STATE_H_
#define _TOZ3_STATE_H_

#include <z3++.h>

#include <map>
#include <utility>
#include <vector>

#include "ir/ir-generated.h"
#include "ir/ir.h"
#include "scope.h"

namespace TOZ3_V2 {

class P4State {
 public:
    z3::context *ctx;
    P4State(z3::context *context) : ctx(context) {
        P4Scope *static_scope = new P4Scope();
        add_scope(static_scope);
    }

    z3::expr formula = ctx->bool_val(true);
    std::map<cstring, const IR::Type *> type_map;
    std::map<cstring, const IR::Declaration *> decl_map;

    std::vector<P4Scope *> scopes;
    P4Z3Instance return_expr = nullptr;

    P4Z3Instance gen_instance(cstring name, const IR::Type *typ,
                              uint64_t id = 0);
    void add_scope(P4Scope *scope);

    const IR::Type *resolve_type(const IR::Type *type);
    void add_type(cstring type_name, const IR::Type *t);
    const IR::Type *get_type(cstring decl_name);

    void add_decl(cstring decl_name, const IR::Declaration *d);
    const IR::Declaration *get_decl(cstring decl_name);

    void insert_var(cstring name, P4Z3Instance var);
    void set_var(const IR::Expression *target, P4Z3Instance var);
    P4Z3Instance find_var(cstring name, P4Scope **owner_scope);
    P4Z3Instance get_var(cstring name);
    void resolve_expr(const IR::Expression *expr);
};

class ControlState : public P4ComplexInstance {
 public:
    std::vector<std::pair<cstring, z3::expr>> state_vars;
    ControlState(std::vector<std::pair<cstring, z3::expr>> state_vars)
        : state_vars(state_vars){};
};

} // namespace TOZ3_V2

#endif // _TOZ3_STATE_H_