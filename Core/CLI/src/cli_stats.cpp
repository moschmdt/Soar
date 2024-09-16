// stats command file.
//
// @author: Jonathan Voigt, voigtjr@gmail.com
// @date  : 2004
//

#include <iomanip>

#include "agent.h"
#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"
#include "explanation_memory.h"
#include "portability.h"
#include "print.h"
#include "rete.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "sml_Names.h"
#include "soar_db.h"
#include "stats.h"

extern const char* bnode_type_names[256];

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoStats(const StatsBitset& options, int sort) {
  // soar_print_detailed_callback_stats();
  agent* thisAgent = m_pAgentSML->GetSoarAgent();

  if (options.test(STATS_AGENT)) {
    GetAgentStats();
    return true;
  }

  if (options.test(STATS_EBC)) {
    thisAgent->explanationMemory->print_EBC_stats();
    return true;
  }

  if (options.test(STATS_DECISION)) {
    m_Result << thisAgent->decide_phases_count;
    return true;
  }

  if (options.test(STATS_CSV)) {
    if (thisAgent->stats_db->get_status() == soar_module::disconnected) {
      return true;  // FIXME: warn
    }

    m_Result << "dc,time,wm_changes,firing_count\n";
    while (thisAgent->stats_stmts->sel_dc_inc->execute() == soar_module::row) {
      m_Result << thisAgent->stats_stmts->sel_dc_inc->column_int(0) << ",";
      m_Result << thisAgent->stats_stmts->sel_dc_inc->column_int(1) << ",";
      m_Result << thisAgent->stats_stmts->sel_dc_inc->column_int(2) << ",";
      m_Result << thisAgent->stats_stmts->sel_dc_inc->column_int(3) << "\n";
    }

    thisAgent->stats_stmts->sel_dc_inc->reinitialize();
    return true;
  }

  // Set precision now, RESET BEFORE RETURN
  size_t oldPrecision = static_cast<size_t>(m_Result.precision(3));
  m_Result << std::setiosflags(std::ios_base::fixed);

  if (options.test(STATS_MEMORY)) {
    GetMemoryStats();
  }
  if (options.test(STATS_MAX)) {
    GetMaxStats();
  }
  if (options.test(STATS_RETE)) {
    GetReteStats();
  }

  if (options.test(STATS_CYCLE)) {
    if (thisAgent->stats_db->get_status() == soar_module::disconnected) {
      return true;  // FIXME: warn
    }

    bool desc = sort < 0;
    sort = abs(sort);

    soar_module::sqlite_statement* select = 0;

    switch (sort) {
      default:
      case 0:  // default sort
      case 1:  // sort dc
        select = desc ? thisAgent->stats_stmts->sel_dc_dec
                      : thisAgent->stats_stmts->sel_dc_inc;
        break;
      case 2:  // sort time
        select = desc ? thisAgent->stats_stmts->sel_time_dec
                      : thisAgent->stats_stmts->sel_time_inc;
        break;
      case 3:  // sort wm_changes
        select = desc ? thisAgent->stats_stmts->sel_wm_changes_dec
                      : thisAgent->stats_stmts->sel_wm_changes_inc;
        break;
      case 4:  // sort firing_count
        select = desc ? thisAgent->stats_stmts->sel_firing_count_dec
                      : thisAgent->stats_stmts->sel_firing_count_inc;
        break;
    }

    for (int count = 0; select->execute() == soar_module::row; ++count) {
      if (count % 20 == 0) {
        m_Result << "\n";
        m_Result << "----------- ----------- ----------- -----------\n";
        m_Result << "DC          Time (sec)  WM Changes  Firing Cnt\n";
        m_Result << "----------- ----------- ----------- -----------\n";
      }

      m_Result << std::setw(11) << select->column_int(0) << " ";
      m_Result << std::setw(11) << (select->column_int(1) / 1000000.0) << " ";
      m_Result << std::setw(11) << select->column_int(2) << " ";
      m_Result << std::setw(11) << select->column_int(3) << "\n";
    }

    select->reinitialize();
  }

  if (options.test(STATS_TRACK)) {
    thisAgent->dc_stat_tracking = true;
  }

  if (options.test(STATS_STOP_TRACK)) {
    stats_close(thisAgent);
    thisAgent->dc_stat_tracking = false;
  }

  if ((!options.test(STATS_CYCLE) && !options.test(STATS_TRACK) &&
       !options.test(STATS_STOP_TRACK) && !options.test(STATS_MEMORY) &&
       !options.test(STATS_RETE) && !options.test(STATS_MAX) &&
       !options.test(STATS_RESET)) ||
      options.test(STATS_SYSTEM)) {
    GetSystemStats();
  }
  m_Result << std::resetiosflags(std::ios_base::fixed);
  m_Result.precision(oldPrecision);

  std::string temp;
  AppendArgTagFast(
      sml_Names::kParamStatsProductionCountDefault, sml_Names::kTypeInt,
      to_string(thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE],
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsProductionCountUser, sml_Names::kTypeInt,
      to_string(thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE],
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsProductionCountChunk, sml_Names::kTypeInt,
      to_string(thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE],
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsProductionCountJustification, sml_Names::kTypeInt,
      to_string(
          thisAgent->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE],
          temp));
  AppendArgTagFast(sml_Names::kParamStatsCycleCountDecision,
                   sml_Names::kTypeInt,
                   to_string(thisAgent->decide_phases_count, temp));
  AppendArgTagFast(sml_Names::kParamStatsCycleCountElaboration,
                   sml_Names::kTypeInt,
                   to_string(thisAgent->e_cycle_count, temp));
  AppendArgTagFast(sml_Names::kParamStatsCycleCountInnerElaboration,
                   sml_Names::kTypeInt,
                   to_string(thisAgent->inner_e_cycle_count, temp));
  AppendArgTagFast(sml_Names::kParamStatsProductionFiringCount,
                   sml_Names::kTypeInt,
                   to_string(thisAgent->production_firing_count, temp));
  AppendArgTagFast(sml_Names::kParamStatsWmeCountAddition, sml_Names::kTypeInt,
                   to_string(thisAgent->wme_addition_count, temp));
  AppendArgTagFast(sml_Names::kParamStatsWmeCountRemoval, sml_Names::kTypeInt,
                   to_string(thisAgent->wme_removal_count, temp));
  AppendArgTagFast(sml_Names::kParamStatsWmeCount, sml_Names::kTypeInt,
                   to_string(thisAgent->num_wmes_in_rete, temp));
  AppendArgTagFast(sml_Names::kParamStatsWmeCountAverage,
                   sml_Names::kTypeDouble,
                   to_string((thisAgent->num_wm_sizes_accumulated
                                  ? (thisAgent->cumulative_wm_size /
                                     thisAgent->num_wm_sizes_accumulated)
                                  : 0.0),
                             temp));
  AppendArgTagFast(sml_Names::kParamStatsWmeCountMax, sml_Names::kTypeInt,
                   to_string(thisAgent->max_wm_size, temp));
