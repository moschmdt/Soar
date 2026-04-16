/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

#include "operator_explanation.h"

#include "agent.h"
#include "condition.h"
#include "instantiation.h"
#include "output_manager.h"
#include "preference.h"
#include "slot.h"
#include "symbol.h"
#include "working_memory.h"

#include <cassert>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/* -----------------------------------------------------------------------
   Schema DDL
   ----------------------------------------------------------------------- */

static const char *SCHEMA_DECISIONS =
    "CREATE TABLE IF NOT EXISTS decisions ("
    "  id             INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  cycle          INTEGER NOT NULL,"
    "  goal_level     INTEGER NOT NULL,"
    "  state_id       TEXT    NOT NULL,"
    "  winner_name    TEXT," /* NULL if impasse */
    "  winner_op_id   TEXT,"
    "  winner_rule    TEXT,"
    "  impasse_type   TEXT    NOT NULL"
    ");";

static const char *SCHEMA_CANDIDATES =
    "CREATE TABLE IF NOT EXISTS candidates ("
    "  id             INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  decision_id    INTEGER NOT NULL,"
    "  op_id          TEXT    NOT NULL,"
    "  op_name        TEXT,"
    "  proposal_rule  TEXT,"
    "  is_winner      INTEGER NOT NULL DEFAULT 0,"
    "  elimination    TEXT" /* NULL for winner */
    ");";

static const char *SCHEMA_CONDITIONS =
    "CREATE TABLE IF NOT EXISTS proposal_conditions ("
    "  id             INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  candidate_id   INTEGER NOT NULL,"
    "  is_negated     INTEGER NOT NULL DEFAULT 0,"
    "  wme_id         TEXT    NOT NULL,"
    "  wme_attr       TEXT    NOT NULL,"
    "  wme_value      TEXT    NOT NULL"
    ");";

static const char *SCHEMA_PREFERENCES =
    "CREATE TABLE IF NOT EXISTS decision_preferences ("
    "  id             INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  decision_id    INTEGER NOT NULL,"
    "  pref_type      TEXT    NOT NULL,"
    "  value_id       TEXT    NOT NULL,"
    "  value_name     TEXT,"
    "  referent_id    TEXT," /* NULL for unary prefs */
    "  referent_name  TEXT,"
    "  producing_rule TEXT"
    ");";

/* -----------------------------------------------------------------------
   OpExpl_Statement_Container
   ----------------------------------------------------------------------- */

OpExpl_Statement_Container::OpExpl_Statement_Container(
    soar_module::sqlite_database *db)
    : soar_module::sqlite_statement_container(db) {
  add_structure(SCHEMA_DECISIONS);
  add_structure(SCHEMA_CANDIDATES);
  add_structure(SCHEMA_CONDITIONS);
  add_structure(SCHEMA_PREFERENCES);

  decision_insert = new soar_module::sqlite_statement(
      db, "INSERT INTO decisions (cycle, goal_level, state_id, winner_name, "
          "winner_op_id, winner_rule, impasse_type) "
          "VALUES (?, ?, ?, ?, ?, ?, ?)");

  candidate_insert = new soar_module::sqlite_statement(
      db, "INSERT INTO candidates (decision_id, op_id, op_name, proposal_rule, "
          "is_winner, elimination) "
          "VALUES (?, ?, ?, ?, ?, ?)");

  condition_insert = new soar_module::sqlite_statement(
      db, "INSERT INTO proposal_conditions (candidate_id, is_negated, wme_id, "
          "wme_attr, wme_value) "
          "VALUES (?, ?, ?, ?, ?)");

  preference_insert = new soar_module::sqlite_statement(
      db, "INSERT INTO decision_preferences (decision_id, pref_type, value_id, "
          "value_name, referent_id, referent_name, producing_rule) "
          "VALUES (?, ?, ?, ?, ?, ?, ?)");

  decision_get_by_name = new soar_module::sqlite_statement(
      db, "SELECT id, cycle, goal_level, state_id, winner_name, winner_op_id, "
          "winner_rule, impasse_type "
          "FROM decisions WHERE winner_name = ? OR "
          "id IN (SELECT decision_id FROM candidates WHERE op_name = ?) "
          "ORDER BY id");

  candidate_get_by_decision = new soar_module::sqlite_statement(
      db, "SELECT id, op_id, op_name, proposal_rule, is_winner, elimination "
          "FROM candidates WHERE decision_id = ? ORDER BY is_winner DESC, id");

  condition_get_by_candidate = new soar_module::sqlite_statement(
      db, "SELECT is_negated, wme_id, wme_attr, wme_value "
          "FROM proposal_conditions WHERE candidate_id = ? ORDER BY id");

  preference_get_by_decision = new soar_module::sqlite_statement(
      db, "SELECT pref_type, value_id, value_name, referent_id, referent_name, "
          "producing_rule "
          "FROM decision_preferences WHERE decision_id = ? ORDER BY id");

  add(decision_insert);
  add(candidate_insert);
  add(condition_insert);
  add(preference_insert);
  add(decision_get_by_name);
  add(candidate_get_by_decision);
  add(condition_get_by_candidate);
  add(preference_get_by_decision);
}

