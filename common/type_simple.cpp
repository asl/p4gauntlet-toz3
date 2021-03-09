#include "type_simple.h"

#include <cstdio>
#include <utility>

#include "state.h"

namespace TOZ3_V2 {

z3::expr pure_bv_cast(const z3::expr &expr, z3::sort dest_type) {
    uint64_t expr_size;
    if (expr.is_bv()) {
        expr_size = expr.get_sort().bv_size();
    } else if (expr.is_int()) {
        auto cast_val = z3::int2bv(dest_type.bv_size(), expr).simplify();
        return z3::int2bv(dest_type.bv_size(), expr).simplify();
    } else {
        BUG("Casting %s to a bit vector is not supported.",
            expr.to_string().c_str());
    }
    // At this point we are only dealing with expr bit vectors
    auto dest_size = dest_type.bv_size();

    if (expr_size < dest_size) {
        // The target value is larger, extend with zeros
        return z3::zext(expr, dest_size - expr_size);
    } else if (expr_size > dest_size) {
        // The target value is smaller, truncate everything on the right
        return expr.extract(dest_size - 1, 0);
    } else {
        // Nothing to do just return
        return expr;
    }
}

const z3::expr align_bitvectors(const P4Z3Instance *target,
                                const z3::sort bv_cast, bool align_bv = false,
                                cstring op = "") {
    const z3::expr *cast_expr;
    if (auto target_int = target->to<Z3Int>()) {
        auto cast_val = pure_bv_cast(target_int->val, bv_cast);
        cast_expr = &cast_val;
    } else if (auto target_expr = target->to<Z3Bitvector>()) {
        if (align_bv) {
            auto cast_val = pure_bv_cast(target_expr->val, bv_cast);
            cast_expr = &cast_val;
        } else {
            cast_expr = &target_expr->val;
        }
    } else {
        P4C_UNIMPLEMENTED("%s: Alignment not implemented for %s.", op,
                          target->get_static_type());
    }
    return *cast_expr;
}

/*
============================================================================
Z3Bitvector
============================================================================
*/

/****** UNARY OPERANDS ******/

Z3Result Z3Bitvector::operator-() const { return Z3Bitvector(-val, is_signed); }

Z3Result Z3Bitvector::operator~() const { return Z3Bitvector(~val, is_signed); }

Z3Result Z3Bitvector::operator!() const { return Z3Bitvector(!val, is_signed); }

/****** BINARY OPERANDS ******/

Z3Result Z3Bitvector::operator*(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "*");
    return Z3Bitvector(val * other_expr, is_signed);
}

Z3Result Z3Bitvector::operator/(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "/");
    if (is_signed) {
        return Z3Bitvector(val / other_expr, is_signed);
    } else {
        return Z3Bitvector(z3::udiv(val, other_expr), is_signed);
    }
}

Z3Result Z3Bitvector::operator%(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "%");
    return Z3Bitvector(z3::urem(val, other_expr), is_signed);
}

Z3Result Z3Bitvector::operator+(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "+");
    return Z3Bitvector(val + other_expr, is_signed);
}

Z3Result Z3Bitvector::operatorAddSat(const P4Z3Instance &other) const {
    if (auto other_expr = other.to<Z3Bitvector>()) {
        auto no_overflow = z3::bvadd_no_overflow(val, other_expr->val, false);
        auto no_underflow = z3::bvadd_no_underflow(val, other_expr->val);
        auto sort = val.get_sort();
        big_int max_return = pow((big_int)2, sort.bv_size()) - 1;
        auto ctx = &sort.ctx();
        cstring big_str = Util::toString(max_return, 0, false, 10);
        z3::expr max_val = ctx->bv_val(big_str.c_str(), sort.bv_size());
        return Z3Bitvector(z3::ite(no_underflow && no_overflow,
                                   val + other_expr->val, max_val),
                           is_signed);
    }
    P4C_UNIMPLEMENTED("|+| not implemented for %s.", get_static_type());
}

Z3Result Z3Bitvector::operator-(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "!=");
    return Z3Bitvector(val - other_expr, is_signed);
}

Z3Result Z3Bitvector::operatorSubSat(const P4Z3Instance &other) const {
    if (auto other_expr = other.to<Z3Bitvector>()) {
        auto no_overflow = z3::bvsub_no_overflow(val, other_expr->val);
        auto no_underflow = z3::bvsub_no_underflow(val, other_expr->val, false);
        auto sort = val.get_sort();
        auto ctx = &sort.ctx();
        z3::expr min_val = ctx->bv_val(0, sort.bv_size());
        return Z3Bitvector(z3::ite(no_underflow && no_overflow,
                                   val - other_expr->val, min_val),
                           is_signed);
    }
    P4C_UNIMPLEMENTED("|+| not implemented for %s.", get_static_type());
}

