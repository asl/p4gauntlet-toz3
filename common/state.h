#ifndef _TOZ3_STATE_H_
#define _TOZ3_STATE_H_

#include <sys/types.h>
#include <z3++.h>

#include <alloca.h>
#include <bits/stdint-uintn.h>
#include <cstdio>

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "ir/ir.h"
#include "scope.h"

namespace TOZ3_V2 {

typedef std::vector<P4Scope> ProgState;

class P4State {

 private:
    ProgState scopes;
    P4Scope main_scope;
    z3::context *ctx;
    P4Z3Instance *expr_result;
    Z3Bitvector z3_expr_buffer;
    Z3Int z3_int_buffer;
    std::vector<P4Z3Instance *> allocated_vars;
    std::vector<ProgState *> cloned_states;

    const IR::Type *find_type(cstring type_name, P4Scope **owner_scope);
    P4Z3Instance *find_var(cstring name, P4Scope **owner_scope);

 public:
    std::vector<std::pair<z3::expr, P4Z3Instance *>> return_exprs;

    P4State(z3::context *context)
        : ctx(context), z3_expr_buffer(context->bool_val(false)),
          z3_int_buffer(context->bool_val(false)) {
        main_scope = P4Scope();
    }
    ~P4State() {
        for (auto var : allocated_vars) {
            delete var;
        }
        allocated_vars.clear();
    }

    /****** GETTERS ******/
    ProgState get_state() { return scopes; }
    z3::context *get_z3_ctx() { return ctx; }
    P4Z3Instance *get_expr_result() { return expr_result; }

    /****** ALLOCATIONS ******/
    P4Z3Instance *gen_instance(cstring name, const IR::Type *type,
                               uint64_t id = 0);
    z3::expr gen_z3_expr(cstring name, const IR::Type *type);
    void add_to_allocated_vars(P4Z3Instance *var) {
        allocated_vars.push_back(var);
    }

    /****** SCOPES AND STATES ******/
    void push_scope();
    void pop_scope();
    P4Scope *get_current_scope();
    void merge_state(z3::expr cond, const ProgState *else_state);
    void restore_state(ProgState *set_scopes) { scopes = *set_scopes; }
    ProgState clone_state();
    ProgState fork_state();

    /****** TYPES ******/
    const IR::Type *resolve_type(const IR::Type *type);
    void add_type(cstring type_name, const IR::Type *t);
    const IR::Type *get_type(cstring decl_name);

    /****** VARIABLES ******/
    void update_var(cstring name, P4Z3Instance *var);
    void declare_local_var(cstring name, P4Z3Instance *var);
    P4Z3Instance *get_var(cstring name);
    template <typename T> T *get_var(cstring name) {
        P4Z3Instance *var = get_var(name);
        if (auto cast_var = var->to_mut<T>()) {
            return cast_var;
        } else {
            BUG("Could not cast to type %s.", typeid(T).name());
        }
    }

    /****** DECLARATIONS ******/
    void declare_static_decl(cstring name, const IR::Declaration *decl);
    const P4Declaration *get_static_decl(cstring name);
    // template <typename T> T *get_static_decl(cstring name) {
    //     P4Z3Instance *var = get_static_decl(name);
    //     return var->to_mut<T>();
    // }
    const P4Declaration *find_static_decl(cstring name,
                                            P4Scope **owner_scope);
    /****** EXPRESSION RESULTS ******/
    P4Z3Instance *copy_expr_result() {
        // FIXME: This is weird...
        if (expr_result->is<ControlState>()) {
            return expr_result;
        }
        auto copy = expr_result->copy();
        add_to_allocated_vars(copy);
        return copy;
    }
    void set_expr_result(P4Z3Instance *result) { expr_result = result; }
    void set_expr_result(Z3Result result) {
        if (auto result_expr = boost::get<Z3Bitvector>(&result)) {
            z3_expr_buffer = *result_expr;
            expr_result = &z3_expr_buffer;
        } else if (auto result_expr = boost::get<Z3Int>(&result)) {
            z3_int_buffer = *result_expr;
            expr_result = &z3_int_buffer;
        } else {
            P4C_UNIMPLEMENTED("Storing reference not supported");
        }
    }
};

z3::expr cast(P4State *state, P4Z3Instance *expr, z3::sort dest_type);
z3::expr z3_bv_cast(const z3::expr *expr, z3::sort dest_type);
z3::expr complex_cast(P4State *state, P4Z3Instance *expr,
                      P4Z3Instance *dest_type);
z3::expr merge_z3_expr(z3::expr *cond, z3::expr *then_expr,
                       const P4Z3Instance *else_expr);

P4Z3Instance *cast(P4State *state, P4Z3Instance *expr,
                   const IR::Type *dest_type);

} // namespace TOZ3_V2

#endif // _TOZ3_STATE_H_