/* -----------------------------------------------------------------------
   OperatorExplanationManager
   ----------------------------------------------------------------------- */

OperatorExplanationManager::OperatorExplanationManager(agent *new_agent)
    : my_agent(new_agent), db(nullptr), stmts(nullptr),
      track_all_operators(false) {
  db = new soar_module::sqlite_database();
}

OperatorExplanationManager::~OperatorExplanationManager() {
  if (stmts) {
    delete stmts;
    stmts = nullptr;
  }
  if (db) {
    if (db->get_status() == soar_module::connected) {
      db->disconnect();
    }
    delete db;
    db = nullptr;
  }
}

void OperatorExplanationManager::init_db() {
  if (db->get_status() != soar_module::disconnected) {
    return;
  }

  db->connect(":memory:");
  assert(db->get_status() == soar_module::connected);

  stmts = new OpExpl_Statement_Container(db);
  stmts->structure();
  stmts->prepare();
}

/* -----------------------------------------------------------------------
   Tracking
   ----------------------------------------------------------------------- */

void OperatorExplanationManager::track_operator(const std::string &name) {
  if (db->get_status() == soar_module::disconnected) {
    init_db();
  }

  if (name == "all" || name == "*") {
    track_all_operators = true;
    tracked_names.clear();
    excluded_names.clear();
    my_agent->outputManager->printa_sf(
        my_agent,
        "Now tracking all operator explanations. "
        "Use 'explain untrack-operator <name>' to exclude operators.\n");
    return;
  }

  if (track_all_operators) {
    excluded_names.erase(name);
  }

  tracked_names.insert(name);
  my_agent->outputManager->printa_sf(
      my_agent, "Now tracking operator explanations for: %s\n", name.c_str());
}

void OperatorExplanationManager::untrack_operator(const std::string &name) {
  if (name == "all" || name == "*") {
    track_all_operators = false;
    excluded_names.clear();
    my_agent->outputManager->printa_sf(
        my_agent, "Track-all mode disabled for operator explanations.\n");
    return;
  }

  if (track_all_operators) {
    excluded_names.insert(name);
    tracked_names.erase(name);
    my_agent->outputManager->printa_sf(
        my_agent, "Excluded operator from tracking: %s\n", name.c_str());
    return;
  }

  auto it = tracked_names.find(name);
  if (it != tracked_names.end()) {
    tracked_names.erase(it);
    my_agent->outputManager->printa_sf(
        my_agent, "No longer tracking operator explanations for: %s\n",
        name.c_str());
  } else {
    my_agent->outputManager->printa_sf(
        my_agent, "Operator '%s' was not being tracked.\n", name.c_str());
  }
}