#ifndef NO_TIMING_STUFF
  AppendArgTagFast(
      sml_Names::kParamStatsKernelCPUTime, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_total_kernel_time.get_sec(), temp));
  AppendArgTagFast(sml_Names::kParamStatsTotalCPUTime, sml_Names::kTypeDouble,
                   to_string(thisAgent->timers_total_cpu_time.get_sec(), temp));
  AppendArgTagFast(
      sml_Names::kParamStatsPhaseTimeInputPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_decision_cycle_phase[INPUT_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsPhaseTimeProposePhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_decision_cycle_phase[PROPOSE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsPhaseTimeDecisionPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_decision_cycle_phase[DECIDE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsPhaseTimeApplyPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_decision_cycle_phase[APPLY_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsPhaseTimeOutputPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_decision_cycle_phase[OUTPUT_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsPhaseTimePreferencePhase, sml_Names::kTypeDouble,
      to_string(
          thisAgent->timers_decision_cycle_phase[PREFERENCE_PHASE].get_sec(),
          temp));
  AppendArgTagFast(
      sml_Names::kParamStatsPhaseTimeWorkingMemoryPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_decision_cycle_phase[WM_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMonitorTimeInputPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_monitors_cpu_time[INPUT_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMonitorTimeProposePhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_monitors_cpu_time[PROPOSE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMonitorTimeDecisionPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_monitors_cpu_time[DECIDE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMonitorTimeApplyPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_monitors_cpu_time[APPLY_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMonitorTimeOutputPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_monitors_cpu_time[OUTPUT_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMonitorTimePreferencePhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_monitors_cpu_time[PREFERENCE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMonitorTimeWorkingMemoryPhase,
      sml_Names::kTypeDouble,
      to_string(thisAgent->timers_monitors_cpu_time[WM_PHASE].get_sec(), temp));
  AppendArgTagFast(
      sml_Names::kParamStatsInputFunctionTime, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_input_function_cpu_time.get_sec(), temp));
  AppendArgTagFast(
      sml_Names::kParamStatsOutputFunctionTime, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_output_function_cpu_time.get_sec(), temp));

#ifdef DETAILED_TIMING_STATS
  AppendArgTagFast(
      sml_Names::kParamStatsMatchTimeInputPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_match_cpu_time[INPUT_PHASE].get_sec(), temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMatchTimePreferencePhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_match_cpu_time[PREFERENCE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMatchTimeWorkingMemoryPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_match_cpu_time[WM_PHASE].get_sec(), temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMatchTimeOutputPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_match_cpu_time[OUTPUT_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMatchTimeDecisionPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_match_cpu_time[DECIDE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMatchTimeProposePhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_match_cpu_time[PROPOSE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMatchTimeApplyPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_match_cpu_time[APPLY_PHASE].get_sec(), temp));
  AppendArgTagFast(
      sml_Names::kParamStatsOwnershipTimeInputPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_ownership_cpu_time[INPUT_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsOwnershipTimePreferencePhase,
      sml_Names::kTypeDouble,
      to_string(
          thisAgent->timers_ownership_cpu_time[PREFERENCE_PHASE].get_sec(),
          temp));
  AppendArgTagFast(
      sml_Names::kParamStatsOwnershipTimeWorkingMemoryPhase,
      sml_Names::kTypeDouble,
      to_string(thisAgent->timers_ownership_cpu_time[WM_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsOwnershipTimeOutputPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_ownership_cpu_time[OUTPUT_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsOwnershipTimeDecisionPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_ownership_cpu_time[DECIDE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsOwnershipTimeProposePhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_ownership_cpu_time[PROPOSE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsOwnershipTimeApplyPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_ownership_cpu_time[APPLY_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsChunkingTimeInputPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_chunking_cpu_time[INPUT_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsChunkingTimePreferencePhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_chunking_cpu_time[PREFERENCE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsChunkingTimeWorkingMemoryPhase,
      sml_Names::kTypeDouble,
      to_string(thisAgent->timers_chunking_cpu_time[WM_PHASE].get_sec(), temp));
  AppendArgTagFast(
      sml_Names::kParamStatsChunkingTimeOutputPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_chunking_cpu_time[OUTPUT_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsChunkingTimeDecisionPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_chunking_cpu_time[DECIDE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsChunkingTimeProposePhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_chunking_cpu_time[PROPOSE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsChunkingTimeApplyPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_chunking_cpu_time[APPLY_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsGDSTimeInputPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_gds_cpu_time[INPUT_PHASE].get_sec(), temp));
  AppendArgTagFast(
      sml_Names::kParamStatsGDSTimePreferencePhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_gds_cpu_time[PREFERENCE_PHASE].get_sec(),
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsGDSTimeWorkingMemoryPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_gds_cpu_time[WM_PHASE].get_sec(), temp));
  AppendArgTagFast(
      sml_Names::kParamStatsGDSTimeOutputPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_gds_cpu_time[OUTPUT_PHASE].get_sec(), temp));
  AppendArgTagFast(
      sml_Names::kParamStatsGDSTimeDecisionPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_gds_cpu_time[DECIDE_PHASE].get_sec(), temp));
  AppendArgTagFast(
      sml_Names::kParamStatsGDSTimeProposePhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_gds_cpu_time[PROPOSE_PHASE].get_sec(), temp));
  AppendArgTagFast(
      sml_Names::kParamStatsGDSTimeApplyPhase, sml_Names::kTypeDouble,
      to_string(thisAgent->timers_gds_cpu_time[APPLY_PHASE].get_sec(), temp));
#endif  // DETAILED_TIMING_STATS

  AppendArgTagFast(sml_Names::kParamStatsMaxDecisionCycleTimeCycle,
                   sml_Names::kTypeInt,
                   to_string(thisAgent->max_dc_time_cycle, temp));
  AppendArgTagFast(sml_Names::kParamStatsMaxDecisionCycleTimeValueSec,
                   sml_Names::kTypeDouble,
                   to_string(thisAgent->max_dc_time_usec / 100000.0, temp));
  AppendArgTagFast(sml_Names::kParamStatsMaxDecisionCycleTimeValueUSec,
                   sml_Names::kTypeInt,
                   to_string(thisAgent->max_dc_time_usec, temp));
  AppendArgTagFast(sml_Names::kParamStatsMaxDecisionCycleEpMemTimeCycle,
                   sml_Names::kTypeInt,
                   to_string(thisAgent->max_dc_epmem_time_cycle, temp));
  AppendArgTagFast(sml_Names::kParamStatsMaxDecisionCycleEpMemTimeValueSec,
                   sml_Names::kTypeDouble,
                   to_string(thisAgent->max_dc_epmem_time_sec, temp));
  AppendArgTagFast(sml_Names::kParamStatsMaxDecisionCycleSMemTimeCycle,
                   sml_Names::kTypeInt,
                   to_string(thisAgent->max_dc_smem_time_cycle, temp));
  AppendArgTagFast(sml_Names::kParamStatsMaxDecisionCycleSMemTimeValueSec,
                   sml_Names::kTypeDouble,
                   to_string(thisAgent->max_dc_smem_time_sec, temp));
#endif  // NO_TIMING_STUFF

  AppendArgTagFast(sml_Names::kParamStatsMaxDecisionCycleWMChangesCycle,
                   sml_Names::kTypeInt,
                   to_string(thisAgent->max_dc_wm_changes_cycle, temp));
  AppendArgTagFast(sml_Names::kParamStatsMaxDecisionCycleWMChangesValue,
                   sml_Names::kTypeInt,
                   to_string(thisAgent->max_dc_wm_changes_value, temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMaxDecisionCycleFireCountCycle, sml_Names::kTypeInt,
      to_string(thisAgent->max_dc_production_firing_count_cycle, temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMaxDecisionCycleFireCountValue, sml_Names::kTypeInt,
      to_string(thisAgent->max_dc_production_firing_count_value, temp));

  AppendArgTagFast(
      sml_Names::kParamStatsMemoryUsageMiscellaneous, sml_Names::kTypeInt,
      to_string(
          thisAgent->memoryManager->memory_for_usage[MISCELLANEOUS_MEM_USAGE],
          temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMemoryUsageHash, sml_Names::kTypeInt,
      to_string(
          thisAgent->memoryManager->memory_for_usage[HASH_TABLE_MEM_USAGE],
          temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMemoryUsageString, sml_Names::kTypeInt,
      to_string(thisAgent->memoryManager->memory_for_usage[STRING_MEM_USAGE],
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMemoryUsagePool, sml_Names::kTypeInt,
      to_string(thisAgent->memoryManager->memory_for_usage[POOL_MEM_USAGE],
                temp));
  AppendArgTagFast(
      sml_Names::kParamStatsMemoryUsageStatsOverhead, sml_Names::kTypeInt,
      to_string(
          thisAgent->memoryManager->memory_for_usage[STATS_OVERHEAD_MEM_USAGE],
          temp));

  if (options.test(STATS_RESET)) {
    reset_max_stats(thisAgent);
  }

  return true;
}

void CommandLineInterface::GetAgentStats() {
  agent* thisAgent = m_pAgentSML->GetSoarAgent();
  m_Result << "Agent counters:\n";

  m_Result << "Counter          Value\n";
  m_Result << "---------------- -----------\n";

  for (std::unordered_map<std::string, uint64_t>::iterator it =
           thisAgent->dyn_counters->begin();
       it != thisAgent->dyn_counters->end(); it++) {
    m_Result << std::setw(16) << it->first << " " << std::setw(11) << it->second
             << "\n";
  }
}

void CommandLineInterface::GetSystemStats() {
  // Hostname
  char hostname[256];
  memset(hostname, 0, 256);
  if (gethostname(hostname, 255) == SOCKET_ERROR) {
    strncpy(hostname, "[host name unknown]", 255);
  }

  // Time
  time_t current_time = time(NULL);

  agent* thisAgent = m_pAgentSML->GetSoarAgent();

#ifndef NO_TIMING_STUFF
  double total_kernel_time = thisAgent->timers_total_kernel_time.get_sec();
  double total_kernel_msec = total_kernel_time * 1000.0;

  double input_function_time =
      thisAgent->timers_input_function_cpu_time.get_sec();
  double output_function_time =
      thisAgent->timers_output_function_cpu_time.get_sec();

  /* Total of the time spent in callback routines. */
  double monitors_sum =
      thisAgent->timers_monitors_cpu_time[INPUT_PHASE].get_sec() +
      thisAgent->timers_monitors_cpu_time[PROPOSE_PHASE].get_sec() +
      thisAgent->timers_monitors_cpu_time[APPLY_PHASE].get_sec() +
      thisAgent->timers_monitors_cpu_time[PREFERENCE_PHASE].get_sec() +
      thisAgent->timers_monitors_cpu_time[WM_PHASE].get_sec() +
      thisAgent->timers_monitors_cpu_time[OUTPUT_PHASE].get_sec() +
      thisAgent->timers_monitors_cpu_time[DECIDE_PHASE].get_sec();

  double derived_kernel_time =
      get_derived_kernel_time_usec(thisAgent) / 1000000.0;
  double derived_total_cpu_time = derived_kernel_time + monitors_sum +
                                  input_function_time + output_function_time;

  /* Total time spent in the input phase */
  double input_phase_total_time =
      thisAgent->timers_decision_cycle_phase[INPUT_PHASE].get_sec() +
      thisAgent->timers_monitors_cpu_time[INPUT_PHASE].get_sec() +
      thisAgent->timers_input_function_cpu_time.get_sec();

  /* Total time spent in the propose phase */
  double propose_phase_total_time =
      thisAgent->timers_decision_cycle_phase[PROPOSE_PHASE].get_sec() +
      thisAgent->timers_monitors_cpu_time[PROPOSE_PHASE].get_sec();

  /* Total time spent in the apply phase */
  double apply_phase_total_time =
      thisAgent->timers_decision_cycle_phase[APPLY_PHASE].get_sec() +
      thisAgent->timers_monitors_cpu_time[APPLY_PHASE].get_sec();

  /* Total time spent in the output phase */
  double output_phase_total_time =
      thisAgent->timers_decision_cycle_phase[OUTPUT_PHASE].get_sec() +
      thisAgent->timers_monitors_cpu_time[OUTPUT_PHASE].get_sec() +
      thisAgent->timers_output_function_cpu_time.get_sec();

  /* Total time spent in the decide phase */
  double decide_phase_total_time =
      thisAgent->timers_decision_cycle_phase[DECIDE_PHASE].get_sec() +
      thisAgent->timers_monitors_cpu_time[DECIDE_PHASE].get_sec();
#endif  // NO_TIMING_STUFF

  /* The sum of these phase timers is exactly equal to the
   * derived_total_cpu_time
   */

  m_Result << "Soar " << sml_Names::kSoarVersionValue << " on " << hostname
           << " at " << ctime(&current_time) << "\n";

  uint64_t totalProductions =
      thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE];
  totalProductions += thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE];
  totalProductions += thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE];

  m_Result << totalProductions << " productions ("
           << thisAgent->num_productions_of_type[DEFAULT_PRODUCTION_TYPE]
           << " default, "
           << thisAgent->num_productions_of_type[USER_PRODUCTION_TYPE]
           << " user, "
           << thisAgent->num_productions_of_type[CHUNK_PRODUCTION_TYPE]
           << " chunks)\n";

  m_Result << "   + "
           << thisAgent->num_productions_of_type[JUSTIFICATION_PRODUCTION_TYPE]
           << " justifications\n";

  /* The fields for the timers are 8.3, providing an upper limit of
  approximately 2.5 hours the printing of the run time calculations.
  Obviously, these will need to be increased if you plan on needing
  run-time data for a process that you expect to take longer than
  2 hours. :) */

#ifndef NO_TIMING_STUFF
  m_Result << "                                                        |   "
              "Computed\n";
  m_Result << "Phases:      Input   Propose   Decide   Apply    Output |     "
              "Totals\n";
  m_Result << "========================================================|======="
              "====\n";

  m_Result << "Kernel:   " << std::setw(8)
           << thisAgent->timers_decision_cycle_phase[INPUT_PHASE].get_sec()
           << " " << std::setw(8)
           << thisAgent->timers_decision_cycle_phase[PROPOSE_PHASE].get_sec()
           << " " << std::setw(8)
           << thisAgent->timers_decision_cycle_phase[DECIDE_PHASE].get_sec()
           << " " << std::setw(8)
           << thisAgent->timers_decision_cycle_phase[APPLY_PHASE].get_sec()
           << " " << std::setw(8)
           << thisAgent->timers_decision_cycle_phase[OUTPUT_PHASE].get_sec()
           << "  | " << std::setw(10) << derived_kernel_time << "\n";

  m_Result << "========================================================|======="
              "====\n";

  m_Result << "Input fn: " << std::setw(8) << input_function_time
           << "                                      | " << std::setw(10)
           << input_function_time << "\n";

  m_Result << "========================================================|======="
              "====\n";
  m_Result << "Outpt fn:                                     " << std::setw(8)
           << output_function_time << "  | " << std::setw(10)
           << output_function_time << "\n";

  m_Result << "========================================================|======="
              "====\n";

  m_Result << "Callbcks: " << std::setw(8)
           << thisAgent->timers_monitors_cpu_time[INPUT_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_monitors_cpu_time[PROPOSE_PHASE].get_sec()
           << " " << std::setw(8)
           << thisAgent->timers_monitors_cpu_time[DECIDE_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_monitors_cpu_time[APPLY_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_monitors_cpu_time[OUTPUT_PHASE].get_sec()
           << "  | " << std::setw(10) << monitors_sum << "\n";

  m_Result << "========================================================|======="
              "====\n";
  m_Result << "Computed------------------------------------------------+-------"
              "----\n";
  m_Result << "Totals:   " << std::setw(8) << input_phase_total_time << " "
           << std::setw(8) << propose_phase_total_time << " " << std::setw(8)
           << decide_phase_total_time << " " << std::setw(8)
           << apply_phase_total_time << " " << std::setw(8)
           << output_phase_total_time << "  | " << std::setw(10)
           << derived_total_cpu_time << "\n\n";

#ifdef DETAILED_TIMING_STATS
  double match_sum =
      thisAgent->timers_match_cpu_time[INPUT_PHASE].get_sec() +
      thisAgent->timers_match_cpu_time[PROPOSE_PHASE].get_sec() +
      thisAgent->timers_match_cpu_time[APPLY_PHASE].get_sec() +
      thisAgent->timers_match_cpu_time[PREFERENCE_PHASE].get_sec() +
      thisAgent->timers_match_cpu_time[WM_PHASE].get_sec() +
      thisAgent->timers_match_cpu_time[OUTPUT_PHASE].get_sec() +
      thisAgent->timers_match_cpu_time[DECIDE_PHASE].get_sec();
  double own_sum =
      thisAgent->timers_ownership_cpu_time[INPUT_PHASE].get_sec() +
      thisAgent->timers_ownership_cpu_time[PROPOSE_PHASE].get_sec() +
      thisAgent->timers_ownership_cpu_time[APPLY_PHASE].get_sec() +
      thisAgent->timers_ownership_cpu_time[PREFERENCE_PHASE].get_sec() +
      thisAgent->timers_ownership_cpu_time[WM_PHASE].get_sec() +
      thisAgent->timers_ownership_cpu_time[OUTPUT_PHASE].get_sec() +
      thisAgent->timers_ownership_cpu_time[DECIDE_PHASE].get_sec();
  double chunk_sum =
      thisAgent->timers_chunking_cpu_time[INPUT_PHASE].get_sec() +
      thisAgent->timers_chunking_cpu_time[PROPOSE_PHASE].get_sec() +
      thisAgent->timers_chunking_cpu_time[APPLY_PHASE].get_sec() +
      thisAgent->timers_chunking_cpu_time[PREFERENCE_PHASE].get_sec() +
      thisAgent->timers_chunking_cpu_time[WM_PHASE].get_sec() +
      thisAgent->timers_chunking_cpu_time[OUTPUT_PHASE].get_sec() +
      thisAgent->timers_chunking_cpu_time[DECIDE_PHASE].get_sec();
  double gds_sum = thisAgent->timers_gds_cpu_time[INPUT_PHASE].get_sec() +
                   thisAgent->timers_gds_cpu_time[PROPOSE_PHASE].get_sec() +
                   thisAgent->timers_gds_cpu_time[APPLY_PHASE].get_sec() +
                   thisAgent->timers_gds_cpu_time[PREFERENCE_PHASE].get_sec() +
                   thisAgent->timers_gds_cpu_time[WM_PHASE].get_sec() +
                   thisAgent->timers_gds_cpu_time[OUTPUT_PHASE].get_sec() +
                   thisAgent->timers_gds_cpu_time[DECIDE_PHASE].get_sec();

  m_Result << "Detailed Timing Stats:   "
           << "\n\n";
  m_Result << "================================================================"
              "=============|===========\n";
  m_Result << "Phases:        Input   Propose   Decide   Apply    Pref     "
              "WrkMem    Output |     Totals\n";
  m_Result << "================================================================"
              "=============|===========\n";
  m_Result << "Match:      " << std::setw(8)
           << thisAgent->timers_match_cpu_time[INPUT_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_match_cpu_time[PROPOSE_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_match_cpu_time[DECIDE_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_match_cpu_time[APPLY_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_match_cpu_time[OUTPUT_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_match_cpu_time[PREFERENCE_PHASE].get_sec()
           << " " << std::setw(8)
           << thisAgent->timers_match_cpu_time[WM_PHASE].get_sec() << "  | "
           << std::setw(10) << match_sum << "\n";
  m_Result << "Ownership:  " << std::setw(8)
           << thisAgent->timers_ownership_cpu_time[INPUT_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_ownership_cpu_time[PROPOSE_PHASE].get_sec()
           << " " << std::setw(8)
           << thisAgent->timers_ownership_cpu_time[DECIDE_PHASE].get_sec()
           << " " << std::setw(8)
           << thisAgent->timers_ownership_cpu_time[APPLY_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_ownership_cpu_time[PREFERENCE_PHASE].get_sec()
           << " " << std::setw(8)
           << thisAgent->timers_ownership_cpu_time[WM_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_ownership_cpu_time[OUTPUT_PHASE].get_sec()
           << "  | " << std::setw(10) << own_sum << "\n";
  m_Result << "Chunking:   " << std::setw(8)
           << thisAgent->timers_chunking_cpu_time[INPUT_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_chunking_cpu_time[PROPOSE_PHASE].get_sec()
           << " " << std::setw(8)
           << thisAgent->timers_chunking_cpu_time[DECIDE_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_chunking_cpu_time[APPLY_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_chunking_cpu_time[PREFERENCE_PHASE].get_sec()
           << " " << std::setw(8)
           << thisAgent->timers_chunking_cpu_time[WM_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_chunking_cpu_time[OUTPUT_PHASE].get_sec()
           << "  | " << std::setw(10) << chunk_sum << "\n";
  m_Result << "GDS:        " << std::setw(8)
           << thisAgent->timers_gds_cpu_time[INPUT_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_gds_cpu_time[PROPOSE_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_gds_cpu_time[DECIDE_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_gds_cpu_time[APPLY_PHASE].get_sec() << " "
           << std::setw(8)
           << thisAgent->timers_gds_cpu_time[PREFERENCE_PHASE].get_sec() << " "
           << std::setw(8) << thisAgent->timers_gds_cpu_time[WM_PHASE].get_sec()
           << " " << std::setw(8)
           << thisAgent->timers_gds_cpu_time[OUTPUT_PHASE].get_sec() << "  | "
           << std::setw(10) << gds_sum << "\n";
#endif  // DETAILED_TIMING_STATS

  m_Result << "Values from single timers:\n";

  m_Result << " Kernel CPU Time: " << std::setw(11) << total_kernel_time
           << " sec. \n";

  m_Result << " Total  CPU Time: " << std::setw(11)
           << thisAgent->timers_total_cpu_time.get_sec() << " sec.\n\n";

  ///* v8.6.2: print out decisions executed, not # full cycles */

  m_Result << thisAgent->decide_phases_count << " decisions ("
           << (thisAgent->decide_phases_count
                   ? total_kernel_msec / thisAgent->decide_phases_count
                   : 0.0)
           << " msec/decision)\n";
  m_Result << thisAgent->e_cycle_count << " elaboration cycles ("
           << (thisAgent->decide_phases_count
                   ? static_cast<double>(thisAgent->e_cycle_count) /
                         thisAgent->decide_phases_count
                   : 0)
           << " ec's per dc, "
           << (thisAgent->e_cycle_count
                   ? total_kernel_msec / thisAgent->e_cycle_count
                   : 0)
           << " msec/ec)\n";
  m_Result << thisAgent->inner_e_cycle_count << " inner elaboration cycles\n";

  m_Result << thisAgent->pe_cycle_count << " p-elaboration cycles ("
           << (thisAgent->decide_phases_count
                   ? static_cast<double>(thisAgent->pe_cycle_count) /
                         thisAgent->decide_phases_count
                   : 0)
           << " pe's per dc, "
           << (thisAgent->pe_cycle_count
                   ? total_kernel_msec / thisAgent->pe_cycle_count
                   : 0)
           << " msec/pe)\n";

  m_Result << thisAgent->production_firing_count << " production firings ("
           << (thisAgent->e_cycle_count
                   ? static_cast<double>(thisAgent->production_firing_count) /
                         thisAgent->e_cycle_count
                   : 0.0)
           << " pf's per ec, "
           << (thisAgent->production_firing_count
                   ? total_kernel_msec / thisAgent->production_firing_count
                   : 0.0)
           << " msec/pf)\n";
#endif  // NO_TIMING_STUFF

  uint64_t wme_changes =
      thisAgent->wme_addition_count + thisAgent->wme_removal_count;
  m_Result << wme_changes << " wme changes (" << thisAgent->wme_addition_count
           << " additions, " << thisAgent->wme_removal_count << " removals)\n";

  m_Result << "WM size: " << thisAgent->num_wmes_in_rete << " current, "
           << (thisAgent->num_wm_sizes_accumulated
                   ? (thisAgent->cumulative_wm_size /
                      thisAgent->num_wm_sizes_accumulated)
                   : 0.0)
           << " mean, " << thisAgent->max_wm_size << " maximum\n\n";
}

void CommandLineInterface::GetMaxStats() {
  agent* thisAgent = m_pAgentSML->GetSoarAgent();
  m_Result << "Single decision cycle maximums:\n";

  m_Result << "Stat             Value       Cycle\n";
  m_Result << "---------------- ----------- -----------\n";

#ifndef NO_TIMING_STUFF
  m_Result << std::setw(16) << "Time (sec)" << std::setw(11)
           << std::setprecision(6) << (thisAgent->max_dc_time_usec / 1000000.0)
           << " " << std::setw(11) << thisAgent->max_dc_time_cycle << "\n";

  m_Result << std::setw(16) << "EpMem Time (sec)" << std::setw(11)
           << std::setprecision(6) << thisAgent->max_dc_epmem_time_sec << " "
           << std::setw(11) << thisAgent->max_dc_epmem_time_cycle << "\n";

  m_Result << std::setw(16) << "SMem Time (sec)" << std::setw(11)
           << std::setprecision(6) << thisAgent->max_dc_smem_time_sec << " "
           << std::setw(11) << thisAgent->max_dc_smem_time_cycle << "\n";
#endif  // NO_TIMING_STUFF

  m_Result << std::setw(16) << "WM changes" << std::setw(11)
           << thisAgent->max_dc_wm_changes_value << " " << std::setw(11)
           << thisAgent->max_dc_wm_changes_cycle << "\n";

  m_Result << std::setw(16) << "Firing count" << std::setw(11)
           << thisAgent->max_dc_production_firing_count_value << " "
           << std::setw(11) << thisAgent->max_dc_production_firing_count_cycle
           << "\n";
}

void CommandLineInterface::GetMemoryStats() {
  agent* thisAgent = m_pAgentSML->GetSoarAgent();
  size_t total = 0;
  for (int i = 0; i < NUM_MEM_USAGE_CODES; i++) {
    total += thisAgent->memoryManager->memory_for_usage[i];
  }

  m_Result << std::setw(8) << total << " bytes total memory allocated\n";
  m_Result
      << std::setw(8)
      << thisAgent->memoryManager->memory_for_usage[STATS_OVERHEAD_MEM_USAGE]
      << " bytes statistics overhead\n";
  m_Result << std::setw(8)
           << thisAgent->memoryManager->memory_for_usage[STRING_MEM_USAGE]
           << " bytes for strings\n";
  m_Result << std::setw(8)
           << thisAgent->memoryManager->memory_for_usage[HASH_TABLE_MEM_USAGE]
           << " bytes for hash tables\n";
  m_Result << std::setw(8)
           << thisAgent->memoryManager->memory_for_usage[POOL_MEM_USAGE]
           << " bytes for various memory pools\n";
  m_Result
      << std::setw(8)
      << thisAgent->memoryManager->memory_for_usage[MISCELLANEOUS_MEM_USAGE]
      << " bytes for miscellaneous other things\n";

  GetMemoryPoolStatistics();
}

void CommandLineInterface::GetMemoryPoolStatistics() {
  agent* thisAgent = m_pAgentSML->GetSoarAgent();
  m_Result << "Memory pool statistics:\n\n";
#ifdef MEMORY_POOL_STATS
  m_Result << "Pool Name        Used Items  Free Items  Item Size  Itm/Blk  "
              "Blocks  Total Bytes\n";
  m_Result << "---------------  ----------  ----------  ---------  -------  "
              "------  -----------\n";
#else
  m_Result << "Pool Name        Item Size  Itm/Blk  Blocks  Total Bytes\n";
  m_Result << "---------------  ---------  -------  ------  -----------\n";
#endif

  for (memory_pool* p = thisAgent->memoryManager->memory_pools_in_use; p != NIL;
       p = p->next) {
    m_Result << std::setw(MAX_POOL_NAME_LENGTH) << p->name;
#ifdef MEMORY_POOL_STATS
    m_Result << "  " << std::setw(10) << p->used_count;
    size_t total_items = p->num_blocks * p->items_per_block;
    m_Result << "  " << std::setw(10) << total_items - p->used_count;
#endif
    m_Result << "  " << std::setw(9) << p->item_size;
    m_Result << "  " << std::setw(7) << p->items_per_block;
    m_Result << "  " << std::setw(6) << p->num_blocks;
    m_Result << "  " << std::setw(11)
             << p->num_blocks * p->items_per_block * p->item_size;
    m_Result << "\n";
  }
}

void CommandLineInterface::GetReteStats() {
  agent* thisAgent = m_pAgentSML->GetSoarAgent();
#ifdef TOKEN_SHARING_STATS
  m_Result << "Token additions: " << thisAgent->token_additions
           << "   If no sharing: " << thisAgent->token_additions_without_sharing
           << "\n";
#endif

  int i;
  uint64_t tot;

  get_all_node_count_stats(thisAgent);

  /* print table headers */
#ifdef SHARING_FACTORS
  m_Result
      << "      Node Type            Actual  If no merging  If no sharing\n";
  m_Result
      << "---------------------  ----------  -------------  -------------\n";
#else
  m_Result << "      Node Type            Actual  If no merging\n";
  m_Result << "---------------------  ----------  -------------\n";
#endif

  /* print main table */
  for (i = 0; i < 256; i++)
    if (*bnode_type_names[i]) {
      m_Result << std::setw(21) << bnode_type_names[i] << "  " << std::setw(10)
               << thisAgent->actual[i] << "  " << std::setw(13)
               << thisAgent->if_no_merging[i];
#ifdef SHARING_FACTORS
      m_Result << "  " << std::setw(13) << thisAgent->if_no_sharing[i];
#endif
      m_Result << "\n";
    }

    /* print table end (totals) */
#ifdef SHARING_FACTORS
  m_Result
      << "---------------------  ----------  -------------  -------------\n";
#else
  m_Result << "---------------------  ----------  -------------\n";
#endif
  m_Result << "                Total";
  for (tot = 0, i = 0; i < 256; i++) {
    tot += thisAgent->actual[i];
  }
  m_Result << "  " << std::setw(10) << tot;

  for (tot = 0, i = 0; i < 256; i++) {
    tot += thisAgent->if_no_merging[i];
  }
  m_Result << "  " << std::setw(13) << tot;

#ifdef SHARING_FACTORS
  for (tot = 0, i = 0; i < 256; i++) {
    tot += thisAgent->if_no_sharing[i];
  }
  m_Result << "  " << std::setw(13) << tot;
#endif
  m_Result << "\n";

  m_Result << "\nActivations: " << thisAgent->num_right_activations
           << " right (" << thisAgent->num_null_right_activations << " null), "
           << thisAgent->num_left_activations << " left ("
           << thisAgent->num_null_left_activations << " null)\n";
}
