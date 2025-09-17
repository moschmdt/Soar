/*************************************************************************
 *
 *  file:  backtrace.cpp
 *
 * =======================================================================
 *  Backtracing structures and routines.  See also explain.c
 * =======================================================================
 */

/* ====================================================================
                        Backtracing routines
   ==================================================================== */

#include "ebc.h"
#include "ebc_timers.h"

#include "agent.h"
#include "condition.h"
#include "explanation_memory.h"
#include "instantiation.h"
#include "mem.h"
#include "memory_manager.h"
#include "output_manager.h"
#include "preference.h"
#include "print.h"
#include "production.h"
#include "instantiation.h"
#include "soar_TraceNames.h"
#include "symbol.h"
#include "symbol_manager.h"
#include "xml.h"

#include <sstream>
#include "test.h"
#include "working_memory.h"
#include "xml.h"

#include <stdlib.h>

// External function defined in ebc_constraints.cpp
extern const char* get_test_type_name(int test_type);

using namespace soar_TraceNames;

/* ====================================================================

                            Backtracing

   Three sets of conditions are maintained during backtracing:  locals,
   grounds and negateds.  Negateds are kept separately throughout backtracing,
   and ground at the very end.  Note that this means during backtracing,
   the grounds and locals are all instantiated top-level positive conditions,
   so they all have a bt.wme_ on them.

   In order to avoid backtracing through the same instantiation twice,
   we mark each instantiation as we BT it, by setting
   inst->backtrace_number = backtrace_number (this is a global variable
   which gets incremented each time we build a chunk).

   The add_to_grounds() and add_to_locals()
   macros below are used to add conditions to these sets.  The negated
   conditions are maintained in the chunk_cond_set "negated_set."

==================================================================== */

void Explanation_Based_Chunker::add_to_grounds(condition* cond)
{
    if ((cond)->bt.wme_->tc != grounds_tc)
    {
        (cond)->bt.wme_->tc = grounds_tc;
        cond->bt.wme_->chunker_bt_last_ground_cond = cond;
        
        // Run identity analysis for first condition that matches this WME
        if (ebc_settings[SETTING_EBC_LEARNING_ON])
        {
            if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
            {
                std::ostringstream message;
                message << "\n[IDENTITY DEBUG] Processing first WME ground condition for identity analysis: " 
                       << cond->bt.wme_->id->to_string() << " ^" << cond->bt.wme_->attr->to_string() << " " << cond->bt.wme_->value->to_string();
                thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
                xml_generate_verbose(thisAgent, message.str().c_str());
            }
            
            check_for_constant_match_literalization(cond);
            
            if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
            {
                std::ostringstream message;
                message << "\n[IDENTITY DEBUG] Completed identity analysis for first WME";
                thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
                xml_generate_verbose(thisAgent, message.str().c_str());
            }
        }
    }
    if ((cond->bt.wme_->chunker_bt_last_ground_cond != cond) && ebc_settings[SETTING_EBC_LEARNING_ON])
    {
        if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
        {
            std::ostringstream message;
            message << "\n[IDENTITY DEBUG] Processing duplicate WME ground condition for identity analysis: " 
                   << cond->bt.wme_->id->to_string() << " ^" << cond->bt.wme_->attr->to_string() << " " << cond->bt.wme_->value->to_string();
            thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
            xml_generate_verbose(thisAgent, message.str().c_str());
        }
        
        check_for_singleton_unification(cond);
        check_for_constant_match_literalization(cond);
        
        if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
        {
            std::ostringstream message;
            message << "\n[IDENTITY DEBUG] Completed identity analysis for duplicate WME";
            thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
            xml_generate_verbose(thisAgent, message.str().c_str());
        }
    }
    push(thisAgent, (cond), grounds);
}

