/**
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 */

/**
 *
 *  file:  wma.h
 *
 * =======================================================================
 */

#ifndef WMA_H
#define WMA_H

#include <queue>
#include <string>

#include "kernel.h"
#include "soar_module.h"

//////////////////////////////////////////////////////////
// WMA Constants
//////////////////////////////////////////////////////////

/**
 * This is the size of the reference history.
 */
#define WMA_DECAY_HISTORY 10

/**
 * How many references are expected per decision
 * (this affects creation of the power/approx cache)
 */
#define WMA_REFERENCES_PER_DECISION 50

/**
 * If an external caller asks for the activation level/value
 * of a WME that is not activated, then this is the value that
 * is returned.
 */
#define WMA_ACTIVATION_NONE 1.0
#define WMA_TIME_SUM_NONE 2.71828182845905

/**
 * If no history, this is a low number to report as activation
 */
#define WMA_ACTIVATION_LOW -1000000000

/**
 * If below decay thresh, but not forgotten, forget_cycle =
 */
#define WMA_FORGOTTEN_CYCLE 0

//////////////////////////////////////////////////////////
// WMA Parameters
//////////////////////////////////////////////////////////

class wma_activation_param;
class wma_decay_param;

class wma_param_container : public soar_module::param_container {
 public:
  wma_activation_param* activation;
  wma_decay_param* decay_rate;
  wma_decay_param* decay_thresh;
  soar_module::boolean_param* petrov_approx;

  enum forgetting_choices { disabled, naive, bsearch, approx };
  soar_module::constant_param<forgetting_choices>* forgetting;

  enum forget_wme_choices { all, lti };
  soar_module::constant_param<forget_wme_choices>* forget_wme;

  soar_module::boolean_param* fake_forgetting;

  // performance
  soar_module::constant_param<soar_module::timer::timer_level>* timers;
  soar_module::integer_param* max_pow_cache;

  wma_param_container(agent* new_agent);
};

class wma_activation_param : public soar_module::boolean_param {
 public:
  wma_activation_param(const char* new_name, boolean new_value,
                       soar_module::predicate<boolean>* new_prot_pred,
                       agent* new_agent);
  virtual void set_value(boolean new_value);

 private:
  agent* thisAgent;
};

class wma_decay_param : public soar_module::decimal_param {
 public:
  wma_decay_param(const char* new_name, double new_value,
                  soar_module::predicate<double>* new_val_pred,
                  soar_module::predicate<double>* new_prot_pred);
  virtual void set_value(double new_value);
};

template <typename T>
class wma_activation_predicate : public soar_module::agent_predicate<T> {
 public:
  wma_activation_predicate(agent* new_agent);
  bool operator()(T val);
};

//////////////////////////////////////////////////////////
// WMA Statistics
//////////////////////////////////////////////////////////

class wma_stat_container : public soar_module::stat_container {
 public:
  soar_module::integer_stat* forgotten_wmes;

  wma_stat_container(agent* new_agent);
};

//////////////////////////////////////////////////////////
// WMA Timers
//////////////////////////////////////////////////////////

class wma_timer_container : public soar_module::timer_container {
 public:
  soar_module::timer* history;
  soar_module::timer* forgetting;

  wma_timer_container(agent* thisAgent);
};

class wma_timer_level_predicate
    : public soar_module::agent_predicate<soar_module::timer::timer_level> {
 public:
  wma_timer_level_predicate(agent* new_agent);
  bool operator()(soar_module::timer::timer_level val);
};

class wma_timer : public soar_module::timer {
 public:
  wma_timer(const char* new_name, agent* new_agent, timer_level new_level);
};

//////////////////////////////////////////////////////////
// WMA Types
//////////////////////////////////////////////////////////

typedef uint64_t wma_reference;
typedef uint64_t wma_d_cycle;

typedef struct wma_cycle_reference_struct {
  wma_reference num_references;
  wma_d_cycle d_cycle;
} wma_cycle_reference;

typedef struct wma_history_struct {
  wma_cycle_reference access_history[WMA_DECAY_HISTORY];
  unsigned int next_p;
  unsigned int history_ct;

  wma_reference history_references;
  wma_reference total_references;
  wma_d_cycle first_reference;
} wma_history;

// attached to o-supported WMEs to keep track of its activation.
typedef struct wma_decay_element_struct {
  // the wme that this element goes with
  wme* this_wme;

  // when a WME is removed from working memory, the data
  // structure is not necessarily deallocated right away
  // because its reference count has not fallen to zero.
  // This flag indicates that the WME is in this "limbo" state.
  bool just_removed;

  // notes the awkward period between first activation
  // and dealing with history changes
  bool just_created;

  // how many times this wme has been referenced so far
  // this cycle
  wma_reference num_references;

  // when and how often this wme has been referenced in recent
  // history.
  wma_history touches;

  // if forgetting is enabled, this tells us when we think
  // we need to forget this wme
  wma_d_cycle forget_cycle;

} wma_decay_element;

enum wma_go_action { wma_histories, wma_forgetting };

//////////////////////////////////////////////////////////
// Parameter Functions
//////////////////////////////////////////////////////////

// shortcut for determining if WMA is enabled
extern bool wma_enabled(agent* thisAgent);

//////////////////////////////////////////////////////////
// Add/Remove Decay Element/Set
//////////////////////////////////////////////////////////

// generic call to activate a wme
extern void wma_activate_wme(agent* thisAgent, wme* w,
                             wma_reference num_references = 1,
                             wme_set* o_set = NULL, bool o_only = false);

// Removes a decay element from an existing WME so that
// it is no longer activated.
extern void wma_remove_decay_element(agent* thisAgent, wme* w);

// Removes an o-support set from an existing preference
extern void wma_remove_pref_o_set(agent* thisAgent, preference* pref);

//////////////////////////////////////////////////////////
// Updating Activation
//////////////////////////////////////////////////////////

/**
 * Given a preference, this routine increments the
 * reference count of all its WMEs (as necessary).
 */
extern void wma_activate_wmes_in_pref(agent* thisAgent, preference* pref);

/**
 * Increments the reference count of all
 * WMEs that have been referenced this
 * cycle.
 */
extern void wma_activate_wmes_tested_in_prods(agent* thisAgent);

/**
 * This routine performs WME activation
 * and forgetting at the end of each cycle.
 */
extern void wma_go(agent* thisAgent, wma_go_action go_action);

//////////////////////////////////////////////////////////
// Retrieving Activation
//////////////////////////////////////////////////////////

/**
 * Retrieve wme activation exact/approximate
 */
extern double wma_get_wme_activation(agent* thisAgent, wme* w, bool log_result);

/**
 * Debugging: get list of wme references
 */
extern void wma_get_wme_history(agent* thisAgent, wme* w, std::string& buffer);

#endif