Z3Result Z3Bitvector::operator>>(const P4Z3Instance &other) const {
    const z3::expr *cast_other;
    const z3::expr *cast_this;
    auto this_sort = val.get_sort();
    if (auto target_int = other.to<Z3Int>()) {
        auto cast_val = pure_bv_cast(target_int->val, this_sort);
        cast_other = &cast_val;
        cast_this = &val;
    } else if (auto other_expr = other.to<Z3Bitvector>()) {
        auto other_sort = other_expr->val.get_sort();
        if (other_sort.bv_size() < this_sort.bv_size()) {
            auto cast_val = pure_bv_cast(other_expr->val, this_sort);
            cast_other = &cast_val;
            cast_this = &val;
        } else {
            auto cast_val = pure_bv_cast(val, other_sort);
            cast_this = &cast_val;
            cast_other = &other_expr->val;
        }
    } else {
        P4C_UNIMPLEMENTED(">> not implemented for %s.",
                          other.get_static_type());
    }
    if (is_signed) {
        auto shift_result = z3::ashr(*cast_this, *cast_other);
        return Z3Bitvector(pure_bv_cast(shift_result, this_sort), is_signed);
    } else {
        auto shift_result = z3::lshr(*cast_this, *cast_other);
        return Z3Bitvector(pure_bv_cast(shift_result, this_sort), is_signed);
    }
}

Z3Result Z3Bitvector::operator<<(const P4Z3Instance &other) const {
    const z3::expr *cast_other;
    const z3::expr *cast_this;
    auto this_sort = val.get_sort();
    if (auto target_int = other.to<Z3Int>()) {
        // Produce a zero for ints that are larger than the target width
        // FIXME: Check big int here
        if (target_int->val.get_numeral_int64() > this_sort.bv_size()) {
            auto bv_val = this_sort.ctx().bv_val(0, this_sort.bv_size());
            return Z3Bitvector(bv_val, is_signed);
        }
        auto cast_val = pure_bv_cast(target_int->val, this_sort);
        cast_other = &cast_val;
        cast_this = &val;
    } else if (auto other_expr = other.to<Z3Bitvector>()) {
        auto other_sort = other_expr->val.get_sort();
        if (other_sort.bv_size() < this_sort.bv_size()) {
            auto cast_val = pure_bv_cast(other_expr->val, this_sort);
            cast_other = &cast_val;
            cast_this = &val;
        } else {
            auto cast_val = pure_bv_cast(val, other_sort);
            cast_this = &cast_val;
            cast_other = &other_expr->val;
        }
    } else {
        P4C_UNIMPLEMENTED("<< not implemented for %s.",
                          other.get_static_type());
    }
    auto shift_result = z3::shl(*cast_this, *cast_other);

    return Z3Bitvector(pure_bv_cast(shift_result, this_sort), is_signed);
}

Z3Result Z3Bitvector::operator==(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "==");
    return Z3Bitvector(val == other_expr, is_signed);
}

Z3Result Z3Bitvector::operator!=(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "!=");
    return Z3Bitvector(val != other_expr, is_signed);
}

Z3Result Z3Bitvector::operator<(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "<");
    if (is_signed) {
        return Z3Bitvector(val < other_expr, is_signed);
    } else {
        return Z3Bitvector(z3::ult(val, other_expr), is_signed);
    }
}

Z3Result Z3Bitvector::operator<=(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "<=");
    if (is_signed) {
        return Z3Bitvector(val <= other_expr, is_signed);
    } else {
        return Z3Bitvector(z3::ule(val, other_expr), is_signed);
    }
}

Z3Result Z3Bitvector::operator>(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, ">");
    if (is_signed) {
        return Z3Bitvector(val > other_expr, is_signed);
    } else {
        return Z3Bitvector(z3::ugt(val, other_expr), is_signed);
    }
}

Z3Result Z3Bitvector::operator>=(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, ">=");
    if (is_signed) {
        return Z3Bitvector(val >= other_expr, is_signed);
    } else {
        return Z3Bitvector(z3::uge(val, other_expr), is_signed);
    }
}

Z3Result Z3Bitvector::operator&(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "&");
    return Z3Bitvector(val & other_expr, is_signed);
}

