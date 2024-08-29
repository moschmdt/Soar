// sp command file.
//
// @author: Jonathan Voigt, voigtjr@gmail.com
// @date  : 2004
//

#include "agent.h"
#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"
#include "parser.h"
#include "portability.h"
#include "production.h"
#include "rete.h"
#include "sml_AgentSML.h"
#include "symbol.h"

using namespace cli;

bool CommandLineInterface::DoSP(const std::string& productionString) {
  // Load the production
  agent* thisAgent = m_pAgentSML->GetSoarAgent();

  production* p;
  unsigned char rete_addition_result = 0;
  p = parse_production(thisAgent, productionString.c_str(),
                       &rete_addition_result);

  if (!p) {
    // There was an error, but duplicate production is just a warning
    if (rete_addition_result != DUPLICATE_PRODUCTION) {
      return SetError("Production addition failed.");
    }
    // production ignored
    m_NumProductionsIgnored += 1;
  } else {
    if (!m_SourceFileStack.empty()) {
      p->filename = make_memory_block_for_string(
          thisAgent, m_SourceFileStack.top().c_str());
    }

    // production was sourced
    m_NumProductionsSourced += 1;
    if (m_RawOutput) {
      m_Result << '*';
    }
  }
  return true;
}
