#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "abys/ir/type.h"

namespace abys::ir {

struct ExprGraph {
  enum class Op : uint8_t {
    kUnknown,
    kConst,
    kInput,
    kLogicalNot,
    kAndReduce,
    kOrReduce,
    kXorReduce,
    kBitwiseNot,
    kAnd,
    kOr,
    kXor,
    kUnaryMinus,
    kAdd,
    kSub,
    kMul,
    // TODO: define backend synthesis constraints for nonconstant division, modulo, and power.
    kDiv,
    kMod,
    kPow,
    kShl,
    kShr,
    kAshr,
    kEq,
    kLt,
    kLe,
    kMux,
    kList,
    kCase,
    kConvert,
    kConcat,
    kGather,
    kSequence,
    kUnpackedAssign,
    kMaskedAssign,
    kReverse,
    kRange,
    kUnpackedRange,
    kUnpackedSelect,
    kBothEdge,
    kCall,
  };

  struct Node {
    Op op = Op::kUnknown;
    SignalWidth width = 0;
    bool sign = false;
    std::vector<ExprId> operands;
  };

  struct Constant {
    ExprId id = kInvalidExprId;
    // TODO: represent constants structurally while preserving arbitrary-width and four-state
    // values.
    std::string value;
  };

  struct Call {
    ExprId id = kInvalidExprId;
    SubrId subr_id = kInvalidSubrId;
    std::string name; // to debug
  };

  struct UnpackedProperties {
    ExprId id = kInvalidExprId;
    ExprId base = kInvalidExprId;
    std::vector<SignalWidth> unpacked_dims;
    SignalWidth width = 0;
    bool sign = false;
  };

  static constexpr ExprId constant_zero = 0;
  static constexpr ExprId constant_one = 1;
  std::vector<Node> nodes = {{Op::kConst, 1, false, {}}, {Op::kConst, 1, false, {}}};
  std::unordered_map<std::string, ExprId> inputs;
  std::vector<Constant> constants = {{constant_zero, "1'b0"}, {constant_one, "1'b1"}};
  std::vector<Call> calls;
  std::vector<UnpackedProperties> unpacked_properties;
};

} // namespace abys::ir
