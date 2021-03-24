#ifndef _TOZ3_SIMPLE_TYPE_H_
#define _TOZ3_SIMPLE_TYPE_H_

#include <cstdio>
#include <z3++.h>

#include <map>     // std::map
#include <string>  // std::to_string
#include <utility> // std::pair
#include <vector>  // std::vector

#include "ir/ir.h"
#include "lib/cstring.h"

#include "type_base.h"

namespace TOZ3_V2 {

// Forward declare state
class P4State;

z3::expr pure_bv_cast(const z3::expr &expr, z3::sort dest_type);

class VoidResult : public P4Z3Instance {
 public:
    VoidResult() {}
    void merge(const z3::expr &, const P4Z3Instance &) override{
        // Merge is a no-op here.
    };
    // TODO: This is a little pointless....
    VoidResult *copy() const override { return new VoidResult(); }

    cstring get_static_type() const override { return "VoidResult"; }
    cstring to_string() const override {
        cstring ret = "VoidResult(";
        ret += ")";
        return ret;
    }
    P4Z3Instance *cast_allocate(const IR::Type *) const override {
        // TODO: This should not be necessary.
        return new VoidResult();
    }
};

class FunctionWrapper : public P4Z3Instance {
 private:
 public:
    Z3P4FunctionCall function_call;
    explicit FunctionWrapper(Z3P4FunctionCall function_call)
        : function_call(function_call) {}
    void merge(const z3::expr &, const P4Z3Instance &) override{
        // Merge is a no-op here.
    };
    // TODO: This is a little pointless....
    FunctionWrapper *copy() const override {
        return new FunctionWrapper(function_call);
    }

    cstring get_static_type() const override { return "FunctionWrapper"; }
    cstring to_string() const override {
        cstring ret = "FunctionWrapper(";
        ret += ")";
        return ret;
    }
    P4Z3Instance *cast_allocate(const IR::Type *) const override {
        // TODO: This should not be necessary.
        return new FunctionWrapper(function_call);
    }
};

class ControlState : public P4Z3Instance {
 public:
    std::vector<std::pair<cstring, z3::expr>> state_vars;
    explicit ControlState(std::vector<std::pair<cstring, z3::expr>> state_vars)
        : state_vars(state_vars) {}
    ControlState() {}
    void merge(const z3::expr &, const P4Z3Instance &) override{
        // Merge is a no-op here.
    };
    // TODO: This is a little pointless....
    ControlState *copy() const override { return new ControlState(state_vars); }

    cstring get_static_type() const override { return "ControlState"; }
    cstring to_string() const override {
        cstring ret = "ControlState(";
        bool first = true;
        for (auto tuple : state_vars) {
            if (!first)
                ret += ", ";
            ret += tuple.first + ": " + tuple.second.to_string().c_str();
            first = false;
        }
        ret += ")";
        return ret;
    }
    P4Z3Instance *cast_allocate(const IR::Type *) const override {
        // TODO: This should not be necessary.
        return new ControlState(state_vars);
    }
};

class Z3Bitvector : public P4Z3Instance {
    const P4State *state;

 public:
    z3::expr val;
    bool is_signed;
    explicit Z3Bitvector(const P4State *state, z3::expr val,
                         bool is_signed = false)
        : state(state), val(val), is_signed(is_signed) {}
    explicit Z3Bitvector(const P4State *state);
    ~Z3Bitvector() {}