Z3Result Z3Bitvector::operator|(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "|");
    return Z3Bitvector(val | other_expr, is_signed);
}

Z3Result Z3Bitvector::operator^(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "^");
    return Z3Bitvector(val ^ other_expr, is_signed);
}

Z3Result Z3Bitvector::operator&&(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "&&");
    return Z3Bitvector(val && other_expr, is_signed);
}

Z3Result Z3Bitvector::operator||(const P4Z3Instance &other) const {
    const z3::expr other_expr =
        align_bitvectors(&other, val.get_sort(), false, "||");
    return Z3Bitvector(val || other_expr, is_signed);
}

Z3Result Z3Bitvector::concat(const P4Z3Instance &other) const {
    const z3::expr *other_expr;
    if (auto other_val = other.to<Z3Bitvector>()) {
        other_expr = &other_val->val;
    } else {
        P4C_UNIMPLEMENTED("concat not implemented for %s.",
                          other.get_static_type());
    }
    return Z3Bitvector(z3::concat(val, *other_expr), is_signed);
}

Z3Result Z3Bitvector::cast(z3::sort &dest_type) const {
    if (dest_type.is_bv()) {
        return Z3Bitvector(pure_bv_cast(val, dest_type));
    } else if (dest_type.is_bool()) {
        if (val.is_bool()) {
            // nothing to do just return a new object
            return Z3Bitvector(val);
        } else if (val.is_bv()) {
            return Z3Bitvector(pure_bv_cast(val, dest_type));
        }
    }
    P4C_UNIMPLEMENTED("cast_allocate to type %s not implemented for %s.",
                      dest_type.to_string().c_str(), get_static_type());
}
Z3Result Z3Bitvector::cast(const IR::Type *dest_type) const {
    if (auto tb = dest_type->to<IR::Type_Bits>()) {
        auto ctx = &val.get_sort().ctx();
        auto dest_sort = ctx->bv_sort(tb->width_bits());
        return cast(dest_sort);
    } else if (dest_type->is<IR::Type_InfInt>()) {
        // FIXME: Clean this up and add some checks
        auto ctx = &val.get_sort().ctx();
        auto dec_str = val.get_decimal_string(0);
        auto int_expr = ctx->int_val(dec_str.c_str());
        return Z3Int(int_expr);
    } else if (dest_type->is<IR::Type_Boolean>()) {
        auto ctx = &val.get_sort().ctx();
        auto dest_sort = ctx->bool_sort();
        return cast(dest_sort);
    }
    P4C_UNIMPLEMENTED("cast not implemented for %s.", get_static_type());
}
P4Z3Instance *Z3Bitvector::cast_allocate(z3::sort &dest_type) const {
    if (dest_type.is_bv()) {
        return new Z3Bitvector(pure_bv_cast(val, dest_type));
    } else if (dest_type.is_bool()) {
        if (val.is_bool()) {
            // nothing to do
            return new Z3Bitvector(val);
        } else if (val.is_bv()) {
            return new Z3Bitvector(pure_bv_cast(val, dest_type));
        }
    }
    P4C_UNIMPLEMENTED("cast_allocate to type %s not implemented for %s.",
                      dest_type.to_string().c_str(), get_static_type());
}
P4Z3Instance *Z3Bitvector::cast_allocate(const IR::Type *dest_type) const {
    if (auto tb = dest_type->to<IR::Type_Bits>()) {
        auto ctx = &val.get_sort().ctx();
        auto dest_sort = ctx->bv_sort(tb->width_bits());
        return cast_allocate(dest_sort);
    } else if (dest_type->is<IR::Type_InfInt>()) {
        // FIXME: Clean this up and add some checks
        auto ctx = &val.get_sort().ctx();
        auto dec_str = val.get_decimal_string(0);
        auto int_expr = ctx->int_val(dec_str.c_str());
        return new Z3Int(int_expr);
    } else if (dest_type->is<IR::Type_Boolean>()) {
        auto ctx = &val.get_sort().ctx();
        auto dest_sort = ctx->bool_sort();
        return cast_allocate(dest_sort);
    }
    P4C_UNIMPLEMENTED("cast_allocate to type %s not implemented for %s.",
                      dest_type->node_type_name(), get_static_type());
}

/****** TERNARY OPERANDS ******/

Z3Result Z3Bitvector::slice(uint64_t, uint64_t, P4Z3Instance &) const {
    P4C_UNIMPLEMENTED("slice not implemented for %s.", get_static_type());
}

