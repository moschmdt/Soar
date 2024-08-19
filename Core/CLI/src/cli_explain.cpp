/////////////////////////////////////////////////////////////////
// explain command file.
//
// Author: Mazin Assanie
// Date  : 2015
//
/////////////////////////////////////////////////////////////////

#include <string>

#include "agent.h"
#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"
#include "condition.h"
#include "explanation_memory.h"
#include "misc.h"
#include "output_manager.h"
#include "portability.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "sml_Names.h"
#include "sml_Utils.h"

using namespace cli;
using namespace sml;

bool is_explain_id(const std::string* pStringParameter) { return false; }

bool CommandLineInterface::DoExplain(const std::string* pArg1,
                                     const std::string* pArg2) {
  agent* thisAgent = m_pAgentSML->GetSoarAgent();
  std::ostringstream tempStringStream;

  if (!pArg1) {
    thisAgent->explanationMemory->print_explain_summary();
    return true;
  } else {
    /* Single command argument */
    soar_module::param* my_param =
        thisAgent->explanationMemory->settings->get(pArg1->c_str());
    if (!my_param) {
      if (!thisAgent->explanationMemory->explain_chunk(pArg1)) {
        return SetError(
            "Invalid explain sub-command.  Use 'explain ?' to see a list of "
            "valid sub-commands and settings.");
      }
    } else if (my_param == thisAgent->explanationMemory->settings->all) {
      if (!pArg2) {
        thisAgent->outputManager->printa_sf(
            thisAgent,
            "The explainer is%scurrently recording all rules formed.\n",
            thisAgent->explanationMemory->is_all_enabled() ? " " : " not ");

      } else {
        if (!my_param->validate_string(pArg2->c_str()) ||
            (!my_param->set_string(pArg2->c_str()))) {
          return SetError(
              "Invalid argument for explain command. Use 'explain ?' to see a "
              "list of valid sub-commands and settings.");
        }
        if (thisAgent->explanationMemory->settings->all->get_value()) {
          thisAgent->explanationMemory->set_all_enabled(true);
          thisAgent->outputManager->printa_sf(
              thisAgent, "Will monitor all chunks created.\n");
        } else {
          thisAgent->explanationMemory->set_all_enabled(false);
          thisAgent->outputManager->printa_sf(
              thisAgent, "Will only monitor specific chunks.\n");
        }
      }
      return true;
    } else if (my_param ==
               thisAgent->explanationMemory->settings->include_justifications) {
      if (!pArg2) {
        thisAgent->outputManager->printa_sf(
            thisAgent,
            "The explainer is%scurrently recording justifications created.\n",
            thisAgent->explanationMemory->isRecordingJustifications()
                ? " "
                : " not ");

      } else {
        if (!my_param->validate_string(pArg2->c_str()) ||
            (!my_param->set_string(pArg2->c_str()))) {
          return SetError(
              "Invalid argument for explain command. Use 'explain ?' to see a "
              "list of valid sub-commands and settings.");
        }
        if (thisAgent->explanationMemory->settings->include_justifications
                ->get_value()) {
          thisAgent->explanationMemory->set_justifications_enabled(true);
          thisAgent->outputManager->printa_sf(
              thisAgent, "Will record justifications created.\n");
        } else {
          thisAgent->explanationMemory->set_justifications_enabled(false);
          thisAgent->outputManager->printa_sf(
              thisAgent, "Will not record justifications created.\n");
        }
      }
      return true;
    } else if (my_param == thisAgent->explanationMemory->settings
                               ->only_print_chunk_identities) {
      if (!pArg2) {
        thisAgent->outputManager->printa_sf(
            thisAgent,
            "The explainer is currently showing identity analysis %s.\n",
            thisAgent->explanationMemory->settings->only_print_chunk_identities
                    ->get_value()
                ? "only for identities that appear in the chunk"
                : "for all identities involved in problem-solving");

      } else {
        if (!my_param->validate_string(pArg2->c_str()) ||
            (!my_param->set_string(pArg2->c_str()))) {
          return SetError(
              "Invalid argument for explain command. Use 'explain ?' to see a "
              "list of valid sub-commands and settings.");
        }
        if (thisAgent->explanationMemory->settings->only_print_chunk_identities
                ->get_value()) {
          thisAgent->explanationMemory->set_justifications_enabled(true);
          thisAgent->outputManager->printa_sf(
              thisAgent,
              "Will only print identities that appear in the chunk.\n");
        } else {
          thisAgent->explanationMemory->set_justifications_enabled(false);
          thisAgent->outputManager->printa_sf(
              thisAgent,
              "Will print all identities involved in problem-solving.\n");
        }
      }
      return true;
    } else if (my_param ==
               thisAgent->explanationMemory->settings->list_chunks) {
      thisAgent->explanationMemory->list_rules_of_type(true);
      return true;
    } else if (my_param ==
               thisAgent->explanationMemory->settings->list_justifications) {
      thisAgent->explanationMemory->list_rules_of_type(false);
      return true;
    } else if (my_param ==
               thisAgent->explanationMemory->settings->record_chunk) {
      if (!pArg2) {
        thisAgent->explanationMemory->list_watched_rules();
        return true;
      } else {
        return thisAgent->explanationMemory->watch_rule(pArg2);
      }
    } else if (my_param ==
               thisAgent->explanationMemory->settings->explain_chunk) {
      if (!pArg2) {
        return SetError(
            "Please specify the chunk using either the chunk's name or its "
            "ID.");
      } else {
        return thisAgent->explanationMemory->explain_chunk(pArg2);
      }
    } else if (my_param ==
               thisAgent->explanationMemory->settings->explain_instantiation) {
      if (!thisAgent->explanationMemory->current_discussed_chunk_exists()) {
        return SetError(
            "Please first specify the chunk you want to discuss with the "
            "command 'explain chunk [chunk-name | chunk ID]'.");
      }
      if (!pArg2) {
        return SetError(
            "Please specify the instantiation id that you'd like explained.");
      } else {
        return thisAgent->explanationMemory->explain_instantiation(pArg2);
      }
    } else if (my_param ==
               thisAgent->explanationMemory->settings->explain_contributors) {
      if (!thisAgent->explanationMemory->current_discussed_chunk_exists()) {
        return SetError(
            "Please first specify the chunk you want to discuss with the "
            "command 'explain chunk [chunk-name | chunk ID]'.");
      }
    } else if (my_param ==
               thisAgent->explanationMemory->settings->explanation_trace) {
      if (!thisAgent->explanationMemory->current_discussed_chunk_exists()) {
        return SetError(
            "Please first specify the chunk you want to discuss with the "
            "command 'explain chunk [chunk-name | chunk ID]'.");
      }
      thisAgent->explanationMemory->switch_to_explanation_trace(true);
      return true;
    } else if (my_param == thisAgent->explanationMemory->settings->wm_trace) {
      if (!thisAgent->explanationMemory->current_discussed_chunk_exists()) {
        return SetError(
            "Please first specify the chunk you want to discuss with the "
            "command 'explain chunk [chunk-name | chunk ID]'.");
      }
      thisAgent->explanationMemory->switch_to_explanation_trace(false);
      return true;
    } else if (my_param == thisAgent->explanationMemory->settings->formation) {
      if (!thisAgent->explanationMemory->current_discussed_chunk_exists()) {
        return SetError(
            "Please first specify the chunk you want to discuss with the "
            "command 'explain chunk [chunk-name | chunk ID]'.");
      }
      thisAgent->explanationMemory->print_formation_explanation();
      return true;
    } else if (my_param ==
               thisAgent->explanationMemory->settings->constraint_analysis) {
      if (!thisAgent->explanationMemory->current_discussed_chunk_exists()) {
        return SetError(
            "Please first specify the chunk you want to discuss with the "
            "command 'explain chunk [chunk-name | chunk ID]'.");
      }
      thisAgent->explanationMemory->print_constraints_enforced();
      return true;
    } else if (my_param ==
               thisAgent->explanationMemory->settings->identity_analysis) {
      if (!thisAgent->explanationMemory->current_discussed_chunk_exists()) {
        return SetError(
            "Please first specify the chunk you want to discuss with the "
            "command 'explain chunk [chunk-name | chunk ID]'.");
      }
      thisAgent->explanationMemory->print_identity_set_explanation();
      return true;

    } else if (my_param == thisAgent->explanationMemory->settings->stats) {
      if (!thisAgent->explanationMemory->current_discussed_chunk_exists()) {
        return SetError(
            "Please first specify the chunk you want to discuss with the "
            "command 'explain chunk [chunk-name | chunk ID]'.");
      }
      thisAgent->explanationMemory->print_chunk_stats(
          thisAgent->explanationMemory->get_current_discussed_chunk());
      return true;

    } else if ((my_param == thisAgent->explanationMemory->settings->help_cmd) ||
               (my_param ==
                thisAgent->explanationMemory->settings->qhelp_cmd)) {
      thisAgent->explanationMemory->settings->print_explanation_settings(
          thisAgent);
      return true;
    } else {
      /* Done with dummy command parameters.  Remaining parameters are all
       * settings*/
      if (!pArg2) {
        tempStringStream << my_param->get_name() << " is";
        PrintCLIMessage_Item(tempStringStream.str().c_str(), my_param, 0);
        return true;
      } else {
        if (!my_param->validate_string(pArg2->c_str())) {
          return SetError(
              "Invalid argument for explain command. Use 'explain ?' to see a "
              "list of valid sub-commands and settings.");
        }

        bool result = my_param->set_string(pArg2->c_str());

        if (!result) {
          return SetError("The explain parameter could not be changed.");
        } else {
          tempStringStream << my_param->get_name() << " is now "
                           << pArg2->c_str();
          PrintCLIMessage(&tempStringStream);
        }
        return result;
      }
    }
  }
  return false;
}
