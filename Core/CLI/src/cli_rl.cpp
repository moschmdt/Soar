// rl command file.
//
// @author: Jonathan Voigt, voigtjr@gmail.com,
//         Nate Derbinsky, nlderbin@umich.edu
// @date  : 2007
//

#include <map>
#include <vector>

#include "agent.h"
#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"
#include "misc.h"
#include "portability.h"
#include "reinforcement_learning.h"
#include "sml_AgentSML.h"
#include "sml_Names.h"

using namespace cli;
using namespace sml;

inline std::string CLI_DoRL_generate_output(const std::string& name,
                                            char* const& param) {
  const std::string output = name + param;
  delete param;
  return output;
}

inline void CLI_DoRL_print(CommandLineInterface& cli, const bool& RawOutput,
                           std::ostringstream& Result, const std::string& text,
                           const bool& newline = true) {
  if (RawOutput) {
    if (newline) {
      Result << text << '\n';
    } else {
      Result << text;
    }
  } else {
    cli.AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, text);
  }
}

/// Formatted for dot: http://www.graphviz.org/content/dot-language

static void CLI_DoRL_print_trace(std::ostream& os, const RL_Trace& rl_trace) {
  os << "digraph RL_Trace {" << std::endl;

  std::ostringstream label;
  std::queue<std::pair<const RL_Trace*, std::string> > trace_queue;
  trace_queue.push(std::make_pair(&rl_trace, "0"));
  size_t c = 1;

  do {
    const RL_Trace* const current_trace = trace_queue.front().first;
    const std::string prev_label = trace_queue.front().second;

    trace_queue.pop();

    for (std::map<std::vector<std::string>, RL_Trace::Entry>::const_iterator
             tt = current_trace->split.begin(),
             tend = current_trace->split.end();
         tt != tend; ++tt) {
      label << c++;

      os << "  " << label.str();

      //       os << " [label=\"";
      //       bool sfirst = true;
      //       for(std::vector<std::string>::const_iterator ss =
      //       tt->first.begin(), send = tt->first.end(); ss != send; ++ss) {
      //         if(sfirst)
      //           sfirst = false;
      //         else
      //           os << "\\n";
      //
      //         os << *ss;
      //       }
      //       os << "\"];";

      os << std::endl;

      os << "  " << prev_label << " -> " << label.str() << " [label=\"";

      for (std::vector<std::string>::const_iterator ss = tt->first.begin(),
                                                    send = tt->first.end();
           ss != send; ++ss) {
        os << *ss << "\\n";
      }

      if (tt->second.probability == tt->second.probability) {
        os << tt->second.probability;
      } else  //< NaN
      {
        os << '-';
      }

      os << " (" << tt->second.init << ")\"];" << std::endl;

      if (tt->second.next) {
        trace_queue.push(std::make_pair(tt->second.next, label.str()));
      }

      label.str("");
      label.clear();
    }
  } while (!trace_queue.empty());

  os << "}" << std::endl;
}