bool OperatorExplanationManager::is_tracked(const std::string &name) const {
  if (track_all_operators) {
    return excluded_names.find(name) == excluded_names.end();
  }
  return tracked_names.find(name) != tracked_names.end();
}

void OperatorExplanationManager::list_tracked(agent *thisAgent) const {
  if (track_all_operators) {
    if (excluded_names.empty()) {
      thisAgent->outputManager->printa_sf(thisAgent,
                                          "Currently tracking explanations for "
                                          "all operators (no exclusions).\n");
      return;
    }

    thisAgent->outputManager->printa_sf(
        thisAgent, "Currently tracking all operators except:\n");
    for (const auto &n : excluded_names) {
      thisAgent->outputManager->printa_sf(thisAgent, "  %s\n", n.c_str());
    }
    return;
  }

  if (tracked_names.empty()) {
    thisAgent->outputManager->printa_sf(
        thisAgent, "No operators are currently being tracked.\n");
    return;
  }
  thisAgent->outputManager->printa_sf(thisAgent,
                                      "Currently tracking explanations for:\n");
  for (const auto &n : tracked_names) {
    thisAgent->outputManager->printa_sf(thisAgent, "  %s\n", n.c_str());
  }
}

/* -----------------------------------------------------------------------
   Helpers
   ----------------------------------------------------------------------- */

const char *OperatorExplanationManager::preference_type_name(byte pref_type) {
  switch (pref_type) {
  case 0:
    return "acceptable";
  case 1:
    return "require";
  case 2:
    return "reject";
  case 3:
    return "prohibit";
  case 4:
    return "reconsider";
  case 5:
    return "unary-indifferent";
  case 6:
    return "unary-parallel";
  case 7:
    return "best";
  case 8:
    return "worst";
  case 9:
    return "binary-indifferent";
  case 10:
    return "binary-parallel";
  case 11:
    return "better";
  case 12:
    return "worse";
  case 13:
    return "numeric-indifferent";
  default:
    return "unknown";
  }
}

const char *OperatorExplanationManager::impasse_type_name(byte impasse_type) {
  /* Values from constants.h:
     NONE_IMPASSE_TYPE=0, CONSTRAINT_FAILURE_IMPASSE_TYPE=1,
     CONFLICT_IMPASSE_TYPE=2, TIE_IMPASSE_TYPE=3, NO_CHANGE_IMPASSE_TYPE=4 */
  switch (impasse_type) {
  case 0:
    return "none";
  case 1:
    return "constraint-failure";
  case 2:
    return "conflict";
  case 3:
    return "tie";
  case 4:
    return "no-change";
  default:
    return "unknown";
  }
}

std::string
OperatorExplanationManager::get_operator_name(Symbol *op_sym) const {
  if (!op_sym) {
    return "";
  }
  Symbol *name_sym = find_name_of_object(my_agent, op_sym);
  if (name_sym) {
    return std::string(name_sym->to_string());
  }
  return std::string(op_sym->to_string());
}

std::string OperatorExplanationManager::get_rule_name(preference *pref) const {
  if (pref && pref->inst && pref->inst->prod_name) {
    return std::string(pref->inst->prod_name->to_string());
  }
  return "";
}