    /****** UNARY OPERANDS ******/
    Z3Result operator-() const override;
    Z3Result operator~() const override;
    Z3Result operator!() const override;
    /****** BINARY OPERANDS ******/
    Z3Result operator*(const P4Z3Instance &other) const override;
    Z3Result operator/(const P4Z3Instance &other) const override;
    Z3Result operator%(const P4Z3Instance &other) const override;
    Z3Result operator+(const P4Z3Instance &other) const override;
    Z3Result operatorAddSat(const P4Z3Instance &other) const override;
    Z3Result operator-(const P4Z3Instance &other) const override;
    Z3Result operatorSubSat(const P4Z3Instance &other) const override;
    Z3Result operator>>(const P4Z3Instance &other) const override;
    Z3Result operator<<(const P4Z3Instance &other) const override;
    z3::expr operator==(const P4Z3Instance &other) const override;
    z3::expr operator!=(const P4Z3Instance &other) const override;
    z3::expr operator<(const P4Z3Instance &other) const override;
    z3::expr operator<=(const P4Z3Instance &other) const override;
    z3::expr operator>(const P4Z3Instance &other) const override;
    z3::expr operator>=(const P4Z3Instance &other) const override;
    z3::expr operator&&(const P4Z3Instance &other) const override;
    z3::expr operator||(const P4Z3Instance &other) const override;
    Z3Result operator&(const P4Z3Instance &other) const override;
    Z3Result operator|(const P4Z3Instance &other) const override;
    Z3Result operator^(const P4Z3Instance &other) const override;
    Z3Result concat(const P4Z3Instance &) const override;
    Z3Result cast(z3::sort &) const override;
    Z3Result cast(const IR::Type *) const override;
    P4Z3Instance *cast_allocate(const IR::Type *) const override;
    /****** TERNARY OPERANDS ******/
    Z3Result slice(const P4Z3Instance &, const P4Z3Instance &) const override;

    void merge(const z3::expr &cond, const P4Z3Instance &other) override;
    Z3Bitvector *copy() const override;
    void set_undefined() override {
        auto sort = val.get_sort();
        auto ctx = &sort.ctx();
        val = ctx->constant("undefined", sort);
    }

    cstring get_static_type() const override { return "Z3Bitvector"; }
    cstring to_string() const override {
        cstring ret = "Z3Bitvector(";
        return ret + val.to_string().c_str() + ")";
    }
};

class Z3Int : public P4Z3Instance {
    const P4State *state;

 public:
    z3::expr val;
    explicit Z3Int(const P4State *state, z3::expr val)
        : state(state), val(val) {}
    Z3Int(const P4State *state, int64_t int_val);
    Z3Int(const P4State *state, big_int int_val);
    explicit Z3Int(const P4State *state);

    Z3Result operator-() const override;
    Z3Result operator~() const override;
    Z3Result operator!() const override;
    /****** BINARY OPERANDS ******/
    Z3Result operator*(const P4Z3Instance &other) const override;
    Z3Result operator/(const P4Z3Instance &other) const override;
    Z3Result operator%(const P4Z3Instance &other) const override;
    Z3Result operator+(const P4Z3Instance &other) const override;
    Z3Result operatorAddSat(const P4Z3Instance &other) const override;
    Z3Result operator-(const P4Z3Instance &other) const override;
    Z3Result operatorSubSat(const P4Z3Instance &other) const override;
    Z3Result operator>>(const P4Z3Instance &other) const override;
    Z3Result operator<<(const P4Z3Instance &other) const override;
    z3::expr operator==(const P4Z3Instance &other) const override;
    z3::expr operator!=(const P4Z3Instance &other) const override;
    z3::expr operator<(const P4Z3Instance &other) const override;
    z3::expr operator<=(const P4Z3Instance &other) const override;
    z3::expr operator>(const P4Z3Instance &other) const override;
    z3::expr operator>=(const P4Z3Instance &other) const override;
    z3::expr operator&&(const P4Z3Instance &other) const override;
    z3::expr operator||(const P4Z3Instance &other) const override;
    Z3Result operator&(const P4Z3Instance &other) const override;
    Z3Result operator|(const P4Z3Instance &other) const override;
    Z3Result operator^(const P4Z3Instance &other) const override;
    Z3Result concat(const P4Z3Instance &) const override;
    Z3Result cast(z3::sort &) const override;
    Z3Result cast(const IR::Type *) const override;
    P4Z3Instance *cast_allocate(const IR::Type *) const override;
    /****** TERNARY OPERANDS ******/
    Z3Result slice(const P4Z3Instance &, const P4Z3Instance &) const override;

    void merge(const z3::expr &cond, const P4Z3Instance &other) override;
    Z3Int *copy() const override;
    void set_undefined() override {
        auto sort = val.get_sort();
        auto ctx = &sort.ctx();
        val = ctx->constant("undefined", sort);
    }

    cstring get_static_type() const override { return "Z3Int"; }
    cstring to_string() const override {
        cstring ret = "Z3Int(";
        return ret + val.to_string().c_str() + ")";
    }
};

} // namespace TOZ3_V2

#endif // _TOZ3_SIMPLE_TYPE_H_
