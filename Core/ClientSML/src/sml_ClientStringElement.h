/////////////////////////////////////////////////////////////////
// StringElement class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has a string value.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_STRING_ELEMENT_H
#define SML_STRING_ELEMENT_H

#include <string>

#include "Export.h"
#include "sml_ClientWMElement.h"

namespace sml {

class WorkingMemory;
class Identifier;

//
// Special note about output-link WMEs: The agent is
// free to remove WMEs from the output-link at any time.
// If you retain a WME for multiple decision cycles,
// you must check output link changes (using
// GetNumOutputLinkChanges, GetOutputLinkChange, and
// IsOutputLinkAdd) to check if the WMEs you have were
// removed during the last decision cycle. Dereferencing
// a removed WME causes a segmentation fault.
class EXPORT StringElement : public WMElement {
  // Allow working memory to create these objects directly (user must use agent
  // class to do this)
  friend class WorkingMemory;

 protected:
  // The value for this wme is a string
  std::string m_Value;

 public:
  // Returns the type of the value stored here (e.g. "string" or "int" etc.)
  virtual char const* GetValueType() const;

  // Returns a string form of the value stored here.
  virtual char const* GetValueAsString() const { return m_Value.c_str(); }

  // Returns the current value
  char const* GetValue() const { return m_Value.c_str(); }

  // Returns a string form of the value, uses buf to create the string
  virtual char const* GetValueAsString(std::string& buf) const {
    buf.assign(m_Value);
    return buf.c_str();
  }

  virtual StringElement* ConvertToStringElement() { return this; }

  /**
   * @brief Update the value of an existing WME.
   *        If "auto commit" is turned off in ClientKernel,
   *        the value is not actually sent to the kernel
   *        until "Commit" is called.
   *
   *        If "BlinkIfNoChange" is false then updating a wme to the
   *        same value it already had will be ignored.
   *        This value is true by default, so updating a wme to the same
   *        value causes the wme to be deleted and a new identical one to be
   *added which will trigger rules to rematch. You can turn this flag on and off
   *around a set of calls to update if you wish.
   */
  void Update(char const* pValue);

 protected:
  StringElement(Agent* pAgent, Identifier* pParent, char const* pID,
                char const* pAttributeName, char const* pValue,
                long long timeTag);
  StringElement(Agent* pAgent, IdentifierSymbol* pParentSymbol, char const* pID,
                char const* pAttributeName, char const* pValue,
                long long timeTag);
  virtual ~StringElement(void);

  void SetValue(char const* pValue) { m_Value = pValue; }

#ifdef SML_DIRECT
  virtual void DirectAdd(Direct_AgentSML_Handle pAgentSML, long long timeTag);
#endif
};

}  // namespace sml

#endif  // SML_STRING_ELEMENT_H
