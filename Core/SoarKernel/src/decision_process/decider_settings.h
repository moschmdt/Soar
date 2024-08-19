/*
 * decider_settings.h
 *
 *  Created on: Sep 11, 2016
 *      Author: mazzin
 */

#ifndef CORE_SOARKERNEL_SRC_DECISION_PROCESS_DECIDER_SETTINGS_H_
#define CORE_SOARKERNEL_SRC_DECISION_PROCESS_DECIDER_SETTINGS_H_

#include "kernel.h"
#include "sml_KernelSML.h"
#include "soar_module.h"

class decider_param_container : public soar_module::param_container {
 public:
  soar_module::constant_param<top_level_phase>* stop_phase;

  soar_module::boolean_param* keep_all_top_oprefs;
  soar_module::integer_param* max_gp;
  soar_module::integer_param* max_dc_time;
  soar_module::integer_param* max_elaborations;
  soar_module::integer_param* max_goal_depth;
  soar_module::integer_param* max_memory_usage;
  soar_module::integer_param* max_nil_output_cycles;
  soar_module::boolean_param* tcl_enabled;
  soar_module::boolean_param* timers_enabled;
  soar_module::boolean_param* wait_snc;

  soar_module::boolean_param* init_cmd;
  soar_module::boolean_param* reset_cmd;
  //        soar_module::boolean_param* run_cmd;
  soar_module::boolean_param* stop_cmd;
  soar_module::boolean_param* version_cmd;
  soar_module::boolean_param* help_cmd;
  soar_module::boolean_param* qhelp_cmd;

  decider_param_container(agent* new_agent, uint64_t pDecider_settings[]);

  void print_status(agent* thisAgent);
  void print_settings(agent* thisAgent);
};

#endif /* CORE_SOARKERNEL_SRC_DECISION_PROCESS_DECIDER_SETTINGS_H_ */