Z3Result Z3Bitvector::mux(const P4Z3Instance &) const {
    P4C_UNIMPLEMENTED("mux not implemented for %s.", get_static_type());
}

Z3Bitvector *Z3Bitvector::copy() const {
    return new Z3Bitvector(val, is_signed);
}

void Z3Bitvector::merge(const z3::expr &cond, const P4Z3Instance &then_expr) {
    if (auto then_expr_var = then_expr.to<Z3Bitvector>()) {
        val = z3::ite(cond, then_expr_var->val, val);
    } else if (auto then_expr_var = then_expr.to<Z3Int>()) {
        z3::expr cast_val = pure_bv_cast(then_expr_var->val, val.get_sort());
        val = z3::ite(cond, cast_val, val);
    } else {
        BUG("Z3 expression merge not supported.");
    }
}

/*
============================================================================
Z3INT
============================================================================
*/

Z3Int::Z3Int(big_int int_val, z3::context *ctx)
    : val(ctx->int_val(Util::toString(int_val, 0, false))) {}

Z3Int::Z3Int(int64_t int_val, z3::context *ctx) : val(ctx->int_val(int_val)) {}

Z3Int *Z3Int::copy() const { return new Z3Int(val); }

void Z3Int::merge(const z3::expr &cond, const P4Z3Instance &then_expr) {
    if (auto then_expr_var = then_expr.to<Z3Int>()) {
        val = z3::ite(cond, then_expr_var->val, val);
    } else if (auto then_expr_var = then_expr.to<Z3Bitvector>()) {
        z3::expr cast_val = pure_bv_cast(val, then_expr_var->val.get_sort());
        val = z3::ite(cond, then_expr_var->val, cast_val);
    } else {
        BUG("Unsupported merge class: %s", &then_expr);
    }
}

Z3Result Z3Int::operator-() const { return Z3Int(-val); }

Z3Result Z3Int::operator~() const {
    P4C_UNIMPLEMENTED("~ not implemented for %s", to_string());
}

Z3Result Z3Int::operator!() const {
    P4C_UNIMPLEMENTED("! not implemented for %s", to_string());
}

/****** BINARY OPERANDS ******/

Z3Result Z3Int::operator*(const P4Z3Instance &other) const {
    if (auto other_int = other.to<Z3Int>()) {
        return Z3Int(val * other_int->val);

    } else if (auto other_val = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_val->val.get_sort());
        return Z3Bitvector(cast_val * other_val->val);
    }
    P4C_UNIMPLEMENTED("* not implemented for %s.", other.get_static_type());
}

Z3Result Z3Int::operator/(const P4Z3Instance &other) const {
    if (auto other_int = other.to<Z3Int>()) {
        return Z3Int(val / other_int->val);

    } else if (auto other_val = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_val->val.get_sort());
        return Z3Bitvector(z3::udiv(cast_val, other_val->val));
    }
    P4C_UNIMPLEMENTED("/ not implemented for %s.", other.get_static_type());
}

Z3Result Z3Int::operator%(const P4Z3Instance &other) const {
    if (auto other_int = other.to<Z3Int>()) {
        return Z3Int(val % other_int->val);

    } else if (auto other_val = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_val->val.get_sort());
        return Z3Bitvector(z3::urem(cast_val, other_val->val));
    }
    P4C_UNIMPLEMENTED("% not implemented for %s.", other.get_static_type());
}

Z3Result Z3Int::operator+(const P4Z3Instance &other) const {
    if (auto other_int = other.to<Z3Int>()) {
        return Z3Int(val + other_int->val);

    } else if (auto other_val = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_val->val.get_sort());
        return Z3Bitvector(cast_val + other_val->val);
    }
    P4C_UNIMPLEMENTED("+ not implemented for %s.", other.get_static_type());
}

Z3Result Z3Int::operatorAddSat(const P4Z3Instance &other) const {
    if (auto other_expr = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_expr->val.get_sort());
        auto no_overflow =
            z3::bvadd_no_overflow(cast_val, other_expr->val, false);
        auto no_underflow = z3::bvadd_no_underflow(cast_val, other_expr->val);
        auto sort = cast_val.get_sort();
        big_int max_return = pow((big_int)2, sort.bv_size()) - 1;
        auto ctx = &sort.ctx();
        cstring big_str = Util::toString(max_return, 0, false, 10);
        z3::expr max_val = ctx->bv_val(big_str.c_str(), sort.bv_size());
        return Z3Bitvector(z3::ite(no_underflow && no_overflow,
                                   cast_val + other_expr->val, max_val));
    }
    P4C_UNIMPLEMENTED("|+| not implemented for %s.", other.get_static_type());
}

