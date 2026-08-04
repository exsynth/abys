#include "abys/frontend/api.h"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "abys/frontend/slang_lowering.h"
#include "abys/frontend/slang_pragma.h"
#include "abys/ir/tig.h"

#include "slang/driver/Driver.h"
#include "slang/text/SourceManager.h"

namespace abys {

namespace {

void add_default_translate_off_formats(slang::driver::Driver &driver) {
  for (std::string_view prefix : frontend::kSynthesisPragmaPrefixes) {
    driver.options.translateOffOptions.push_back(std::string(prefix) +
                                                 ",translate_off,translate_on");
  }
}

TigBuildResult build_tig(slang::driver::Driver &driver, std::string_view top,
                         Diagnostics &diagnostics, const NamingOptions &naming) {
  if (!top.empty()) {
    driver.options.topModules.push_back(std::string(top));
  }

  if (!driver.processOptions()) {
    return {false, "failed to process slang options", {}};
  }

  if (!driver.parseAllSources()) {
    return {false, "failed to parse SystemVerilog sources", {}};
  }

  auto compilation = driver.createCompilation();
  if (!compilation) {
    return {false, "failed to create slang compilation", {}};
  }

  driver.reportCompilation(*compilation, true);
  if (driver.diagEngine.getNumErrors() > 0) {
    return {false, "slang reported compilation errors", {}};
  }

  frontend::PragmaMap pragmas = frontend::collect_pragmas(driver, diagnostics);
  ir::Tig design =
      frontend::lower_slang_ast_to_ir(compilation->getRoot(), diagnostics, pragmas, top, naming);

  return {true, "ok", std::move(design)};
}

} // namespace

ParseResult parse_systemverilog(const std::vector<std::string> &files, std::string_view top,
                                Diagnostics &diagnostics) {
  if (files.empty()) {
    return {false, "no input files provided"};
  }

  slang::driver::Driver driver;
  driver.addStandardArgs();
  add_default_translate_off_formats(driver);

  for (const auto &file : files) {
    driver.sourceLoader.addFiles(file);
  }

  if (!top.empty()) {
    driver.options.topModules.push_back(std::string(top));
  }

  if (!driver.processOptions()) {
    return {false, "failed to process slang options"};
  }

  if (!driver.parseAllSources()) {
    return {false, "failed to parse SystemVerilog sources"};
  }

  auto compilation = driver.createCompilation();
  if (!compilation) {
    return {false, "failed to create slang compilation"};
  }

  driver.reportCompilation(*compilation, true);
  if (driver.diagEngine.getNumErrors() > 0) {
    return {false, "slang reported compilation errors"};
  }

  (void)frontend::collect_pragmas(driver, diagnostics);

  return {true, "ok"};
}

TigBuildResult build_tig_from_systemverilog(const std::vector<std::string> &files,
                                            std::string_view top, Diagnostics &diagnostics,
                                            const NamingOptions &naming) {
  if (files.empty()) {
    return {false, "no input files provided", {}};
  }

  slang::driver::Driver driver;
  driver.addStandardArgs();
  add_default_translate_off_formats(driver);

  for (const auto &file : files) {
    driver.sourceLoader.addFiles(file);
  }

  return build_tig(driver, top, diagnostics, naming);
}

TigBuildResult build_tig_from_systemverilog_text(std::string_view source,
                                                 std::string_view source_name, std::string_view top,
                                                 Diagnostics &diagnostics,
                                                 const NamingOptions &naming) {
  if (source.empty()) {
    return {false, "no input source provided", {}};
  }

  slang::driver::Driver driver;
  driver.addStandardArgs();
  add_default_translate_off_formats(driver);
  driver.sourceLoader.addBuffer(driver.sourceManager.assignText(source_name, source));

  return build_tig(driver, top, diagnostics, naming);
}

} // namespace abys
