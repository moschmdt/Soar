/////////////////////////////////////////////////////////////////
// AgentOutputFlusher class file.
//
// Author: Jonathan Voigt
// Date  : February 2005
//
/////////////////////////////////////////////////////////////////
#ifndef AGENT_OUTPUT_FLUSHER_H
#define AGENT_OUTPUT_FLUSHER_H

#include "sml_Events.h"
#include "sml_KernelCallback.h"

namespace sml {

class PrintListener;

class AgentOutputFlusher : public KernelCallback {
 protected:
  int m_EventID;

  // Only one listener will be filled in.
  PrintListener* m_pPrintListener;

 public:
  AgentOutputFlusher(PrintListener* pPrintListener, AgentSML* pAgent,
                     smlPrintEventId eventID);
  virtual ~AgentOutputFlusher();

  virtual void OnKernelEvent(int eventID, AgentSML* pAgentSML, void* pCallData);
};

}  // namespace sml

#endif