Z3Result Z3Int::operator-(const P4Z3Instance &other) const {
    if (auto other_int = other.to<Z3Int>()) {
        return Z3Int(val - other_int->val);

    } else if (auto other_val = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_val->val.get_sort());
        return Z3Bitvector(cast_val - other_val->val);
    }
    P4C_UNIMPLEMENTED("- not implemented for %s.", other.get_static_type());
}

Z3Result Z3Int::operatorSubSat(const P4Z3Instance &) const {
    P4C_UNIMPLEMENTED("|-| not implemented for %s.", get_static_type());
}

Z3Result Z3Int::operator>>(const P4Z3Instance &other) const {
    if (auto other_int = other.to<Z3Int>()) {
        big_int result = val >> other_int->val;
        auto ctx = &val.get_sort().ctx();
        return Z3Int(result, ctx);

    } else if (auto other_val = other.to<Z3Bitvector>()) {
        z3::expr cast_val = pure_bv_cast(val, other_val->val.get_sort());
        return Z3Bitvector(z3::lshr(cast_val, other_val->val));
    }
    P4C_UNIMPLEMENTED(">> not implemented for %s.", other.get_static_type());
}

Z3Result Z3Int::operator<<(const P4Z3Instance &other) const {
    if (auto other_int = other.to<Z3Int>()) {
        big_int result = val << other_int->val;
        auto ctx = &val.get_sort().ctx();
        return Z3Int(result, ctx);

    } else if (auto other_val = other.to<Z3Bitvector>()) {
        z3::expr cast_val = pure_bv_cast(val, other_val->val.get_sort());
        return Z3Bitvector(z3::shl(cast_val, other_val->val));
    }
    P4C_UNIMPLEMENTED("<< not implemented for %s.", other.get_static_type());
}

Z3Result Z3Int::operator==(const P4Z3Instance &other) const {
    const z3::expr *this_expr;
    const z3::expr *other_expr;

    if (auto other_int = other.to<Z3Int>()) {
        this_expr = &val;
        other_expr = &other_int->val;
    } else if (auto other_val = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_val->val.get_sort());
        this_expr = &cast_val;
        other_expr = &other_val->val;
    } else {
        P4C_UNIMPLEMENTED("== not implemented for %s.",
                          other.get_static_type());
    }
    return Z3Bitvector(*this_expr == *other_expr);
}

Z3Result Z3Int::operator!=(const P4Z3Instance &other) const {
    const z3::expr *this_expr;
    const z3::expr *other_expr;

    if (auto other_int = other.to<Z3Int>()) {
        this_expr = &val;
        other_expr = &other_int->val;
    } else if (auto other_val = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_val->val.get_sort());
        this_expr = &cast_val;
        other_expr = &other_val->val;
    } else {
        P4C_UNIMPLEMENTED("!= not implemented for %s.",
                          other.get_static_type());
    }
    return Z3Bitvector(*this_expr != *other_expr);
}

Z3Result Z3Int::operator<(const P4Z3Instance &other) const {
    const z3::expr *this_expr;
    const z3::expr *other_expr;

    if (auto other_int = other.to<Z3Int>()) {
        this_expr = &val;
        other_expr = &other_int->val;
    } else if (auto other_val = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_val->val.get_sort());
        this_expr = &cast_val;
        other_expr = &other_val->val;
    } else {
        P4C_UNIMPLEMENTED("< not implemented for %s.", other.get_static_type());
    }
    return Z3Bitvector(*this_expr < *other_expr);
}

Z3Result Z3Int::operator<=(const P4Z3Instance &other) const {
    const z3::expr *this_expr;
    const z3::expr *other_expr;

    if (auto other_int = other.to<Z3Int>()) {
        this_expr = &val;
        other_expr = &other_int->val;
    } else if (auto other_val = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_val->val.get_sort());
        this_expr = &cast_val;
        other_expr = &other_val->val;
    } else {
        P4C_UNIMPLEMENTED("<= not implemented for %s.",
                          other.get_static_type());
    }
    return Z3Bitvector(*this_expr <= *other_expr);
}

