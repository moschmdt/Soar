/**
 * ebc.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Mazin Assanie
 */

#ifndef EBC_EXPLAIN_CONDITION_RECORD_H_
#define EBC_EXPLAIN_CONDITION_RECORD_H_

#include "kernel.h"
#include "stl_typedefs.h"

class instantiation_record;
class action_record;

class condition_record {
  friend class Explanation_Memory;
  friend class GraphViz_Visualizer;
  friend class instantiation_record;
  friend class chunk_record;

 public:
  condition_record(){};
  ~condition_record(){};

  void init(agent* myAgent, condition* pCond, uint64_t pCondID,
            instantiation_record* pInst, bool isChunkInstantiation = false);
  void clean_up();

  uint64_t get_conditionID() { return conditionID; };
  goal_stack_level get_level() { return wme_level_at_firing; };
  instantiation_record* get_parent_inst() { return parent_instantiation; };
  instantiation_record* get_instantiation() { return my_instantiation; };
  inst_record_list* get_path_to_base() { return path_to_base; };
  preference* get_cached_pref() { return cached_pref; };
  wme* get_cached_wme() { return cached_wme; };

  void connect_to_action();
  void viz_connect_to_action(goal_stack_level pMatchLevel,
                             bool isChunkInstantiation = false);
  void set_path_to_base(inst_record_list* pPath) {
    assert(pPath);
    if (!path_to_base)
      path_to_base = new inst_record_list();
    else
      path_to_base->clear();
    (*path_to_base) = (*pPath);
  }
  void set_instantiation(instantiation_record* pInst) {
    my_instantiation = pInst;
  };
  void set_matched_wme_for_cond(condition* pCond);
  void update_condition(condition* pCond, instantiation_record* pInst,
                        bool isChunkInstantiation = false);

  void viz_combo_test(test pTest, test pTestIdentity, uint64_t pNode_id,
                      WME_Field pField, bool isNegative, bool printAcceptable,
                      bool isSuper);
  void viz_matched_test(test pTest, Symbol* pMatchedWME, uint64_t pNode_id,
                        WME_Field pField, bool isNegative, bool printIdentity,
                        bool printAcceptable, bool isSuper);
  void visualize_for_wm_trace(goal_stack_level match_level);
  void visualize_for_explanation_trace(condition* pCond,
                                       goal_stack_level match_level);
  void visualize_for_chunk();

 private:
  agent* thisAgent;

  uint64_t conditionID;
  preference* cached_pref;
  wme* cached_wme;
  instantiation_record* my_instantiation;
  instantiation_record* parent_instantiation;
  action_record* parent_action;

  byte type;
  goal_stack_level wme_level_at_firing;
  inst_record_list* path_to_base;

  test_triple condition_tests;
  symbol_triple matched_wme;
  bool test_for_acceptable_preference;
};

#endif /** EBC_EXPLAIN_CONDITION_RECORD_H_ */
