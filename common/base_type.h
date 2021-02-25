#ifndef _TOZ3_BASE_TYPE_H_
#define _TOZ3_BASE_TYPE_H_

#include <cstdio>
#include <z3++.h>

#include <map>    // std::map
#include <vector> // std::vector

#include "ir/ir.h"
#include "lib/cstring.h"

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
    virtual ~P4ComplexInstance() = default;
};

typedef boost::variant<P4ComplexInstance *, z3::expr> P4Z3Instance;
typedef std::map<cstring, P4Z3Instance> P4Z3Result;

template <typename T> T *to_type(P4Z3Instance *type) {
    if (type->which() == 0) {
        P4ComplexInstance *pi = boost::get<P4ComplexInstance *>(*type);
        return dynamic_cast<T *>(pi);
    } else if (type->which() == 1) {
        return boost::get<T>(type);
    } else {
        BUG("Unsupported  type cast");
    }
}

// template <typename T> bool is_type(P4Z3Instance *type) {
//     if (type->which() == 0) {
//         P4ComplexInstance *pi = boost::get<P4ComplexInstance *>(*type);
//         return dynamic_cast<T *>(pi) != nullptr;
//     } else if (type->which() == 1) {
//         return boost::get<T>(type);
//     }
// }

} // namespace TOZ3_V2

#endif // _TOZ3_BASE_TYPE_H_
