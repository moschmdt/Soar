#include <gtest/gtest.h>

#include "sml_Client.h"

namespace {
TEST(IdentifierFindByAttributeTests, LogsSpdlogErrorWhenAttributeMissing) {
  sml::Kernel *kernel =
      sml::Kernel::CreateKernelInCurrentThread(true, sml::Kernel::kUseAnyPort);
  ASSERT_NE(kernel, nullptr);

  sml::Agent *agent = kernel->CreateAgent("gtest-identifier");
  ASSERT_NE(agent, nullptr);

  sml::Identifier *input_link = agent->GetInputLink();
  ASSERT_NE(input_link, nullptr);

  sml::WMElement *result = input_link->FindByAttribute("does-not-exist", 0);
  EXPECT_EQ(result, nullptr);

  kernel->Shutdown();
  delete kernel;
}

} // namespace
