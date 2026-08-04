#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace abys {

enum class DiagnosticLevel : uint8_t {
  kWarning,
  kError,
};

enum class DiagnosticId : uint16_t {
  kLoweringUnresolvedSignalInput,
  kLoweringLevelSensitiveEventIgnored,
  kLoweringMixedAssignmentTreatedAsNonblocking,
  kLoweringSystemFunctionReplacedWithZero,
  kLoweringSystemCallIgnored,
  kLoweringFullCaseWithDefaultIgnored,
  kLoweringUnconnectedInputPort,
  kLoweringProceduralBlockIgnored,
  kLoweringTaskIgnored,
  kLoweringLargeCompileTimeLoop,
  kLoweringUnsupportedAstNode,
  kLoweringUnsupportedExpressionReplacedWithZero,
  kLoweringUnsupportedStatementIgnored,
  kLoweringUnsupportedTimingControlIgnored,
  kLoweringUnsupportedPortIgnored,
  kLoweringUnsupportedDefinitionIgnored,
  kLoweringUnsupportedInstanceIgnored,
  kLoweringUnsupportedInstanceConnectionIgnored,
  kLoweringUnsupportedSubroutineFormalTreatedAsInput,
  kLoweringUnsupportedAssignmentIgnored,
  kLoweringSubroutineLimitExceeded,
  kLoweringUnsupportedTypeReplacedWithBit,
  kLoweringInvalidFfTreatedAsCombinational,
  kLoweringDuplicateSubroutineIgnored,
  kLoweringUndecidedProcessTreatedAsCombOrLatch,
  kEmitterInvalidEdgeTreatedAsPosedge,
  kEmitterMissingExpressionValueReplacedWithZero,
  kEmitterUnsupportedExpressionReplacedWithZero,
  kFrontendUnhandledSynthesisAttribute,
  kFrontendUnhandledSynthesisComment,
};

std::string_view diagnostic_message(DiagnosticId id);

class Diagnostics {
private:
  struct Entry {
    DiagnosticLevel level = DiagnosticLevel::kWarning;
    DiagnosticId id;
    std::string detail;
  };

public:
  Diagnostics() = default;
  explicit Diagnostics(std::ostream &output);

  void warning(DiagnosticId id, std::string detail = {});
  void error(DiagnosticId id, std::string detail = {});

private:
  static std::string format(const Entry &entry);

  std::ostream *output_ = nullptr;
  std::vector<Entry> entries_;
};

} // namespace abys
