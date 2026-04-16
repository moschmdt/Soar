#include <gtest/gtest.h>

#include "sml_Client.h"

namespace {

class OperatorExplanationTest : public ::testing::Test {
protected:
  void SetUp() override {
    kernel = sml::Kernel::CreateKernelInCurrentThread(true,
                                                      sml::Kernel::kUseAnyPort);
    ASSERT_NE(kernel, nullptr);

    agent = kernel->CreateAgent("operator-test");
    ASSERT_NE(agent, nullptr);
  }

  void TearDown() override {
    if (kernel) {
      kernel->Shutdown();
      delete kernel;
    }
  }

  /* Helper: run a command and check there was no CLI-level error */
  void RunOK(const char *cmd) {
    agent->ExecuteCommandLine(cmd);
    EXPECT_FALSE(agent->HadError())
        << "Command '" << cmd
        << "' failed: " << agent->GetLastErrorDescription();
  }

  sml::Kernel *kernel = nullptr;
  sml::Agent *agent = nullptr;
};

/* Test 1: list-tracked with no operators reports the empty case */
TEST_F(OperatorExplanationTest, ListTrackedWhenEmpty) {
  std::string result = agent->ExecuteCommandLine("explain track-operator");
  EXPECT_FALSE(agent->HadError());
  EXPECT_NE(result.find("No operators"), std::string::npos)
      << "Expected 'No operators' in: " << result;
}

/* Test 2: track-operator records the name and reports it */
TEST_F(OperatorExplanationTest, TrackOperatorReportsName) {
  std::string result =
      agent->ExecuteCommandLine("explain track-operator move-north");
  EXPECT_FALSE(agent->HadError());
  EXPECT_NE(result.find("move-north"), std::string::npos)
      << "Expected operator name in output: " << result;
  EXPECT_NE(result.find("tracking"), std::string::npos)
      << "Expected 'tracking' in output: " << result;
}

/* Test 3: after tracking, list-tracked shows the operator name */
TEST_F(OperatorExplanationTest, ListTrackedShowsTrackedName) {
  RunOK("explain track-operator move-north");
  std::string result = agent->ExecuteCommandLine("explain track-operator");
  EXPECT_FALSE(agent->HadError());
  EXPECT_NE(result.find("move-north"), std::string::npos)
      << "Tracked name should appear in list: " << result;
}

/* Test 4: untrack a tracked operator */
TEST_F(OperatorExplanationTest, UntrackTrackedOperator) {
  RunOK("explain track-operator move-north");
  std::string result =
      agent->ExecuteCommandLine("explain untrack-operator move-north");
  EXPECT_FALSE(agent->HadError());
  EXPECT_NE(result.find("move-north"), std::string::npos)
      << "Expected operator name in untrack output: " << result;
}

/* Test 5: untrack an operator that was never tracked */
TEST_F(OperatorExplanationTest, UntrackNonexistentOperator) {
  std::string result =
      agent->ExecuteCommandLine("explain untrack-operator nonexistent");
  EXPECT_FALSE(agent->HadError());
  EXPECT_NE(result.find("not"), std::string::npos)
      << "Expected 'not being tracked' message: " << result;
}

/* Test 6: explain operator with no prior tracking reports the DB is empty */
TEST_F(OperatorExplanationTest, ExplainBeforeTracking) {
  std::string result = agent->ExecuteCommandLine("explain operator move-north");
  EXPECT_FALSE(agent->HadError());
  EXPECT_NE(result.find("No explanation"), std::string::npos)
      << "Expected 'No explanation data' message: " << result;
}

/* Test 7: explain operator after tracking but before running reports no data */
TEST_F(OperatorExplanationTest, ExplainAfterTrackingBeforeRun) {
  RunOK("explain track-operator move-north");
  std::string result = agent->ExecuteCommandLine("explain operator move-north");
  EXPECT_FALSE(agent->HadError());
  EXPECT_NE(result.find("No decisions"), std::string::npos)
      << "Expected 'No decisions recorded' message: " << result;
}

/* Test 8: explain operator --json with no decisions produces valid JSON */
TEST_F(OperatorExplanationTest, JSONOutputEmptyDecisions) {
  RunOK("explain track-operator move-north");
  std::string result =
      agent->ExecuteCommandLine("explain operator move-north --json");
  EXPECT_FALSE(agent->HadError()) << agent->GetLastErrorDescription();
  ASSERT_FALSE(result.empty());
  EXPECT_EQ(result[0], '{') << "JSON must start with '{', got: " << result;
  EXPECT_NE(result.find("\"decisions\""), std::string::npos)
      << "JSON must contain 'decisions' key: " << result;
  EXPECT_NE(result.find("\"error\""), std::string::npos)
      << "JSON must contain 'error' key when no data: " << result;
  /* Last character should be '}' */
  EXPECT_EQ(result.back(), '}') << "JSON must end with '}', got: " << result;
}

/* Test 9: explain operator without --json does NOT produce JSON */
TEST_F(OperatorExplanationTest, TextOutputIsNotJSON) {
  RunOK("explain track-operator move-north");
  std::string result = agent->ExecuteCommandLine("explain operator move-north");
  EXPECT_FALSE(agent->HadError());
  /* Text output should not start with '{' */
  if (!result.empty()) {
    EXPECT_NE(result[0], '{') << "Text output should not be JSON: " << result;
  }
}

/* Test 10: track multiple operators, list shows all */
TEST_F(OperatorExplanationTest, TrackMultipleOperators) {
  RunOK("explain track-operator move-north");
  RunOK("explain track-operator stay-put");
  std::string result = agent->ExecuteCommandLine("explain track-operator");
  EXPECT_FALSE(agent->HadError());
  EXPECT_NE(result.find("move-north"), std::string::npos)
      << "Expected move-north in list: " << result;
  EXPECT_NE(result.find("stay-put"), std::string::npos)
      << "Expected stay-put in list: " << result;
}

/* Test 11: track all operators with command-line argument */
TEST_F(OperatorExplanationTest, TrackAllOperatorsWithFlag) {
  std::string result =
      agent->ExecuteCommandLine("explain track-operator --all");
  EXPECT_FALSE(agent->HadError()) << agent->GetLastErrorDescription();
  EXPECT_NE(result.find("all operator"), std::string::npos)
      << "Expected track-all confirmation: " << result;

  std::string list = agent->ExecuteCommandLine("explain track-operator");
  EXPECT_FALSE(agent->HadError());
  EXPECT_NE(list.find("all operators"), std::string::npos)
      << "Expected list to report track-all mode: " << list;
}

/* Test 12: untrack in all-mode excludes by name */
TEST_F(OperatorExplanationTest, UntrackActsAsExclusionInAllMode) {
  RunOK("explain track-operator --all");

  std::string result =
      agent->ExecuteCommandLine("explain untrack-operator move-north");
  EXPECT_FALSE(agent->HadError());
  EXPECT_NE(result.find("Excluded operator"), std::string::npos)
      << "Expected exclusion message: " << result;

  std::string list = agent->ExecuteCommandLine("explain track-operator");
  EXPECT_FALSE(agent->HadError());
  EXPECT_NE(list.find("move-north"), std::string::npos)
      << "Expected excluded operator in all-mode list: " << list;
}

/* Test 13: track name in all-mode re-includes excluded operator */
TEST_F(OperatorExplanationTest, TrackInAllModeReincludesOperator) {
  RunOK("explain track-operator --all");
  RunOK("explain untrack-operator move-north");
  RunOK("explain track-operator move-north");

  std::string list = agent->ExecuteCommandLine("explain track-operator");
  EXPECT_FALSE(agent->HadError());
  EXPECT_EQ(list.find("move-north"), std::string::npos)
      << "Operator should be removed from exclusion list after re-track: "
      << list;
}

} // namespace