Z3Result Z3Int::operator>(const P4Z3Instance &other) const {
    const z3::expr *this_expr;
    const z3::expr *other_expr;

    if (auto other_int = other.to<Z3Int>()) {
        this_expr = &val;
        other_expr = &other_int->val;
    } else if (auto other_val = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_val->val.get_sort());
        this_expr = &cast_val;
        other_expr = &other_val->val;
    } else {
        P4C_UNIMPLEMENTED("> not implemented for %s.", other.get_static_type());
    }
    return Z3Bitvector(*this_expr > *other_expr);
}

Z3Result Z3Int::operator>=(const P4Z3Instance &other) const {
    const z3::expr *this_expr;
    const z3::expr *other_expr;

    if (auto other_int = other.to<Z3Int>()) {
        this_expr = &val;
        other_expr = &other_int->val;
    } else if (auto other_val = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_val->val.get_sort());
        this_expr = &cast_val;
        other_expr = &other_val->val;
    } else {
        P4C_UNIMPLEMENTED(">= not implemented for %s.",
                          other.get_static_type());
    }
    return Z3Bitvector(*this_expr >= *other_expr);
}

Z3Result Z3Int::operator&(const P4Z3Instance &) const {
    P4C_UNIMPLEMENTED("& not implemented for %s.", get_static_type());
}

Z3Result Z3Int::operator|(const P4Z3Instance &other) const {
    if (auto other_int = other.to<Z3Int>()) {
        // FIXME: Support big_int
        uint64_t result = val | other_int->val;
        auto ctx = &val.get_sort().ctx();
        return Z3Int(result, ctx);

    } else if (auto other_val = other.to<Z3Bitvector>()) {
        auto cast_val = pure_bv_cast(val, other_val->val.get_sort());
        return Z3Bitvector(cast_val | other_val->val);
    }
    P4C_UNIMPLEMENTED("| not implemented for %s.", other.get_static_type());
}

Z3Result Z3Int::operator^(const P4Z3Instance &) const {
    P4C_UNIMPLEMENTED("^ not implemented for %s.", get_static_type());
}

Z3Result Z3Int::operator&&(const P4Z3Instance &) const {
    P4C_UNIMPLEMENTED("&& not implemented for %s.", get_static_type());
}

Z3Result Z3Int::operator||(const P4Z3Instance &) const {
    P4C_UNIMPLEMENTED("|| not implemented for %s.", get_static_type());
}

Z3Result Z3Int::concat(const P4Z3Instance &) const {
    P4C_UNIMPLEMENTED("concat not implemented for %s.", get_static_type());
}
Z3Result Z3Int::cast(z3::sort &dest_type) const {
    if (dest_type.is_bv()) {
        return Z3Bitvector(pure_bv_cast(val, dest_type));
    } else {
    }
    P4C_UNIMPLEMENTED("cast not implemented for %s.", get_static_type());
}
Z3Result Z3Int::cast(const IR::Type *dest_type) const {
    if (auto tb = dest_type->to<IR::Type_Bits>()) {
        auto ctx = &val.get_sort().ctx();
        auto dest_sort = ctx->bv_sort(tb->width_bits());
        return cast(dest_sort);
    } else if (dest_type->is<IR::Type_InfInt>()) {
        // nothing to do, return a copy
        return *this;
    }
    P4C_UNIMPLEMENTED("cast not implemented for %s.", get_static_type());
}
P4Z3Instance *Z3Int::cast_allocate(z3::sort &dest_type) const {
    if (dest_type.is_bv()) {
        return new Z3Bitvector(pure_bv_cast(val, dest_type));
    }
    P4C_UNIMPLEMENTED("cast_allocate not implemented for %s.",
                      get_static_type());
}
P4Z3Instance *Z3Int::cast_allocate(const IR::Type *dest_type) const {
    if (auto tb = dest_type->to<IR::Type_Bits>()) {
        auto ctx = &val.get_sort().ctx();
        auto dest_sort = ctx->bv_sort(tb->width_bits());
        return cast_allocate(dest_sort);
    } else if (dest_type->is<IR::Type_InfInt>()) {
        // nothing to do, return a new allocation
        return new Z3Int(val);
    }
    P4C_UNIMPLEMENTED("cast_allocate not implemented for %s.",
                      get_static_type());
}

/****** TERNARY OPERANDS ******/

Z3Result Z3Int::slice(uint64_t, uint64_t, P4Z3Instance &) const {
    P4C_UNIMPLEMENTED("slice not implemented for %s.", get_static_type());
}

Z3Result Z3Int::mux(const P4Z3Instance &) const {
    P4C_UNIMPLEMENTED("mux not implemented for %s.", get_static_type());
}

} // namespace TOZ3_V2