void Explanation_Based_Chunker::add_to_locals(condition* cond)
{
    if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
    {
        std::ostringstream message;
        message << "\n[DEBUG] add_to_locals called for condition with wme: " << cond->bt.wme_->id->to_string() << " ^" << cond->bt.wme_->attr->to_string() << " " << cond->bt.wme_->value->to_string();
        thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
        xml_generate_verbose(thisAgent, message.str().c_str());
    }
    
    // Check if this local condition has constant match tests
    check_for_constant_match_literalization(cond);
    
    push(thisAgent, (cond), locals);
}

/* -------------------------------------------------------------------
                     Backtrace Through Instantiation

   This routine BT's through a given instantiation.  The general method
   is as follows:

     1. If we've already BT'd this instantiation, then skip it.
     2. Mark the TC (in the instantiated conditions) of all higher goal
        ids tested in top-level positive conditions
     3. Scan through the instantiated conditions; add each one to the
        appropriate set (locals, grounds, negated_set).
------------------------------------------------------------------- */

void Explanation_Based_Chunker::backtrace_through_instantiation(preference* pPref, condition* trace_cond, uint64_t bt_depth, BTSourceType bt_type)
{

    instantiation* inst = pPref->inst;
    rhs_quadruple rhs_funcs = pPref->rhs_func_inst_identities;

    condition* c;
    cons *negateds_to_print;

    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
    {
        thisAgent->outputManager->printa_sf(thisAgent,  "... BT through instantiation of ");
        thisAgent->outputManager->printa_sf(thisAgent, "%y\n", inst->prod ? inst->prod_name : thisAgent->symbolManager->soarSymbols.architecture_inst_symbol);
        xml_begin_tag(thisAgent, kTagBacktrace);
        xml_att_val(thisAgent, kProduction_Name, inst->prod ? inst->prod_name: thisAgent->symbolManager->soarSymbols.architecture_inst_symbol);
    }

    // Debug output to show all conditions in this instantiation
    if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
    {
        std::ostringstream message;
        message << "\n[INSTANTIATION DEBUG] Backtracing through instantiation of " 
               << (inst->prod ? inst->prod_name->to_string() : "architecture");
        thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
        xml_generate_verbose(thisAgent, message.str().c_str());
        
        // Count and list all conditions
        int condition_count = 0;
        for (condition* temp_c = inst->top_of_instantiated_conditions; temp_c != NIL; temp_c = temp_c->next)
        {
            condition_count++;
        }
        
        message.str("");
        message << "\n[INSTANTIATION DEBUG] This instantiation has " << condition_count << " conditions:";
        thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
        xml_generate_verbose(thisAgent, message.str().c_str());
        
        int cond_index = 0;
        for (condition* temp_c = inst->top_of_instantiated_conditions; temp_c != NIL; temp_c = temp_c->next, cond_index++)
        {
            message.str("");
            message << "\n[INSTANTIATION DEBUG] Condition[" << cond_index << "] type=" << (int)temp_c->type 
                   << " wme: " << temp_c->bt.wme_->id->to_string() << " ^" << temp_c->bt.wme_->attr->to_string() << " " << temp_c->bt.wme_->value->to_string();
            thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
            xml_generate_verbose(thisAgent, message.str().c_str());
        }
    }

    if (trace_cond && ebc_settings[SETTING_EBC_LEARNING_ON])
    {
        unify_lhs_rhs_connection(trace_cond, pPref->identities, rhs_funcs);
    }

    if (thisAgent->explanationMemory->isCurrentlyRecording())
    {
        ++bt_depth;
        if (inst->explain_depth > bt_depth)
        {
            inst->explain_depth = bt_depth;
        }
    }
    /* --- if the instantiation has already been BT'd, don't repeat it --- */
    if (inst->backtrace_number == backtrace_number)
    {
        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            thisAgent->outputManager->printa(thisAgent, "(We already backtraced through this instantiation.)\n");
            xml_att_val(thisAgent, kBacktracedAlready, "true");
            xml_end_tag(thisAgent, kTagBacktrace);
        }
        return;
    }

    inst->backtrace_number = backtrace_number;
    thisAgent->explanationMemory->add_bt_instantiation(inst, bt_type);
    thisAgent->explanationMemory->increment_stat_instantations_backtraced();

    if (inst->tested_quiescence) m_tested_quiescence = true;
    if (inst->tested_local_negation) m_tested_local_negation = true;
    if (inst->tested_LTM) m_tested_ltm_recall = true;
    if (inst->creates_deep_copy) m_tested_deep_copy = true;

    /* We must backtrace through OSK even if we're not adding OSK because prohibits also use this mechanism */
    if (inst->OSK_prefs)
    {
        backtrace_through_OSK(inst->OSK_prefs, inst->explain_depth);
    }
    if (inst->OSK_proposal_prefs)
    {
        backtrace_through_OSK(inst->OSK_proposal_prefs, inst->explain_depth);
    }

    /* --- scan through conditions, collect grounds and locals --- */
    negateds_to_print = NIL;

    for (c = inst->top_of_instantiated_conditions; c != NIL; c = c->next)
    {
        if (c->type == POSITIVE_CONDITION)
        {
            // Show constant match test structure when found
            if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM] && c->data.tests.value_test->type == CONJUNCTIVE_TEST)
            {
                bool has_constant_match = false;
                for (cons* sub_c = c->data.tests.value_test->data.conjunct_list; sub_c; sub_c = sub_c->rest)
                {
                    test sub_test = static_cast<test>(sub_c->first);
                    if (sub_test->type == CONSTANT_MATCH_TEST && sub_test->force_literalize)
                    {
                        has_constant_match = true;
                        break;
                    }
                }
                
                if (has_constant_match)
                {
                    std::ostringstream message;
                    message << "\n[CONSTANT_MATCH DEBUG] Found CONSTANT_MATCH_TEST in condition: " 
                           << c->bt.wme_->id->to_string() << " ^" << c->bt.wme_->attr->to_string() << " " << c->bt.wme_->value->to_string();
                    message << "\n[CONSTANT_MATCH DEBUG] Conjunctive test has:";
                    int sub_index = 0;
                    for (cons* sub_c = c->data.tests.value_test->data.conjunct_list; sub_c; sub_c = sub_c->rest, sub_index++)
                    {
                        test sub_test = static_cast<test>(sub_c->first);
                        message << "\n[CONSTANT_MATCH DEBUG]   [" << sub_index << "] " << get_test_type_name(sub_test->type);
                        if (sub_test->type == CONSTANT_MATCH_TEST)
                        {
                            message << " *** FORCE_LITERALIZE=" << (sub_test->force_literalize ? "TRUE" : "false") << " ***";
                        }
                        if (sub_test->data.referent)
                        {
                            message << " referent=" << sub_test->data.referent->to_string();
                        }
                    }
                    thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
                    xml_generate_verbose(thisAgent, message.str().c_str());
                }
            }
            
            /* Check operationality */
            if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
            {
                std::ostringstream message;
                message << "\n[BACKTRACE DEBUG] POSITIVE_CONDITION - checking operationality";
                thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
                xml_generate_verbose(thisAgent, message.str().c_str());
            }
            
            /* Check operationality */
            if (c->data.tests.id_test->eq_test->data.referent->id->level <= m_goal_level)
            {
                if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
                {
                    std::ostringstream message;
                    message << "\n[BACKTRACE DEBUG] Operational condition - goal level check passed";
                    thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
                    xml_generate_verbose(thisAgent, message.str().c_str());
                }
                
                if (c->bt.wme_->tc != grounds_tc)                   /* First time we've seen something matching this wme*/
                {
                    if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
                    {
                        std::ostringstream message;
                        message << "\n[BACKTRACE DEBUG] First time seeing WME - calling add_to_grounds";
                        thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
                        xml_generate_verbose(thisAgent, message.str().c_str());
                    }
                    add_to_grounds(c);
                }
                else if (ebc_settings[SETTING_EBC_LEARNING_ON])     /* Another condition that matches the same wme */
                {
                    if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
                    {
                        std::ostringstream message;
                        message << "\n[BACKTRACE DEBUG] Duplicate WME with learning on - calling add_to_grounds";
                        thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
                        xml_generate_verbose(thisAgent, message.str().c_str());
                    }
                    add_to_grounds(c);
                }
            } else {                                                /* A local sub-state WME */
                if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
                {
                    std::ostringstream message;
                    message << "\n[BACKTRACE DEBUG] Local sub-state WME - caching constraints and adding to locals";
                    thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
                    xml_generate_verbose(thisAgent, message.str().c_str());
                }
                if (ebc_settings[SETTING_EBC_LEARNING_ON])
                    cache_constraints_in_cond(c);
                add_to_locals(c);
            }
        }
        else
        {
            if (thisAgent->trace_settings[TRACE_CHUNKS_WARNINGS_SYSPARAM])
            {
                std::ostringstream message;
                message << "\n[BACKTRACE DEBUG] NON-POSITIVE condition type " << (int)c->type << " - adding to negated set";
                thisAgent->outputManager->printa_sf(thisAgent, message.str().c_str());
                xml_generate_verbose(thisAgent, message.str().c_str());
            }
            add_to_chunk_cond_set(&negated_set, make_chunk_cond_for_negated_condition(c));
            if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM]) push(thisAgent, c, negateds_to_print);
        }
    }

    /* --- if tracing BT, print the resulting conditions, etc. --- */
    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
    {
        thisAgent->outputManager->printa(thisAgent, "  -->Grounds:\n");
        xml_begin_tag(thisAgent, kTagGrounds);
        print_consed_list_of_condition_wmes(thisAgent, grounds, 0);
        xml_end_tag(thisAgent, kTagGrounds);
        thisAgent->outputManager->printa(thisAgent,  "\n");

        thisAgent->outputManager->printa(thisAgent, "  -->Locals:\n");
        xml_begin_tag(thisAgent, kTagLocals);
        print_consed_list_of_condition_wmes(thisAgent, locals, 0);
        xml_end_tag(thisAgent, kTagLocals);
        thisAgent->outputManager->printa_sf(thisAgent,  "\n");

        thisAgent->outputManager->printa(thisAgent, "  -->Negated:\n");
        xml_begin_tag(thisAgent, kTagNegated);
        print_consed_list_of_conditions(thisAgent, negateds_to_print, 0);
        xml_end_tag(thisAgent, kTagNegated);
        thisAgent->outputManager->printa_sf(thisAgent,  "\n");
        free_list(thisAgent, negateds_to_print);

        xml_end_tag(thisAgent, kTagBacktrace);
    }

}