void OperatorExplanationManager::record_conditions(int64_t candidate_id,
                                                   preference *prop_pref) {
  if (!prop_pref || !prop_pref->inst) {
    return;
  }

  condition *cond = prop_pref->inst->top_of_instantiated_conditions;
  while (cond) {
    /* bt.wme_ is only valid for POSITIVE_CONDITION; negative conditions
       succeeded because no WME matched, so there is nothing to record. */
    if (cond->type == POSITIVE_CONDITION) {
      wme *w = cond->bt.wme_;
      if (w) {
        std::string wid = w->id ? std::string(w->id->to_string()) : "";
        std::string wattr = w->attr ? std::string(w->attr->to_string()) : "";
        std::string wval = w->value ? std::string(w->value->to_string()) : "";

        stmts->condition_insert->bind_int(1, candidate_id);
        stmts->condition_insert->bind_int(2, 0);
        stmts->condition_insert->bind_text_transient(3, wid.c_str());
        stmts->condition_insert->bind_text_transient(4, wattr.c_str());
        stmts->condition_insert->bind_text_transient(5, wval.c_str());
        stmts->condition_insert->execute(soar_module::op_reinit);
      }
    }
    /* CONJUNCTIVE_NEGATION_CONDITION: recurse into sub-conditions.
       Only positive sub-conditions carry a valid bt.wme_. */
    else if (cond->type == CONJUNCTIVE_NEGATION_CONDITION) {
      condition *sub = cond->data.ncc.top;
      while (sub) {
        if (sub->type == POSITIVE_CONDITION && sub->bt.wme_) {
          wme *w = sub->bt.wme_;
          std::string wid = w->id ? std::string(w->id->to_string()) : "";
          std::string wattr = w->attr ? std::string(w->attr->to_string()) : "";
          std::string wval = w->value ? std::string(w->value->to_string()) : "";

          stmts->condition_insert->bind_int(1, candidate_id);
          stmts->condition_insert->bind_int(2, 1); /* negated context */
          stmts->condition_insert->bind_text_transient(3, wid.c_str());
          stmts->condition_insert->bind_text_transient(4, wattr.c_str());
          stmts->condition_insert->bind_text_transient(5, wval.c_str());
          stmts->condition_insert->execute(soar_module::op_reinit);
        }
        sub = sub->next;
      }
    }
    cond = cond->next;
  }
}

std::string OperatorExplanationManager::get_elimination_reason(
    preference *loser_pref, preference *winner_pref, slot *s) const {
  if (!loser_pref) {
    return "";
  }

  Symbol *loser_val = loser_pref->value;
  Symbol *winner_val = winner_pref ? winner_pref->value : nullptr;

  /* Check reject */
  for (preference *p = s->preferences[2]; p; p = p->next) {
    if (p->value == loser_val) {
      return std::string("reject [") + get_rule_name(p) + "]";
    }
  }
  /* Check prohibit */
  for (preference *p = s->preferences[3]; p; p = p->next) {
    if (p->value == loser_val) {
      return std::string("prohibit [") + get_rule_name(p) + "]";
    }
  }
  /* Check better (binary): if there is a better(winner > loser) */
  for (preference *p = s->preferences[11]; p; p = p->next) {
    if (p->value == winner_val && p->referent == loser_val) {
      return std::string("dominated: better(") + get_operator_name(winner_val) +
             " > " + get_operator_name(loser_val) + ") [" + get_rule_name(p) +
             "]";
    }
  }
  /* Check worse (binary): if there is a worse(loser < winner) */
  for (preference *p = s->preferences[12]; p; p = p->next) {
    if (p->value == loser_val && p->referent == winner_val) {
      return std::string("dominated: worse(") + get_operator_name(loser_val) +
             " < " + get_operator_name(winner_val) + ") [" + get_rule_name(p) +
             "]";
    }
  }
  /* Check best: loser was not given best, but winner was */
  if (winner_val) {
    bool winner_has_best = false;
    bool loser_has_best = false;
    for (preference *p = s->preferences[7]; p; p = p->next) {
      if (p->value == winner_val)
        winner_has_best = true;
      if (p->value == loser_val)
        loser_has_best = true;
    }
    if (winner_has_best && !loser_has_best) {
      return std::string("not-best: winner has best preference");
    }
  }

  if (winner_val) {
    return std::string("selection (indifferent or tie-breaking)");
  }
  return "impasse";
}

/* -----------------------------------------------------------------------
   record_decision  — main hook called from decide_context_slot()
   ----------------------------------------------------------------------- */

