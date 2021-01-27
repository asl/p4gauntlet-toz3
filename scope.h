#ifndef _TOZ3_CONTEXT_H_
#define _TOZ3_CONTEXT_H_

#include <z3++.h>

#include <map>
#include <vector>

#include "boost/any.hpp"
#include "ir/ir.h"

#define BOOST_VARIANT_USE_RELAXED_GET_BY_DEFAULT
#include <boost/variant.hpp>
#include <boost/variant/get.hpp>

namespace TOZ3_V2 {

class P4ComplexInstance {
 public:
    P4ComplexInstance() {}
    template <typename T> bool is() const { return to<T>() != nullptr; }
    template <typename T> const T *to() const {
        return dynamic_cast<const T *>(this);
    }
    template <typename T> const T &as() const {
        return dynamic_cast<const T &>(*this);
    }
};

typedef boost::variant<z3::ast, P4ComplexInstance> P4Z3Type;

class P4Scope {
 public:
    // a map of local values
    std::map<cstring, P4Z3Type *> value_map;
};
} // namespace TOZ3_V2

#endif // _TOZ3_CONTEXT_H_