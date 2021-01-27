#ifndef _TOZ3_COMPLEX_TYPE_H_
#define _TOZ3_COMPLEX_TYPE_H_

#include <z3++.h>

#include <map>    // std::map
#include <string> // std::to_string
#include <vector> // std::vector

#include "ir/ir.h"
#include "state.h"

namespace TOZ3_V2 {


class StructInstance : public P4ComplexInstance {
 public:
    const IR::Type_StructLike *p4_type;
    std::map<cstring, P4Z3Type *> members;
    uint64_t member_id;
    uint64_t width;
    StructInstance(P4State *state, const IR::Type_StructLike *type,
                   uint64_t member_id);
    void bind(z3::ast bind_const);
    std::vector<z3::ast> get_z3_vars();
};

class EnumInstance : public P4ComplexInstance {
 private:
 public:
    const IR::Type_Enum *p4_type;
    std::map<cstring, P4Z3Type *> members;
    uint64_t width;
    EnumInstance(P4State *state, const IR::Type_Enum *type);
};

class ErrorInstance : public P4ComplexInstance {
 private:
 public:
    const IR::Type_Error *p4_type;
    std::map<cstring, P4Z3Type *> members;
    uint64_t width;
    ErrorInstance(P4State *state, const IR::Type_Error *type);
};

class ExternInstance : public P4ComplexInstance {
 private:
 public:
    const IR::Type_Extern *p4_type;
    std::map<cstring, P4Z3Type *> members;
    uint64_t width;
    ExternInstance(P4State *state, const IR::Type_Extern *type);
};

} // namespace TOZ3_V2

#endif // _TOZ3_COMPLEX_TYPE_H_