/* ---------------------------------------------------------------
                             Trace Locals

   This routine backtraces through locals, and keeps doing so until
   there are no more locals to BT.
--------------------------------------------------------------- */
void Explanation_Based_Chunker::backtrace_through_OSK(cons* pOSKPrefList, uint64_t lExplainDepth)
{
    cons* l_OSK_prefs;
    preference* p;

    for (l_OSK_prefs = pOSKPrefList; l_OSK_prefs != NIL; l_OSK_prefs = l_OSK_prefs->rest)
    {
        p = static_cast<preference_struct*>(l_OSK_prefs->first);
        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            thisAgent->outputManager->printa(thisAgent, "     Tracing through OSK preference: ");
            xml_begin_tag(thisAgent, kTagOSKPreference);
            print_preference(thisAgent, p);
        }

        backtrace_through_instantiation(p, NULL, lExplainDepth, BT_OSK);

        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            xml_end_tag(thisAgent, kTagOSKPreference);
        }
    }
}
void Explanation_Based_Chunker::trace_locals()
{

    /* mvp 5-17-94 */
    cons* c;
    condition* cond;
    preference* bt_pref;

    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
    {
        thisAgent->outputManager->printa(thisAgent, "\n\n*** Tracing Locals ***\n");
        xml_begin_tag(thisAgent, kTagLocals);
    }

    while (locals)
    {
        c = locals;
        locals = locals->rest;
        cond = static_cast<condition_struct*>(c->first);
        free_cons(thisAgent, c);

        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            thisAgent->outputManager->printa(thisAgent, "\nFor local ");
            xml_begin_tag(thisAgent, kTagLocal);
            print_wme(thisAgent, cond->bt.wme_);
            thisAgent->outputManager->printa(thisAgent, " ");
        }

        bt_pref = NULL;
        if (cond->bt.trace)
        {
            bt_pref = (cond->bt.trace->level != m_results_match_goal_level) ? find_clone_for_level(cond->bt.trace, m_results_match_goal_level) : cond->bt.trace;
        }
        if (bt_pref)
        {
            backtrace_through_instantiation(bt_pref, cond, cond->inst->explain_depth, BT_Normal);

            if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
            {
                xml_end_tag(thisAgent, kTagLocal);
            }
            continue;
        }

        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            thisAgent->outputManager->printa(thisAgent, "...no trace, can't BT");
            // add an empty <backtrace> tag to make parsing XML easier
            xml_begin_tag(thisAgent, kTagBacktrace);
            xml_end_tag(thisAgent, kTagBacktrace);
        }
        /* --- for augmentations of the local goal id, either handle the "^quiescence t" test or discard it --- */
        Symbol* thisID = cond->data.tests.id_test->eq_test->data.referent;
        Symbol* thisAttr = cond->data.tests.attr_test->eq_test->data.referent;
        Symbol* thisValue = cond->data.tests.value_test->eq_test->data.referent;
        if (thisID->id->isa_goal)
        {
            if (cond->inst->tested_quiescence ||
               ((thisAttr == thisAgent->symbolManager->soarSymbols.quiescence_symbol) &&
                (thisValue == thisAgent->symbolManager->soarSymbols.t_symbol) &&
                (! cond->test_for_acceptable_preference)))
            {
                m_tested_quiescence = true;
                cond->inst->tested_quiescence = true;
            }
            if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
            {
                xml_end_tag(thisAgent, kTagLocal);
            }
            continue;
        }

        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            xml_end_tag(thisAgent, kTagLocal);
        }

    } /* end of while locals loop */

    if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
    {
        xml_end_tag(thisAgent, kTagLocals);
    }
}

void Explanation_Based_Chunker::perform_dependency_analysis()
{
    preference* pref;
    m_goal_level = m_inst->match_goal_level - 1;

    outputManager->set_print_test_format(true, true);

    increment_counter(backtrace_number);
    increment_counter(grounds_tc);
    grounds = NIL;
    locals = NIL;

    thisAgent->explanationMemory->set_backtrace_number(backtrace_number);

    /* Backtrace through the instantiation that produced each result --- */
    for (pref = m_results; pref != NIL; pref = pref->next_result)
    {
        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            thisAgent->outputManager->printa(thisAgent, "\nFor result preference ");
            xml_begin_tag(thisAgent, kTagBacktraceResult);
            print_preference(thisAgent, pref);
            thisAgent->outputManager->printa(thisAgent, " ");
        }
        backtrace_through_instantiation(pref, NULL, 0, (pref->inst == m_inst) ? BT_BaseInstantiation : BT_ExtraResults);

        if (thisAgent->trace_settings[TRACE_BACKTRACING_SYSPARAM])
        {
            xml_end_tag(thisAgent, kTagBacktraceResult);
        }
    }

    trace_locals();

    outputManager->clear_print_test_format();

}
