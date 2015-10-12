#include "gmock/gmock.h"
#include "spark/error/reporter.h"

namespace spark {
namespace error {

class MockReporter : public spark::error::IndentingReporter {
public:
  MOCK_METHOD3(report, void (
      spark::error::Severity sev,
      spark::source::Location loc,
      spark::collections::StringRef msg));
};

}}
