#include "abys/infra/diagnostics.h"

#include <ostream>
#include <utility>

namespace abys {

std::string_view diagnostic_message(DiagnosticId id) {
  switch (id) {
  case DiagnosticId::kLoweringUnresolvedSignalInput:
    return "leaving unresolved signal input unconnected";
  case DiagnosticId::kLoweringLevelSensitiveEventIgnored:
    return "ignoring level-sensitive event in edge-sensitive procedural block";
  case DiagnosticId::kLoweringMixedAssignmentTreatedAsNonblocking:
    return "treating mixed blocking/nonblocking assignments as nonblocking";
  case DiagnosticId::kLoweringSystemFunctionReplacedWithZero:
    return "replacing non-synthesizable system function with zero";
  case DiagnosticId::kLoweringSystemCallIgnored:
    return "ignoring non-synthesizable system call";
  case DiagnosticId::kLoweringFullCaseWithDefaultIgnored:
    return "ignoring full_case on case statement with default";
  case DiagnosticId::kLoweringUnconnectedInputPort:
    return "leaving unconnected input port";
  case DiagnosticId::kLoweringProceduralBlockIgnored:
    return "ignoring procedural block";
  case DiagnosticId::kLoweringTaskIgnored:
    return "ignoring task in synthesis lowering";
  case DiagnosticId::kLoweringLargeCompileTimeLoop:
    return "compile-time loop iteration count is large";
  case DiagnosticId::kLoweringUnsupportedAstNode:
    return "unsupported AST node ignored";
  case DiagnosticId::kLoweringUnsupportedExpressionReplacedWithZero:
    return "unsupported expression replaced with zero";
  case DiagnosticId::kLoweringUnsupportedStatementIgnored:
    return "unsupported statement ignored";
  case DiagnosticId::kLoweringUnsupportedTimingControlIgnored:
    return "unsupported timing control ignored";
  case DiagnosticId::kLoweringUnsupportedPortIgnored:
    return "unsupported port ignored";
  case DiagnosticId::kLoweringUnsupportedDefinitionIgnored:
    return "unsupported definition ignored";
  case DiagnosticId::kLoweringUnsupportedInstanceIgnored:
    return "unsupported instance ignored";
  case DiagnosticId::kLoweringUnsupportedInstanceConnectionIgnored:
    return "unsupported instance connection ignored";
  case DiagnosticId::kLoweringUnsupportedSubroutineFormalTreatedAsInput:
    return "unsupported subroutine formal treated as input";
  case DiagnosticId::kLoweringUnsupportedAssignmentIgnored:
    return "unsupported assignment ignored";
  case DiagnosticId::kLoweringSubroutineLimitExceeded:
    return "subroutine limit exceeded";
  case DiagnosticId::kLoweringUnsupportedTypeReplacedWithBit:
    return "unsupported type represented as one unsigned bit";
  case DiagnosticId::kLoweringInvalidFfTreatedAsCombinational:
    return "invalid flip-flop process treated as combinational";
  case DiagnosticId::kLoweringDuplicateSubroutineIgnored:
    return "duplicate subroutine definition ignored";
  case DiagnosticId::kLoweringUndecidedProcessTreatedAsCombOrLatch:
    return "undecided process treated as combinational or latch";
  case DiagnosticId::kEmitterInvalidEdgeTreatedAsPosedge:
    return "invalid sequential edge treated as posedge";
  case DiagnosticId::kEmitterMissingExpressionValueReplacedWithZero:
    return "missing expression value emitted as zero";
  case DiagnosticId::kEmitterUnsupportedExpressionReplacedWithZero:
    return "unsupported expression emitted as zero";
  case DiagnosticId::kFrontendUnhandledSynthesisAttribute:
    return "unhandled synthesis attribute";
  case DiagnosticId::kFrontendUnhandledSynthesisComment:
    return "unhandled synthesis comment";
  }
  return "unknown diagnostic";
}

std::string Diagnostics::format(const Entry &entry) {
  std::string result(diagnostic_message(entry.id));
  if (!entry.detail.empty()) {
    result += ": " + entry.detail;
  }
  return result;
}

Diagnostics::Diagnostics(std::ostream &output) : output_(&output) {}

void Diagnostics::warning(DiagnosticId id, std::string detail) {
  entries_.push_back({DiagnosticLevel::kWarning, id, std::move(detail)});
  if (output_) {
    *output_ << "warning: " << format(entries_.back()) << '\n';
  }
}

void Diagnostics::error(DiagnosticId id, std::string detail) {
  entries_.push_back({DiagnosticLevel::kError, id, std::move(detail)});
  if (output_) {
    *output_ << "error: " << format(entries_.back()) << '\n';
  }
}

} // namespace abys
