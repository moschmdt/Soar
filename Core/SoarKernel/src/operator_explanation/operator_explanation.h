/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  operator_explanation.h
 *
 * =======================================================================
 *  OperatorExplanationManager: records why an operator was selected.
 *
 *  For every tracked operator name, captures on each decision cycle:
 *    - all proposed candidates (acceptable preferences)
 *    - the LHS conditions (ground-truth WME values) of each proposal rule
 *    - all OSK preferences that influenced the selection
 *    - the winner and, for each loser, an elimination reason
 *
 *  Data is stored in an SQLite database (in-memory by default) and
 *  queried via the 'explain' CLI command:
 *    explain track-operator   <name>
 *    explain untrack-operator <name>
 *    explain operator         <name>
 * =======================================================================
 */

#ifndef OPERATOR_EXPLANATION_H
#define OPERATOR_EXPLANATION_H

#include "kernel.h"
#include "soar_db.h"

#include <string>
#include <unordered_set>

/* Forward declarations using the actual struct names from the codebase */
typedef struct agent_struct agent;
typedef struct slot_struct slot;
typedef struct symbol_struct Symbol;
typedef struct preference_struct preference;
typedef unsigned char byte;

/* -----------------------------------------------------------------------
   Statement container for operator explanation DB
   ----------------------------------------------------------------------- */
class OpExpl_Statement_Container
    : public soar_module::sqlite_statement_container {
public:
  /* decisions table */
  soar_module::sqlite_statement *decision_insert;
  /* candidates table */
  soar_module::sqlite_statement *candidate_insert;
  /* proposal_conditions table */
  soar_module::sqlite_statement *condition_insert;
  /* decision_preferences table */
  soar_module::sqlite_statement *preference_insert;
  /* query: select decisions for a given operator name */
  soar_module::sqlite_statement *decision_get_by_name;
  /* query: select candidates for a given decision id */
  soar_module::sqlite_statement *candidate_get_by_decision;
  /* query: select conditions for a given candidate id */
  soar_module::sqlite_statement *condition_get_by_candidate;
  /* query: select preferences for a given decision id */
  soar_module::sqlite_statement *preference_get_by_decision;

  OpExpl_Statement_Container(soar_module::sqlite_database *db);
};

/* -----------------------------------------------------------------------
   OperatorExplanationManager
   ----------------------------------------------------------------------- */
class OperatorExplanationManager {
public:
  OperatorExplanationManager(agent *new_agent);
  ~OperatorExplanationManager();

  /* Lifecycle */
  void init_db();

  /* Tracking control */
  void track_operator(const std::string &name);
  void untrack_operator(const std::string &name);
  bool is_tracked(const std::string &name) const;
  void list_tracked(agent *thisAgent) const;

  /* Called from decide_context_slot() after run_preference_semantics() */
  void record_decision(agent *thisAgent, slot *s, Symbol *goal,
                       preference *winner, /* NIL if impasse */
                       byte impasse_type);

  /* Query / print */
  void print_decisions(agent *thisAgent, const std::string &name) const;

  /* Query / JSON output */
  void print_decisions_json(agent *thisAgent, const std::string &name) const;

private:
  agent *my_agent;
  soar_module::sqlite_database *db;
  OpExpl_Statement_Container *stmts;
  bool track_all_operators;
  std::unordered_set<std::string> tracked_names;
  std::unordered_set<std::string> excluded_names;

  /* Helpers */
  static const char *preference_type_name(byte pref_type);
  static const char *impasse_type_name(byte impasse_type);

  /* Returns the ^name value of an operator identifier as a string,
     falling back to the identifier print string (e.g. "O3"). */
  std::string get_operator_name(Symbol *op_sym) const;

  /* Returns the production name that generated a preference. */
  std::string get_rule_name(preference *pref) const;

  /* Records all LHS ground-truth conditions for a proposal instantiation */
  void record_conditions(int64_t candidate_id, preference *prop_pref);

  /* Returns the elimination reason for a losing candidate, given the
     winning preference and the full slot. */
  std::string get_elimination_reason(preference *loser_pref,
                                     preference *winner_pref, slot *s) const;
};

#endif /* OPERATOR_EXPLANATION_H */