bool CommandLineInterface::DoRL(const char pOp, const std::string* pAttr,
                                const std::string* pVal) {
  agent* thisAgent = m_pAgentSML->GetSoarAgent();
  if (!pOp) {
    CLI_DoRL_print(*this, m_RawOutput, m_Result, "");

    CLI_DoRL_print(*this, m_RawOutput, m_Result,
                   CLI_DoRL_generate_output(
                       "Soar-RL learning: ",
                       thisAgent->RL->rl_params->learning->get_cstring()));

    CLI_DoRL_print(
        *this, m_RawOutput, m_Result,
        CLI_DoRL_generate_output(
            "temporal-extension: ",
            thisAgent->RL->rl_params->temporal_extension->get_cstring()));

    CLI_DoRL_print(*this, m_RawOutput, m_Result, "");
    CLI_DoRL_print(*this, m_RawOutput, m_Result, "Discount");
    CLI_DoRL_print(*this, m_RawOutput, m_Result, "--------");

    CLI_DoRL_print(*this, m_RawOutput, m_Result,
                   CLI_DoRL_generate_output(
                       "discount-rate: ",
                       thisAgent->RL->rl_params->discount_rate->get_cstring()));

    CLI_DoRL_print(*this, m_RawOutput, m_Result, "");
    CLI_DoRL_print(*this, m_RawOutput, m_Result, "Learning");
    CLI_DoRL_print(*this, m_RawOutput, m_Result, "--------");

    CLI_DoRL_print(
        *this, m_RawOutput, m_Result,
        CLI_DoRL_generate_output(
            "learning-policy: ",
            thisAgent->RL->rl_params->learning_policy->get_cstring()));

    CLI_DoRL_print(*this, m_RawOutput, m_Result,
                   CLI_DoRL_generate_output(
                       "learning-rate: ",
                       thisAgent->RL->rl_params->learning_rate->get_cstring()));

    CLI_DoRL_print(*this, m_RawOutput, m_Result,
                   CLI_DoRL_generate_output(
                       "hrl-discount: ",
                       thisAgent->RL->rl_params->hrl_discount->get_cstring()));

    CLI_DoRL_print(
        *this, m_RawOutput, m_Result,
        CLI_DoRL_generate_output(
            "step-size-parameter: ",
            thisAgent->RL->rl_params->step_size_parameter->get_cstring()));

    CLI_DoRL_print(*this, m_RawOutput, m_Result, "");
    CLI_DoRL_print(*this, m_RawOutput, m_Result, "Eligibility Traces");
    CLI_DoRL_print(*this, m_RawOutput, m_Result, "------------------");

    CLI_DoRL_print(*this, m_RawOutput, m_Result,
                   CLI_DoRL_generate_output(
                       "eligibility-trace-decay-rate: ",
                       thisAgent->RL->rl_params->et_decay_rate->get_cstring()));

    CLI_DoRL_print(*this, m_RawOutput, m_Result,
                   CLI_DoRL_generate_output(
                       "eligibility-trace-tolerance: ",
                       thisAgent->RL->rl_params->et_tolerance->get_cstring()));

    CLI_DoRL_print(*this, m_RawOutput, m_Result, "");
    CLI_DoRL_print(*this, m_RawOutput, m_Result, "Experimental");
    CLI_DoRL_print(*this, m_RawOutput, m_Result, "------------");

    CLI_DoRL_print(*this, m_RawOutput, m_Result,
                   CLI_DoRL_generate_output(
                       "chunk-stop: ",
                       thisAgent->RL->rl_params->chunk_stop->get_cstring()));

    CLI_DoRL_print(*this, m_RawOutput, m_Result,
                   CLI_DoRL_generate_output(
                       "decay-mode: ",
                       thisAgent->RL->rl_params->decay_mode->get_cstring()));

    CLI_DoRL_print(
        *this, m_RawOutput, m_Result,
        CLI_DoRL_generate_output(
            "meta: ", thisAgent->RL->rl_params->meta->get_cstring()));

    CLI_DoRL_print(
        *this, m_RawOutput, m_Result,
        CLI_DoRL_generate_output(
            "meta-learning-rate: ",
            thisAgent->RL->rl_params->meta_learning_rate->get_cstring()));

    CLI_DoRL_print(
        *this, m_RawOutput, m_Result,
        CLI_DoRL_generate_output(
            "update-log-path: ",
            thisAgent->RL->rl_params->update_log_path->get_cstring()));

    CLI_DoRL_print(*this, m_RawOutput, m_Result, "");

    CLI_DoRL_print(
        *this, m_RawOutput, m_Result,
        CLI_DoRL_generate_output(
            "apoptosis: ", thisAgent->RL->rl_params->apoptosis->get_cstring()));

    CLI_DoRL_print(
        *this, m_RawOutput, m_Result,
        CLI_DoRL_generate_output(
            "apoptosis-decay: ",
            thisAgent->RL->rl_params->apoptosis_decay->get_cstring()));

    CLI_DoRL_print(
        *this, m_RawOutput, m_Result,
        CLI_DoRL_generate_output(
            "apoptosis-thresh: ",
            thisAgent->RL->rl_params->apoptosis_thresh->get_cstring()));

    CLI_DoRL_print(*this, m_RawOutput, m_Result, "");

    CLI_DoRL_print(
        *this, m_RawOutput, m_Result,
        CLI_DoRL_generate_output(
            "trace: ", thisAgent->RL->rl_params->trace->get_cstring()));

    CLI_DoRL_print(*this, m_RawOutput, m_Result, "");

    return true;
  } else if (pOp == 'g') {
    if (pVal) {
      return SetError("Unneccessary argument to rl -g");
    }

    soar_module::param* my_param =
        thisAgent->RL->rl_params->get(pAttr->c_str());
    if (!my_param) {
      return SetError("Invalid setting.");
    }

    CLI_DoRL_print(*this, m_RawOutput, m_Result,
                   CLI_DoRL_generate_output("", my_param->get_cstring()),
                   false);

    return true;
  } else if (pOp == 's') {
    soar_module::param* my_param =
        thisAgent->RL->rl_params->get(pAttr->c_str());
    if (!my_param) {
      return SetError("Invalid setting.");
    }

    if (!my_param->validate_string(pVal->c_str())) {
      return SetError("Invalid value for setting.");
    }

    if (!my_param->set_string(pVal->c_str())) {
      return SetError("Failed to set value.");
    }

    return true;
  } else if (pOp == 't') {
    if (pAttr && *pAttr == "clear") {
      if (pVal) {
        const int goal_level = atoi(pVal->c_str());
        if (goal_level < 1) {
          return SetError("Invalid RL goal level for rl -t clear.");
        }

        thisAgent->RL->rl_trace.erase(goal_level);
      } else {
        thisAgent->RL->rl_trace.clear();
      }
    } else if (pAttr && *pAttr == "init") {
      goal_stack_level level = 1;

      if (pVal) {
        const int goal_level = atoi(pVal->c_str());
        if (goal_level < 1) {
          return SetError("Invalid RL goal level for rl -t init.");
        }

        for (Symbol* goal = thisAgent->top_goal; goal;
             goal = goal->id->lower_goal, ++level) {
          if (level == goal_level) {
            goal->id->rl_trace = &thisAgent->RL->rl_trace[level];
            break;
          }
        }
      } else {
        for (Symbol* goal = thisAgent->top_goal; goal;
             goal = goal->id->lower_goal, ++level) {
          goal->id->rl_trace = &thisAgent->RL->rl_trace[level];
        }
      }

      ++thisAgent->RL->rl_init_count;
    } else {
      int goal_level = 1;
      if (pAttr) {
        if (*pAttr == "print") {
          if (pVal) {
            goal_level = atoi(pVal->c_str());
          }
        } else if (pVal) {
          return SetError("Invalid arguments to rl -t.");
        } else {
          goal_level = atoi(pAttr->c_str());
        }

        if (goal_level < 1) {
          return SetError("Invalid RL goal level for rl -t.");
        }
      }

      std::ostringstream oss;

      oss << "# RL Trace, Goal Level " << goal_level << ':' << std::endl;

      std::map<goal_stack_level, RL_Trace>::const_iterator tt =
          thisAgent->RL->rl_trace.find(goal_level);
      if (tt != thisAgent->RL->rl_trace.end()) {
        CLI_DoRL_print_trace(oss, tt->second);
      }

      CLI_DoRL_print(*this, m_RawOutput, m_Result, oss.str().c_str(), false);
    }

    return true;
  } else if (pOp == 'S') {
    if (pVal) {
      return SetError("Unneccessary argument to rl -S");
    }

    if (!pAttr) {
      CLI_DoRL_print(*this, m_RawOutput, m_Result,
                     CLI_DoRL_generate_output(
                         "Error from last update: ",
                         thisAgent->RL->rl_stats->update_error->get_cstring()));

      CLI_DoRL_print(*this, m_RawOutput, m_Result,
                     CLI_DoRL_generate_output(
                         "Total reward in last cycle: ",
                         thisAgent->RL->rl_stats->total_reward->get_cstring()));

      CLI_DoRL_print(
          *this, m_RawOutput, m_Result,
          CLI_DoRL_generate_output(
              "Global reward since init: ",
              thisAgent->RL->rl_stats->global_reward->get_cstring()));
    } else {
      // check attribute name
      soar_module::statistic* my_stat =
          thisAgent->RL->rl_stats->get(pAttr->c_str());
      if (!my_stat) {
        return SetError("Invalid statistic.");
      }

      CLI_DoRL_print(*this, m_RawOutput, m_Result,
                     CLI_DoRL_generate_output("", my_stat->get_cstring()),
                     false);
    }

    return true;
  }

  return SetError("Unknown option.");
}
