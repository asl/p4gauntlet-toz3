#ifndef _TOZ3_CONTEXT_H_
#define _TOZ3_CONTEXT_H_

#include <z3++.h>

#include <cstdio>

#include <map>
#include <utility>
#include <vector>

#include "ir/ir.h"

#include "type_complex.h"

namespace TOZ3_V2 {

using VarMap = std::map<cstring, std::pair<P4Z3Instance *, const IR::Type *>>;

class P4Scope {
 private:
    // maps of local values and types
    std::map<cstring, P4Declaration *> static_decls;
    VarMap var_map;
    std::map<cstring, const IR::Type *> type_map;
    bool is_returned = false;

    std::vector<std::pair<z3::expr, P4Z3Instance *>> return_exprs;
    std::vector<std::pair<z3::expr, VarMap>> return_states;
    std::vector<z3::expr> forward_conds;
    std::vector<std::pair<const IR::Expression *, cstring>> copy_out_args;

 public:
    /****** GETTERS ******/
    std::map<cstring, const IR::Type *> *get_type_map() { return &type_map; }

    /****** STATIC DECLS ******/
    P4Declaration *get_static_decl(cstring name) {
        auto it = static_decls.find(name);
        if (it != static_decls.end()) {
            return it->second;
        }
        BUG("Key %s not found in static declaration map.", name);
    }
    void declare_static_decl(cstring name, P4Declaration *val) {
        static_decls.insert({name, val});
    }
    bool has_static_decl(cstring name) { return static_decls.count(name) > 0; }
    const std::map<cstring, P4Declaration *> *get_decl_map() const {
        return &static_decls;
    }
    /****** VARIABLES ******/
    P4Z3Instance *get_var(cstring name) {
        auto it = var_map.find(name);
        if (it != var_map.end()) {
            return it->second.first;
        }
        BUG("Key %s not found in var map.", name);
    }
    const IR::Type *get_var_type(cstring name) {
        auto it = var_map.find(name);
        if (it != var_map.end()) {
            return it->second.second;
        }
        BUG("Key %s not found in var map.", name);
    }
    void update_var(cstring name, P4Z3Instance *val) {
        var_map.at(name).first = val;
    }
    void declare_var(cstring name, P4Z3Instance *val,
                     const IR::Type *decl_type) {
        var_map.insert({name, {val, decl_type}});
    }
    bool has_var(cstring name) { return var_map.count(name) > 0; }
    const VarMap &get_var_map() const { return var_map; }

    /****** TYPES ******/
    void add_type(cstring type_name, const IR::Type *t) {
        type_map[type_name] = t;
    }

    const IR::Type *get_type(cstring type_name) const {
        auto it = type_map.find(type_name);
        if (it != type_map.end()) {
            return it->second;
        }
        BUG("Key %s not found in scope type map.", type_name);
    }

    bool has_type(cstring name) const { return type_map.count(name) > 0; }

    const IR::Type *resolve_type(const IR::Type *type) {
        const IR::Type *ret_type = type;
        if (auto tn = type->to<IR::Type_Name>()) {
            cstring type_name = tn->path->name.name;
            return get_type(type_name);
        }
        return ret_type;
    }
    /****** RETURN AND EXIT MANAGEMENT ******/
    void set_copy_out_args(
        const std::vector<std::pair<const IR::Expression *, cstring>>
            &input_args) {
        copy_out_args = input_args;
    }
    std::vector<std::pair<const IR::Expression *, cstring>>
    get_copy_out_args() {
        return copy_out_args;
    }
    bool has_returned() { return is_returned; }
    void set_returned(bool return_state) { is_returned = return_state; }

    void push_forward_cond(const z3::expr &forward_cond) {
        return forward_conds.push_back(forward_cond);
    }
    std::vector<z3::expr> get_forward_conds() const { return forward_conds; }
    void pop_forward_cond() { forward_conds.pop_back(); }

    void push_return_expr(const z3::expr &cond, P4Z3Instance *return_expr) {
        return return_exprs.push_back({cond, return_expr});
    }
    std::vector<std::pair<z3::expr, P4Z3Instance *>> get_return_exprs() const {
        return return_exprs;
    }
    void push_return_state(const z3::expr &cond, const VarMap &state) {
        return return_states.push_back({cond, state});
    }
    std::vector<std::pair<z3::expr, VarMap>> get_return_states() const {
        return return_states;
    }
    void clear_return_states() { return_states.clear(); }
    void clear_return_exprs() { return_exprs.clear(); }

    P4Scope clone() {
        auto new_scope = *this;
        for (auto &value_tuple : get_var_map()) {
            auto var_name = value_tuple.first;
            auto member_cpy = value_tuple.second.first->copy();
            new_scope.update_var(var_name, member_cpy);
        }
        return new_scope;
    }
    VarMap clone_vars() const {
        VarMap cloned_map;
        for (auto &value_tuple : get_var_map()) {
            auto var_name = value_tuple.first;
            auto member_cpy = value_tuple.second.first->copy();
            auto member_type = value_tuple.second.second;
            cloned_map.insert({var_name, {member_cpy, member_type}});
        }
        return cloned_map;
    }
};
typedef std::vector<P4Scope> ProgState;

} // namespace TOZ3_V2

inline std::ostream &operator<<(std::ostream &out,
                                const TOZ3_V2::P4Scope &scope) {
    auto var_map = scope.get_var_map();
    for (auto it = var_map.begin(); it != var_map.end(); ++it) {
        const cstring name = it->first;
        auto val = it->second;
        out << name << ": " << *val.first;
        if (std::next(it) != var_map.end()) {
            out << ", ";
        }
    }
    return out;
}

#endif // _TOZ3_CONTEXT_H_