void OperatorExplanationManager::record_decision(agent *thisAgent, slot *s,
                                                 Symbol *goal,
                                                 preference *winner,
                                                 byte impasse_type) {
  if (!stmts || (!track_all_operators && tracked_names.empty())) {
    return;
  }

  /* Determine if any candidate (or winner) has a tracked name */
  bool relevant = false;
  if (winner && is_tracked(get_operator_name(winner->value))) {
    relevant = true;
  }
  if (!relevant) {
    for (preference *p = s->preferences[0]; p; p = p->next) {
      if (is_tracked(get_operator_name(p->value))) {
        relevant = true;
        break;
      }
    }
  }
  if (!relevant) {
    return;
  }

  /* ----- Insert decisions row ----- */
  std::string state_str = goal ? std::string(goal->to_string()) : "";
  std::string winner_name = winner ? get_operator_name(winner->value) : "";
  std::string winner_opid =
      winner ? std::string(winner->value->to_string()) : "";
  std::string winner_rule = get_rule_name(winner);
  const char *imp_str = impasse_type_name(impasse_type);

  int64_t goal_level = goal ? static_cast<int64_t>(goal->id->level) : -1;

  stmts->decision_insert->bind_int(
      1, static_cast<int64_t>(thisAgent->d_cycle_count));
  stmts->decision_insert->bind_int(2, goal_level);
  stmts->decision_insert->bind_text_transient(3, state_str.c_str());
  if (winner) {
    stmts->decision_insert->bind_text_transient(4, winner_name.c_str());
    stmts->decision_insert->bind_text_transient(5, winner_opid.c_str());
    stmts->decision_insert->bind_text_transient(6, winner_rule.c_str());
  } else {
    stmts->decision_insert->bind_null(4);
    stmts->decision_insert->bind_null(5);
    stmts->decision_insert->bind_null(6);
  }
  stmts->decision_insert->bind_text_transient(7, imp_str);
  stmts->decision_insert->execute(soar_module::op_reinit);

  int64_t decision_id = db->last_insert_rowid();

  /* ----- Insert candidate rows ----- */
  for (preference *p = s->preferences[0]; p; p = p->next) {
    bool is_win = (winner && p->value == winner->value);
    std::string op_id_str = std::string(p->value->to_string());
    std::string op_nm = get_operator_name(p->value);
    std::string rule_nm = get_rule_name(p);
    std::string elim_str = is_win ? "" : get_elimination_reason(p, winner, s);

    stmts->candidate_insert->bind_int(1, decision_id);
    stmts->candidate_insert->bind_text_transient(2, op_id_str.c_str());
    stmts->candidate_insert->bind_text_transient(
        3, op_nm.empty() ? nullptr : op_nm.c_str());
    stmts->candidate_insert->bind_text_transient(
        4, rule_nm.empty() ? nullptr : rule_nm.c_str());
    stmts->candidate_insert->bind_int(5, is_win ? 1 : 0);
    stmts->candidate_insert->bind_text_transient(
        6, elim_str.empty() ? nullptr : elim_str.c_str());
    stmts->candidate_insert->execute(soar_module::op_reinit);

    int64_t candidate_id = db->last_insert_rowid();
    record_conditions(candidate_id, p);
  }

  /* ----- Insert OSK preference rows ----- */
  for (cons *c = s->OSK_prefs; c; c = c->rest) {
    preference *osk = static_cast<preference *>(c->first);
    if (!osk || !osk->value)
      continue;

    const char *ptype = preference_type_name(static_cast<byte>(osk->type));
    std::string val_id = std::string(osk->value->to_string());
    std::string val_nm = get_operator_name(osk->value);
    std::string ref_id =
        osk->referent ? std::string(osk->referent->to_string()) : "";
    std::string ref_nm = osk->referent ? get_operator_name(osk->referent) : "";
    std::string rule_nm = get_rule_name(osk);

    stmts->preference_insert->bind_int(1, decision_id);
    stmts->preference_insert->bind_text_transient(2, ptype);
    stmts->preference_insert->bind_text_transient(3, val_id.c_str());
    stmts->preference_insert->bind_text_transient(
        4, val_nm.empty() ? nullptr : val_nm.c_str());
    stmts->preference_insert->bind_text_transient(
        5, ref_id.empty() ? nullptr : ref_id.c_str());
    stmts->preference_insert->bind_text_transient(
        6, ref_nm.empty() ? nullptr : ref_nm.c_str());
    stmts->preference_insert->bind_text_transient(
        7, rule_nm.empty() ? nullptr : rule_nm.c_str());
    stmts->preference_insert->execute(soar_module::op_reinit);
  }
}

