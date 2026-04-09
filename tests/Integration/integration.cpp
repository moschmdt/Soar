#include "sml_Client.h"

#include <cassert>
#include <cstdio>

int main() {
  sml::Kernel *kernel = sml::Kernel::CreateKernelInNewThread();
  assert(kernel != nullptr);
  assert(!kernel->HadError());

  sml::Agent *agent = kernel->CreateAgent("test-agent");
  assert(agent != nullptr);
  assert(!kernel->HadError());

  kernel->DestroyAgent(agent);
  kernel->Shutdown();
  delete kernel;

  std::puts("integration test passed");
  return 0;
}
