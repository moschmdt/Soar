/**
 * @file working_memory.h
 * @brief Working Memory management and utility routines for WMEs
 *
 * Reset_wme_timetags() resets the wme timetag generator back to 1.
 * Make_wme() creates and returns a new wme. The caller should add the
 * wme onto the appropriate dll (e.g., my_slot->wmes) and should call
 * add_wme_to_wm() on it.
 *
 * Add_wme_to_wm() and remove_wme_from_wm() make changes to WM. The caller
 * is responsible for manipulating the appropriate dll. WM changes are
 * buffered and applied at the end of the phase by do_buffered_wm_changes().
 *
 * Remove_wme_list_from_wm() is a utility that scans a list of wmes
 * (linked by their "next" fields) and calls remove_wme_from_wm() on each.
 *
 * Wme_add_ref() and wme_remove_ref() increment and decrement the
 * reference count on a wme. deallocate_wme() deallocates a wme; it should
 * only be invoked via wme_remove_ref().
 *
 * Find_name_of_object() returns the value of the ^name attribute on a
 * given object symbol, or nullptr/NIL if none exists.
 */

/* ------------------------------------------------------------------------
 * Working Memory Elements (WMEs)
 *
 * The WME structure contains fields for id, attr, value, timetag, reference
 * counting and a variety of pointers used by the Rete and other subsystems.
 * The struct fields are documented in detail in the typedef below.
 * --------------------------------------------------------------------- */

#ifndef WMEM_H
#define WMEM_H

#include "kernel.h"

#include <spdlog/spdlog.h>

#include "semantic_memory.h"
#include "stl_typedefs.h"
#include "symbol.h"

/**
 * Reset the global WME timetag generator back to 1.
 * @param thisAgent the agent instance
 */
void reset_wme_timetags(agent *thisAgent);

/**
 * Create a new WME structure.
 * @param thisAgent the agent instance
 * @param id the id symbol
 * @param attr the attribute symbol
 * @param value the value symbol
 * @param acceptable whether this WME represents an acceptable preference
 * @return pointer to the newly allocated wme
 */
wme *make_wme(agent *thisAgent, Symbol *id, Symbol *attr, Symbol *value,
              bool acceptable);

/**
 * Add a WME to working memory (buffers change for later application).
 * @param thisAgent the agent instance
 * @param w pointer to the wme to add
 */
void add_wme_to_wm(agent *thisAgent, wme *w);

/**
 * Remove a WME from working memory (buffers change for later application).
 * @param thisAgent the agent instance
 * @param w pointer to the wme to remove
 */
void remove_wme_from_wm(agent *thisAgent, wme *w);

/**
 * Remove a linked list of WMEs from working memory.
 * @param thisAgent the agent instance
 * @param w head of the wme list to remove
 * @param updateWmeMap if true, update internal WME mapping structures
 */
void remove_wme_list_from_wm(agent *thisAgent, wme *w,
                             bool updateWmeMap = false);

/**
 * Apply all buffered working memory and ownership changes to the rete.
 * @param thisAgent the agent instance
 */
void do_buffered_wm_changes(agent *thisAgent);

/**
 * Deallocate a WME and free associated resources.
 * @param thisAgent the agent instance
 * @param w the wme to deallocate
 */
void deallocate_wme(agent *thisAgent, wme *w);

/**
 * Find the value of the ^name attribute on a given object symbol.
 * @param thisAgent the agent instance
 * @param id the object symbol to query
 * @return the name symbol, or nullptr/NIL if not present
 */
Symbol *find_name_of_object(agent *thisAgent, Symbol *id);

/**
 * WM_Manager
 *
 * Manager object that contains working-memory-related parameters, stats
 * and data structures (WMA / decay / forget queues, etc.) used per-agent.
 */
class WM_Manager {
public:
  WM_Manager(agent *myAgent);
  ~WM_Manager() {};

  void clean_up_for_agent_deletion();

  wma_param_container *wma_params;
  wma_stat_container *wma_stats;
  wma_timer_container *wma_timers;

  wme_set *wma_touched_elements;
  wma_forget_p_queue *wma_forget_pq;
  wma_decay_cycle_set *wma_touched_sets;

  unsigned int wma_power_size;
  double *wma_power_array;
  wma_d_cycle *wma_approx_array;
  double wma_thresh_exp;
  bool wma_initialized;
  tc_number wma_tc_counter;
  wma_d_cycle wma_d_cycle_count;

  deep_copy_wme_list glbDeepCopyWMEs;

private:
  agent *thisAgent;
};

/**
 * @brief Working Memory Element structure.
 *
 * @warning The next three fields (`id`, `attr`, `value`) MUST be consecutive.
 * The Rete code depends on this layout.
 *
 * Fields include timetag, reference_count, links used by the rete
 * (`rete_next`, `rete_prev`), right memory and token lists, list links
 * for placement in slots, a supporting `preference` pointer (if any),
 * I/O `output_link` pointer, bookkeeping fields used by EBC/chunker,
 * and GDS/epmem/WMA fields for goal-dependency and memory activation.
 */
typedef struct wme_struct {
  /* WARNING:  The next three fields (id,attr,value) MUST be consecutive.  The
   * rete code relies on this! */
  Symbol *id;
  Symbol *attr;
  Symbol *value;
  bool acceptable;
  uint64_t timetag;
  uint64_t reference_count;

  struct wme_struct *rete_next, *rete_prev;
  struct right_mem_struct *right_mems;
  struct token_struct *tokens;
  struct wme_struct *next, *prev;

  struct preference_struct *preference;   /* pref. supporting it, or NIL */
  struct output_link_struct *output_link; /* for top-state output commands */

  tc_number tc;
  struct condition_struct *chunker_bt_last_ground_cond;
  bool is_singleton;
  bool singleton_status_checked;
  Identity *local_singleton_id_identity_set;
  Identity *local_singleton_value_identity_set;

  struct gds_struct *gds;
  struct wme_struct *gds_next, *gds_prev; /* wmes in gds */

  epmem_node_id epmem_id;
  uint64_t epmem_valid;

  wma_decay_element *wma_decay_el;
  tc_number wma_tc_value;
} wme;

/**
 * Increment the reference count for a WME.
 * @param w the wme to increment
 * @param always_add (unused) placeholder to match signature expectations
 */
inline void wme_add_ref(wme *w, bool always_add = false) {
  (w)->reference_count++;
}

/**
 * Decrement the reference count for a WME and deallocate when zero.
 * @param thisAgent the agent instance
 * @param w the wme to decrement
 */
inline void wme_remove_ref(agent *thisAgent, wme *w) {
  if ((w)->reference_count != 0)
    (w)->reference_count--;
  if ((w)->reference_count == 0)
    deallocate_wme(thisAgent, w);
}

/**
 * Return a specific element (id/attr/value) from a WME.
 * @param w the wme to query
 * @param f the WME_Field to return
 * @return the requested Symbol pointer or NULL if not available
 */
inline Symbol *get_wme_element(wme *w, WME_Field f) {
  if (!w)
    return NULL;
  if (f == ID_ELEMENT)
    return w->id;
  if (f == ATTR_ELEMENT)
    return w->attr;
  if (f == VALUE_ELEMENT)
    return w->value;
  return NULL;
}

#endif