/* -----------------------------------------------------------------------
   print_decisions  — output for 'explain operator <name>'
   ----------------------------------------------------------------------- */

void OperatorExplanationManager::print_decisions(
    agent *thisAgent, const std::string &name) const {
  if (!stmts) {
    thisAgent->outputManager->printa_sf(
        thisAgent,
        "No explanation data recorded yet.  Use 'explain track-operator %s' "
        "first.\n",
        name.c_str());
    return;
  }

  stmts->decision_get_by_name->bind_text_transient(1, name.c_str());
  stmts->decision_get_by_name->bind_text_transient(2, name.c_str());

  bool any = false;
  while (stmts->decision_get_by_name->execute() == soar_module::row) {
    any = true;
    int64_t dec_id = stmts->decision_get_by_name->column_int(0);
    int64_t cycle = stmts->decision_get_by_name->column_int(1);
    int64_t goal_level = stmts->decision_get_by_name->column_int(2);
    const char *state_id = stmts->decision_get_by_name->column_text(3);
    const char *win_name = stmts->decision_get_by_name->column_text(4);
    const char *win_op_id = stmts->decision_get_by_name->column_text(5);
    const char *win_rule = stmts->decision_get_by_name->column_text(6);
    const char *imp_type = stmts->decision_get_by_name->column_text(7);

    thisAgent->outputManager->printa_sf(
        thisAgent, "\n=== Decision at cycle %d  [level %d, state %s] ===\n",
        cycle, goal_level, state_id ? state_id : "?");

    if (win_name) {
      thisAgent->outputManager->printa_sf(
          thisAgent, "  WINNER: %s (%s)  [%s]\n", win_name,
          win_op_id ? win_op_id : "?", win_rule ? win_rule : "unknown rule");
    } else {
      thisAgent->outputManager->printa_sf(thisAgent, "  IMPASSE: %s\n",
                                          imp_type ? imp_type : "?");
    }

    /* Candidates */
    stmts->candidate_get_by_decision->bind_int(1, dec_id);
    while (stmts->candidate_get_by_decision->execute() == soar_module::row) {
      int64_t cand_id = stmts->candidate_get_by_decision->column_int(0);
      const char *op_id = stmts->candidate_get_by_decision->column_text(1);
      const char *op_nm = stmts->candidate_get_by_decision->column_text(2);
      const char *rule_nm = stmts->candidate_get_by_decision->column_text(3);
      int64_t is_win = stmts->candidate_get_by_decision->column_int(4);
      const char *elim = stmts->candidate_get_by_decision->column_text(5);

      if (is_win) {
        thisAgent->outputManager->printa_sf(
            thisAgent, "\n  [WINNER]  %s (%s)  proposed by: %s\n",
            op_nm ? op_nm : op_id, op_id, rule_nm ? rule_nm : "?");
      } else {
        thisAgent->outputManager->printa_sf(
            thisAgent, "\n  [LOSER]   %s (%s)  proposed by: %s\n",
            op_nm ? op_nm : op_id, op_id, rule_nm ? rule_nm : "?");
        if (elim) {
          thisAgent->outputManager->printa_sf(
              thisAgent, "            eliminated: %s\n", elim);
        }
      }

      /* LHS conditions */
      stmts->condition_get_by_candidate->bind_int(1, cand_id);
      bool first_cond = true;
      while (stmts->condition_get_by_candidate->execute() == soar_module::row) {
        int64_t is_neg = stmts->condition_get_by_candidate->column_int(0);
        const char *w_id = stmts->condition_get_by_candidate->column_text(1);
        const char *w_attr = stmts->condition_get_by_candidate->column_text(2);
        const char *w_val = stmts->condition_get_by_candidate->column_text(3);

        if (first_cond) {
          thisAgent->outputManager->printa_sf(thisAgent,
                                              "            LHS matched:\n");
          first_cond = false;
        }
        if (is_neg) {
          thisAgent->outputManager->printa_sf(
              thisAgent, "              -(%s ^%s %s)\n", w_id ? w_id : "?",
              w_attr ? w_attr : "?", w_val ? w_val : "?");
        } else {
          thisAgent->outputManager->printa_sf(
              thisAgent, "              (%s ^%s %s)\n", w_id ? w_id : "?",
              w_attr ? w_attr : "?", w_val ? w_val : "?");
        }
      }
      stmts->condition_get_by_candidate->reinitialize();
    }
    stmts->candidate_get_by_decision->reinitialize();

    /* OSK preferences */
    stmts->preference_get_by_decision->bind_int(1, dec_id);
    bool first_pref = true;
    while (stmts->preference_get_by_decision->execute() == soar_module::row) {
      const char *ptype = stmts->preference_get_by_decision->column_text(0);
      const char *val_id = stmts->preference_get_by_decision->column_text(1);
      const char *val_nm = stmts->preference_get_by_decision->column_text(2);
      const char *ref_id = stmts->preference_get_by_decision->column_text(3);
      const char *ref_nm = stmts->preference_get_by_decision->column_text(4);
      const char *rule_nm = stmts->preference_get_by_decision->column_text(5);

      if (first_pref) {
        thisAgent->outputManager->printa_sf(thisAgent,
                                            "\n  Selection preferences:\n");
        first_pref = false;
      }

      if (ref_id) {
        thisAgent->outputManager->printa_sf(
            thisAgent, "    %s(%s, %s)  [%s]\n", ptype ? ptype : "?",
            val_nm ? val_nm : val_id, ref_nm ? ref_nm : ref_id,
            rule_nm ? rule_nm : "?");
      } else {
        thisAgent->outputManager->printa_sf(
            thisAgent, "    %s(%s)  [%s]\n", ptype ? ptype : "?",
            val_nm ? val_nm : val_id, rule_nm ? rule_nm : "?");
      }
    }
    stmts->preference_get_by_decision->reinitialize();
  }
  stmts->decision_get_by_name->reinitialize();

  if (!any) {
    thisAgent->outputManager->printa_sf(
        thisAgent,
        "No decisions recorded for operator '%s'.\n"
        "  Make sure 'explain track-operator %s' was called before running.\n",
        name.c_str(), name.c_str());
  }
}

