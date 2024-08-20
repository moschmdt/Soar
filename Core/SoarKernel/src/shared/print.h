/**
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 */

/** ======================================================================

                                print.h

====================================================================== */

#ifndef PRINT_H
#define PRINT_H
#include <stdio.h>  // Needed for FILE token below

#include <string>

#include "kernel.h"

typedef struct wme_filter_struct {
  Symbol* id;
  Symbol* attr;
  Symbol* value;
  bool adds;
  bool removes;
} wme_filter;

void print_phase(agent* thisAgent, const char* s, bool end_phase);
void print_spaces(agent* thisAgent, int n);

void filtered_print_wme_remove(agent* thisAgent, wme* w);
void filtered_print_wme_add(agent* thisAgent, wme* w);

/** ------------------------------------------------------------------------
                String to Escaped String Conversion
           {Symbol, Test, RHS Value} to String Conversion

   These routines produce strings.  Each takes an optional parameter "dest"
   which, if non-nil, points to the destination buffer for the result string.
   If dest is nil, these routines use a global buffer, and return a pointer
   to it.  (Otherwise "dest" itself is returned.)  Note that a single global
   buffer is shared by all three routines, so callers should assume the
   buffer will be destroyed by the next call to these routines with dest=NIL.

   String_to_escaped_string() takes a string and a first/last char,
   and produces an "escaped string" representation of the string; i.e.,
   a string that uses '\' escapes to include special characters.
   For example, input 'ab"c' with first/last character '"' yields
   '"ab\"c"'.  This is used for printing quoted strings and for printing
   symbols using |vbar| notation.

   Rhs_value_to_string() takes an rhs_value and produces a string
   representation.  The rhs_value MUST NOT be a reteloc.
----------------------------------------------------------------------- */

const std::string string_to_escaped_string(const char* s,
                                           char first_and_last_char);

/** -----------------------------------------------------------------------
             Print Condition List, Action List, Production

   Print_condition_list() prints a list of conditions.  The "indent"
   parameter tells how many spaces to indent each line other than the
   first--the first line is not indented (the caller must handle this).
   The last line is printed without a trailing linefeed.  The "internal"
   parameter, if true, indicates that the condition list should be printed
   in internal format--one condition per line, without grouping all the
   conditions for the same id into one line.

   Print_action_list() is similar except it prints actions instead of
   conditions.  The actions MUST NOT contain any reteloc's.

   Print_production() prints a given production, optionally using internal
   format.

----------------------------------------------------------------------- */

void print_condition_list(agent* thisAgent, condition* conds, int indent,
                          bool internal);
void print_action_list(agent* thisAgent, action* actions, int indent,
                       bool internal);
void print_production(agent* thisAgent, production* p, bool internal);
void print_consed_list_of_condition_wmes(agent* thisAgent, cons* c, int indent);
void print_consed_list_of_conditions(agent* thisAgent, cons* c, int indent);

/** -----------------------------------------------------------------------
                       Other Printing Utilities

   Print_condition() prints a single condition.  Print_action() prints
   a single action (which MUST NOT contain any reteloc's).
   Note that these routines work by calling print_condition_list() and
   print_action_list(), respectively, so they print a linefeed if the
   output would go past COLUMNS_PER_LINE.

   Preference_type_indicator() returns a character corresponding to
   a given preference type (byte)--for example, given BEST_PREFERENCE_TYPE,
   it returns '>'.

   Print_preference() prints a given preference.  Print_wme() prints a
   wme (including the timetag).  Print_instantiation_with_wmes() prints
   an instantiation's production name and the wmes it matched, using a
   given wme_trace_type (e.g., TIMETAG_WME_TRACE). Action is printing,
   firing or retracting -- added March 05 KJC.
----------------------------------------------------------------------- */

void print_condition(agent* thisAgent, condition* cond);
void print_action(agent* thisAgent, action* a);
char preference_to_char(byte type);
void print_preference(agent* thisAgent, preference* pref, bool add_lf = true);
void print_wme(agent* thisAgent, wme* w);
void print_wme_without_timetag(agent* thisAgent, wme* w);
void print_instantiation_with_wmes(agent* thisAgent, instantiation* inst,
                                   wme_trace_type wtt, int action);

void print_list_of_conditions(agent* thisAgent, condition* cond);

void print_sysparam_trace(agent* thisAgent, int64_t sysParamIndex,
                          const char* format, ...);

#endif