/* -----------------------------------------------------------------------
   print_decisions_json  — JSON output for 'explain operator <name> --json'
   ----------------------------------------------------------------------- */

void OperatorExplanationManager::print_decisions_json(
    agent *thisAgent, const std::string &name) const {
  if (!stmts) {
    thisAgent->outputManager->printa_sf(thisAgent, "{\"decisions\":[]}");
    return;
  }

  json root;
  root["decisions"] = json::array();

  stmts->decision_get_by_name->bind_text_transient(1, name.c_str());
  stmts->decision_get_by_name->bind_text_transient(2, name.c_str());

  while (stmts->decision_get_by_name->execute() == soar_module::row) {
    int64_t dec_id = stmts->decision_get_by_name->column_int(0);
    int64_t cycle = stmts->decision_get_by_name->column_int(1);
    int64_t goal_level = stmts->decision_get_by_name->column_int(2);
    const char *state_id = stmts->decision_get_by_name->column_text(3);
    const char *win_name = stmts->decision_get_by_name->column_text(4);
    const char *win_op_id = stmts->decision_get_by_name->column_text(5);
    const char *win_rule = stmts->decision_get_by_name->column_text(6);
    const char *imp_type = stmts->decision_get_by_name->column_text(7);

    json decision;
    decision["cycle"] = cycle;
    decision["level"] = goal_level;
    decision["state_id"] = state_id ? state_id : "";

    if (win_name) {
      decision["winner"] = {{"name", win_name ? win_name : ""},
                            {"op_id", win_op_id ? win_op_id : ""},
                            {"rule", win_rule ? win_rule : ""}};
    } else {
      decision["impasse"] = imp_type ? imp_type : "";
    }

    /* Candidates */
    json candidates = json::array();
    stmts->candidate_get_by_decision->bind_int(1, dec_id);
    while (stmts->candidate_get_by_decision->execute() == soar_module::row) {
      int64_t cand_id = stmts->candidate_get_by_decision->column_int(0);
      const char *op_id = stmts->candidate_get_by_decision->column_text(1);
      const char *op_nm = stmts->candidate_get_by_decision->column_text(2);
      const char *rule_nm = stmts->candidate_get_by_decision->column_text(3);
      int64_t is_win = stmts->candidate_get_by_decision->column_int(4);
      const char *elim = stmts->candidate_get_by_decision->column_text(5);

      json cand;
      cand["name"] = op_nm ? op_nm : (op_id ? op_id : "");
      cand["op_id"] = op_id ? op_id : "";
      cand["proposal_rule"] = rule_nm ? rule_nm : "";
      cand["is_winner"] = (is_win != 0);
      if (!is_win && elim) {
        cand["elimination"] = elim;
      }

      /* LHS conditions */
      json conditions = json::array();
      stmts->condition_get_by_candidate->bind_int(1, cand_id);
      while (stmts->condition_get_by_candidate->execute() == soar_module::row) {
        int64_t is_neg = stmts->condition_get_by_candidate->column_int(0);
        const char *w_id = stmts->condition_get_by_candidate->column_text(1);
        const char *w_attr = stmts->condition_get_by_candidate->column_text(2);
        const char *w_val = stmts->condition_get_by_candidate->column_text(3);

        conditions.push_back({{"negated", is_neg != 0},
                              {"id", w_id ? w_id : ""},
                              {"attr", w_attr ? w_attr : ""},
                              {"value", w_val ? w_val : ""}});
      }
      stmts->condition_get_by_candidate->reinitialize();
      cand["conditions"] = std::move(conditions);

      candidates.push_back(std::move(cand));
    }
    stmts->candidate_get_by_decision->reinitialize();
    decision["candidates"] = std::move(candidates);

    /* OSK preferences */
    json preferences = json::array();
    stmts->preference_get_by_decision->bind_int(1, dec_id);
    while (stmts->preference_get_by_decision->execute() == soar_module::row) {
      const char *ptype = stmts->preference_get_by_decision->column_text(0);
      const char *val_id = stmts->preference_get_by_decision->column_text(1);
      const char *val_nm = stmts->preference_get_by_decision->column_text(2);
      const char *ref_id = stmts->preference_get_by_decision->column_text(3);
      const char *ref_nm = stmts->preference_get_by_decision->column_text(4);
      const char *rule_nm = stmts->preference_get_by_decision->column_text(5);

      json pref;
      pref["type"] = ptype ? ptype : "";
      pref["value_id"] = val_id ? val_id : "";
      pref["value_name"] = val_nm ? val_nm : "";
      if (ref_id && ref_id[0] != '\0') {
        pref["referent_id"] = ref_id;
        pref["referent_name"] = ref_nm ? ref_nm : "";
      }
      pref["rule"] = rule_nm ? rule_nm : "";

      preferences.push_back(std::move(pref));
    }
    stmts->preference_get_by_decision->reinitialize();
    decision["preferences"] = std::move(preferences);

    root["decisions"].push_back(std::move(decision));
  }
  stmts->decision_get_by_name->reinitialize();

  if (root["decisions"].empty()) {
    root["error"] =
        std::string("No decisions recorded for operator '") + name + "'";
  }

  std::string output = root.dump();
  thisAgent->outputManager->printa_sf(thisAgent, output.c_str());
}
